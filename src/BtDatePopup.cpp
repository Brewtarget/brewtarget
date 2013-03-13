/*
 * BtDatePopup.cpp is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#include <QtGui>
#include "BtDatePopup.h"

//! \brief set up the popup window.
BtDatePopup::BtDatePopup(QWidget* parent) : QDialog(parent, Qt::Popup)
{
   // No resizing a dialog
   setSizeGripEnabled(false);
   resize(260,230);

   widget = new QWidget(this);
   widget->setObjectName(QString("btDatePopup_widget"));
   widget->setGeometry(QRect(0,10,258,215));

   calendar = new QCalendarWidget(widget);
   calendar->setObjectName(QString("btDatePopup_calendar"));
   calendar->setNavigationBarVisible(true);


   buttonbox = new QDialogButtonBox(widget);
   buttonbox->setObjectName(QString("btDatePopup_buttonbox"));
   buttonbox->setOrientation(Qt::Horizontal);
   buttonbox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

   vertical = new QVBoxLayout(widget);
   vertical->setObjectName(QString("btDatePopup_verticalbox"));
   vertical->setContentsMargins(0,0,0,0);

   vertical->addWidget(calendar);
   vertical->addWidget(buttonbox);

   connect(buttonbox, SIGNAL(accepted()), this, SLOT(accept()));
   connect(buttonbox, SIGNAL(rejected()), this, SLOT(reject()));
}

QDateTime BtDatePopup::selectedDate() const
{
   return QDateTime(calendar->selectedDate());
}
