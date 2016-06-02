/*
 * MashStepTableWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include <QTableView>
#include <QWidget>
#include <QDebug>
#include "MashStepTableModel.h"
#include "MashStepTableWidget.h"

MashStepTableWidget::MashStepTableWidget(QWidget* parent)
        : QTableView(parent)
{
   model = new MashStepTableModel(this);
   setModel(model);
   setItemDelegate(new MashStepItemDelegate(this));
}

MashStepTableModel* MashStepTableWidget::getModel()
{
   return model;
}

void MashStepTableWidget::moveSelectedStepUp()
{
   QModelIndexList list = selectedIndexes();

   /* jazzbeerman 8/11/10 changed
   if( list.size() > 1 || list.size() < 0 )
      */
   if( list.size() != 1 )
      return;

   model->moveStepUp(list[0].row());
}

void MashStepTableWidget::moveSelectedStepDown()
{
   QModelIndexList list = selectedIndexes();

   /*jazzbeerman 8/11/10 changed
   if( list.size() > 1 || list.size() < 0 )
    */
   if (list.size() != 1 )
      return;

   model->moveStepDown(list[0].row());
}
