/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtTextEdit.cpp is part of Brewtarget, and is copyright the following authors 2009-2022:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Anderson <matt.anderson@is4s.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "BtTextEdit.h"

#include <QDebug>
#include <QSettings>

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BtTextEdit.cpp"
#endif

BtTextEdit::BtTextEdit([[maybe_unused]] QWidget * parent) : wasModified{false} {

   // We will see if this works...
   connect(this, &BtTextEdit::textChanged, this, &BtTextEdit::setTextChanged);
   return;
}

BtTextEdit::BtTextEdit(QString const & text,
                       [[maybe_unused]] QWidget * parent) : wasModified{false} {
   setPlainText(text);

   // We will see if this works...
   connect(this, &BtTextEdit::textChanged, this, &BtTextEdit::setTextChanged);
   return;
}

// I don't have faith in this. The concept is to call the super and then clear
// the modified flag. The intent is that this is only done via the code, not
// the user (e.g., loads and things)
void BtTextEdit::setPlainText(const QString & text) {
   QPlainTextEdit::setPlainText(text);
   wasModified = false;
   return;
}

void BtTextEdit::focusOutEvent([[maybe_unused]] QFocusEvent * e) {
   if (wasModified) {
      wasModified = false;
      emit textModified();
   }
   return;
}

bool BtTextEdit::isModified()  { return wasModified; }
void BtTextEdit::setTextChanged() { wasModified = true; }
