/*======================================================================================================================
 * ScreenshotTaker.h is part of Brewtarget, and is copyright the following authors 2026:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#ifndef SCREENSHOTTAKER_H
#define SCREENSHOTTAKER_H
#pragma once

#include <QString>

class MainWindow;

/**
 * \brief Generates the screenshots for the user manual (docs/manual).
 *
 *        This is run when the program is started with the --screenshots option (see main.cpp), which in turn is
 *        normally done via docs/manual/screenshots.sh.  The program then starts with a fresh database containing just
 *        the default content, and this code builds the example recipe from Chapter 1 of the manual step by step, in
 *        the same way as a user would, taking a screenshot at each point where the manual has one.
 *
 *        The recipe is built from the default content (rather than, say, imported from a file) so that the steps here
 *        match the steps in the manual, including the use of Auto-Adjust for the mash and the water.  If an item it
 *        needs is renamed or removed from the default content, the screenshots fail with a message saying which.
 *
 *        Each screenshot is saved as a PNG file named to match the image macro in the manual (eg mashEditor.png).
 *
 *        To add a screenshot, add a step to \c ScreenshotTaker::takeAll and an image macro to the manual.
 */
namespace ScreenshotTaker {
   /**
    * \brief Build the example recipe and take all the screenshots
    *
    * \param mainWindow The (already visible) main window
    * \param directory Where to save the screenshots.  Created if it does not exist.
    *
    * \return \c true if all the screenshots were taken, \c false otherwise (with the reason logged)
    */
   bool takeAll(MainWindow & mainWindow, QString const & directory);
}

#endif
