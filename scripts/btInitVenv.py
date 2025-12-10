#!/usr/bin/env python3
#-----------------------------------------------------------------------------------------------------------------------
# scripts/btInitVenv.py is part of Brewtarget, and is copyright the following authors 2022-2025:
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
import shutil
import subprocess
import sys

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btLogger
import btExecute

#-----------------------------------------------------------------------------------------------------------------------
# Globals
#-----------------------------------------------------------------------------------------------------------------------
#log = btLogger.getLogger()
exe_python = shutil.which('python3')

btLogger.log.info('sys.version: ' + sys.version + '; exe_python: ' + exe_python + '; ' + sys.executable)

# This is long but it can be a useful diagnostic
btLogger.log.info('Environment variables vvv')
envVars = dict(os.environ)
for key, value in envVars.items():
   btLogger.log.info('Env: ' + key + '=' + value)
btLogger.log.info('Environment variables ^^^')

# Since (courtesy of the 'bt' script that invokes us) we're running in a venv, the pip we find should be the one in the
# venv.  This means that it will install to the venv and not mind about external package managers.
exe_pip = shutil.which('pip3')
# If Pip still isn't installed we need to bail here.
if (exe_pip is None or exe_pip == ''):
   pathEnvVar = ''
   if ('PATH' in os.environ):
      pathEnvVar = os.environ['PATH']
   btLogger.log.critical(
      'Cannot find pip (PATH=' + pathEnvVar + ') - please see https://pip.pypa.io/en/stable/installation/ for how to ' +
      'install'
   )
   exit(1)

btLogger.log.info('Found pip at: ' + exe_pip)

#
# Of course, when you run the pip in the venv, it might complain that it is not up-to-date.  So we should ensure that
# first.  Note that it is Python we must run to upgrade pip, as pip cannot upgrade itself.  (Pip will happily _try_ to
# upgrade itself, but then, on Windows at least, will get stuck when it tries to remove the old version of itself
# because "process cannot access the file because it is being used by another process".)
#
# You might think we could use sys.executable instead of exe_python here.  However, on Windows at least, that gives the
# wrong python executable: the "system" one rather than the venv one.
#
btLogger.log.info('Running ' + exe_python + '-m pip install --upgrade pip')
btExecute.abortOnRunFail(subprocess.run([exe_python, '-m', 'pip', 'install', '--upgrade', 'pip']))

#
# Per https://docs.python.org/3/library/sys.html#sys.path, this is the search path for Python modules, which is useful
# for debugging import problems.  Provided that we  started with a clean venv (so there is only one version of Python
# installed in it), then the search path for packages should include the directory
# '/some/path/to/.venv/lib/pythonX.yy/site-packages' (where 'X.yy' is the Python version number (eg 3.11, 3.12, etc).
# If there is more than one version of Python in the venv, then none of these site-packages directories will be in the
# path.  (We could, in principle, add it manually, but it's a bit fiddly and not necessary since we don't use the venv
# for anything other than running this script.)
#
btLogger.log.info('Initial module search paths:\n   ' + '\n   '.join(sys.path))

#
# Mostly, from here on out we'd be fine to invoke pip directly, eg via:
#
#    btExecute.abortOnRunFail(subprocess.run([exe_pip, 'install', 'setuptools']))
#
# However, in practice, it turns out this can lead to problems in the Windows MSYS2 environment.  According to
# https://stackoverflow.com/questions/12332975/how-can-i-install-a-python-module-within-code, the recommended and most
# robust way to invoke pip from within a Python script is via:
#
#    subprocess.check_call([sys.executable, "-m", "pip", "install", package])
#
# Where package is whatever package you want to install.  However, note comments above that we need exe_python rather
# than sys.executable.
#

#
# We use the packaging module (see https://pypi.org/project/packaging/) for handling version numbers (as described at
# https://packaging.pypa.io/en/stable/version.html).
#
# On some platforms, we also need to install setuptools to be able to access packaging.version.  (NB: On MacOS,
# setuptools is now installed by default by Homebrew when it installs Python, so we'd get an error if we try to install
# it via pip here.  On Windows in MSYS2, packaging and setuptools need to be installed via pacman.)
#
btLogger.log.info('pip install packaging')
btExecute.abortOnRunFail(subprocess.run([exe_python, '-m', 'pip', 'install', 'packaging']))
from packaging import version
btLogger.log.info('pip install setuptools')
btExecute.abortOnRunFail(subprocess.run([exe_python, '-m', 'pip', 'install', 'setuptools']))
import packaging.version

# The requests library (see https://pypi.org/project/requests/) is used for downloading files in a more Pythonic way
# than invoking wget through the shell.
btLogger.log.info('pip install requests')
btExecute.abortOnRunFail(subprocess.run([exe_python, '-m', 'pip', 'install', 'requests']))
import requests

#
# Once all platforms we're running on have Python version 3.11 or above, we will be able to use the built-in tomllib
# library (see https://docs.python.org/3/library/tomllib.html) for parsing TOML.  Until then, it's easier to import the
# tomlkit library (see https://pypi.org/project/tomlkit/) which actually has rather more functionality than we need
#
btExecute.abortOnRunFail(subprocess.run([sys.executable, '-m', 'pip', 'install', 'tomlkit']))
import tomlkit