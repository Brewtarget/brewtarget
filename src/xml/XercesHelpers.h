/*
 * xml/XercesHelpers.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _XML_XERCESHELPERS_H
#define _XML_XERCESHELPERS_H
#pragma once

#include <xercesc/dom/DOMConfiguration.hpp>

class QString;

namespace XercesHelpers {
   /**
    * \brief Extract the settings of the supplied DOMConfiguration into a string that we can log for debugging purposes.
    *        Typically you get back a lot of parameters (50) so they are broken out onto separate lines.
    */
   QString getParameterSettings(xercesc::DOMConfiguration & domConfiguration);
}

#endif
