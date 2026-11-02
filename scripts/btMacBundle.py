#-----------------------------------------------------------------------------------------------------------------------
# scripts/btMacBundle.py is part of Brewtarget, and is copyright the following authors 2026:
#   • Matt Young <mfsy@yahoo.com>
#
# Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# Mac app bundle fix-up, verification and disk image creation.
#
# This does extra work on bundle dependencies because neither dylibbundler nor macdeployqt reliably produces a
# self-contained bundle for us:
#
#   - macdeployqt only deploys dependencies that are referenced by absolute path.  If something has already rewritten a
#     reference to "@executable_path/../Frameworks/libfoo.dylib" (eg our manual QtNetwork/QtDBus handling in
#     buildTool.py), macdeployqt assumes libfoo is already deployed, even if it isn't.  This is how 5.1.1 ended up with
#     QtNetwork referring to a libbrotlidec.1.dylib that was never copied into the bundle.
#
#   - dylibbundler rewrites _every_ LC_RPATH entry of the file it fixes to the (slash-terminated) install path, via
#     `install_name_tool -rpath <old> @executable_path/../Frameworks/`.  If the executable has more than one rpath
#     (eg the one Meson adds from install_rpath plus one from a dependency's link flags), we end up with several
#     identical LC_RPATH entries.  Older versions of dyld ignored that; newer ones (macOS 15.4+/26) abort with
#     "duplicate LC_RPATH".  This is what broke 5.2.0.
#
# So, after macdeployqt has done its work, we walk every Mach-O file in the bundle to a fixed point, copying in and
# re-pointing anything still missing or external, then tidy up rpaths, then _verify_ the result (failing the build if
# anything is unresolved), then re-sign (ad hoc) because install_name_tool invalidates code signatures.  Finally we
# build the disk image ourselves (rather than with `macdeployqt -dmg`) so that it contains the fixed-up bundle.
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import glob
import os
import pathlib
import re
import shutil
import stat
import subprocess
import sys

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btExecute
import btLogger

#
# Libraries under these prefixes are part of the OS and must never be bundled
#
SYSTEM_PREFIXES = ('/usr/lib/', '/System/Library/', '/System/iOSSupport/')

#
# Where, relative to the executable, bundled libraries and frameworks live
#
FRAMEWORKS_REF_PREFIX = '@executable_path/../Frameworks/'

#
# First four bytes of Mach-O files (thin 32/64-bit in either byte order, plus universal "fat" binaries)
#
MACH_O_MAGICS = {
   b'\xfe\xed\xfa\xce', b'\xce\xfa\xed\xfe',
   b'\xfe\xed\xfa\xcf', b'\xcf\xfa\xed\xfe',
   b'\xca\xfe\xba\xbe', b'\xbe\xba\xfe\xca',
}

def _isMachO(path: pathlib.Path) -> bool:
   if path.is_symlink() or not path.is_file():
      return False
   try:
      with open(path, 'rb') as f:
         magic = f.read(4)
   except OSError:
      return False
   if magic not in MACH_O_MAGICS:
      return False
   # 0xcafebabe is also the magic number for Java .class files.  We don't ship any, but be safe.
   return path.suffix != '.class'

def _isSystem(ref: str) -> bool:
   return ref.startswith(SYSTEM_PREFIXES)

def _run(args: list[str]) -> str:
   return btExecute.abortOnRunFail(subprocess.run(args, capture_output=True)).stdout.decode('UTF-8')

def _makeWritable(path: pathlib.Path):
   # Homebrew installs its libraries read-only (0444), so copies of them need fixing before install_name_tool can
   # modify them.
   mode = os.stat(path).st_mode
   if not (mode & stat.S_IWUSR):
      os.chmod(path, mode | stat.S_IWUSR)

