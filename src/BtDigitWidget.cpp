/*
 * BtDigitWidget.cpp is part of Brewtarget, and is Copyright the following
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

#include <QFrame>
#include <iostream>
#include <QLocale>
#include <QSettings>
#include <QDebug>

#include "BtDigitWidget.h"
#include "brewtarget.h"
#include "UnitSystems.h"
#include "UnitSystem.h"
#include "unit.h"

BtDigitWidget::BtDigitWidget(QWidget *parent, Unit::UnitType type, Unit* units) : QLabel(parent),
   m_type(type),
   m_forceUnit( Unit::noUnit ),
   m_forceScale( Unit::noScale ),
   m_units(units),
   m_parent(parent),
   m_rgblow(0x0000d0),
   m_rgbgood(0x008000),
   m_rgbhigh(0xd00000),
   m_lowLim(0.0),
   m_highLim(1.0),
   m_styleSheet(QString("QLabel { font-weight: bold; color: #%1 }")),
   m_constantColor(false),
   m_lastNum(1.5),
   m_lastPrec(3),
   m_low_msg(tr("Too low for style.")),
   m_good_msg(tr("In range for style.")),
   m_high_msg(tr("Too high for style."))
{
   setStyleSheet(m_styleSheet.arg(0,6,16,QChar('0')));
   setFrameStyle(QFrame::Box);
   setFrameShadow(QFrame::Sunken);

}

void BtDigitWidget::display(QString str)
{
   static bool converted;
  
   m_lastNum = Brewtarget::toDouble(str,&converted);
   m_lastPrec = str.length() - str.lastIndexOf(QLocale().decimalPoint()) - 1;
   if( converted )
      display(m_lastNum,m_lastPrec);
   else
   {
      Brewtarget::logW( 
            QString("%1 : could not convert %2 to double")
            .arg(Q_FUNC_INFO)
            .arg(str)
      );
      QLabel::setText("-");
   }
}

void BtDigitWidget::display(double num, int prec)
{
   QString str = QString("%L1").arg(num,0,'f',prec);
   QString style = m_styleSheet;

   m_lastNum = num;
   m_lastPrec = prec;

   if( (!m_constantColor && (num < m_lowLim)) || (m_constantColor && m_color == LOW))
   {
      style = m_styleSheet.arg(m_rgblow,6,16,QChar('0'));
      setToolTip(m_constantColor? "" : m_low_msg);
   }
   else if( (!m_constantColor && (num <= m_highLim)) || (m_constantColor && m_color == GOOD))
   {
      style = m_styleSheet.arg(m_rgbgood,6,16,QChar('0'));
      setToolTip(m_constantColor? "" : m_good_msg);
   }
   else
   {
      if( m_constantColor && m_color == BLACK )
         style = m_styleSheet.arg(0,6,16,QChar('0'));
      else
      {
         style = m_styleSheet.arg(m_rgbhigh,6,16,QChar('0'));
         setToolTip(m_high_msg);
      }
   }

   setStyleSheet(style);
   QLabel::setText(str);
}

void BtDigitWidget::adjustColors()
{

   QString str = displayAmount(m_lastNum, m_lastPrec);
   QString style = m_styleSheet;

   if( (!m_constantColor && (m_lastNum < m_lowLim)) || (m_constantColor && m_color == LOW))
   {
      style = m_styleSheet.arg(m_rgblow,6,16,QChar('0'));
      setToolTip(m_constantColor? "" : m_low_msg);
   }
   else if( (!m_constantColor && (m_lastNum <= m_highLim)) || (m_constantColor && m_color == GOOD))
   {
      style = m_styleSheet.arg(m_rgbgood,6,16,QChar('0'));
      setToolTip(m_constantColor? "" : m_good_msg);
   }
   else
   {
      if( m_constantColor && m_color == BLACK )
         style = m_styleSheet.arg(0,6,16,QChar('0'));
      else
      {
         style = m_styleSheet.arg(m_rgbhigh,6,16,QChar('0'));
         setToolTip(m_high_msg);
      }
   }

   setStyleSheet(style);
   QLabel::setText(str);
}

void BtDigitWidget::setLowLim(double num)
{
   if( num < m_highLim )
      m_lowLim = num;
   display(m_lastNum, m_lastPrec);
}

void BtDigitWidget::setHighLim(double num)
{
   if( num > m_lowLim )
      m_highLim = num;
   display(m_lastNum, m_lastPrec);
}

void BtDigitWidget::setConstantColor(ColorType c)
{
   m_constantColor = (c == LOW || c == GOOD || c == HIGH || c == BLACK );
   m_color = c;
   update(); // repaint.
}

void BtDigitWidget::setLimits(double low, double high)
{
   if( low <  high ) {
      m_lowLim = low;
      m_highLim = high;
   }
   adjustColors();
   update(); // repaint.
}

void BtDigitWidget::setLowMsg(  QString msg ) { m_low_msg  = msg; update();}
void BtDigitWidget::setGoodMsg( QString msg ) { m_good_msg = msg; update();}
void BtDigitWidget::setHighMsg( QString msg ) { m_high_msg = msg; update();}

void BtDigitWidget::setMessages( QStringList msgs )
{
   if ( msgs.size() != 3 ) {
      Brewtarget::logW("Wrong number of messages");
      return;
   }
   m_low_msg = msgs[0];
   m_good_msg = msgs[1];
   m_high_msg = msgs[2];

   adjustColors();
}


int BtDigitWidget::type() const { return (int)m_type; }
QString BtDigitWidget::editField() const { return m_editField; }
QString BtDigitWidget::configSection()
{
   if ( m_section.isEmpty() ) {
      setConfigSection("");
   }

   return m_section;
}

// Once we require >qt5.5, we can replace this noise with
// QMetaEnum::fromType()
QString BtDigitWidget::forcedUnit() const
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitDisplay");
   QMetaEnum unitEnum = mo.enumerator(index);

   return QString( unitEnum.valueToKey(m_forceUnit) );
}

QString BtDigitWidget::forcedScale() const
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitScale");
   QMetaEnum scaleEnum = mo.enumerator(index);

   return QString( scaleEnum.valueToKey(m_forceScale) );
}

void BtDigitWidget::setType(int type) { m_type = (Unit::UnitType)type;}
void BtDigitWidget::setEditField( QString editField) { m_editField = editField; }

// The cascade looks a little odd, but it is intentional.
void BtDigitWidget::setConfigSection(QString configSection)
{
   m_section = configSection;

   if ( m_section.isEmpty() )
      m_section = m_parent->property("configSection").toString();

   if ( m_section.isEmpty() )
      m_section = m_parent->objectName();
}

// previous comment about qt5.5 applies
void BtDigitWidget::setForcedUnit( QString forcedUnit )
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitDisplay");
   QMetaEnum unitEnum = mo.enumerator(index);

   m_forceUnit = (Unit::unitDisplay)unitEnum.keyToValue(forcedUnit.toStdString().c_str());
}

void BtDigitWidget::setForcedScale( QString forcedScale )
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitScale");
   QMetaEnum unitEnum = mo.enumerator(index);

   m_forceScale = (Unit::unitScale)unitEnum.keyToValue(forcedScale.toStdString().c_str());
}

void BtDigitWidget::displayChanged(Unit::unitDisplay oldUnit, Unit::unitScale oldScale)
{
   // This is where it gets hard
   double val = -1.0;
   QString amt;
   bool ok = false;

   if (text().isEmpty()) {
      return;
   }

   // The idea here is we need to first translate the field into a known
   // amount (aka to SI) and then into the unit we want.
   switch( m_type ) {
      case Unit::Mass:
         amt = displayAmount(m_lastNum,2);
         break;
      case Unit::String:
         amt = text();
         break;
      case Unit::None:
      default:
         val = Brewtarget::toDouble(text(),&ok);
         if ( ! ok )
            Brewtarget::logW( QString("%1: failed to convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(text()).arg(m_section).arg(m_editField) );
         amt = displayAmount(val);
   }
   QLabel::setText(amt);
}

QString BtDigitWidget::displayAmount(double amount, int precision)
{
   Unit::unitDisplay unitDsp;
   Unit::unitScale scale;

   unitDsp  = static_cast<Unit::unitDisplay>(Brewtarget::option(m_editField, Unit::noUnit, m_section, Brewtarget::UNIT).toInt());
   scale    = static_cast<Unit::unitScale>(Brewtarget::option(m_editField, Unit::noScale, m_section, Brewtarget::SCALE).toInt());

   // I find this a nice level of abstraction. This lets all of the setText()
   // methods make a single call w/o having to do the logic for finding the
   // unit and scale.
   return Brewtarget::displayAmount(amount, m_units, precision, unitDsp, scale);
}

void BtDigitWidget::setText(QString amount, int precision)
{
   double amt;
   bool ok = false;


   setConfigSection("");
   if ( m_type == Unit::String )
      QLabel::setText(amount);
   else
   {
      amt = Brewtarget::toDouble(amount,&ok);
      if ( !ok ) {
         Brewtarget::logW( QString("%1 could not convert %2 (%3:%4) to double")
               .arg(Q_FUNC_INFO)
               .arg(amount)
               .arg(m_section)
               .arg(m_editField) );
      }
      m_lastNum = amt;
      m_lastPrec = precision;
      QLabel::setText(displayAmount(amt, precision));
   }
}
void BtDigitWidget::setText(double amount, int precision)
{
   m_lastNum = amount;
   m_lastPrec = precision;
   setConfigSection("");
   QLabel::setText( displayAmount(amount,precision) );
}

BtMassDigit::BtMassDigit(QWidget* parent) : BtDigitWidget(parent,Unit::Mass,Units::kilograms) {}
BtGenericDigit::BtGenericDigit(QWidget* parent): BtDigitWidget(parent,Unit::None,nullptr) {}
