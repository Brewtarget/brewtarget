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
import hashlib
import os
import pathlib
import plistlib
import re
import shutil
import stat
import struct
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
   # is killed.  So we do an ad hoc signature.  This is not a substitute for proper signing and notarisation (users will
   # still have to get past Gatekeeper), but it means the bundle is at least internally consistent.
   #
   signAppBundle(bundle.appDir)
   verifyAllSignatures(bundle.appDir)
   return

#
# Mach-O code signature constants.  See Apple's cs_blobs.h (in the XNU sources) for the definitions.
#
_FAT_MAGICS              = (0xcafebabe, 0xcafebabf)        # Big-endian; 32-bit and 64-bit offsets respectively
_THIN_MAGICS             = (0xfeedface, 0xfeedfacf)        # Little-endian; 32-bit and 64-bit
_LC_CODE_SIGNATURE       = 0x1d
_CSMAGIC_EMBEDDED_SIGNATURE = 0xfade0cc0
_CSMAGIC_CODEDIRECTORY      = 0xfade0c02
_CS_HASH_FUNCTIONS = {1: hashlib.sha1, 2: hashlib.sha256, 3: hashlib.sha256, 4: hashlib.sha384}

def _machOSlices(data: bytes):
   """
   Yields (arch, sliceBytes) for each architecture in a Mach-O file (one for a thin file, several for a universal one).
   """
   bigEndianMagic = struct.unpack('>I', data[:4])[0]
   if bigEndianMagic in _FAT_MAGICS:
      numSlices = struct.unpack('>I', data[4:8])[0]
      for ii in range(numSlices):
         if bigEndianMagic == _FAT_MAGICS[0]:
            cpuType, _, offset, size, _ = struct.unpack('>iiIII', data[8 + 20 * ii : 28 + 20 * ii])
         else:
            cpuType, _, offset, size, _, _ = struct.unpack('>iiQQII', data[8 + 32 * ii : 40 + 32 * ii])
         yield 'cpu ' + hex(cpuType & 0xffffffff), data[offset : offset + size]
   elif struct.unpack('<I', data[:4])[0] in _THIN_MAGICS:
      yield 'cpu ' + hex(struct.unpack('<i', data[4:8])[0] & 0xffffffff), data

def checkPageHashes(path: pathlib.Path) -> list[str]:
   """
   Check that every page of every architecture in the Mach-O file at path matches the hashes in its embedded code
   signature.  Returns a list of problems (empty if all is well).

   This is the check the kernel does, page by page, as code is loaded on Apple Silicon, and a failure is fatal: the
   process is killed with "CODESIGNING ... Invalid Page".  We do it ourselves, rather than relying solely on codesign,
   because codesign --verify --deep on the bundle does not check every file -- see comment in signAppBundle().
   """
   problems = []
   data = path.read_bytes()
   for arch, machO in _machOSlices(data):
      magic, _, _, _, numCommands, _, _ = struct.unpack('<IiiIIII', machO[:28])
      offset = 32 if magic == _THIN_MAGICS[1] else 28
      signature = None
      for _ in range(numCommands):
         command, commandSize = struct.unpack('<II', machO[offset : offset + 8])
         if command == _LC_CODE_SIGNATURE:
            signature = struct.unpack('<II', machO[offset + 8 : offset + 16])
         offset += commandSize
      if signature is None:
         problems.append(path.as_posix() + ' [' + arch + '] is not signed')
         continue
      dataOffset, dataSize = signature
      superBlob = machO[dataOffset : dataOffset + dataSize]
      superMagic, _, numBlobs = struct.unpack('>III', superBlob[:12])
      if superMagic != _CSMAGIC_EMBEDDED_SIGNATURE:
         problems.append(path.as_posix() + ' [' + arch + '] has a malformed code signature')
         continue
      numCodeDirectories = 0
      for ii in range(numBlobs):
         _, blobOffset = struct.unpack('>II', superBlob[12 + 8 * ii : 20 + 8 * ii])
         codeDirectory = superBlob[blobOffset:]
         if struct.unpack('>I', codeDirectory[:4])[0] != _CSMAGIC_CODEDIRECTORY:
            continue
         numCodeDirectories += 1
         (_, _, _, _, hashOffset, _, _, numCodeSlots, codeLimit,
          hashSize, hashType, _, pageSizeLog2) = struct.unpack('>IIIIIIIIIBBBB', codeDirectory[:40])
         hashFunction = _CS_HASH_FUNCTIONS.get(hashType)
         if hashFunction is None:
            problems.append(path.as_posix() + ' [' + arch + '] uses unknown code signature hash type ' + str(hashType))
            continue
         pageSize = (1 << pageSizeLog2) if pageSizeLog2 else codeLimit
         badPages = []
         for page in range(numCodeSlots):
            start = page * pageSize
            expected = codeDirectory[hashOffset + page * hashSize : hashOffset + (page + 1) * hashSize]
            if hashFunction(machO[start : min(start + pageSize, codeLimit)]).digest()[:hashSize] != expected:
               badPages.append(page)
         if badPages:
            problems.append(
               path.as_posix() + ' [' + arch + '] has ' + str(len(badPages)) + ' page(s) that do not match the code ' +
               'signature (eg page ' + str(badPages[0]) + ') -- probably modified after signing'
            )
      if numCodeDirectories == 0:
         problems.append(path.as_posix() + ' [' + arch + '] code signature has no CodeDirectory')
   return problems

