/*======================================================================================================================
 * undoRedo/UndoableAutoAdjust.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "undoRedo/UndoableAutoAdjust.h"

#include <QDebug>

UndoableAutoAdjust::UndoableAutoAdjust(QString const & text,
                                       std::function<bool()> isKeptInSync,
                                       std::function<void(bool)> setKeptInSync,
                                       QUndoCommand * parent) :
   QUndoCommand{text, parent},
   m_isKeptInSync {std::move(isKeptInSync )},
   m_setKeptInSync{std::move(setKeptInSync)} {
   return;
}

UndoableAutoAdjust::~UndoableAutoAdjust() = default;

void UndoableAutoAdjust::redo() {
   // Redo the child commands in order
   this->QUndoCommand::redo();
   if (this->m_undoTurnedOffKeepInSync) {
      qDebug() << Q_FUNC_INFO << "Turning keep in sync back on for" << this->text();
      this->m_setKeptInSync(true);
      this->m_undoTurnedOffKeepInSync = false;
   }
   return;
}

void UndoableAutoAdjust::undo() {
   // Undo the child commands in reverse order
   this->QUndoCommand::undo();
   //
   // See class comment in header for why we do this.  NB: If one of the child commands was turning keep in sync on,
   // then undoing it will already have turned it off, so we won't do anything here.
   //
   if (this->m_isKeptInSync()) {
      qDebug() << Q_FUNC_INFO << "Turning keep in sync off for" << this->text();
      this->m_setKeptInSync(false);
      this->m_undoTurnedOffKeepInSync = true;
   }
   return;
}

bool UndoableAutoAdjust::undoTurnedOffKeepInSync() const {
   return this->m_undoTurnedOffKeepInSync;
}