def _installNameTool(args: list[str], target: pathlib.Path):
   _makeWritable(target)
   btLogger.log.debug('Running install_name_tool ' + ' '.join(args) + ' ' + target.as_posix())
   btExecute.abortOnRunFail(subprocess.run(['install_name_tool'] + args + [target.as_posix()], capture_output=False))

def parseOtoolDashL(otoolOutput: str) -> list[str]:
   """
   Given the output of `otool -L <file>`, return the list of install names it references.  The first line of the output
   is the file name, so we skip it.  NB: for a dylib, the first _entry_ is the library's own ID, which callers need to
   filter out (see _ownId).
   """
   refs = []
   for line in otoolOutput.splitlines()[1:]:
      match = re.match(r'^\s+(.+?) \(compatibility version ', line)
      if match:
         refs.append(match[1])
   return refs

def parseOtoolRpaths(otoolOutput: str) -> list[str]:
   """
   Given the output of `otool -l <file>`, return the LC_RPATH entries in order (including any duplicates).
   """
   rpaths = []
   inRpath = False
   for line in otoolOutput.splitlines():
      stripped = line.strip()
      if stripped.startswith('cmd '):
         inRpath = (stripped == 'cmd LC_RPATH')
      elif inRpath and stripped.startswith('path '):
         match = re.match(r'^path (.+?) \(offset \d+\)$', stripped)
         if match:
            rpaths.append(match[1])
         inRpath = False
   return rpaths

def _ownId(path: pathlib.Path) -> str:
   lines = _run(['otool', '-D', path.as_posix()]).splitlines()
   return lines[1].strip() if len(lines) > 1 else ''

def _dependencies(path: pathlib.Path) -> list[str]:
   ownId = _ownId(path)
   return [ref for ref in parseOtoolDashL(_run(['otool', '-L', path.as_posix()])) if ref != ownId]

def _rpaths(path: pathlib.Path) -> list[str]:
   return parseOtoolRpaths(_run(['otool', '-l', path.as_posix()]))

def bundleRelativeTail(ref: str) -> str:
   """
   For a reference to a library or framework, return the path it should have relative to Contents/Frameworks.  Eg:
      /opt/homebrew/opt/brotli/lib/libbrotlidec.1.dylib                    -> libbrotlidec.1.dylib
      @rpath/QtNetwork.framework/Versions/A/QtNetwork                      -> QtNetwork.framework/Versions/A/QtNetwork
      /opt/homebrew/opt/qtbase/lib/QtDBus.framework/Versions/A/QtDBus      -> QtDBus.framework/Versions/A/QtDBus
   """
   match = re.search(r'(?:^|/)([^/]+\.framework/.+)$', ref)
   if match:
      return match[1]
   return ref.rsplit('/', 1)[-1]

