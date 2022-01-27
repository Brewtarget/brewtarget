/*
 * UiAmountWithUnits.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2022:
 * - Brian Rower <brian.rower@gmail.com>
 * - Mark de Wever <koraq@xs4all.nl>
 * - Mattias Måhl <mattias@kejsarsten.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mike Evans <mikee@saxicola.co.uk>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Théophane Martin <theophane.m@gmail.com>
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
#include "UiAmountWithUnits.h"

#include <QDebug>
#include <QVariant>
#include <QWidget>

#include "Localization.h"
#include "measurement/Measurement.h"
#include "utils/OptionalToStream.h"

UiAmountWithUnits::UiAmountWithUnits(QWidget * parent,
                                     BtFieldType fieldType,
                                     Measurement::Unit const * units) :
   parent{parent},
   fieldType{fieldType},
   units{units} {
   return;
}

UiAmountWithUnits::~UiAmountWithUnits() = default;

BtFieldType UiAmountWithUnits::getFieldType() const {
   // If it's not Measurement::PhysicalQuantity::Mixed, just return what it is
   if (!std::holds_alternative<Measurement::PhysicalQuantity>(this->fieldType) ||
       Measurement::PhysicalQuantity::Mixed != std::get<Measurement::PhysicalQuantity>(this->fieldType)) {
      return this->fieldType;
   }
   // If it is Measurement::PhysicalQuantity::Mixed, then should return either Mass or Volume;
   auto const physicalQuantity = this->units->getPhysicalQuantity();
   // It's a coding error if we somehow have units other than Mass or Volume
   Q_ASSERT(Measurement::PhysicalQuantity::Mass == physicalQuantity ||
            Measurement::PhysicalQuantity::Volume == physicalQuantity);
   return physicalQuantity;
}

void UiAmountWithUnits::setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement) {
   Measurement::setForcedSystemOfMeasurementForField(this->editField, this->configSection, systemOfMeasurement);
   return;
}

std::optional<Measurement::SystemOfMeasurement> UiAmountWithUnits::getForcedSystemOfMeasurement() const {
   return Measurement::getForcedSystemOfMeasurementForField(this->editField, this->configSection);
}

void UiAmountWithUnits::setForcedSystemOfMeasurementViaString(QString systemOfMeasurementAsString) {
   qDebug() << Q_FUNC_INFO << systemOfMeasurementAsString;
   this->setForcedSystemOfMeasurement(Measurement::getFromUniqueName(systemOfMeasurementAsString));
   return;
}

QString UiAmountWithUnits::getForcedSystemOfMeasurementViaString() const {
   auto forcedSystemOfMeasurement = this->getForcedSystemOfMeasurement();
   return forcedSystemOfMeasurement ? Measurement::getUniqueName(*forcedSystemOfMeasurement) : "";
}

void UiAmountWithUnits::setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> relativeScale) {
   Measurement::setForcedRelativeScaleForField(this->editField, this->configSection, relativeScale);
   return;
}

std::optional<Measurement::UnitSystem::RelativeScale> UiAmountWithUnits::getForcedRelativeScale() const {
   return Measurement::getForcedRelativeScaleForField(this->editField, this->configSection);
}

void UiAmountWithUnits::setForcedRelativeScaleViaString(QString relativeScaleAsString) {
   qDebug() << Q_FUNC_INFO << relativeScaleAsString;
   this->setForcedRelativeScale(Measurement::UnitSystem::getScaleFromUniqueName(relativeScaleAsString));
   return;
}

QString UiAmountWithUnits::getForcedRelativeScaleViaString() const {
   auto forcedRelativeScale = this->getForcedRelativeScale();
   return forcedRelativeScale ? Measurement::UnitSystem::getUniqueName(*forcedRelativeScale) : "";
}

void UiAmountWithUnits::setEditField(QString editField) {
   this->editField = editField;
   return;
}

QString UiAmountWithUnits::getEditField() const {
   return this->editField;
}

void UiAmountWithUnits::setConfigSection(QString configSection) {
   // The cascade looks a little odd, but it is intentional.
   this->configSection = configSection;
   if (this->configSection.isEmpty()) {
      this->configSection = this->parent->property("configSection").toString();
   }
   if (this->configSection.isEmpty()) {
      this->configSection = this->parent->objectName();
   }
   return;
}

QString UiAmountWithUnits::getConfigSection() {
   if (this->configSection.isEmpty()) {
      // Setting the config section to blank will actually attempt to populate it with default values -- see
      // UiAmountWithUnits::setConfigSection()
      this->setConfigSection("");
   }

   return this->configSection;
}

double UiAmountWithUnits::toDoubleRaw(bool * ok) const {
   if (ok) {
      *ok = true;
   }

   // Make sure we get the right decimal point (. or ,) and the right grouping
   // separator (, or .). Some locales write 1.000,10 and other write
   // 1,000.10. We need to catch both
   QString decimal = QRegExp::escape(QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   QRegExp amtUnit;
   amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

   // if the regex dies, return 0.0
   if (amtUnit.indexIn(this->getWidgetText()) == -1) {
      if (ok) {
         *ok = false;
      }
      return 0.0;
   }

   return Localization::toDouble(amtUnit.cap(1), Q_FUNC_INFO);
}

Measurement::Amount UiAmountWithUnits::toSI() {
   // It's a coding error to call this function if we are not dealing with a physical quantity
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(this->getFieldType()));
   Q_ASSERT(this->units);

   return Measurement::qStringToSI(this->getWidgetText(),
                                   this->units->getPhysicalQuantity(),
                                   this->getForcedSystemOfMeasurement(),
                                   this->getForcedRelativeScale());
}


QString UiAmountWithUnits::displayAmount(double amount, int precision) {
   // I find this a nice level of abstraction. This lets all of the setText()
   // methods make a single call w/o having to do the logic for finding the
   // unit and scale.
   if (this->units) {
      return Measurement::displayAmount(Measurement::Amount{amount, *this->units},
                                        precision,
                                        this->getForcedSystemOfMeasurement(),
                                        this->getForcedRelativeScale());
   }
   return Measurement::displayQuantity(amount, precision);
}

void UiAmountWithUnits::textOrUnitsChanged(PreviousScaleInfo previousScaleInfo) {
   // This is where it gets hard
   //
   // We may need to fix the text that the user entered, eg if this field is set to show US Customary volumes and user
   // enters an amount in litres then we need to convert it to display in pints or quarts etc.
   QString correctedText;

   QString rawValue = this->getWidgetText();
   qDebug() << Q_FUNC_INFO << "rawValue" << rawValue;

   if (rawValue.isEmpty()) {
      return;
   }

   if (!std::holds_alternative<Measurement::PhysicalQuantity>(this->getFieldType())) {
      correctedText = rawValue;
   } else {
      // The idea here is we need to first translate the field into a known
      // amount (aka to SI) and then into the unit we want.
      Measurement::Amount amountAsCanonical = this->convertToSI(previousScaleInfo);

      Measurement::PhysicalQuantity physicalQuantity = std::get<Measurement::PhysicalQuantity>(this->getFieldType());
      int precision = 3;
      if (physicalQuantity == Measurement::PhysicalQuantity::Color) {
         precision = 0;
      }
      correctedText = this->displayAmount(amountAsCanonical.quantity, precision);
      qDebug() <<
         Q_FUNC_INFO << "Interpreted" << rawValue << "as" << amountAsCanonical << "and corrected to" << correctedText;
      qDebug() << Q_FUNC_INFO << "this->units=" << this->units;
   }
   this->setWidgetText(correctedText);
   return;
}

Measurement::Amount UiAmountWithUnits::convertToSI(PreviousScaleInfo previousScaleInfo) {
   QString rawValue = this->getWidgetText();
   qDebug() <<
      Q_FUNC_INFO << "rawValue:" << rawValue <<  ", old SystemOfMeasurement:" << previousScaleInfo.oldSystemOfMeasurement <<
      ", old ForcedScale: " << previousScaleInfo.oldForcedScale;

   // It is a coding error if this function is called for a field is not holding a physical quantity.  Eg, it makes no
   // sense to call convertToSI for a date or a string field, not least as there will be no sane value to supply for
   // oldSystemOfMeasurement.
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(this->getFieldType()));
   Measurement::PhysicalQuantity physicalQuantity = std::get<Measurement::PhysicalQuantity>(this->getFieldType());

   Measurement::UnitSystem const & oldUnitSystem =
      Measurement::UnitSystem::getInstance(previousScaleInfo.oldSystemOfMeasurement, physicalQuantity);

   Measurement::Unit const * defaultUnit{
      previousScaleInfo.oldForcedScale ? oldUnitSystem.scaleUnit(*previousScaleInfo.oldForcedScale) : oldUnitSystem.unit()
   };

   // It's a coding error if defaultUnit is null, because it means previousScaleInfo.oldForcedScale was not valid for
   // oldUnitSystem.  However, we can recover.
   if (!defaultUnit) {
      qWarning() << Q_FUNC_INFO << "previousScaleInfo.oldForcedScale invalid?" << previousScaleInfo.oldForcedScale;
      defaultUnit = oldUnitSystem.unit();
   }

   //
   // Normally, we display units with the text.  If the user just edits the number, then the units will still be there.
   // Alternatively, if the user specifies different units in the text, we should try to honour those.  Otherwise, if,
   // no units are specified in the text, we need to go to defaults.  Defaults are either what is "forced" for this
   // specific field or, failing that, what is configured globally.
   //
   // Measurement::UnitSystem::qStringToSI will handle all the logic to deal with any units specified by the user in the
   // string.  (In theory, we just grab the units that the user has specified in the input text.  In reality, it's not
   // that easy as we sometimes need to disambiguate - eg between Imperial gallons and US customary ones.  So, if we
   // have old or current units then that helps with this - eg, if current units are US customary cups and user enters
   // gallons, then we'll go with US customary gallons over Imperial ones.)
   //
   auto amount = oldUnitSystem.qstringToSI(rawValue, *defaultUnit);
   qDebug() << Q_FUNC_INFO << "Converted to" << amount;
   return amount;
}
