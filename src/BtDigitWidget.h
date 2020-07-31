/*
 * BtDigitWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef BTDIGITWIDGET_H
#define BTDIGITWIDGET_H

class BtDigitWidget;

#include <QLabel>
#include <QWidget>
#include <QString>
#include "unit.h"
#include "UnitSystem.h"
#include "UnitSystems.h"

/*!
 * \class BtDigitWidget
 * \author Philip G. Lee
 *
 * \brief Widget that displays colored numbers, depending on if the number is ok, high, or low.
 * \todo Make this thing directly accept signals from the model items it is supposed to watch.
 */
class BtDigitWidget : public QLabel
{
   Q_OBJECT
   Q_PROPERTY( int     type              READ type              WRITE setType              STORED false)
   Q_PROPERTY( QString configSection     READ configSection     WRITE setConfigSection     STORED false)
   Q_PROPERTY( QString editField         READ editField         WRITE setEditField         STORED false)
   Q_PROPERTY( QString forcedUnit        READ forcedUnit        WRITE setForcedUnit        STORED false)
   Q_PROPERTY( QString forcedScale       READ forcedScale       WRITE setForcedScale       STORED false)

public:
   enum ColorType{ NONE, LOW, GOOD, HIGH, BLACK };

   BtDigitWidget(QWidget* parent = 0, Unit::UnitType type = Unit::None, Unit* units = nullptr );

   //! \brief Displays the given \c num with precision \c prec.
   void display( double num, int prec = 0 );
   //! \brief Display a QString. 
   void display(QString str);

   //! \brief Set the lower limit of the "good" range.
   void setLowLim(double num);
   //! \brief Set the upper limit of the "good" range.
   void setHighLim(double num);
   //! \brief Always use a constant color. Use a constantColor of NONE to
   //!  unset
   void setConstantColor( ColorType c );
   //! \brief Convience method to set high and low limits in one call
   void setLimits(double low, double high);
   //! \brief Methods to set the low, good and high messages
   void setLowMsg(QString msg);
   void setGoodMsg(QString msg);
   void setHighMsg(QString msg);
   //! \brief the array needs to be low, good, high
   void setMessages(QStringList msgs);

   void setText( double amount, int precision = 2);
   void setText( QString amount, int precision = 2);

   // By defining the setters/getters, we can remove the need for
   // initializeProperties.
   QString editField() const;
   void setEditField( QString editField );

   QString configSection();
   void setConfigSection( QString configSection );

   int type() const;
   void setType(int type);

   QString forcedUnit() const;
   void setForcedUnit(QString forcedUnit);

   QString forcedScale() const;
   void setForcedScale(QString forcedScale);

   QString displayAmount( double amount, int precision = 2 );

public slots:
   void displayChanged(Unit::unitDisplay oldUnit, Unit::unitScale oldScale);

private:
   QString m_section, m_editField;
   Unit::UnitType m_type;
   Unit::unitDisplay m_forceUnit;
   Unit::unitScale m_forceScale;
   Unit* m_units;
   QWidget* m_parent;

   unsigned int m_rgblow;
   unsigned int m_rgbgood;
   unsigned int m_rgbhigh;
   double m_lowLim;
   double m_highLim;
   QString m_styleSheet;
   bool m_constantColor;
   ColorType m_color;
   double m_lastNum;
   int m_lastPrec;

   QString m_low_msg;
   QString m_good_msg;
   QString m_high_msg;

   void adjustColors();
};

class BtMassDigit: public BtDigitWidget
{
   Q_OBJECT

public:
   BtMassDigit(QWidget* parent);
};

class BtGenericDigit: public BtDigitWidget
{
   Q_OBJECT

public:
   BtGenericDigit(QWidget* parent);
};

#endif // BTDIGITWIDGET_H
