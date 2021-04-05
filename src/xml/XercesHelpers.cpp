/*
 * xml/XercesHelpers.cpp is part of Brewtarget, and is Copyright the following
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

#include "xml/XercesHelpers.h"


#include <QString>
#include <QTextStream>

#include <xercesc/dom/DOMException.hpp>

#include "xml/XQString.h"


QString XercesHelpers::getParameterSettings(xercesc::DOMConfiguration & domConfiguration) {
   QString settings;
   QTextStream settingsAsStream(&settings);
   xercesc::DOMStringList const * parameterNames = domConfiguration.getParameterNames();
   if (parameterNames) {
      int const numberOfParameters = parameterNames->getLength();
      settingsAsStream << numberOfParameters << " parameters:\n";
      for (int ii = 0; ii < numberOfParameters; ++ii) {
         if (ii != 0) {
            settingsAsStream << ";\n";
         }
         XMLCh const * currentParameter = parameterNames->item(ii);
         if (currentParameter) {
            settingsAsStream << "   #" << ii << ": " << XQString(currentParameter) << " = ";
            try {
               void const * parameterValue = domConfiguration.getParameter(currentParameter);
               // Depending on the parameter, its value is either a boolean (0 or 1) or a real pointer to something
               if (0 == parameterValue) {
                  // We can't infer just from the value whether this is a pointer or a boolean
                  settingsAsStream << "unset (false)";
               } else if (reinterpret_cast<void const *>(1) == parameterValue) {
                  // Must be a boolean
                  settingsAsStream << "set (true)";
               } else {
                  // It's a pointer to something, but it's beyond the scope of this function to know what
                  settingsAsStream << "set (to " << parameterValue << ")";
               }
            } catch (const xercesc::DOMException & de) {
               // Yes, you really can generate an exception just by trying to read a config parameter. Sigh.
               settingsAsStream << "Unreadable! (xerces::DOMException #" << de.code << ": " << XQString(de.getMessage()) << ")";
            }

         } else {
            settingsAsStream << "(Parameter " << ii << " not set!)";
         }
      }
   } else {
      settingsAsStream << "None!";
   }

   settingsAsStream.flush();
   return settings;
}
