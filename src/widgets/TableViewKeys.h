/*======================================================================================================================
 * widgets/TableViewKeys.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef WIDGETS_TABLEVIEWKEYS_H
#define WIDGETS_TABLEVIEWKEYS_H
#pragma once

#include <functional>

class QAbstractItemView;

namespace TableViewKeys {
   /**
    * \brief Make the Delete key (and Backspace, which is what Mac keyboards label "delete") call \c onDelete when
    *        \c view has the keyboard focus.
    *
    *        This takes precedence over any window-wide shortcut for the same key.  In particular, \c MainWindow's
    *        "Delete selected" action (\c actionDeleteSelected) acts on whichever tree view was last used, which is not
    *        what the user expects if they have just selected, say, a hop addition in the recipe's table of hops.
    *
    *        Note that, if the user is editing a cell in the table, the key goes to the cell editor (eg to delete a
    *        character), not to \c view, so this does not interfere with normal editing.
    */
   void handleDeleteKey(QAbstractItemView & view, std::function<void()> onDelete);
}

#endif
