#-----------------------------------------------------------------------------------------------------------------------
# scripts/btFileSystem.py is part of Brewtarget, and is copyright the following authors 2022-2025:
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
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import os
import pathlib
import platform
import shutil

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btLogger

log = btLogger.getLogger()

#-----------------------------------------------------------------------------------------------------------------------
# Helper function to return the 'base' directory (ie the one above the directory in which this file lives).
#-----------------------------------------------------------------------------------------------------------------------
def getBaseDir():
   dir_thisScript = pathlib.Path(__file__).parent.resolve()
   dir_base = dir_thisScript.parent.resolve()
   return dir_base


#-----------------------------------------------------------------------------------------------------------------------
# Helper function for copying one or more files to a directory that might not yet exist
#-----------------------------------------------------------------------------------------------------------------------
def copyFilesToDir(files, directory):
   os.makedirs(directory, exist_ok=True)
   for currentFile in files:
      log.debug('Copying ' + currentFile + ' to ' + directory)
      shutil.copy2(currentFile, directory)
   return

#-----------------------------------------------------------------------------------------------------------------------
# Helper function for counting files in a directory tree
#-----------------------------------------------------------------------------------------------------------------------
def numFilesInTree(path):
   numFiles = 0
   for root, dirs, files in os.walk(path):
      numFiles += len(files)
   return numFiles

#-----------------------------------------------------------------------------------------------------------------------
# Helper function for finding the first match of file under path
#-----------------------------------------------------------------------------------------------------------------------
def findFirstMatchingFile(fileName, path):
   for root, dirs, files in os.walk(path):
      if fileName in files:
         return os.path.join(root, fileName)
   return ''

#-----------------------------------------------------------------------------------------------------------------------
# Set various global directory locations
#
# This is a bit horrible, but works for now
#-----------------------------------------------------------------------------------------------------------------------
def setGlobalDirVars():
   global dir_base
   global dir_gitInfo
   global dir_build
   global dir_gitSubmodules
   global num_gitSubmodules
   global dir_packages
   global dir_packages_platform
   global dir_packages_source
   global dir_containerized
   global dir_appImage

#   dir_base          = btFileSystem.getBaseDir()
   dir_base          = getBaseDir()
   dir_gitInfo       = dir_base.joinpath('.git')
   dir_build         = dir_base.joinpath('mbuild')
   # Where submodules live and how many there are.  Currently there are 2: libbacktrace and valijson
   dir_gitSubmodules = dir_base.joinpath('third-party')
   num_gitSubmodules = 2
   # Top-level packaging directory - NB deliberately different name from 'packaging' (= dir_base.joinpath('packaging'))
   dir_packages          = dir_build.joinpath('packages')
   dir_packages_platform = dir_packages.joinpath(platform.system().lower())   # Platform-specific packaging directory
   dir_packages_source   = dir_packages.joinpath('source')
   #
   # App Image has to live somewhere too.  It's morally equivalent to a Linux package, but we create it separately (for
   # reasons explained in the `bt` script).
   #
   # We'll assume we're going to get to Snap and Flatpak too, and put them all in the same top-level directory.  The
   # best generic name I found for these formats is "Containerized Application Packages", which is too long.  I think
   # "CAPs" is too short however, especially if we lower-case it all to "caps".  The general idea is that these are
   # supposed to be self-contained and portable across a lot of distros, so "linux-portable" is the best I came up with.
   #
   dir_containerized = dir_base.joinpath('linux-portable')
   dir_appImage      = dir_containerized.joinpath('appimage')