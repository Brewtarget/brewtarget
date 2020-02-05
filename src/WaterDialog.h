/*
 * WaterDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
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
#ifndef _WATERDIALOG_H
#define _WATERDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QButtonGroup>
#include "ui_waterDialog.h"
#include "unit.h"

class WaterListModel;
class WaterSortFilterProxyModel;
class WaterEditor;
class SaltTableModel;
class Salt;

/*!
 * \class WaterDialog
 * \author mik firestone
 *
 * \brief Trying my hand at making water chemistry work
 */
class WaterDialog : public QDialog, public Ui::waterDialog
{
   Q_OBJECT

public:
   WaterDialog(QWidget* parent = nullptr);
   void setRecipe(Recipe* rec);

   ~WaterDialog();

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
   void newSalt(Salt* drop);
   void newSalts(QList<Salt*> drops);

private:

   WaterListModel *baseListModel;
   WaterListModel *targetListModel;
   SaltTableModel *saltTableModel;
   WaterEditor* baseProfileEdit;
   WaterEditor* targetProfileEdit;
   Recipe* recObs;
   Water *base, *target;
   double m_mashRO;
   double m_spargeRO;
   double m_total_grains;
   double m_thickness;
   double m_weighted_colors;

   WaterSortFilterProxyModel *baseFilter;
   WaterSortFilterProxyModel *targetFilter;

   void setSlider(RangedSlider* slider, double data);
   void calculateGrainEquivalent();

   double calculateRA() const;
   double calculateGristpH();
   double calculateMashpH();
   double calculateSaltpH();
   double calculateAddedSaltpH();
   double calculateAcidpH();

};

#endif