def _codesign(args: list[str]):
   result = subprocess.run(['codesign'] + args, capture_output=True)
   if result.returncode != 0:
      btLogger.log.critical('codesign ' + ' '.join(args) + ' failed: ' + result.stderr.decode('UTF-8'))
      sys.exit(1)
   return

def _frameworkMainBinary(path: pathlib.Path) -> bool:
   """
   True if path is the main binary of a framework, ie Foo.framework/Versions/X/Foo
   """
   parts = path.parts
   return len(parts) >= 4 and parts[-3] == 'Versions' and parts[-4] == parts[-1] + '.framework'

def signAppBundle(appDir: pathlib.Path):
   """
   Ad hoc sign everything in the bundle, from the inside out.

   We used to do this in one go with `codesign --force --deep --sign -` on the bundle, but this is not recommended by
   Apple and doesn't work properly: --deep only re-signs nested code in the places it recognises as such, and it treats
   the plugin dylibs under Contents/Plugins/<type>/ as ordinary resources.  This meant plugins we modify with
   install_name_tool retained their original (now invalid) signatures, so the app was killed as soon as Qt loaded one of
   them.  (Unhelpfully, `codesign --verify --deep` on the bundle still passed, because it only checks that resources
   match the hashes recorded when the bundle was signed.)

   Now, we sign:
     1. every Mach-O file that is not the main binary of a framework or the app, deepest first
     2. every framework, deepest first
     3. the app bundle itself (which signs the main executable and seals everything else as resources)
   """
   contentsDir = appDir.joinpath('Contents')
   depth = lambda path: len(path.parts)
   mainExecutable = contentsDir.joinpath('MacOS', _bundleExecutableName(appDir))

   machOFiles = [p for p in contentsDir.rglob('*') if _isMachO(p)]
   looseFiles = [p for p in machOFiles if not _frameworkMainBinary(p) and p != mainExecutable]
   frameworks = [p for p in contentsDir.rglob('*.framework') if p.is_dir() and not p.is_symlink()]

   btLogger.log.info(
      'Ad hoc signing ' + str(len(looseFiles)) + ' library/plugin file(s), ' + str(len(frameworks)) +
      ' framework(s) and then ' + appDir.as_posix()
   )
   for file in sorted(looseFiles, key=depth, reverse=True):
      _codesign(['--force', '--sign', '-', file.as_posix()])
   for framework in sorted(frameworks, key=depth, reverse=True):
      _codesign(['--force', '--sign', '-', framework.as_posix()])
   _codesign(['--force', '--sign', '-', appDir.as_posix()])
   return

