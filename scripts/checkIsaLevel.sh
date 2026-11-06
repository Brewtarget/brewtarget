#!/bin/bash
#-----------------------------------------------------------------------------------------------------------------------
# scripts/checkIsaLevel.sh is part of Brewtarget, and is copyright the following authors 2026:
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
#
# Checks that x86-64 Linux binaries (or the binaries inside .deb packages) will run on any x86-64 CPU, ie that they
# do not require x86-64-v2 or higher.  See https://github.com/Brewtarget/brewtarget/issues/1179 for background.
#
# Usage: checkIsaLevel.sh <file.deb | ELF binary> [...]
#
# Two checks are made on each binary:
#   1. The "x86 ISA needed" note (which is what the glibc dynamic loader checks, giving "CPU ISA level is lower than
#      required" if the CPU doesn't support it) must not mention x86-64-v2, -v3 or -v4.
#   2. The disassembly must not contain VEX-encoded (AVX/AVX2/FMA) instructions or BMI2 instructions, which would cause
#      SIGILL on older CPUs even if the ISA note is missing or understates the requirement.
#
#   Note that we do not flag tzcnt, because it is encoded as "rep bsf", which older CPUs execute as bsf, and compilers
#   only emit it in places where that gives the same result.  Nor do we flag SSE4.2/PCLMUL instructions, because
#   libraries (eg Sourcemeta Core) legitimately use these in functions that are only called after a run-time CPU check.
#
# Exit code is non-zero if any check fails.
#
set -euo pipefail

if [[ $# -eq 0 ]]; then
   echo "Usage: $0 <file.deb | ELF binary> [...]" >&2
   exit 2
fi

workDir=$(mktemp -d)
trap 'rm -rf "${workDir}"' EXIT

status=0

checkBinary() {
   local bin="$1"
   echo "Checking ${bin}"

   local isaNote
   isaNote=$(readelf -n "${bin}" | grep 'x86 ISA needed' || true)
   echo "   ISA note: ${isaNote:-<none>}"
   if grep -qE 'x86-64-v[234]' <<< "${isaNote}"; then
      echo "   ❌ Binary is marked as needing more than the x86-64 baseline"
      status=1
   fi

   local numBad
   numBad=$(objdump -d --no-show-raw-insn "${bin}" |
            grep -cE $'\t(v[a-z0-9]+|shlx|shrx|sarx|mulx|rorx|pdep|pext)[ \t]' || true)
   echo "   AVX/BMI2 instructions found: ${numBad}"
   if [[ ${numBad} -ne 0 ]]; then
      echo "   ❌ Binary contains instructions that older x86-64 CPUs cannot execute.  First few:"
      objdump -d --no-show-raw-insn "${bin}" |
         awk '/^[0-9a-f]+ <.*>:$/ {fn=$2} /\t(v[a-z0-9]+|shlx|shrx|sarx|mulx|rorx|pdep|pext)[ \t]/ {print fn}' |
         sort | uniq -c | sort -rn | head -10 | c++filt || true # Ignore SIGPIPE from head closing the pipe early
      status=1
   fi
}

for arg in "$@"; do
   case "${arg}" in
      *.deb)
         extractDir="${workDir}/$(basename "${arg}" .deb)"
         mkdir -p "${extractDir}"
         dpkg-deb -x "${arg}" "${extractDir}"
         while IFS= read -r -d '' elfFile; do
            checkBinary "${elfFile}"
         done < <(find "${extractDir}" -type f -exec sh -c 'head -c 4 "$1" | grep -q "ELF"' _ {} \; -print0)
         ;;
      *)
         checkBinary "${arg}"
         ;;
   esac
done

if [[ ${status} -eq 0 ]]; then
   echo "✅ All binaries OK for baseline x86-64"
fi
exit ${status}
