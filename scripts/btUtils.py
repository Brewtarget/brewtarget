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
import logging
import os
import pathlib
import subprocess

#-----------------------------------------------------------------------------------------------------------------------
# Helper function to return the 'base' directory (ie the one above the directory in which this file lives).
#-----------------------------------------------------------------------------------------------------------------------
def getBaseDir():
   dir_thisScript = pathlib.Path(__file__).parent.resolve()
   dir_base = dir_thisScript.parent.resolve()
   return dir_base

#-----------------------------------------------------------------------------------------------------------------------
# Helper function to return a logger that logs to stderr
#
# param logLevel - Initial level to log at
#-----------------------------------------------------------------------------------------------------------------------
def getLogger(logLevel = logging.INFO):
   logging.basicConfig(format='%(message)s')
   # Per https://docs.python.org/3/library/logging.html __name__ is the module’s name in the Python package namespace.
   # This is fine for us.  I don't think we care too much what the logger's name is.
   log = logging.getLogger(__name__)
   log.setLevel(logLevel)
   # Include the log level in the message
   handler = logging.StreamHandler()
   handler.setFormatter(
      # You can add timestamps etc to logs, but that's overkill for this script.  Source file location of log message is
      # however pretty useful for debugging.
      logging.Formatter('{levelname:s}:  {message}  [{filename:s}:{lineno:d}]', style='{')
   )
   log.addHandler(handler)
   # If we don't do this, everything gets printed twice
   log.propagate = False
   return log

#-----------------------------------------------------------------------------------------------------------------------
# Helper function for checking result of running external commands
#
# Given a CompletedProcess object returned from subprocess.run(), this checks the return code and, if it is non-zero
# stops this script with an error message and the same return code.  Otherwise the CompletedProcess object is returned
# to the caller (to make it easier to chain things together).
#-----------------------------------------------------------------------------------------------------------------------
def abortOnRunFail(runResult: subprocess.CompletedProcess):
   if (runResult.returncode != 0):
      # Per https://docs.python.org/3/library/logging.html, "Multiple calls to logging.getLogger() with the same name
      # [parameter] will always return a reference to the same Logger object."  So we are safe to set log in this way.
      log = logging.getLogger(__name__)

      # According to https://docs.python.org/3/library/subprocess.html#subprocess.CompletedProcess,
      # CompletedProcess.args (the arguments used to launch the process) "may be a list or a string", but it's not clear
      # when it would be one or the other.
      if (isinstance(runResult.args, str)):
         log.critical('Error running ' + runResult.args)
      else:
         commandName = os.path.basename(runResult.args[0])
         log.critical('Error running ' + commandName + ' (' + ' '.join(str(ii) for ii in runResult.args) + ')')
      if runResult.stderr:
         log.critical('stderr: ' + runResult.stderr.decode('UTF-8'))
      exit(runResult.returncode)

   return runResult


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
# Helper function for downloading a file
#-----------------------------------------------------------------------------------------------------------------------
def downloadFile(url):
   filename = url.split('/')[-1]
   log.info('Downloading ' + url + ' to ' + filename + ' in directory ' + pathlib.Path.cwd().as_posix())
   response = requests.get(url)
   if (response.status_code != 200):
      log.critical('Error code ' + str(response.status_code) + ' while downloading ' + url)
      exit(1)
   with open(filename, 'wb') as fd:
      for chunk in response.iter_content(chunk_size = 128):
         fd.write(chunk)
   return

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
            log.debug('Found ' + str(numMatches) + ' match(es) for ' + extraLib + ' in ' + searchDir)
            if (numMatches > 1):
               log.warning('Found more matches than expected (' + str(numMatches) + ' ' +
                           'instead of 1) when searching for library "' + extraLib + '".  This is not an ' +
                           'error, but means we are possibly shipping additional shared libraries that we '+
                           'don\'t need to.')
            for match in matches:
               fullPathOfMatch = pathlib.Path(searchDir).joinpath(match)
               log.debug('Copying ' + fullPathOfMatch.as_posix() + ' to ' + targetDirectory.as_posix())
               shutil.copy2(fullPathOfMatch, targetDirectory)
            found = True
            break;
      if (not found):
         log.critical('Could not find '+ extraLib + ' library in any of the following directories: ' + ', '.join(pathsToSearch))
         exit(1)
   return
