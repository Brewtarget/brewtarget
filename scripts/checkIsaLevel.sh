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
#   2. The disassembly should not contain VEX-encoded (AVX/AVX2/FMA) instructions or BMI2 instructions, which would
#      cause SIGILL on older CPUs even if the ISA note is missing or understates the requirement.
#
#      However, some libraries legitimately contain a few such instructions in SIMD kernels that are only called after
#      a run-time CPU check.  Eg Sourcemeta Core (used by Blaze) has an AVX2 JSON scanning kernel
#      (scan_hashes_blocks_wide in src/spec/json/json_value.cc) that it only uses if __builtin_cpu_supports("avx2").
#      So, rather than requiring zero such instructions, we distinguish two cases:
#
#      - If the binary has a full symbol table (eg the unstripped binary in the build directory), we know which
#        function each instruction is in, and we fail if any function not matching ALLOWED_FUNCTIONS contains them.
#
#      - If the binary is stripped (eg the one in the .deb), only exported symbols remain, so we can't reliably say
#        which function each instruction is in (it gets attributed to the nearest preceding exported symbol, which is
#        usually wrong).  We therefore just fail if there are more than MAX_STRIPPED_HITS.  When the whole program has
#        been compiled for a higher ISA level, there are typically tens of thousands of such instructions, whereas the
#        legitimate run-time-dispatched kernels contain a few dozen.
#
#   Note that we do not flag tzcnt, because it is encoded as "rep bsf", which older CPUs execute as bsf, and compilers
#   only emit it in places where that gives the same result.  Nor do we flag SSE4.x/PCLMUL instructions, because
#   libraries (eg Sourcemeta Core) legitimately use these in functions that are only called after a run-time CPU check.
#
# Exit code is non-zero if any check fails.
#
set -euo pipefail

if [[ $# -eq 0 ]]; then
   echo "Usage: $0 <file.deb | ELF binary> [...]" >&2
   exit 2
fi

#
# Extended regex matching (demangled) names of functions that are allowed to contain AVX/BMI2 instructions because
# they are only called after a run-time check that the CPU supports them.  If you add to this, please say why!
#
#   scan_hashes_blocks_wide : Sourcemeta Core AVX2 JSON scanning kernel -- see above
#
ALLOWED_FUNCTIONS='scan_hashes_blocks_wide'

#
# See comment above.  Sourcemeta Core's kernels account for ~30 at the time of writing.
#
MAX_STRIPPED_HITS=200

# The instructions we look for.  (Leading tab is how objdump separates the address from the instruction.)
SUSPECT_INSTRUCTIONS=$'\t(v[a-z0-9]+|shlx|shrx|sarx|mulx|rorx|pdep|pext)[ \t]'

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

   #
   # List of "<count> <function>" for each function containing suspect instructions, most hits first.  (Function names
   # are demangled, which is why we don't use the label after the address directly.)
   #
   local perFunction
   perFunction=$(objdump -d -C --no-show-raw-insn "${bin}" |
                 awk -v pat="${SUSPECT_INSTRUCTIONS}" '/^[0-9a-f]+ <.*>:$/ {fn=$0; sub(/^[0-9a-f]+ </, "", fn); sub(/>:$/, "", fn)}
                                                       $0 ~ pat {print fn}' |
                 sort | uniq -c | sort -rn || true)

   local numHits=0
   if [[ -n "${perFunction}" ]]; then
      numHits=$(awk '{sum += $1} END {print sum + 0}' <<< "${perFunction}")
   fi
   echo "   AVX/BMI2 instructions found: ${numHits}"
   if [[ ${numHits} -eq 0 ]]; then
      return
   fi

   if readelf -S "${bin}" | grep -qE '\.symtab[[:space:]]'; then
      #
      # Unstripped binary, so we can trust the function names
      #
      local notAllowed
      notAllowed=$(grep -vE "${ALLOWED_FUNCTIONS}" <<< "${perFunction}" || true)
      local allowed
      allowed=$(grep -E "${ALLOWED_FUNCTIONS}" <<< "${perFunction}" || true)
      if [[ -n "${allowed}" ]]; then
         echo "   Allowed (run-time CPU check) functions containing them:"
         sed 's/^/      /' <<< "${allowed}"
      fi
      if [[ -n "${notAllowed}" ]]; then
         echo "   ❌ Functions that are not on the allowed list contain instructions that older x86-64 CPUs cannot" \
              "execute.  First few:"
         head -20 <<< "${notAllowed}" | sed 's/^/      /'
         status=1
      fi
   else
      #
      # Stripped binary, so function names are approximate.  Only the count is meaningful.
      #
      if [[ ${numHits} -gt ${MAX_STRIPPED_HITS} ]]; then
         echo "   ❌ Stripped binary contains more than ${MAX_STRIPPED_HITS} instructions that older x86-64 CPUs cannot" \
              "execute.  Nearest exported symbols (approximate) are:"
         head -10 <<< "${perFunction}" | sed 's/^/      /'
         status=1
      else
         echo "   (At most ${MAX_STRIPPED_HITS} allowed in a stripped binary, for run-time-dispatched SIMD kernels." \
              "Check the unstripped binary for details.)"
      fi
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
