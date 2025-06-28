/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartDigitWidget.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#ifndef WIDGETS_SMARTDIGITWIDGET_H
#define WIDGETS_SMARTDIGITWIDGET_H
#pragma once

#include <memory> // For PImpl

#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "widgets/SmartValueDisplay.h"

/*!
 * \class SmartDigitWidget
 *
 * \brief Widget that displays colored numbers, depending on if the number is ok, high, or low.  Currently only used in
 *        \c waterProfileAdjustmentTool.ui.
 *
 * \todo Make this thing directly accept signals from the model items it is supposed to watch.
 *
 *        NB: Per https://doc.qt.io/qt-5/moc.html#multiple-inheritance-requires-qobject-to-be-first, "If you are using
 *        multiple inheritance, moc [Qt's Meta-Object Compiler] assumes that the first inherited class is a subclass of
 *        QObject. Also, be sure that only the first inherited class is a QObject."  In particular, this means we must
 *        put Q_PROPERTY declarations for SmartField attributes here rather than in SmartField itself.
 */
class SmartDigitWidget : public SmartValueDisplay {
   Q_OBJECT

public:
   enum class ColorType{None, Low, Good, High, Black};

   SmartDigitWidget(QWidget * parent);
   virtual ~SmartDigitWidget();

   virtual void setRawText(QString const & text) override;

   /**
    * \brief Set the lower limit of the "good" range.  NB: If we are displaying a \c PhysicalQuantity then num must be
    *        in canonical units.
    */
   void setLowLim(double num);

   /**
    * \brief Set the upper limit of the "good" range.  NB: If we are displaying a \c PhysicalQuantity then num must be
    *        in canonical units.
    */
   void setHighLim(double num);

   /**
    * \brief Always use a constant color. Use a constantColor of NONE to unset
    */
   void setConstantColor(ColorType c);

   //! \brief Convenience method to set high and low limits in one call
   void setLimits(double low, double high);

   //! \brief Methods to set the low, good and high messages
   void setLowMsg(QString msg);
   void setGoodMsg(QString msg);
   void setHighMsg(QString msg);

   //! \brief Set all the messages
   void setMessages(QString lowMsg, QString goodMsg, QString highMsg);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
