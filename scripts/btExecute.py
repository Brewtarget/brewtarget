#!/usr/bin/env python3
#-----------------------------------------------------------------------------------------------------------------------
# scripts/btExecute.py is part of Brewtarget, and is copyright the following authors 2022-2025:
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
import logging
import os
import subprocess

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
