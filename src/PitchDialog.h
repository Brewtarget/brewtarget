/*
 * PitchDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2010.
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

#ifndef PITCHDIALOG_H
#define PITCHDIALOG_H

class PitchDialog;

#include <QDialog>
#include <QWidget>
#include "ui_pitchDialog.h"

class PitchDialog : public QDialog, public Ui::pitchDialog
{
   Q_OBJECT

public:
   PitchDialog(QWidget* parent=0);
   ~PitchDialog();

public slots:
   void calculate();
   void updateShownPitchRate(int percent);

private:

};

#endif // PITCHDIALOG_H
