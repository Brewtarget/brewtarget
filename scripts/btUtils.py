#-----------------------------------------------------------------------------------------------------------------------
# scripts/btUtils.py is part of Brewtarget, and is copyright the following authors 2022-2024:
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
# This file contains various util functions used in our other Python build scripts
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import glob
import logging
import os
import packaging.version
import pathlib
import re
import shutil
import subprocess
import tomlkit

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btExecute
import btLogger
import btFileSystem

#-----------------------------------------------------------------------------------------------------------------------
# Helper function for finding and copying extra libraries
#
# This is used in both the Windows and Mac packaging
#
#    pathsToSearch    = array of paths to search
#    extraLibs        = array of base names of libraries to search for
#    libExtension     = 'dll' on Windows, 'dylib' on MacOS
#    libRegex         = '-?[0-9]*.dll' on Windows, '.*.dylib' on MacOS
#    targetDirectory  = where to copy found libraries to
#-----------------------------------------------------------------------------------------------------------------------
def findAndCopyLibs(pathsToSearch, extraLibs, libExtension, libRegex, targetDirectory):
   for extraLib in extraLibs:
      found = False
      for searchDir in pathsToSearch:
         # We do a glob match to get approximate matches and then filter it with a regular expression for exact
         # ones
         matches = []
         globMatches = glob.glob(extraLib + '*.' + libExtension, root_dir=searchDir, recursive=False)
         for globMatch in globMatches:
            # We need to remove the first part of the glob match before doing a regexp match because we don't want
            # the first part of the filename to be treated as a regular expression.  In particular, this would be
            # a problem for 'libstdc++'!
            suffixOfGlobMatch = globMatch.removeprefix(extraLib)
            # On Python 3.11 or later, we would write flags=re.NOFLAG instead of flags=0
            if re.fullmatch(re.compile(libRegex), suffixOfGlobMatch, flags=0):
               matches.append(globMatch)
         numMatches = len(matches)
         if (numMatches > 0):
            btLogger.log.debug('Found ' + str(numMatches) + ' match(es) for ' + extraLib + ' in ' + searchDir)
            if (numMatches > 1):
               btLogger.log.warning('Found more matches than expected (' + str(numMatches) + ' ' +
                           'instead of 1) when searching for library "' + extraLib + '".  This is not an ' +
                           'error, but means we are possibly shipping additional shared libraries that we '+
                           'don\'t need to.')
            for match in matches:
               fullPathOfMatch = pathlib.Path(searchDir).joinpath(match)
               btLogger.log.debug('Copying ' + fullPathOfMatch.as_posix() + ' to ' + targetDirectory.as_posix())
               shutil.copy2(fullPathOfMatch, targetDirectory)
            found = True
            break;
      if (not found):
         btLogger.log.critical('Could not find '+ extraLib + ' library in any of the following directories: ' + ', '.join(pathsToSearch))
         exit(1)
   return


#-----------------------------------------------------------------------------------------------------------------------
# Helper function to get Linux distro name and release number
#
# Returns a dictionary with:
#   "name" set to distro name (string)
#   "release" set to the release number (string)
#   "major" set to release major number (int)
#   "minor" set to release minor number (int)
#
# Example outputs:
#                     Ubuntu                 Debian
#                     ======                 ======
#       name:         "Ubuntu"               "Debian"
#       release:      "24.04"                "12"
#       major:        24                     12
#       minor:        4                      0
#
#-----------------------------------------------------------------------------------------------------------------------
def getLinuxDistroInfo():
   # See comment above about why it's OK to call logging.getLogger() more than once
   log = logging.getLogger(__name__)

   distroInfo = {
      "name": "Unknown",
      "release": "",
      "major":   "",
      "minor":   ""
   }

   #
   # We run lsb_release -a to give full output for logging.  But we then run again just to output the parameters we
   # want (which seems less work and more reliable than parsing the full output with regular expressions).
   #
   # It is not a fatal error if we cannot run lsb_release.  (Caller has to decide what to do if we couldn't determine
   # release info.)  So, we deliberately don't use btUtils.abortOnRunFail here.
   #
   lsbResult = subprocess.run(['lsb_release', '-a'], capture_output=True)
   if (lsbResult.returncode != 0):
      btLogger.log.info('Ignoring error running lsb_release -a: ' + lsbResult.stderr.decode('UTF-8'))
   else:
      lsbOutput = lsbResult.stdout.decode('UTF-8').rstrip()
      btLogger.log.info('Output from running lsb_release -a: ' + lsbOutput)
      #
      # We assume that if `lsb_release -a` ran OK then the other invocations below will too
      #
      distroInfo["name"] = str(
         subprocess.run(['lsb_release', '-is'], encoding = "utf-8", capture_output = True).stdout
      ).rstrip()
      distroInfo["release"] = str(
         subprocess.run(['lsb_release', '-rs'], encoding = "utf-8", capture_output = True).stdout
      ).rstrip()

      #
      # Now split release into major and minor
      #
      parsedRelease = packaging.version.parse(distroInfo["release"])
      distroInfo["major"] = parsedRelease.major
      distroInfo["minor"] = parsedRelease.minor

   return distroInfo


