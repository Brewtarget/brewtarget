#!/usr/bin/env python3
#-----------------------------------------------------------------------------------------------------------------------
# scripts/btInit.py is part of Brewtarget, and is copyright the following authors 2022-2024:
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
# This Python script is intended to be invoked by the `bt` bash script in the parent directory.  See comments in that
# script for why.
#
# Here we do some general set-up, including ensuring that a suitable Python virtual environment (venv) exists, before
# returning to the `bt` script to switch into that environment and invoke the main Python build tool script
# (buildTool.py)
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import platform
import shutil
import subprocess
import sys

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btUtils

log = btUtils.getLogger()


#
# First step is to ensure we have the minimum-required Python packages, including pip, are installed at system level.
#
# Prior to Python 3.12, setuptools is required to create and use virtual environments.
#
match platform.system():
   case 'Linux':
      # We don't want to run a sudo command every time the script is invoked, so check whether it's necessary
      exe_pip = shutil.which('pip3')
      ranUpdate = False
      if (exe_pip is None or exe_pip == ''):
         log.info('Need to install pip')
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
         ranUpdate = True
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'python3-pip']))
      # This is a bit clunky, but it's the simplest way to see if setuptools is already installed.  (Alternatively we
      # could run `pip3 list` and search for setuptools in the outpout.)
      foundSetupTools = False
      try:
         import setuptools
      except ImportError:
         log.info('Need to install setuptools')
      else:
         foundSetupTools = True
      if (not foundSetupTools):
         if (not ranUpdate):
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
            ranUpdate = True
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'python3-setuptools']))
      # It's a similar process for ensurepip, which is needed by venv below
      foundEnsurepip = False
      try:
         import ensurepip
      except ImportError:
         log.info('Need to install ensurepip')
      else:
         log.info('Found ensurepip')
         foundEnsurepip = True
      if (not foundEnsurepip):
         if (not ranUpdate):
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
            ranUpdate = True
         # Yes, I know it's confusing that we have to install a package called venv to ensure that the venv command
         # below doesn't complain about ensurepip not being present.  We're just doing what the error messages tell us
         # to.
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'python3-venv']))
   case 'Windows':
      #
      # In the past, we were able to install pip via Python, with the following code:
      #
      #    # https://docs.python.org/3/library/sys.html#sys.executable says sys.executable is '"the absolute path of the
      #    # executable binary for the Python interpreter, on systems where this makes sense".
      #    log.info(
      #       'Attempting to ensure latest version of pip is installed via  ' + sys.executable + ' -m ensurepip --upgrade'
      #    )
      #    btUtils.abortOnRunFail(subprocess.run([sys.executable, '-m', 'ensurepip', '--upgrade']))
      #
      # However, as of 2024-11, this gives "error: externally-managed-environment" and a direction "To install Python
      # packages system-wide, try 'pacman -S $MINGW_PACKAGE_PREFIX-python-xyz', where xyz is the package you are trying
      # to install."  So now we do that instead.  (Note that MINGW_PACKAGE_PREFIX will normally be set to
      # "mingw-w64-x86_64".)
      #
      log.info('Install pip via pacman')
      btUtils.abortOnRunFail(subprocess.run(['pacman', '-S', '$MINGW_PACKAGE_PREFIX-python-pip']))
      # See comment in scripts/buildTool.py about why we have to run pip via Python rather than just invoking pip
      # directly eg via `shutil.which('pip3')`.
      log.info('python -m pip install setuptools')
      btUtils.abortOnRunFail(subprocess.run([sys.executable, '-m', 'pip', 'install', 'setuptools']))
   case 'Darwin':
      # Assuming it was Homebrew that installed Python, then, according to https://docs.brew.sh/Homebrew-and-Python,
      # it bundles various packages, including pip.  Since Python version 3.12, Homebrew marks itself as package manager
      # for the Python packages it bundles, so it's an error to try to install or update them via Python.
      log.info('Assuming pip is already up-to-date; installing python-setuptools')
      btUtils.abortOnRunFail(subprocess.run(['brew', 'install', 'python-setuptools']))

   case _:
      log.critical('Unrecognised platform: ' + platform.system())
      exit(1)

#
# At this point we should have enough installed to set up a virtual environment.  (It doesn't matter if the virtual
# environment already exists.  We are only using it to run the scripts/buildTool.py script.)
#
dir_venv = btUtils.getBaseDir().joinpath('.venv')
log.info('Create Python virtual environment in ' + dir_venv.as_posix())
btUtils.abortOnRunFail(subprocess.run([sys.executable, '-m', 'venv', dir_venv.as_posix()]))

# Control now returns to the bt bash script in the parent directory
