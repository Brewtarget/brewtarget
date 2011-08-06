/*
 * MiscTableWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include "MiscTableWidget.h"
#include "MiscTableModel.h"
#include "MiscSortFilterProxyModel.h"

MiscTableWidget::MiscTableWidget(QWidget *parent)
        : QTableView(parent)
{
   proxy = new MiscSortFilterProxyModel(this);
   model = new MiscTableModel(this);

   proxy->setSourceModel(model);
   proxy->setDynamicSortFilter(true);

   setModel(proxy);
   sortByColumn(MISCNAMECOL, Qt::AscendingOrder);
   setItemDelegate(new MiscItemDelegate());
}

MiscTableModel* MiscTableWidget::getModel()
{
   return model;
}

MiscSortFilterProxyModel* MiscTableWidget::getProxy()
{
   return proxy;
}
