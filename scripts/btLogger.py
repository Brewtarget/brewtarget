#-----------------------------------------------------------------------------------------------------------------------
# scripts/btLogger.py is part of Brewtarget, and is copyright the following authors 2022-2025:
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
