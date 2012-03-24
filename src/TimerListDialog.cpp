/*
* TimerListDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "TimerListDialog.h"
#include "TimerWidget.h"

TimerListDialog::TimerListDialog(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   
   timer1 = new TimerWidget(this);
   timer2 = new TimerWidget(this);
   timer3 = new TimerWidget(this);
   
   verticalLayout->addWidget(timer1);
   verticalLayout->addWidget(timer2);
   verticalLayout->addWidget(timer3);
}

TimerListDialog::~TimerListDialog()
{
   timer1->deleteLater();
   timer2->deleteLater();
   timer3->deleteLater();
}
