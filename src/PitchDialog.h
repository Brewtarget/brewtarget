/*
 * PitchDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
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

#ifndef PITCHDIALOG_H
#define PITCHDIALOG_H

class PitchDialog;

#include <QDialog>
#include <QWidget>
#include "ui_pitchDialog.h"

/*!
 * \class PitchDialog
 * \author Philip G. Lee
 *
 * \brief Dialog to calculate how much yeast to pitch.
 */
class PitchDialog : public QDialog, public Ui::pitchDialog
{
   Q_OBJECT

public:
   PitchDialog(QWidget* parent=0);
   ~PitchDialog();

   //! \brief Set the wort volume in liters.
   void setWortVolume_l(double volume);
   //! \brief Set the wort gravity in 20C/20C SG.
   void setWortDensity(double sg);
   
public slots:
   void calculate();
   void updateShownPitchRate(int percent);
   void toggleViabilityFromDate(int state);
   void updateViabilityFromDate(QDate date);

   void updateProductionDate(Unit::unitDisplay dsp, Unit::unitScale scl);

private:

};

#endif // PITCHDIALOG_H