class _Bundle:
   def __init__(self, appDir: pathlib.Path, executableName: str, extraSearchDirs: list[str]):
      self.appDir = appDir.resolve()
      self.contentsDir = self.appDir.joinpath('Contents')
      self.exeDir = self.contentsDir.joinpath('MacOS')
      self.executable = self.exeDir.joinpath(executableName)
      self.frmDir = self.contentsDir.joinpath('Frameworks')
      self.searchDirs = self._librarySearchDirs(extraSearchDirs)
      self.problems = []

   def _librarySearchDirs(self, extraSearchDirs: list[str]) -> list[str]:
      dirs = [d for d in extraSearchDirs if d]
      brewPrefixes = []
      brew = shutil.which('brew')
      if brew:
         brewPrefixes.append(subprocess.run([brew, '--prefix'], capture_output=True).stdout.decode('UTF-8').strip())
      brewPrefixes += ['/opt/homebrew', '/usr/local']
      for prefix in brewPrefixes:
         if prefix:
            dirs.append(prefix + '/lib')
            # Keg-only formulae (and some Qt modules) are not linked into <prefix>/lib
            dirs += sorted(glob.glob(prefix + '/opt/*/lib'))
      dirs += ['/opt/local/lib'] + sorted(glob.glob('/opt/local/libexec/*/lib'))
      # Remove duplicates and non-existent directories, preserving order
      result = []
      for d in dirs:
         if d not in result and os.path.isdir(d):
            result.append(d)
      btLogger.log.debug('Library search dirs for bundle fix-up: ' + ', '.join(result))
      return result

   def isInside(self, path: pathlib.Path) -> bool:
      try:
         path.resolve().relative_to(self.appDir)
         return True
      except ValueError:
         return False

   def expand(self, ref: str, loaderDir: pathlib.Path) -> pathlib.Path | None:
      if ref.startswith('@executable_path/'):
         return self.exeDir.joinpath(ref.removeprefix('@executable_path/'))
      if ref.startswith('@loader_path/'):
         return loaderDir.joinpath(ref.removeprefix('@loader_path/'))
      if ref.startswith('/'):
         return pathlib.Path(ref)
      return None

   def resolveInBundle(self, ref: str, file: pathlib.Path) -> pathlib.Path | None:
      """
      Return the file inside the bundle that dyld would load for this reference, or None if it would not find one
      inside the bundle.  For @rpath we approximate dyld by using the rpaths of the referencing file and of the main
      executable.
      """
      candidates = []
      if ref.startswith('@rpath/'):
         rpaths = _rpaths(file) + (_rpaths(self.executable) if file != self.executable else [])
         for rpath in rpaths:
            base = self.expand(rpath, file.parent)
            if base is not None:
               candidates.append(base.joinpath(ref.removeprefix('@rpath/')))
      else:
         expanded = self.expand(ref, file.parent)
         if expanded is not None:
            candidates.append(expanded)
      for candidate in candidates:
         if candidate.exists() and self.isInside(candidate):
            return candidate
      return None

   def findSource(self, ref: str) -> pathlib.Path | None:
      """
      Find, outside the bundle, the library or framework we need to copy in to satisfy a reference.
      Returns the path of the file (for a dylib) or of the .framework directory (for a framework).
      """
      tail = bundleRelativeTail(ref)
      frameworkMatch = re.match(r'^([^/]+\.framework)/', tail)
      if ref.startswith('/') and os.path.exists(ref):
         if frameworkMatch:
            return pathlib.Path(ref[:ref.index(frameworkMatch[1]) + len(frameworkMatch[1])])
         return pathlib.Path(ref)
      for searchDir in self.searchDirs + [self.exeDir.as_posix()]:
         if frameworkMatch:
            candidate = pathlib.Path(searchDir).joinpath(frameworkMatch[1])
            if candidate.is_dir():
               return candidate
         else:
            candidate = pathlib.Path(searchDir).joinpath(tail)
            if candidate.exists() and candidate.parent.resolve() != self.frmDir.resolve():
               return candidate
      return None

   def copyIn(self, source: pathlib.Path, tail: str) -> list[pathlib.Path]:
      """
      Copy a library or framework into Contents/Frameworks.  Returns the Mach-O files that were added (so they can in
      turn be fixed up).
      """
      if source.name.endswith('.framework'):
         target = self.frmDir.joinpath(source.name)
         btLogger.log.info('Bundling framework ' + source.as_posix() + ' → ' + target.as_posix())
         # Symlinks inside frameworks are relative and internal, so copy them as symlinks.  (Following them causes
         # codesign to complain that the "bundle format is ambiguous".)
         shutil.copytree(source, target, symlinks=True)
         added = [p for p in target.rglob('*') if _isMachO(p)]
      else:
         target = self.frmDir.joinpath(tail)
         btLogger.log.info('Bundling library ' + source.as_posix() + ' → ' + target.as_posix())
         # Follow symlinks: we want the real file, under the name it is referenced by
         shutil.copy2(source.resolve(), target)
         added = [target]
      for path in added:
         _makeWritable(path)
      return added

   def fixDependencies(self):
      """
      Iterate to a fixed point: every non-system dependency of every Mach-O file in the bundle must resolve to a file
      inside the bundle.
      """
      pending = [p for p in self.contentsDir.rglob('*') if _isMachO(p)]
      done = set()
      while pending:
         file = pending.pop()
         key = file.resolve()
         if key in done:
            continue
         done.add(key)

         if file != self.executable and file.parent.resolve() == self.frmDir.resolve() and file.suffix == '.dylib':
            # Give bundled dylibs a sensible ID.  (Not strictly needed for loading, but avoids leaking build-machine
            # paths and confusing future tools.)
            ownId = _ownId(file)
            wantedId = FRAMEWORKS_REF_PREFIX + file.name
            if ownId and ownId != wantedId:
               _installNameTool(['-id', wantedId], file)

         for ref in _dependencies(file):
            if _isSystem(ref):
               continue
            if not ref.startswith('/') and self.resolveInBundle(ref, file) is not None:
               continue  # Already relative and resolvable inside the bundle

            tail = bundleRelativeTail(ref)
            target = self.frmDir.joinpath(tail)
            if not target.exists():
               source = self.findSource(ref)
               if source is None:
                  self.problems.append(
                     file.relative_to(self.appDir).as_posix() + ' needs ' + ref + ', which could not be found'
                  )
                  continue
               pending += self.copyIn(source, tail)

            newRef = FRAMEWORKS_REF_PREFIX + tail
            if ref != newRef:
               _installNameTool(['-change', ref, newRef], file)

   def fixRpaths(self):
      """
      Remove LC_RPATH entries that are duplicates (which newer dyld rejects outright) or that point outside the bundle
      (which leak build-machine paths and can mask missing libraries on machines that happen to have Homebrew).
      Paths differing only by a trailing slash are treated as duplicates.
      """
      for file in [p for p in self.contentsDir.rglob('*') if _isMachO(p)]:
         rpaths = _rpaths(file)
         if not rpaths:
            continue
         keep = []
         for rpath in rpaths:
            normalised = rpath.rstrip('/')
            if rpath.startswith('/') and not rpath.startswith(self.appDir.as_posix()):
               continue
            if normalised not in keep:
               keep.append(normalised)
         if [r.rstrip('/') for r in rpaths] == keep and len(set(rpaths)) == len(rpaths):
            continue
         btLogger.log.info(
            'Rewriting rpaths of ' + file.relative_to(self.appDir).as_posix() + ': ' + str(rpaths) + ' → ' + str(keep)
         )
         # install_name_tool's handling of -delete_rpath with duplicate entries has varied between Xcode versions, so
         # we delete repeatedly until each entry is gone, then add back the ones we want, once each.
         for rpath in set(rpaths):
            for _ in range(rpaths.count(rpath)):
               _makeWritable(file)
               result = subprocess.run(['install_name_tool', '-delete_rpath', rpath, file.as_posix()],
                                       capture_output=True)
               if result.returncode != 0:
                  break
         for rpath in keep:
            _installNameTool(['-add_rpath', rpath], file)

   def verify(self):
      """
      Check, without running anything, that dyld should be able to load everything.  This is what lets us catch
      problems on CI even though the build machine has all the libraries installed outside the bundle.
      """
      for file in [p for p in self.contentsDir.rglob('*') if _isMachO(p)]:
         relName = file.relative_to(self.appDir).as_posix()
         rpaths = _rpaths(file)
         if len(rpaths) != len(set(rpaths)):
            self.problems.append(relName + ' has duplicate LC_RPATH entries: ' + str(rpaths))
         for rpath in rpaths:
            if rpath.startswith('/') and not rpath.startswith(self.appDir.as_posix()):
               self.problems.append(relName + ' has LC_RPATH outside the bundle: ' + rpath)
         for ref in _dependencies(file):
            if _isSystem(ref):
               continue
            if self.resolveInBundle(ref, file) is None:
               self.problems.append(relName + ' references ' + ref + ', which does not resolve inside the bundle')

