/*======================================================================================================================
 * widgets/WaterAdjuster.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
 =====================================================================================================================*/
#ifndef WIDGETS_WATERADJUSTER_H
#define WIDGETS_WATERADJUSTER_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>
#include <QWidget>

#include "ui_waterAdjuster.h"

/*!
 * \class WaterAdjuster
 *
 * \brief Helps you work out what water adjustments etc to add to water of one profile to make it closer to another profile.
 */
class WaterAdjuster : public QWidget, public Ui::waterAdjuster {
   Q_OBJECT

public:
   explicit WaterAdjuster(QWidget * parent = nullptr);
   ~WaterAdjuster() override;

   /**
    * Certain parts of initialisation need to wait until \c MainWindow is constructed, so \c MainWindow calls this
    * function when it is ready.
    */
   void init();

   void setRecipe(Recipe * rec);

public slots:
   void updateBaseProfile(int selected) const;
   void updateTargetProfile(int selected) const;
   void newTotals();
   void removeSelectedWaterAdjustments();
   void removeSelectedAcidMalts();
   void updateRoWaterMash_pct(int val);
   void updateRoWaterSparge_pct(int val);

   /**
    * \brief Work out the amounts of brewing salts needed to get as close as possible to the target water profile (and,
    *        if a target mash pH is set, the amount of acid needed to get it) and update the recipe accordingly.  This is
    *        the water equivalent of \c MashEditor::autoAdjust.  See \c WaterAutoAdjuster::prepare for which water
    *        agents are used and \c WaterAutoAdjuster for how the amounts are calculated.
    *
    *        The changes are a single undoable step (see \c UndoableAutoAdjust).
    */
   void autoAdjust();

   /**
    * \brief Turn on or off "Keep in sync with recipe" (see \c Recipe::waterKeptInSync).  Turning it on also adjusts the
    *        water agents straight away, and the two together are a single undoable step, so undo turns it off and puts
    *        the water agents back how they were.  Turning it off is also undoable.
    */
   void keptInSyncToggled(bool const checked);

   //! Called when the user ticks or unticks "Adjust acid for mash pH of", or changes the target pH
   void targetMashPhChanged();

   //! Called when anything is done, undone or redone, as the auto-adjust controls might need updating
   void refreshAutoAdjustControls();

   //! Called when the recipe changes, eg its grain bill or mash, as the predicted water and pH might need updating
   void recipeChanged(QMetaProperty prop, QVariant val);

signals:

private:
   /**
    * \brief Does the work for \c autoAdjust and \c keptInSyncToggled
    *
    * \param turnOnKeepInSync If \c true, the change to turn on keep in sync is part of the same undoable step
    */
   void doAutoAdjust(bool const turnOnKeepInSync);

   /**
    * \brief Show the result of an auto-adjust: \c heading goes under the Auto-Adjust button, and \c details (eg ions
    *        we couldn't get close to the target) go in the "Auto-Adjust Notes" tab.  Empty \c heading and \c details
    *        clears both.
    */
   void showAutoAdjustMessage(QString const & heading, QStringList const & details = {});

   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif