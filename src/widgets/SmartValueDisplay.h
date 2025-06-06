/*======================================================================================================================
 * widgets/SmartValueDisplay.h is part of Brewtarget, and is copyright the following authors 2025:
 *   • Matt Young <mfsy@yahoo.com>
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
 =====================================================================================================================*/
#ifndef WIDGETS_SMARTVALUEDISPLAY_H
#define WIDGETS_SMARTVALUEDISPLAY_H
#pragma once

#include <QLabel>
#include <QString>
#include <QWidget>

#include "widgets/SmartField.h"

/*!
 * \class SmartValueDisplay
 *
 * \brief Widget that displays read-only values - typically calculated fields such as \c Mash::totalMashWater_l.
 *
 * \todo Make this thing directly accept signals from the model items it is supposed to watch.
 *
 *        NB: Per https://doc.qt.io/qt-5/moc.html#multiple-inheritance-requires-qobject-to-be-first, "If you are using
 *        multiple inheritance, moc [Qt's Meta-Object Compiler] assumes that the first inherited class is a subclass of
 *        QObject. Also, be sure that only the first inherited class is a QObject."  In particular, this means we must
 *        put Q_PROPERTY declarations for SmartField attributes here rather than in SmartField itself.
 */
class SmartValueDisplay : public QLabel, public SmartField {
   Q_OBJECT

public:

   SmartValueDisplay(QWidget * parent);
   virtual ~SmartValueDisplay();

   virtual QString getRawText() const override;
   virtual void setRawText(QString const & text) override;
   virtual void connectSmartLabelSignal(SmartLabel & smartLabel) override;
   virtual void doPostInitWork() override;

public slots:
   /**
    * \brief Received from \c SmartLabel when the user has change \c UnitSystem
    *
    * This is mostly referenced in .ui files.  (NB this means that the signal connections are only checked at run-time.)
    */
   void displayChanged(SmartAmounts::ScaleInfo previousScaleInfo);

};

#endif
