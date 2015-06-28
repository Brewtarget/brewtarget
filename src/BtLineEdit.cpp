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

BtLineEdit::BtLineEdit(QWidget *parent, FieldType type) :
   QLineEdit(parent)
{
   btParent = parent;
   _type = type;

   switch( _type )
   {
      case MASS:
         // I don't ... oh bugger
         _units = Units::kilograms;
         break;
      case VOLUME:
         _units = Units::liters;
         break;
      case TEMPERATURE:
         _units = Units::celsius;
         break;
      case TIME:
         _units = Units::minutes;
         break;
      case COLOR:
         _units = Units::srm;
         break;
      case DENSITY:
         _units = Units::sp_grav;
         break;
      case MIXED:
         _units = Units::kilograms;
         break;
      case GENERIC:
      case STRING:
         _units = 0;
         break;
   }

   connect(this,SIGNAL(editingFinished()),this,SLOT(lineChanged()));
}

void BtLineEdit::lineChanged()
{
   lineChanged(Unit::noUnit,Unit::noScale);
}

// Dynamic properties need to be evaluated late, so we do it this way
void BtLineEdit::initializeProperties()
{
   QVariant unitName = property("forcedUnit");
   _property = property("editField").toString();


   if ( unitName.isValid() ) 
   {
      const QMetaObject &mo = Unit::staticMetaObject;
      int index = mo.indexOfEnumerator("unitDisplay");
      QMetaEnum unitEnum = mo.enumerator(index);

      _forceUnit = (Unit::unitDisplay)unitEnum.keyToValue(unitName.toString().toStdString().c_str());
   }
   else 
   {
      _forceUnit = Unit::noUnit;
   }
}

void BtLineEdit::initializeSection()
{

   if ( property("configSection").isValid() )
      _section = property("configSection").toString();
   else if ( btParent->property("configSection").isValid() )
      _section = btParent->property("configSection").toString();
   else
      _section = btParent->objectName();

}

void BtLineEdit::lineChanged(Unit::unitDisplay oldUnit, Unit::unitScale oldScale)
{
   // This is where it gets hard
   double val = -1.0;
   QString amt;
   bool force = false;
   bool ok = false;

   // editingFinished happens on focus being lost, regardless of anything
   // being changed. I am hoping this short circuits properly and we do
   // nothing if nothing changed.
   if ( sender() == this && ! isModified() )
   {
      return;
   }

   // If we are here because somebody else sent the signal (ie, a label) or we
   // generated the signal but nothing has changed then don't try to guess the
   // units.
   if ( sender() != this )
   {
      force = true;
   }

   if ( _section.isEmpty() )
      initializeSection();

   if ( _property.isEmpty() )
      initializeProperties();

   if ( text().isEmpty() )
   {
      amt = "";
   }
   else
   {
      // The idea here is we need to first translate the field into a known
      // amount (aka to SI) and then into the unit we want.
      switch( _type )
      {
         case MASS:
         case VOLUME:
         case TEMPERATURE:
         case TIME:
            val = toSI(oldUnit,oldScale,force);
            amt = displayAmount(val,3);
            break;
         case DENSITY:
         case COLOR:
            val = toSI(oldUnit,oldScale,force);
            amt = displayAmount(val,0);
            break;
         case STRING:
            amt = text();
            break;
         case GENERIC:
         default:
            val = Brewtarget::toDouble(text(),&ok);
            if ( ! ok )
               Brewtarget::logW( QString("BtLineEdit::lineChanged: failed to convert %1 toDouble").arg(text()) );
            amt = displayAmount(val);
      }
   }

   QLineEdit::setText(amt);

   if ( ! force )
   {
      emit textModified();
   }
}

