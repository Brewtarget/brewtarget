/*
 * FermentableTableWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _FERMENTABLETABLEWIDGET_H
#define	_FERMENTABLETABLEWIDGET_H

#include <QTableView>
#include <QWidget>
#include "FermentableTableModel.h"

class FermentableTableWidget;

class FermentableTableWidget : public QTableView
{
   Q_OBJECT
           
public:
   FermentableTableWidget(QWidget* parent=0);
   FermentableTableModel* getModel();
   
private:
   FermentableTableModel* model;
};

#endif	/* _FERMENTABLETABLEWIDGET_H */
