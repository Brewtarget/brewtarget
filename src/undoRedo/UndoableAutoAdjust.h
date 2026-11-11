/*======================================================================================================================
 * undoRedo/UndoableAutoAdjust.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef UNDOREDO_UNDOABLEAUTOADJUST_H
#define UNDOREDO_UNDOABLEAUTOADJUST_H
#pragma once

#include <functional>

#include <QString>
#include <QUndoCommand>

/**
 * \brief Groups together all the changes made by one click of an "Auto-Adjust" button (see \c MashAutoAdjuster and
 *        \c WaterAutoAdjuster) -- and, where the user has just turned on "Keep in sync with recipe", the change to
 *        that flag too -- so that they can be undone and redone in one go.  The individual changes are child commands
 *        (ie are constructed with this object as their parent).
 *
 *        The twist is how this interacts with "keep in sync".  Normally, undo restores everything to exactly how it
 *        was before.  But, if keep in sync is on when the user undoes an auto-adjust, then the old values would just
 *        get overwritten again the next time anything in the recipe changed, which is not what the user would expect.
 *        (The user is saying "I want the values from before the auto-adjust".)  So, in this case, we bend the normal
 *        rules and also turn keep in sync off.  Redo turns it back on again.
 *
 *        Note that there are two cases here:
 *          - Keep in sync was already on before the auto-adjust (ie the user clicked the Auto-Adjust button anyway, eg
 *            because they changed the sparge method in the mash editor).  Undo restores the previous values and turns
 *            keep in sync off.
 *          - The auto-adjust happened because the user turned on keep in sync.  In this case, the change to the flag is
 *            one of the child commands, so undoing that child turns keep in sync off and there is nothing extra for us
 *            to do.
 *
 *        Automatic adjustments that happen because keep in sync is on (see \c Recipe::scheduleMashAutoAdjust and
 *        \c Recipe::scheduleWaterAutoAdjust) do NOT go on the undo stack.  If they did, then, each time the user undid
 *        something (eg removing a grain), the automatic re-adjustment that followed would wipe out the redo stack.
 *        Instead, because an automatic adjustment depends only on the recipe, undoing the change that triggered it
 *        (eg adding the grain) triggers another automatic adjustment that puts things back as they were.
 */
class UndoableAutoAdjust : public QUndoCommand {
public:
   /**
    * \param text             Description of the change, as shown in the Edit menu (eg "Undo Auto-adjust mash")
    * \param isKeptInSync     Returns whether "keep in sync" is currently on for whatever is being adjusted
    * \param setKeptInSync    Turns "keep in sync" on or off for whatever is being adjusted
    * \param parent
    */
   UndoableAutoAdjust(QString const & text,
                      std::function<bool()> isKeptInSync,
                      std::function<void(bool)> setKeptInSync,
                      QUndoCommand * parent = nullptr);
   ~UndoableAutoAdjust();

   void redo() override;
   void undo() override;

   //! \return \c true if the last undo turned off keep in sync (so that the next redo will turn it back on)
   bool undoTurnedOffKeepInSync() const;

private:
   std::function<bool()>     m_isKeptInSync;
   std::function<void(bool)> m_setKeptInSync;
   bool                      m_undoTurnedOffKeepInSync = false;
};

#endif
