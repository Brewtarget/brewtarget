/*
 * widgets/SmartDigitWidget.h is part of Brewtarget, and is Copyright the following authors 2009-2023:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef WIDGETS_BTDIGITWIDGET_H
#define WIDGETS_BTDIGITWIDGET_H
#pragma once

#include <memory> // For PImpl

#include <QLabel>
#include <QString>
#include <QWidget>

#include "BtFieldType.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "SmartField.h"

/*!
 * \class SmartDigitWidget
 *
 * \brief Widget that displays colored numbers, depending on if the number is ok, high, or low.  Currently only used in
 *        waterDialog.ui (ie Water Chemistry Dialog).
 *
 * \todo Make this thing directly accept signals from the model items it is supposed to watch.
 *
 *        NB: Per https://doc.qt.io/qt-5/moc.html#multiple-inheritance-requires-qobject-to-be-first, "If you are using
 *        multiple inheritance, moc [Qt's Meta-Object Compiler] assumes that the first inherited class is a subclass of
 *        QObject. Also, be sure that only the first inherited class is a QObject."  In particular, this means we must
 *        put Q_PROPERTY declarations for SmartField attributes here rather than in SmartField itself.
 */
class SmartDigitWidget : public QLabel, public SmartField {
   Q_OBJECT

public:
   enum ColorType{NONE, LOW, GOOD, HIGH, BLACK};

   SmartDigitWidget(QWidget * parent);
   virtual ~SmartDigitWidget();

   virtual QString getRawText() const;
   virtual void setRawText(QString const & text);
   virtual void connectSmartLabelSignal(SmartLabel & smartLabel);
   virtual void doPostInitWork();

   //! \brief Displays the given \c num with precision \c prec.
   void display(double num, int prec = 0);

   //! \brief Display a QString.
   void display(QString str);

   //! \brief Set the lower limit of the "good" range.
   void setLowLim(double num);

   //! \brief Set the upper limit of the "good" range.
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

   //! \brief the array needs to be low, good, high
   void setMessages(QStringList msgs);

   void setText(QString amount, int precision = 2);
   void setText(double  amount, int precision = 2);

   /**
    * \brief Use this when you want to get the text as a number (and ignore any units or other trailling letters or
    *        symbols)
    */
   template<typename T> T getValueAs() const;

public slots:
   /**
    * \brief Received from \c SmartLabel when the user has change \c UnitSystem
    *
    * This is mostly referenced in .ui files.  (NB this means that the signal connections are only checked at run-time.)
    */
   void displayChanged(SmartAmounts::ScaleInfo previousScaleInfo);

protected:
   int getPrecision() const;

   BtFieldType fieldType;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

//
// See comment in widgets/BtAmountDigitWidget.h for why we need these trivial child classes to use in .ui files
//
class BtGenericDigit : public SmartDigitWidget { Q_OBJECT public: BtGenericDigit(QWidget * parent); };

#endif
