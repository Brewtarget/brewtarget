#!/bin/bash
#
# packaging/generateCompressedChangeLog.sh is part of Brewtarget, and is copyright the following authors 2022:
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
#

#-----------------------------------------------------------------------------------------------------------------------
# NB: This script is intended to be invoked from the bt build tool (see ../bt) with the following environment variables
# set:
#    CONFIG_APPLICATION_NAME_LC     - Same as projectName in meson.build
#    CONFIG_CHANGE_LOG_UNCOMPRESSED - Input file - same as filesToInstall_changeLogUncompressed in meson.build
#    CONFIG_CHANGE_LOG_COMPRESSED   - Output file
#    CONFIG_PACKAGE_MAINTAINER      - Name and email of a project maintainer conforming to
#                                     https://www.debian.org/doc/debian-policy/ch-binary.html#s-maintainer
#
# We assume that none of these variables contains single or double quotes (so we can save ourselves having to escape
# the values when we use them below).
#
# First thing we do is check that all these variables are set to something.
#-----------------------------------------------------------------------------------------------------------------------
for var in CONFIG_APPLICATION_NAME_LC CONFIG_CHANGE_LOG_UNCOMPRESSED CONFIG_CHANGE_LOG_COMPRESSED CONFIG_PACKAGE_MAINTAINER
do
   if [ -z "${!var}" ]
   then
      echo "ERROR $var is unset or blank" >&2
      exit 1
   fi
done

echo "Parsing ${CONFIG_CHANGE_LOG_UNCOMPRESSED}"

#
# The rest of this script creates a compressed changelog in a Debian-friendly format
#
# Our change log (CHANGES.markdown) uses markdown format, with the following raw structure:
#    ## v1.2.3
#
#    Optional one-line description of the release.
#
#    ### New Features
#
#    * Blah blah blah
#    * etc
#
#    ### Bug Fixes
#
#    * Blah blah blah
#    * etc
#
#    ### Incompatibilities
#
#    None
#
#    ### Release Timestamp
#    Sun, 06 Feb 2022 12:02:58 +0100
#
# However, per https://www.debian.org/doc/debian-policy/ch-source.html#debian-changelog-debian-changelog, Debian change
# logs need to be in the following format:
#    package (version) distribution(s); urgency=urgency
#      [optional blank line(s), stripped]
#      * change details
#      more change details
#      [blank line(s), included in output of dpkg-parsechangelog]
#      * even more change details
#      [optional blank line(s), stripped]
#     -- maintainer name <email address>[two spaces]  date
#
# We are being a bit fast-and-loose in hard-coding the same maintainer name for each release, but I don't thing it's a
# huge issue.
#
# Note that, to keep us on our toes, Debian change log lines are not supposed to be more than 80 characters long.  This
# is non-trivial, but the ghastly bit of awk below gets us most of the way there.
#
cat "${CONFIG_CHANGE_LOG_UNCOMPRESSED}" |
   # Skip over the introductory headings and paragraphs of CHANGES.markdown until we get to the first version line
   sed -n '/^## v/,$p' |
   # We want to change the release timestamp to maintainer + timestamp, but we don't want to create too long a line
   # before we do the fold command below, so use "÷÷maintainer÷÷" as a placeholder for
   # " -- ${CONFIG_PACKAGE_MAINTAINER}  "
   sed -z "s/\\n### Release Timestamp\\n\\([^\\n]*\\)\\n/\\n÷÷maintainer÷÷\\1\\n/g" |
   # Join continued lines in bullet lists
   sed -z "s/\\n  / /g" |
   # Change the version to package (version) etc.  Stick a '÷' on the front of the line to protect it from
   # modification below
   sed "s/^## v\\(.*\\)$/÷${CONFIG_APPLICATION_NAME_LC} (\\1-1) unstable\; urgency=low/" |
   # Change bullets to sub-bullets
   sed "s/^\\* /    - /" |
   # Change headings to bullets
   sed "s/^### /  * /" |
   # Change any lines that don't start with space OR a ÷ character to be bullets
   sed "s/^\\([^ ÷]\\)/  * \\1/" |
   # Split any long lines.  Make the width less than 80 so we've got a margin go insert spaces at the start of
   # bullet continuation lines.
   fold -s --width=72 |
   # With a lot of help from awk, reindent the lines that were split off from a long bullet line so that they align
   # with that previous line.
   awk "BEGIN { inBullet=0 }
   {
      if (!inBullet) {
         inBullet=match(\$0, \"^( +)[^ ] \", spaces);
         print;
      } else {
         bulletContinues=match(\$0, \"^[^ ]\");
         if (!bulletContinues) {
            inBullet=match(\$0, \"^( +)[^ ] \", spaces);
            print;
         } else {
            print spaces[1] \"  \" \$0;
         }
      }
   }" |
   # Fix the "÷÷maintainer÷÷" placeholders
   sed "s/÷÷maintainer÷÷/ -- ${CONFIG_PACKAGE_MAINTAINER}  /" |
   # Remove the protective "÷" from the start of any other lines
   sed "s/^÷//" |
   gzip --best -n --to-stdout > "${CONFIG_CHANGE_LOG_COMPRESSED}"

echo "Wrote to ${CONFIG_CHANGE_LOG_COMPRESSED}"
exit 0
