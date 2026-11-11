/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MashEditor.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#ifndef EDITORS_MASHEDITOR_H
#define EDITORS_MASHEDITOR_H
#pragma once

#include <memory>

#include <QDialog>
#include <QMetaProperty>
#include <QShowEvent>
#include <QStringList>
#include <QVariant>
#include "ui_mashEditor.h"

#include "MashAutoAdjuster.h"
#include "editors/EditorBase.h"
#include "model/Mash.h"

#define MashEditorOptions EditorBaseOptions{ .recipe = true, .idDisplay = true, .numRecipesUsing = true }
/*!
 * \class MashEditor
 *
 * \brief View/controller dialog for editing a mash.
 */
class MashEditor : public QDialog,
                   public Ui::mashEditor,
                   public EditorBase<MashEditor, Mash, MashEditorOptions> {
   Q_OBJECT

   EDITOR_COMMON_DECL(Mash, MashEditorOptions)

public:
   //! Called by \c EditorBase after the edit item is set
   void postSetEditItem();

   /**
    * \brief Show the "Auto Adjust" part of the editor, with the focus on the Auto-Adjust button.  Used when the user
    *        asks to calculate the mash from the main window.
    */
   void focusAutoAdjust();

public slots:
   /**
    * \brief Use \c MashAutoAdjuster to work out the water amounts and temperatures (etc) for the mash being edited, for
    *        the current recipe.  If the mash is locked or shared with other recipes, we first offer to make a copy for
    *        the current recipe.
    *
    *        The changes are a single undoable step (see \c UndoableAutoAdjust).
    */
   void autoAdjust();

   //! Enable/disable the auto-adjust controls that depend on the selected sparge method
   void updateAutoAdjustControls();

   //! Show (or hide) the summary of any problems found when checking the steps (see \c MashStepTableModel::validate)
   //  and highlight any of the mash's own fields that have problems
   void updateMashProblems();

   /**
    * \brief Turn on or off "Keep in sync with recipe" (see \c Mash::keepInSync).  When turning it on, if the
    *        mash is locked or shared, we first offer to make a copy (as for \c autoAdjust), and we adjust the mash
    *        straight away.  Turning it on and the resulting adjustment are a single undoable step, so undo turns it
    *        off and puts the mash back how it was.  Turning it off is also undoable.
    */
   void keptInSyncToggled(bool const checked);

   //! Called when anything is done, undone or redone, as the "Keep in sync" check box might need updating
   void undoStackChanged();

protected:
   virtual void showEvent(QShowEvent * event) override;

private:
   /**
    * \return The recipe whose grain bill and equipment we use to auto-adjust the mash -- ie the current recipe, if it
    *         uses the mash being edited (or the mash is a new one for the recipe) -- or \c nullptr if there isn't one.
    */
   Recipe * recipeForAutoAdjust() const;

   //! Set up the auto-adjust controls for the current edit item and recipe
   void refreshAutoAdjust();

   /**
    * \brief Does the work for \c autoAdjust and \c keptInSyncToggled
    *
    * \param turnOnKeepInSync If \c true, the change to turn on keep in sync is part of the same undoable step
    */
   void doAutoAdjust(bool const turnOnKeepInSync);

   /**
    * \brief Replace values in \c inputs that come from the mash's own fields (eg initial grain temperature) with what
    *        the user has entered in the editor, which might not yet have been saved.
    */
   void useFieldValues(MashAutoAdjuster::Inputs & inputs) const;

   /**
    * \brief Before adjusting the mash for \c recipe, check that it can be changed and is not shared with another
    *        recipe.  If it is locked or shared, offer the user a copy for \c recipe.
    *
    * \return \c true if the mash being edited can now be adjusted, \c false if not (eg the user declined the copy)
    */
   bool ensureChangeableForRecipe(Recipe & recipe);

   //! Highlight \c field (with \c problems in its tool tip) or, if \c problems is empty, remove any highlighting
   static void highlightField(QWidget & field, QStringList const & problems);

   //! Show the result of an auto-adjust (or why it couldn't be done)
   void showAutoAdjustMessage(QString const & heading, QStringList const & details = {});

   //! Position on the undo stack just after our last auto-adjust, so we know when it's been undone, or -1 if none
   int m_autoAdjustUndoIndex = -1;

   //
   // TBD: We used to have the following function invoked by a button:
   //
   //    //! Get the tun mass and specific heat from the equipment.
   //    void fromEquipment();
   //
   // But, now that we are more explicitly allowing a Mash to be shared by more than one Recipe, this makes less sense.
   // We would either have to make the button only appear when the Mash is associated with exactly one Recipe, or we
   // would need to have some more complicated control to allow the user to select (either directly or indirectly) the
   // Equipment from which to copy the info.
   //
};

#endif