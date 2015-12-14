/*
 * TimerListDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Aidan Roberts <aidanr67@gmail.com>
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

#include "TimerListDialog.h"
#include <QScrollBar>

TimerListDialog::TimerListDialog(QWidget* parent, QList<TimerWidget*>* timers) : QDialog(parent)
{
    this->setWindowTitle(tr("Addition Timers"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    this->setLayout(mainLayout);

    scrollArea = new QScrollArea(this);
    mainLayout->addWidget(scrollArea);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollWidget = new QWidget(scrollArea);
    layout = new QVBoxLayout(scrollWidget);  
    scrollWidget->setLayout(layout);
    scrollArea->setWidget(scrollWidget);
    setTimers(timers);
}

TimerListDialog::~TimerListDialog()
{

}

void TimerListDialog::setTimers(QList<TimerWidget *>* timers)
{
    if (!timers->isEmpty()) {
        foreach (TimerWidget* t, *timers) {
            layout->addWidget(t);
        }
    }
}

void TimerListDialog::setTimerVisible(TimerWidget *t)
{
    //Focus scrollArea on timer t
    scrollArea->verticalScrollBar()->setValue(t->y());
}

void TimerListDialog::hideTimers()
{
    this->hide();
}
