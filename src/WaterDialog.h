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
class SaltTableModel;

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

protected:
   void dropEvent(QDropEvent* dpEvent);
   void dragEnterEvent(QDragEnterEvent *deEvent);
   QString acceptMime;

protected slots:
   void newTotals();

signals:
   void droppedSalt(Salt* drop);
   void droppedSalts(QList<Salt*> drops);

private:

   WaterListModel *baseListModel;
   WaterListModel *targetListModel;
   SaltTableModel *saltTableModel;
   Recipe* recObs;
   Water *base, *target;

   WaterSortFilterProxyModel *baseFilter;
   WaterSortFilterProxyModel *targetFilter;

   void setSlider(RangedSlider* slider, double data);
   void addSalts(QList<Misc*>dropped);

};

#endif
