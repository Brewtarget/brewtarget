/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtDatePopup.h is part of Brewtarget, and is copyright the following authors 2009-2021:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef BTDATEPOPUP_H
#define BTDATEPOPUP_H
#pragma once

#include <QCalendarWidget>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

/*!
 * \class BtDatePopup
 *
 * \brief Gives us a calendar popup so that we can redate a brewnote. A lot of
 * setup needs to be done to make this behave, so it is easier to do it as a
 * separate class.
 *
 * This is largely taken from jordenysp's example on stackoverflow
 * http://stackoverflow.com/questions/1352334/qcalendarwidget-as-pop-up-not-as-new-window
 */
class BtDatePopup : public QDialog {
   Q_OBJECT

public:
      BtDatePopup(QWidget* parent=0);
      QDate selectedDate() const;

private:
      QWidget* widget;
      QCalendarWidget* calendar;
      QDialogButtonBox* buttonbox;
      QVBoxLayout* vertical;
};

#endif
