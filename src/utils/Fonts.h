/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/Fonts.h is part of Brewtarget, and is copyright the following authors 2023:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef UTILS_FONTS_H
#define UTILS_FONTS_H
#pragma once

#include <QString>

#include "utils/NoCopy.h"

/**
 * \class Fonts  Holds info about open source fonts that we ship with the application.
 *
 *        This is a bit of a nothing singleton class, but it's a mechanism for having constant values
 *        (\c Fonts::TexGyreSchola_Regular etc) that are initialised after the program starts running.
 *
 *        We we cannot rely on automatic initialisation of the font family name variables, because there isn't an easy
 *        way for the compiler to know that it would need to initialise the Qt resource system before initialising
 *        \c Fonts::TexGyreSchola_Regular etc.
 *
 *        We rely on the constructor being called after \c main is invoked (the first time \c getInstance is called) so
 *        that we can read in the font resources at a time they are definitely available.
 *
 *        It doesn't matter how early in the program execution you call \c Fonts::getInstance because, per
 *        https://doc.qt.io/qt-6/resources.html, "Resources embedded in C++ executable or library code are
 *        automatically registered to the Qt resource system in a constructor of an internal global variable.  Since
 *        the global variables are initialized before main() runs, the resources are available when the program
 *        starts to run."
 */
class Fonts {
public:
   static Fonts const & getInstance();

   /**
    * \brief Names of the font families we ship.  We must call \c Fonts::init() early in the application to initialise
    *        these values (which are obtained from the font files themselves after they are loaded in).
    *
    *        In a perfect world, these would be \c const but, because they have to be initialised at
    */
   //! @{
   QString const TexGyreSchola_BoldItalic;
   QString const TexGyreSchola_Bold      ;
   QString const TexGyreSchola_Italic    ;
   QString const TexGyreSchola_Regular   ;
   //! @}

private:
   //! Hidden constructor.
   Fonts();
   //! Destructor hidden.
   ~Fonts();

   // Insert all the usual boilerplate to prevent copy/assignment/move
   NO_COPY_DECLARATIONS(Fonts)
};

#endif