def fixUpAppBundle(appDir: pathlib.Path, executableName: str, extraSearchDirs: list[str]):
   """
   Fix up, verify and ad-hoc sign the app bundle.  Exits with an error if the bundle is not self-contained.
   """
   bundle = _Bundle(appDir, executableName, extraSearchDirs)
   btLogger.log.info('Fixing up dependencies in ' + bundle.appDir.as_posix())
   bundle.fixDependencies()
   bundle.fixRpaths()
   bundle.verify()
   if bundle.problems:
      for problem in bundle.problems:
         btLogger.log.error(problem)
      btLogger.log.critical('Mac app bundle is not self-contained (' + str(len(bundle.problems)) + ' problem(s) above)')
      sys.exit(1)
   btLogger.log.info('Mac app bundle dependencies verified OK')

   #
   # install_name_tool invalidates any existing code signature, and on Apple Silicon unsigned (or invalidly signed) code
   # is killed on launch.  So we do an ad hoc signature.  This is not a substitute for proper signing and notarisation
   # (users will still have to get past Gatekeeper), but it means the bundle is at least internally consistent.
   #
   btLogger.log.info('Ad hoc signing ' + bundle.appDir.as_posix())
   signResult = subprocess.run(['codesign', '--force', '--deep', '--sign', '-', bundle.appDir.as_posix()],
                               capture_output=True)
   if signResult.returncode != 0:
      btLogger.log.warning('Ad hoc signing failed: ' + signResult.stderr.decode('UTF-8'))
   else:
      verifyResult = subprocess.run(['codesign', '--verify', '--deep', '--strict', '--verbose=2', bundle.appDir.as_posix()],
                                    capture_output=True)
      btLogger.log.info('codesign --verify: ' + verifyResult.stderr.decode('UTF-8'))
   return

