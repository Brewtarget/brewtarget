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

signals:

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
