/*
 * BtLineEdit.cpp is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#include "BtLineEdit.h"
#include "brewtarget.h"
#include "BeerXMLElement.h"
#include "UnitSystems.h"
#include "UnitSystem.h"
#include "unit.h"
#include "Algorithms.h"
#include <QSettings>
#include <QDebug>

BtLineEdit::BtLineEdit(QWidget *parent, Unit::UnitType type) :
   QLineEdit(parent),
   btParent(parent),
   _type(type),
   _forceUnit(Unit::noUnit),
   _forceScale(Unit::noScale)
{
   _section = property("configSection").toString();
   /*
   btParent = parent;
   _type = type;
   _forceUnit = Unit::noUnit;
   _forceScale = Unit::noScale;
   */
    
   connect(this,&QLineEdit::editingFinished,this,&BtLineEdit::onLineChanged);
}

void BtLineEdit::onLineChanged()
{
   lineChanged(Unit::noUnit,Unit::noScale);
}

void BtLineEdit::lineChanged(Unit::unitDisplay oldUnit, Unit::unitScale oldScale)
{
   // This is where it gets hard
   double val = -1.0;
   QString amt;
   bool force = Brewtarget::hasUnits(text());
   bool ok = false;
   bool wasChanged = sender() == this;

   // editingFinished happens on focus being lost, regardless of anything
   // being changed. I am hoping this short circuits properly and we do
   // nothing if nothing changed.
   if ( sender() == this && ! isModified() )
   {
      return;
   }

   if (text().isEmpty())
   {
      return;
   }

   // The idea here is we need to first translate the field into a known
   // amount (aka to SI) and then into the unit we want.
   switch( _type )
   {
      case Unit::Mass:
      case Unit::Volume:
      case Unit::Temp:
      case Unit::Time:
      case Unit::Density:
         val = toSI(oldUnit,oldScale,force);
         amt = displayAmount(val,3);
         break;
      case Unit::Color:
         val = toSI(oldUnit,oldScale,force);
         amt = displayAmount(val,0);
         break;
      case Unit::String:
         amt = text();
         break;
      case Unit::DiastaticPower:
         val = toSI(oldUnit,oldScale,force);
         amt = displayAmount(val,3);
         break;
      case Unit::None:
      default:
         val = Brewtarget::toDouble(text(),&ok);
         if ( ! ok )
            Brewtarget::logW( QString("%1: failed to convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(text()).arg(_section).arg(_editField) );
         amt = displayAmount(val);
   }
   QLineEdit::setText(amt);

   if ( wasChanged ) {
      emit textModified();
   }
}

double BtLineEdit::toSI(Unit::unitDisplay oldUnit,Unit::unitScale oldScale,bool force)
{
   UnitSystem* temp;
   Unit*       works;
   Unit::unitDisplay dspUnit  = oldUnit;
   Unit::unitScale   dspScale = oldScale;

   // If force is set, just use what is provided in the call. If we are
   // not forcing the unit & scale, we need to read the configured properties
   if ( ! force )
   {
      // If the display unit is forced, use this unit the default one.
      if ( _forceUnit != Unit::noUnit )
         dspUnit = _forceUnit;
      else
         dspUnit   = (Unit::unitDisplay)Brewtarget::option(_editField, Unit::noUnit, _section, Brewtarget::UNIT).toInt();

      // If the display scale is forced, use this scale as the default one.
      if( _forceScale != Unit::noScale )
         dspScale = _forceScale;
      else
         dspScale  = (Unit::unitScale)Brewtarget::option(_editField, Unit::noScale, _section, Brewtarget::SCALE).toInt();
   }

   // Find the unit system containing dspUnit
   temp = Brewtarget::findUnitSystem(_units,dspUnit);
   if ( temp )
   {
      // If we found it, find the unit referred by dspScale
      works = temp->scaleUnit(dspScale);
      if (! works )
         // If we didn't find the unit, default to the UnitSystem's default
         // unit
         works = temp->unit();

      // get the qstringToSI() from the unit system, using the found unit.
      // Force the issue in qstringToSI() unless dspScale is Unit::noScale.

      return temp->qstringToSI(text(), works, dspScale != Unit::noScale, dspScale);
   }
   else if ( _type == Unit::String )
      return 0.0;

   // If all else fails, simply try to force the contents of the field to a
   // double. This doesn't seem advisable?
   bool ok = false;
   double amt = toDouble(&ok);
   if ( ! ok )
      Brewtarget::logW( QString("%1 : could not convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(text()).arg(_section).arg(_editField) );
   return amt;
}

QString BtLineEdit::displayAmount( double amount, int precision)
{
   Unit::unitDisplay unitDsp;
   Unit::unitScale scale;

   if ( _forceUnit != Unit::noUnit )
      unitDsp = _forceUnit;
   else
      unitDsp  = (Unit::unitDisplay)Brewtarget::option(_editField, Unit::noUnit, _section, Brewtarget::UNIT).toInt();

   scale    = (Unit::unitScale)Brewtarget::option(_editField, Unit::noScale, _section, Brewtarget::SCALE).toInt();

   // I find this a nice level of abstraction. This lets all of the setText()
   // methods make a single call w/o having to do the logic for finding the
   // unit and scale.
   return Brewtarget::displayAmount(amount, _units, precision, unitDsp, scale);
}

double BtLineEdit::toDouble(bool* ok)
{
   QRegExp amtUnit;

   if ( ok )
      *ok = true;
   // Make sure we get the right decimal point (. or ,) and the right grouping
   // separator (, or .). Some locales write 1.000,10 and other write
   // 1,000.10. We need to catch both
   QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

   // if the regex dies, return 0.0
   if (amtUnit.indexIn(text()) == -1)
   {
      if ( ok )
         *ok = false;
      return 0.0;
   }

   return Brewtarget::toDouble(amtUnit.cap(1), "BtLineEdit::toDouble()");
}

void BtLineEdit::setText( double amount, int precision)
{
   QLineEdit::setText( displayAmount(amount,precision) );
}

void BtLineEdit::setText( BeerXMLElement* element, int precision )
{
   double amount = 0.0;
   QString display;

   if ( _type == Unit::String )
      display = element->property(_editField.toLatin1().constData()).toString();
   else if ( element->property(_editField.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      bool ok = false;
      // Get the value from the element, and put it in a QVariant
      QVariant tmp = element->property(_editField.toLatin1().constData());
      // It is important here to use QVariant::toDouble() instead of going
      // through toString() and then Brewtarget::toDouble().
      amount = tmp.toDouble(&ok);
      if ( !ok ) {
         Brewtarget::logW( QString("%1 could not convert %2 (%3:%4) to double")
                              .arg(Q_FUNC_INFO)
                              .arg(tmp.toString())
                              .arg(_section)
                              .arg(_editField) );
      }

      display = displayAmount(amount, precision);
   }
   else
   {
      display = "?";
   }

   QLineEdit::setText(display);
}

void BtLineEdit::setText( QString amount, int precision)
{
   double amt;
   bool ok = false;

   if ( _type == Unit::String )
      QLineEdit::setText(amount);
   else
   {
      amt = Brewtarget::toDouble(amount,&ok);
      if ( !ok )
         Brewtarget::logW( QString("%1 could not convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(amount).arg(_section).arg(_editField) );
      QLineEdit::setText(displayAmount(amt, precision));
   }
}

void BtLineEdit::setText( QVariant amount, int precision)
{
   setText(amount.toString(), precision);
}

int BtLineEdit::type() const { return (int)_type; }
QString BtLineEdit::editField() const { return _editField; }
QString BtLineEdit::configSection()
{
   if ( _section.isEmpty() ) {
      setConfigSection("");
   }

   return _section;
}

// Once we require >qt5.5, we can replace this noise with
// QMetaEnum::fromType()
QString BtLineEdit::forcedUnit() const
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitDisplay");
   QMetaEnum unitEnum = mo.enumerator(index);

   return QString( unitEnum.valueToKey(_forceUnit) );
}

QString BtLineEdit::forcedScale() const
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitScale");
   QMetaEnum scaleEnum = mo.enumerator(index);

   return QString( scaleEnum.valueToKey(_forceScale) );
}

void BtLineEdit::setType(int type) { _type = (Unit::UnitType)type;}
void BtLineEdit::setEditField( QString editField) { _editField = editField; }

// The cascade looks a little odd, but it is intentional.
void BtLineEdit::setConfigSection( QString configSection)
{
   _section = configSection;

   if ( _section.isEmpty() )
      _section = btParent->property("configSection").toString();

   if ( _section.isEmpty() )
      _section = btParent->objectName();
}

// previous comment about qt5.5 applies
void BtLineEdit::setForcedUnit( QString forcedUnit )
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitDisplay");
   QMetaEnum unitEnum = mo.enumerator(index);

   _forceUnit = (Unit::unitDisplay)unitEnum.keyToValue(forcedUnit.toStdString().c_str());
}

void BtLineEdit::setForcedScale( QString forcedScale )
{
   const QMetaObject &mo = Unit::staticMetaObject;
   int index = mo.indexOfEnumerator("unitScale");
   QMetaEnum unitEnum = mo.enumerator(index);

   _forceScale = (Unit::unitScale)unitEnum.keyToValue(forcedScale.toStdString().c_str());
}

BtGenericEdit::BtGenericEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::None)
{
   _units = 0;
}

BtMassEdit::BtMassEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Mass)
{
   _units = Units::kilograms;
}

BtVolumeEdit::BtVolumeEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Volume)
{
   _units = Units::liters;
}

BtTemperatureEdit::BtTemperatureEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Temp)
{
   _units = Units::celsius;
}

BtTimeEdit::BtTimeEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Time)
{
   _units = Units::minutes;
}

BtDensityEdit::BtDensityEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Density)
{
   _units = Units::sp_grav;
}

BtColorEdit::BtColorEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Color)
{
   _units = Units::srm;
}

BtStringEdit::BtStringEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::String)
{
   _units = 0;
}

BtMixedEdit::BtMixedEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Mixed)
{
   // This is probably pure evil I will later regret
   _type = Unit::Volume;
   _units = Units::liters;
}

void BtMixedEdit::setIsWeight(bool state)
{
   // But you have to admit, this is clever
   if (state)
   {
      _type = Unit::Mass;
      _units = Units::kilograms;
   }
   else
   {
      _type = Unit::Volume;
      _units = Units::liters;
   }

   // maybe? My head hurts now
   onLineChanged();
}

BtDiastaticPowerEdit::BtDiastaticPowerEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::DiastaticPower)
{
   _units = Units::lintner;
}

