/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtDatePopup.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Mitch Lillie <mitch@mitchlillie.com>
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

#include <QtGui>
#include "BtDatePopup.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BtDatePopup.cpp"
#endif

//! \brief set up the popup window.
BtDatePopup::BtDatePopup(QWidget * parent) :
   QDialog(parent, Qt::Popup) {
   // No resizing a dialog
   this->setSizeGripEnabled(false);
   this->resize(260, 230);

   this->m_widget = new QWidget(this);
   this->m_widget->setObjectName(QString("btDatePopup_widget"));
   this->m_widget->setGeometry(QRect(0,10,258,215));

   this->m_calendar = new QCalendarWidget(this->m_widget);
   this->m_calendar->setObjectName(QString("btDatePopup_calendar"));
   this->m_calendar->setNavigationBarVisible(true);
   this->m_calendar->setSelectedDate(QDate::currentDate());


   this->m_buttonbox = new QDialogButtonBox(this->m_widget);
   this->m_buttonbox->setObjectName(QString("btDatePopup_buttonbox"));
   this->m_buttonbox->setOrientation(Qt::Horizontal);
   this->m_buttonbox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

   this->m_vertical = new QVBoxLayout(this->m_widget);
   this->m_vertical->setObjectName(QString("btDatePopup_verticalbox"));
   this->m_vertical->setContentsMargins(0,0,0,0);

   this->m_vertical->addWidget(this->m_calendar);
   this->m_vertical->addWidget(this->m_buttonbox);

   connect(this->m_buttonbox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(this->m_buttonbox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   return;
}

QDate BtDatePopup::selectedDate() const {
   return this->m_calendar->selectedDate();
}