def createDmg(appDir: pathlib.Path, dmgPath: pathlib.Path, volumeName: str):
   """
   Build a compressed disk image containing just the app bundle (plus a link to /Applications for drag-and-drop install).
   """
   stagingDir = dmgPath.parent.joinpath('dmgStaging')
   if stagingDir.exists():
      shutil.rmtree(stagingDir)
   os.makedirs(stagingDir)
   # ditto preserves symlinks, extended attributes and code signatures, which shutil.copytree does not fully do
   btExecute.abortOnRunFail(
      subprocess.run(['ditto', appDir.as_posix(), stagingDir.joinpath(appDir.name).as_posix()], capture_output=False)
   )
   os.symlink('/Applications', stagingDir.joinpath('Applications'))
   if dmgPath.exists():
      dmgPath.unlink()
   btExecute.abortOnRunFail(
      subprocess.run(['hdiutil', 'create',
                      '-volname', volumeName,
                      '-srcfolder', stagingDir.as_posix(),
                      '-format', 'UDZO',
                      '-ov',
                      dmgPath.as_posix()],
                     capture_output=False)
   )
   shutil.rmtree(stagingDir)
   return

def smokeTest(executable: pathlib.Path):
   """
   Run the executable with --version.  This constructs the QApplication (so loads the platform plugin) and then exits,
   so it will catch dyld failures ("Library not loaded", "duplicate LC_RPATH", etc) and signature problems.
   """
   btLogger.log.info('Smoke testing ' + executable.as_posix())
   try:
      result = subprocess.run([executable.as_posix(), '--version'], capture_output=True, timeout=120)
   except subprocess.TimeoutExpired:
      btLogger.log.warning('Smoke test timed out (but did not crash on load)')
      return
   output = result.stdout.decode('UTF-8') + result.stderr.decode('UTF-8')
   btLogger.log.info('Smoke test exit code ' + str(result.returncode) + ', output: ' + output)
   if result.returncode != 0:
      btLogger.log.critical('Smoke test of packaged app failed')
      sys.exit(1)
   return