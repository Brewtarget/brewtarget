/*
 * BtLineEdit.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2020:
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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
#include "BtLineEdit.h"

#include <QSettings>
#include <QDebug>
#include <QStyle>

#include "brewtarget.h"
#include "UnitSystem.h"
#include "Unit.h"
#include "Algorithms.h"

const int min_text_size = 8;
const int max_text_size = 50;

BtLineEdit::BtLineEdit(QWidget *parent, Unit::UnitType type, QString const & maximalDisplayString) :
   QLineEdit(parent),
   btParent(parent),
   _type(type),
   _forceUnit(Unit::noUnit),
   _forceScale(Unit::noScale)
{
   _section = property("configSection").toString();
   connect(this,&QLineEdit::editingFinished,this,&BtLineEdit::onLineChanged);

   // We can work out (and store) our display size here, but not yet set it.  The way the Designer UI Files work is to
   // generate code that calls setters such as setMaximumWidth() etc, which would override anything we do here in the
   // constructor.  So we set our size when setText() is called.
   this->calculateDisplaySize(maximalDisplayString);

   return;
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
            qWarning() << QString("%1: failed to convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(text()).arg(_section).arg(_editField);
         amt = displayAmount(val);
   }
   QLineEdit::setText(amt);

   if ( wasChanged ) {
      emit textModified();
   }
}

double BtLineEdit::toSI(Unit::unitDisplay oldUnit,Unit::unitScale oldScale,bool force)
{
   UnitSystem const * temp;
   Unit const *       works;
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
      qWarning() << QString("%1 : could not convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(text()).arg(_section).arg(_editField);
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
   this->setDisplaySize();
   return;
}

void BtLineEdit::setText( NamedEntity* element, int precision )
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
         qWarning() << QString("%1 could not convert %2 (%3:%4) to double")
                              .arg(Q_FUNC_INFO)
                              .arg(tmp.toString())
                              .arg(_section)
                              .arg(_editField);
      }

      display = displayAmount(amount, precision);
   }
   else
   {
      display = "?";
   }

   QLineEdit::setText(display);
   this->setDisplaySize( _type == Unit::String);
   return;
}

void BtLineEdit::setText( QString amount, int precision)
{
   double amt;
   bool ok = false;
   bool force = false;

   if ( _type == Unit::String ) {
      QLineEdit::setText(amount);
      force = true;
   }
   else
   {
      amt = Brewtarget::toDouble(amount,&ok);
      if ( !ok )
         qWarning() << QString("%1 could not convert %2 (%3:%4) to double").arg(Q_FUNC_INFO).arg(amount).arg(_section).arg(_editField);
      QLineEdit::setText(displayAmount(amt, precision));
   }

   this->setDisplaySize(force);
   return;
}

void BtLineEdit::setText( QVariant amount, int precision)
{
   setText(amount.toString(), precision);
   return;
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

void BtLineEdit::calculateDisplaySize(QString const & maximalDisplayString)
{
   //
   // By default, some, but not all, boxes have a min and max width of 100 pixels, but this is not wide enough on a
   // high DPI display.  We instead calculate width here based on font-size - but without reducing any existing minimum
   // width.
   //
   // Unfortunately, for a QLineEdit object, calculating the width is hard because, besides the text, we need to allow
   // for the width of padding and frame, which is non-trivial to discover.  Eg, typically:
   //   marginsAroundText() and contentsMargins() both return 0 for left and right margins
   //   contentsRect() and frameSize() both give the same width as width()
   // AFAICT, the best option is to query via pixelMetric() calls to the widget's style, but we need to check this works
   // in practice on a variety of different systems.
   //
   QFontMetrics displayFontMetrics(this->font());
   QRect minimumTextRect = displayFontMetrics.boundingRect(maximalDisplayString);
   QMargins marginsAroundText = this->textMargins();
   auto myStyle = this->style();
   // NB: 2× frame width as on left and right; same for horizontal spacing
   int totalWidgetWidthForMaximalDisplayString = minimumTextRect.width() +
                                                 marginsAroundText.left() +
                                                 marginsAroundText.right() +
                                                 (2 * myStyle->pixelMetric(QStyle::PM_DefaultFrameWidth)) +
                                                 (2 * myStyle->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));

   this->desiredWidthInPixels = qMax(this->minimumWidth(), totalWidgetWidthForMaximalDisplayString);
   return;
}

void BtLineEdit::setDisplaySize(bool recalculate)
{
   if ( recalculate ) {
      QString sizing_string = text();

      // this is a dirty bit of cheating. If we do not reset the minimum
      // width, the field only ever gets bigger. This forces the resize I
      // want, but only when we are instructed to force it
      setMinimumWidth(0);
      if ( sizing_string.length() < min_text_size ) {
         sizing_string = QString(min_text_size,'a');
      }
      else if ( sizing_string.length() > max_text_size ) {
         sizing_string = QString(max_text_size,'a');
      }
      calculateDisplaySize(sizing_string);
   }
   this->setFixedWidth(this->desiredWidthInPixels);
   return;
}

BtGenericEdit::BtGenericEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::None)
{
   _units = 0;
}

BtMassEdit::BtMassEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Mass)
{
   _units = &Units::kilograms;
}

BtVolumeEdit::BtVolumeEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Volume)
{
   _units = &Units::liters;
}

BtTemperatureEdit::BtTemperatureEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Temp)
{
   _units = &Units::celsius;
}

BtTimeEdit::BtTimeEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Time)
{
   _units = &Units::minutes;
}

BtDensityEdit::BtDensityEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Density)
{
   _units = &Units::sp_grav;
}

BtColorEdit::BtColorEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::Color)
{
   _units = &Units::srm;
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
   _units = &Units::liters;
}

void BtMixedEdit::setIsWeight(bool state)
{
   // But you have to admit, this is clever
   if (state)
   {
      _type = Unit::Mass;
      _units = &Units::kilograms;
   }
   else
   {
      _type = Unit::Volume;
      _units = &Units::liters;
   }

   // maybe? My head hurts now
   onLineChanged();
}

BtDiastaticPowerEdit::BtDiastaticPowerEdit(QWidget *parent)
   : BtLineEdit(parent,Unit::DiastaticPower)
{
   _units = &Units::lintner;
}

