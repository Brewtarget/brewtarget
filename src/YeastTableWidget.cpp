/*
 * YeastTableWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTableView>
#include <QWidget>
#include "YeastTableModel.h"
#include "YeastTableWidget.h"

YeastTableWidget::YeastTableWidget(QWidget* parent)
        : QTableView(parent)
{


   yfpm = new YeastSortFilterProxyModel(parent);
   model = new YeastTableModel(this);

   yfpm->setSourceModel(model);
   yfpm->setDynamicSortFilter(true);

   setModel(yfpm);
   setItemDelegate(new YeastItemDelegate(this));

}

YeastTableModel* YeastTableWidget::getModel()
{
   return model;
}

YeastSortFilterProxyModel* YeastTableWidget::getProxy()
{
    return yfpm;
}