#-----------------------------------------------------------------------------------------------------------------------
# Set global variables exe_git and exe_meson with the locations of the git and meson executables plus mesonVersion with
# the version of meson installed
#
# We want to give helpful error messages if Meson or Git is not installed.  For other missing dependencies we can rely
# on Meson itself to give explanatory error messages.
#-----------------------------------------------------------------------------------------------------------------------
def findMesonAndGit():
   # Advice at https://docs.python.org/3/library/subprocess.html is "For maximum reliability, use a fully qualified path
   # for the executable. To search for an unqualified name on PATH, use shutil.which()"

   # Check Meson is installed.  (See installDependencies() below for what we do to attempt to install it from this
   # script.)
   global exe_meson
   exe_meson = shutil.which("meson")
   if (exe_meson is None or exe_meson == ""):
      btLogger.log.critical('Cannot find meson - please see https://mesonbuild.com/Getting-meson.html for how to install')
      exit(1)

   global mesonVersion
   rawVersion = btExecute.abortOnRunFail(subprocess.run([exe_meson, '--version'], capture_output=True)).stdout.decode('UTF-8').rstrip()
   btLogger.log.debug('Meson version raw: ' + rawVersion)
   mesonVersion = packaging.version.parse(rawVersion)
   btLogger.log.debug('Meson version parsed: ' + str(mesonVersion))

   # Check Git is installed if its magic directory is present
   global exe_git
   exe_git   = shutil.which("git")
   if (btFileSystem.dir_gitInfo.is_dir()):
      btLogger.log.debug('Found git information directory:' + btFileSystem.dir_gitInfo.as_posix())
      if (exe_git is None or exe_git == ""):
         btLogger.log.critical('Cannot find git - please see https://git-scm.com/downloads for how to install')
         exit(1)

   return

def findWget():
   global exe_wget
   exe_wget = shutil.which("wget")
   if (exe_wget is None or exe_wget == ""):
      btLogger.log.critical('Cannot find wget')
      exit(1)

   return

#-----------------------------------------------------------------------------------------------------------------------
# Ensure git submodules are present
#
# When a git repository is cloned, the submodules don't get cloned until you specifically ask for it via the
# --recurse-submodules flag.
#
# (Adding submodules is done via Git itself.  Eg:
#    cd ../third-party
#    git submodule add https://github.com/ianlancetaylor/libbacktrace
# But this only needs to be done once, by one person, and committed to our repository, where the connection is
# stored in the .gitmodules file.)
#-----------------------------------------------------------------------------------------------------------------------
def ensureSubmodulesPresent():
   findMesonAndGit()
   if (not btFileSystem.dir_gitSubmodules.is_dir()):
      btLogger.log.info('Creating submodules directory: ' + btFileSystem.dir_gitSubmodules.as_posix())
      os.makedirs(btFileSystem.dir_gitSubmodules, exist_ok=True)
   if (btFileSystem.numFilesInTree(btFileSystem.dir_gitSubmodules) < btFileSystem.num_gitSubmodules):
      btLogger.log.info('Pulling in submodules in ' + btFileSystem.dir_gitSubmodules.as_posix())
      btExecute.abortOnRunFail(subprocess.run([exe_git, "submodule", "init"], capture_output=False))
      btExecute.abortOnRunFail(subprocess.run([exe_git, "submodule", "update"], capture_output=False))
   return

#-----------------------------------------------------------------------------------------------------------------------
# Read config variables exported by the Meson build
#-----------------------------------------------------------------------------------------------------------------------
def readBuildConfigFile():
   global buildConfig
   with open(btFileSystem.dir_build.joinpath('config.toml').as_posix()) as buildConfigFile:
      buildConfig = tomlkit.parse(buildConfigFile.read())
   return

#-----------------------------------------------------------------------------------------------------------------------
# Create fileToDistribute.sha256sum for a given fileToDistribute in a given directory
#-----------------------------------------------------------------------------------------------------------------------
def writeSha256sum(directory, fileToDistribute):
   btLogger.log.info('Generating checksum file for ' + fileToDistribute + ' in ' + directory.as_posix())
   #
   # In Python 3.11 we could use the file_digest() function from the hashlib module to do this.  But it's rather
   # more work to do in Python 3.10, so we just use the `sha256sum` command instead.
   #
   # Note however, that `sha256sum` includes the supplied directory path of a file in its output.  We want just the
   # filename, not its full or partial path on the build machine.  So we change into the directory of the file before
   # running the `sha256sum` command.
   #
   previousWorkingDirectory = pathlib.Path.cwd().as_posix()
   os.chdir(directory)
   with open(directory.joinpath(fileToDistribute + '.sha256sum').as_posix(),'w') as sha256File:
      btExecute.abortOnRunFail(
         subprocess.run(['sha256sum', fileToDistribute],
                        capture_output=False,
                        stdout=sha256File)
      )
   os.chdir(previousWorkingDirectory)
   return