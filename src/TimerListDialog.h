/*
 * TimerListDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef TIMERLISTDIALOG_H
#define TIMERLISTDIALOG_H

class TimerListDialog;

#include <QDialog>
#include <QWidget>
#include "TimerWidget.h"
#include <QDebug>
#include "MainWindow.h"

/*!
 * \class TimerListDialog
 * \author Aidan Roberts
 *
 * \brief Dialog to hold addition timers
 */
class TimerListDialog : public QDialog
{
   Q_OBJECT
   
public:
      TimerListDialog(QWidget* parent, QList<TimerWidget*> * timers);
      ~TimerListDialog();
      void setTimerVisible(TimerWidget* t);

private slots:
      void hideTimers();

private:
      QScrollArea* scrollArea;
      QWidget* scrollWidget;
      QVBoxLayout* layout;

      void setTimers(QList<TimerWidget*>* timers);
};

#endif
