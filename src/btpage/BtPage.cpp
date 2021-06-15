/*
 * BtPage.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Mattias Måhl <mattias@kejsarsten.com>
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

#include "BtPage.h"
namespace nBtPage
{

   BtPage::BtPage(QPrinter *printer) :
      painter(printer)
   {
      this->printer = printer;
   }

   void BtPage::renderPage()
   {
      foreach (PageChildObject *child, _children)
      {
         child->render(&painter);
         if ( child->needPageBrake ) {
            //We're obviously dealing with a large text, let's create a new page and continue rendering the text.
            printer->newPage();
            child->nextSection->render( &painter );
         }
      }
      painter.end();
   }
}