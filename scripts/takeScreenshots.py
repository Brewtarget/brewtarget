#!/usr/bin/env python3
#-----------------------------------------------------------------------------------------------------------------------
# scripts/takeScreenshots.py is part of Brewtarget, and is copyright the following authors 2026:
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
# Regenerates the screenshots for the user manual (in docs/manual/en/assets/screenshots) from a local build.
#
# Usage, from the top of the repository, after building with Meson:
#
#    scripts/takeScreenshots.py [build directory]
#
# The build directory defaults to mbuild.  The script does a staged install (`meson install --destdir`) of that build
# into a temporary directory, because the program finds its data files relative to where it is installed, and runs it
# from there with:
#    - a fresh, temporary user directory, so that it starts with just the default content, and your own data is never
#      touched;
#    - Qt's "offscreen" platform, so that no windows appear (and it works without a display, eg over ssh);
#    - double-size rendering, so the images are sharp in the PDF and on high-resolution screens;
#    - a fixed locale, so the numbers and dates come out the same wherever it is run.
#
# See src/ScreenshotTaker.cpp for what is in each screenshot.
#
import os
import subprocess
import sys
import tempfile
from pathlib import Path

def main() -> int:
   repo_dir = Path(__file__).resolve().parent.parent
   output_dir = repo_dir / "docs/manual/en/assets/screenshots"

   build_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else repo_dir / "mbuild"

   if not (build_dir / "build.ninja").is_file():
      print(
         f"{build_dir} is not a Meson build directory -- "
         "build with Meson first, or give the build directory as an argument",
         file=sys.stderr,
      )
      return 1

   with tempfile.TemporaryDirectory() as temp_dir:
      temp_dir = Path(temp_dir)
      stage_dir = temp_dir / "stage"
      user_dir = temp_dir / "userDir"
      user_dir.mkdir()

      print(f"Installing {build_dir} into {stage_dir}")
      subprocess.run(
         [
            "meson",
            "install",
            "-C", str(build_dir),
            "--destdir", str(stage_dir),
            "--quiet",
         ],
         check=True,
      )

      executable = next(
         (
            path
            for path in stage_dir.rglob("*")
            if path.is_file()
               and path.parent.name == "bin"
               and path.name in ("brewken", "brewtarget")
         ),
         None,
      )

      if executable is None:
         print(f"Cannot find the program in {stage_dir}", file=sys.stderr)
         return 1

      print(f"Running {executable}")

      env = os.environ.copy()
      env.update({
         "QT_QPA_PLATFORM": "offscreen",
         "QT_SCALE_FACTOR": "2",
         "LC_ALL": "C.UTF-8",
         "LANG": "C.UTF-8",
      })

      subprocess.run(
         [
            str(executable),
            "--user-dir", str(user_dir),
            "--screenshots", str(output_dir),
         ],
         env=env,
         check=True,
      )

   print(f"Screenshots are in {output_dir}")
   return 0

if __name__ == "__main__":
   sys.exit(main())