def _bundleExecutableName(appDir: pathlib.Path) -> str:
   with open(appDir.joinpath('Contents', 'Info.plist'), 'rb') as infoPlist:
      return plistlib.load(infoPlist)['CFBundleExecutable']

def verifyAllSignatures(appDir: pathlib.Path):
   """
   Check the code signature of every Mach-O file in the bundle, every framework, and the bundle as a whole.  Exits with
   an error if anything does not verify.  (On Apple Silicon, a bad signature on any library or plugin gets the app
   killed when that file is loaded, which may be well after startup.)
   """
   contentsDir = appDir.joinpath('Contents')
   machOFiles = sorted(p for p in contentsDir.rglob('*') if _isMachO(p))
   problems = []
   for file in machOFiles:
      problems += checkPageHashes(file)
      result = subprocess.run(['codesign', '--verify', '--strict', '--verbose=2', file.as_posix()], capture_output=True)
      if result.returncode != 0:
         problems.append('codesign --verify failed for ' + file.as_posix() + ': ' + result.stderr.decode('UTF-8').strip())
   for framework in sorted(p for p in contentsDir.rglob('*.framework') if p.is_dir() and not p.is_symlink()):
      result = subprocess.run(['codesign', '--verify', '--strict', '--verbose=2', framework.as_posix()], capture_output=True)
      if result.returncode != 0:
         problems.append('codesign --verify failed for ' + framework.as_posix() + ': ' + result.stderr.decode('UTF-8').strip())
   result = subprocess.run(['codesign', '--verify', '--deep', '--strict', '--verbose=2', appDir.as_posix()], capture_output=True)
   if result.returncode != 0:
      problems.append('codesign --verify failed for ' + appDir.as_posix() + ': ' + result.stderr.decode('UTF-8').strip())
   if problems:
      for problem in problems:
         btLogger.log.error(problem)
      btLogger.log.critical('Code signature problems in ' + appDir.as_posix() + ' (' + str(len(problems)) + ' above)')
      sys.exit(1)
   btLogger.log.info('Code signatures OK for all ' + str(len(machOFiles)) + ' Mach-O file(s) in ' + appDir.as_posix())
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

#
# dyld prints one line per image loaded when DYLD_PRINT_LIBRARIES is set.  The format has varied between dyld versions:
#    dyld: loaded: /usr/lib/libSystem.B.dylib                                     (dyld3 and earlier)
#    dyld[12345]: <B1A2...-UUID> /usr/lib/libSystem.B.dylib                        (dyld4, macOS 12+)
#    dyld[12345]: /usr/lib/libSystem.B.dylib
#
DYLD_LOADED_LINE = re.compile(r'^dyld(?:\[\d+\])?:\s+(?:loaded:\s+)?(?:<[^>]*>\s+)?(/.+?)\s*$', flags=re.MULTILINE)

#
# Things dyld can legitimately load from outside the bundle.  As well as SYSTEM_PREFIXES, this covers Apple's own
# system extensions.
#
SMOKE_TEST_ALLOWED_PREFIXES = SYSTEM_PREFIXES + ('/Library/Apple/',)

def parseDyldPrintLibraries(output: str) -> list[str]:
   """
   Given output produced with DYLD_PRINT_LIBRARIES=1, return the paths of all the images dyld loaded.
   """
   return DYLD_LOADED_LINE.findall(output)