double BtLineEdit::toSI(Unit::unitDisplay oldUnit,Unit::unitScale oldScale,bool force)
{
   UnitSystem* temp;
   Unit*       works;
   Unit::unitDisplay dspUnit  = oldUnit;
   Unit::unitScale   dspScale = oldScale;

   if ( _section.isEmpty() )
      initializeSection();
   if ( _property.isEmpty() )
      initializeProperties();

   // If force is set, just use what is provided in the call. If we are
   // not forcing the unit & scale, we need to read the configured properties
   if ( ! force )
   {
      // If the display unit is forced, use this unit the default one.
      if ( _forceUnit != Unit::noUnit )
         dspUnit = _forceUnit;
      else
         dspUnit   = (Unit::unitDisplay)Brewtarget::option(_property, Unit::noUnit, _section, Brewtarget::UNIT).toInt();

      dspScale  = (Unit::unitScale)Brewtarget::option(_property, Unit::noUnit, _section, Brewtarget::SCALE).toInt();
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
      return temp->qstringToSI(text(), works, dspScale != Unit::noScale);
   }
   else if ( _type == STRING )
      return 0.0;

   // If all else fails, simply try to force the contents of the field to a
   // double. This doesn't seem advisable?
   bool ok = false;
   double amt = Brewtarget::toDouble(text(), &ok);
   if ( ! ok )
      Brewtarget::logW( QString("BtLineEdit::toSI : could not convert %1 to double").arg(text()) );
   return amt;
}

QString BtLineEdit::displayAmount( double amount, int precision)
{
   Unit::unitDisplay unitDsp;
   Unit::unitScale scale;

   if ( _section.isEmpty() )
      initializeSection();
   if ( _property.isEmpty() )
      initializeProperties();

   if ( _forceUnit != Unit::noUnit )
      unitDsp = _forceUnit;
   else
      unitDsp  = (Unit::unitDisplay)Brewtarget::option(_property, Unit::noUnit, _section, Brewtarget::UNIT).toInt();

   scale    = (Unit::unitScale)Brewtarget::option(_property, Unit::noScale, _section, Brewtarget::SCALE).toInt();

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

   if ( _section.isEmpty() )
      initializeSection();
   if ( _property.isEmpty() )
      initializeProperties();

   if ( _type == STRING )
      display = element->property(_property.toLatin1().constData()).toString();
   else if ( element->property(_property.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      // Get the amount
      bool ok = false;
      QString tmp = element->property(_property.toLatin1().constData()).toString();
      amount = Brewtarget::toDouble(tmp, &ok);
      if ( !ok )
         Brewtarget::logW( QString("BtLineEdit::setText(BeerXMLElement*,int) could not convert %1 to double").arg(tmp) );

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

   if ( _type == STRING )
      QLineEdit::setText(amount);
   else
   {
      amt = Brewtarget::toDouble(amount,&ok);
      if ( !ok )
         Brewtarget::logW( QString("BtLineEdit::setText(QString,int) could not conver %1 to double").arg(amount) );
      QLineEdit::setText(displayAmount(amt, precision));
   }
}

void BtLineEdit::setText( QVariant amount, int precision)
{
   setText(amount.toString(), precision);
}

BtGenericEdit::BtGenericEdit(QWidget *parent)
   : BtLineEdit(parent,GENERIC)
{
}

BtMassEdit::BtMassEdit(QWidget *parent)
   : BtLineEdit(parent,MASS)
{
}

BtVolumeEdit::BtVolumeEdit(QWidget *parent)
   : BtLineEdit(parent,VOLUME)
{
}

BtTemperatureEdit::BtTemperatureEdit(QWidget *parent)
   : BtLineEdit(parent,TEMPERATURE)
{
}

BtTimeEdit::BtTimeEdit(QWidget *parent)
   : BtLineEdit(parent,TIME)
{
}

BtDensityEdit::BtDensityEdit(QWidget *parent)
   : BtLineEdit(parent,DENSITY)
{
}

BtColorEdit::BtColorEdit(QWidget *parent)
   : BtLineEdit(parent,COLOR)
{
}

BtStringEdit::BtStringEdit(QWidget *parent)
   : BtLineEdit(parent,STRING)
{
}

BtMixedEdit::BtMixedEdit(QWidget *parent)
   : BtLineEdit(parent,MIXED)
{
   // This is probably pure evil I will later regret
   _type = VOLUME;
   _units = Units::liters;
}

void BtMixedEdit::setIsWeight(bool state)
{
   // But you have to admit, this is clever
   if (state)
   {
      _type = MASS;
      _units = Units::kilograms;
   }
   else
   {
      _type = VOLUME;
      _units = Units::liters;
   }

   // maybe? My head hurts now
   lineChanged();

}

