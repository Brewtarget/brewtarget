/*
 * SaltTableWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef _SALTTABLEWIDGET_H
#define _SALTTABLEWIDGET_H

class SaltTableWidget;

#include <QTableView>
#include <QWidget>
class SaltTableModel;

/*!
 * \class SaltTableWidget
 * \author Philip G. Lee
 *
 * \brief Completely redundant class. Remove and just use QTableView.
 */
class SaltTableWidget : public QTableView
{
   Q_OBJECT

public:
   SaltTableWidget(QWidget* parent=nullptr);
   SaltTableModel* getModel();

private:
   SaltTableModel* model;
};

#endif   /* _SALTTABLEWIDGET_H */