def smokeTest(executable: pathlib.Path):
   """
   Run the executable with --version.  This constructs the QApplication (so loads the platform plugin) and then exits,
   so it will catch dyld failures ("Library not loaded", "duplicate LC_RPATH", etc) and signature problems.

   The build machine has Homebrew (and so all our dependencies) installed, which could mask a library missing from the
   bundle.  verify() above should already have caught any reference to something outside the bundle, but there are
   other routes by which something can get loaded from outside it -- eg a library dlopen()ed by leaf name, found via
   dyld's fallback search path (which includes /usr/local/lib, where Homebrew lives on Intel Macs), or a Qt plugin found
   via the Homebrew Qt's compiled-in plugin path.  So we:
     - clear the environment variables that could point dyld or Qt somewhere else, and restrict dyld's fallback search
       path to /usr/lib;
     - set DYLD_PRINT_LIBRARIES so that dyld logs every image it loads, and then fail if any of them came from outside
       the bundle and the OS.
   (DYLD_* variables are ignored for binaries signed with the hardened runtime.  Our ad hoc signature does not enable
   the hardened runtime, so they work here.  If we ever do proper signing with `--options runtime`, the "loaded from
   outside" check will see no dyld output and just log a warning.)

   We also re-check the code signatures of everything in the bundle as shipped (ie on the mounted disk image).
   """
   appDir = executable.parents[2]  # .../Foo.app/Contents/MacOS/Foo -> .../Foo.app
   verifyAllSignatures(appDir)

   env = {k: v for k, v in os.environ.items()
          if not k.startswith('DYLD_') and k not in ('QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH', 'QT_QPA_PLATFORM')}
   env['DYLD_FALLBACK_LIBRARY_PATH'] = '/usr/lib'
   env['DYLD_PRINT_LIBRARIES'] = '1'
   env['QT_DEBUG_PLUGINS'] = '1'

   btLogger.log.info('Smoke testing ' + executable.as_posix())
   try:
      # Run from a neutral directory so nothing relative to the build tree can be picked up
      result = subprocess.run([executable.as_posix(), '--version'], capture_output=True, timeout=120, env=env, cwd='/')
   except subprocess.TimeoutExpired:
      btLogger.log.critical('Smoke test timed out: the application did not exit within 2 minutes of being run with --version')
      sys.exit(1)
   stdout = result.stdout.decode('UTF-8', errors='replace')
   stderr = result.stderr.decode('UTF-8', errors='replace')

   # Don't flood the log with the (several hundred) dyld lines; we report the interesting ones below.
   btLogger.log.info('Smoke test exit code ' + str(result.returncode) + ', stdout: ' + stdout)
   otherStderr = '\n'.join(line for line in stderr.splitlines() if not DYLD_LOADED_LINE.match(line))
   btLogger.log.info('Smoke test stderr (excluding dyld load messages): ' + otherStderr)

   if result.returncode != 0:
      if result.returncode < 0:
         btLogger.log.critical(
            'Smoke test of packaged app failed: killed by signal ' + str(-result.returncode) +
            ('.  (On Apple Silicon, SIGKILL on launch usually means a code signature problem.)' if result.returncode == -9 else '')
         )
      else:
         btLogger.log.critical('Smoke test of packaged app failed with exit code ' + str(result.returncode))
      sys.exit(1)

   loaded = parseDyldPrintLibraries(stderr)
   if not loaded:
      btLogger.log.warning('Smoke test: dyld did not report any loaded images, so cannot check where they came from')
      return
   appPrefix = appDir.resolve().as_posix() + '/'
   outside = sorted({path for path in loaded
                     if not path.startswith(SMOKE_TEST_ALLOWED_PREFIXES)
                     and not pathlib.Path(path).resolve().as_posix().startswith(appPrefix)})
   btLogger.log.info('Smoke test: dyld loaded ' + str(len(loaded)) + ' image(s)')
   if outside:
      for path in outside:
         btLogger.log.error('Smoke test: loaded from outside the app bundle: ' + path)
      btLogger.log.critical(
         'Smoke test of packaged app failed: ' + str(len(outside)) + ' image(s) were loaded from outside the bundle, ' +
         'so the app would probably not start on a machine without them'
      )
      sys.exit(1)
   btLogger.log.info('Smoke test passed: everything loaded from inside the bundle or the OS')
   return