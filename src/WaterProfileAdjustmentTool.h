/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * WaterProfileAdjustmentTool.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef WATERPROFILEADJUSTMENTTOOL_H
#define WATERPROFILEADJUSTMENTTOOL_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>
#include <QWidget>

#include "ui_waterProfileAdjustmentTool.h"

#include "model/Salt.h"

/*!
 * \class WaterProfileAdjustmentTool
 *
 * \brief Helps you work out what salts etc to add to water of one profile to make it closer to another profile.
 *
 *        TODO: This class / tool still needs a bit more thinking through.  It's not currently as useful as it perhaps
 *              might be.  But we also need to decide properly how we are going to use \c RecipeUseOfWater.
 */
class WaterProfileAdjustmentTool : public QDialog, public Ui::waterProfileAdjustmentTool {
   Q_OBJECT

public:
   WaterProfileAdjustmentTool(QWidget * parent = nullptr);
   ~WaterProfileAdjustmentTool();

   void setRecipe(Recipe * rec);

public slots:
   void update_baseProfile(int selected);
   void update_targetProfile(int selected);
   void newTotals();
   void removeSalts();
   void setMashRO(int val);
   void setSpargeRO(int val);
   void saveAndClose();
   void clearAndClose();

signals:
   void newSalt(Salt * drop);
   void newSalts(QList<Salt *> drops);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
