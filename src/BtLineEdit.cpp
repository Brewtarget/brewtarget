/*
 * BtLineEdit.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021:
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

#include <QDebug>
#include <QSettings>
#include <QStyle>

#include "Algorithms.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "model/NamedEntity.h"
#include "PersistentSettings.h"
#include "utils/OptionalToStream.h"

namespace {
   int const min_text_size = 8;
   int const max_text_size = 50;
}

BtLineEdit::BtLineEdit(QWidget *parent,
                       BtFieldType fieldType,
                       Measurement::Unit const * units,
                       QString const & maximalDisplayString) :
   QLineEdit(parent),
   UiAmountWithUnits(parent, fieldType, units) {
   this->configSection = property("configSection").toString();
   connect(this, &QLineEdit::editingFinished, this, &BtLineEdit::onLineChanged);

   // We can work out (and store) our display size here, but not yet set it.  The way the Designer UI Files work is to
   // generate code that calls setters such as setMaximumWidth() etc, which would override anything we do here in the
   // constructor.  So we set our size when setText() is called.
   this->calculateDisplaySize(maximalDisplayString);

   return;
}

BtLineEdit::~BtLineEdit() = default;

QString BtLineEdit::getWidgetText() const {
   return this->text();
}

void BtLineEdit::setWidgetText(QString text) {
   this->QLineEdit::setText(text);
   return;
}

void BtLineEdit::onLineChanged() {
   auto const myFieldType = this->getFieldType();
   qDebug() <<
      Q_FUNC_INFO << "this->fieldType=" << myFieldType << ", this->units=" << this->units <<
      ", forcedSystemOfMeasurement=" << this->getForcedSystemOfMeasurement() << ", forcedRelativeScale=" <<
      this->getForcedRelativeScale();

   if (!std::holds_alternative<Measurement::PhysicalQuantity>(myFieldType)) {
      return;
   }

   Measurement::UnitSystem const & oldUnitSystem =
      Measurement::getUnitSystemForField(this->editField,
                                         this->configSection,
                                         std::get<Measurement::PhysicalQuantity>(myFieldType));
   auto oldForcedRelativeScale = Measurement::getForcedRelativeScaleForField(this->editField, this->configSection);
   PreviousScaleInfo previousScaleInfo{
      oldUnitSystem.systemOfMeasurement,
      oldForcedRelativeScale
   };

   qDebug() <<
      Q_FUNC_INFO << "oldUnitSystem=" << oldUnitSystem << ", oldForcedRelativeScale=" << oldForcedRelativeScale;

   this->lineChanged(previousScaleInfo);
   return;
}

void BtLineEdit::lineChanged(PreviousScaleInfo previousScaleInfo) {
   // editingFinished happens on focus being lost, regardless of anything
   // being changed. I am hoping this short circuits properly and we do
   // nothing if nothing changed.
   if (this->sender() == this && !isModified()) {
      qDebug() << Q_FUNC_INFO << "Nothing changed";
      return;
   }

   this->textOrUnitsChanged(previousScaleInfo);

   if (sender() == this) {
      emit textModified();
   }

   return;
}

void BtLineEdit::setText(double amount, int precision) {
   this->setWidgetText(this->displayAmount(amount, precision));
   this->setDisplaySize();
   return;
}

void BtLineEdit::setText(NamedEntity * element, int precision) {
   QString display;

   char const * const propertyName = this->editField.toLatin1().constData();
   QVariant const propertyValue = element->property(propertyName);
   qDebug() << Q_FUNC_INFO << "Read property" << propertyName << "as" << propertyValue;
   bool force = false;
   auto const myFieldType = this->getFieldType();
   if (std::holds_alternative<NonPhysicalQuantity>(myFieldType) &&
       NonPhysicalQuantity::String == std::get<NonPhysicalQuantity>(myFieldType)) {
      display = propertyValue.toString();
      force = true;
   } else if (propertyValue.canConvert(QVariant::Double)) {
      bool ok = false;
      // It is important here to use QVariant::toDouble() instead of going
      // through toString() and then Localization::toDouble().
      double amount = propertyValue.toDouble(&ok);
      if (!ok) {
         qWarning() <<
            Q_FUNC_INFO << "Could not convert " << propertyValue.toString() << " (" << this->configSection << ":" <<
            this->editField << ") to double";
      }

      display = this->displayAmount(amount, precision);
   } else {
      display = "?";
   }

   this->setWidgetText(display);
   this->setDisplaySize(force);
   return;
}

void BtLineEdit::setText(QString amount, int precision) {
   bool force = false;

   auto const myFieldType = this->getFieldType();
   if (std::holds_alternative<NonPhysicalQuantity>(myFieldType) &&
       NonPhysicalQuantity::String == std::get<NonPhysicalQuantity>(myFieldType)) {
      this->setWidgetText(amount);
      force = true;
   } else {
      bool ok = false;
      double amt = Localization::toDouble(amount, &ok);
      if (!ok) {
         qWarning() <<
            Q_FUNC_INFO << "Could not convert" << amount << "(" << this->configSection << ":" << this->editField <<
            ") to double";
      }
      this->setWidgetText(displayAmount(amt, precision));
   }

   this->setDisplaySize(force);
   return;
}

void BtLineEdit::setText(QVariant amount, int precision) {
   this->setText(amount.toString(), precision);
   return;
}

void BtLineEdit::calculateDisplaySize(QString const & maximalDisplayString) {
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

void BtLineEdit::setDisplaySize(bool recalculate) {
   if ( recalculate ) {
      QString sizing_string = text();

      // this is a dirty bit of cheating. If we do not reset the minimum
      // width, the field only ever gets bigger. This forces the resize I
      // want, but only when we are instructed to force it
      setMinimumWidth(0);
      if ( sizing_string.length() < min_text_size ) {
         sizing_string = QString(min_text_size,'a');
      } else if ( sizing_string.length() > max_text_size ) {
         sizing_string = QString(max_text_size,'a');
      }
      calculateDisplaySize(sizing_string);
   }
   this->setFixedWidth(this->desiredWidthInPixels);
   return;
}

BtGenericEdit::BtGenericEdit(QWidget *parent) : BtLineEdit(parent, NonPhysicalQuantity::String, nullptr) {
   return;
}

BtMassEdit::BtMassEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Mass, &Measurement::Units::kilograms) {
   return;
}

BtVolumeEdit::BtVolumeEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Volume, &Measurement::Units::liters) {
   return;
}

BtTemperatureEdit::BtTemperatureEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Temperature, &Measurement::Units::celsius) {
   return;
}

BtTimeEdit::BtTimeEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Time, &Measurement::Units::minutes) {
   return;
}

BtDensityEdit::BtDensityEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Density, &Measurement::Units::sp_grav) {
   return;
}

BtColorEdit::BtColorEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Color, &Measurement::Units::srm) {
   return;
}

BtStringEdit::BtStringEdit(QWidget *parent) : BtLineEdit(parent, NonPhysicalQuantity::String, nullptr) {
   return;
}

BtMixedEdit::BtMixedEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Mixed) {
   // This is probably pure evil I will later regret
   this->units = &Measurement::Units::liters;
   return;
}

void BtMixedEdit::setIsWeight(bool state) {
   // But you have to admit, this is clever
   if (state) {
      this->units = &Measurement::Units::kilograms;
   } else {
      this->units = &Measurement::Units::liters;
   }

   // maybe? My head hurts now
   this->onLineChanged();
   return;
}

BtDiastaticPowerEdit::BtDiastaticPowerEdit(QWidget *parent) :
   BtLineEdit(parent, Measurement::PhysicalQuantity::DiastaticPower, &Measurement::Units::lintner) {
   return;
}
