/*
 * BtLineEdit.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023:
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

#include <string>

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
#include "utils/OptionalHelpers.h"

namespace {
   int const min_text_size = 8;
   int const max_text_size = 50;


   /**
    * @brief Convert a QString to a number.  Called by \c BtLineEdit::getValueAs()
    *
    *        There isn't a generic version of this function, just the specialisations below
    *
    *        Note that the reason we don't use \c QString::toInt(), \c QString::toUInt(), \c QString::toDouble(), etc in
    *        the specialization implementations is that they are not very accepting of extra characters.  Eg
    *        \c QString::toInt() will give 0 when parsing "12.34" as it barfs on the decimal point, whereas
    *        \c std::stoi() will give 12 on the same string input.  Of course, this means we have to convert from
    *        \c QString to \c std::string, but it's better than doing our own parsing.
    */
   template<typename T> T stringTo(QString const & input) {
      // This compile-time assert relies on the fact that no type has size 0
      static_assert(sizeof(T) == 0, "Only specializations of stringTo() can be used");
   }
   template<> int          stringTo<int>         (QString const & input) { return std::stoi(input.toStdString()); }
   template<> unsigned int stringTo<unsigned int>(QString const & input) { return std::stoul(input.toStdString()); }
   template<> double       stringTo<double>      (QString const & input) { return std::stod(input.toStdString()); }

}

BtLineEdit::BtLineEdit(QWidget *parent,
                       BtFieldType fieldType,
                       Measurement::Unit const * units,
                       int const defaultPrecision,
                       QString const & maximalDisplayString) :
   QLineEdit{parent},
   UiAmountWithUnits{parent, fieldType, units},
   defaultPrecision{defaultPrecision} {
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

template<typename T> T BtLineEdit::getValueAs() const {
   T returnValue = 0;
   try {
      returnValue = stringTo<T>(this->text());
   } catch (std::invalid_argument const & ex) {
      qWarning() << Q_FUNC_INFO << "Could not parse" << this->text() << "as number:" << ex.what();
   } catch(std::out_of_range const& ex) {
      qWarning() << Q_FUNC_INFO << "Out of range parsing" << this->text() << "as number:" << ex.what();
   }
   return returnValue;
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// saves having to put a bunch of std::string stuff there.)
//
template int BtLineEdit::getValueAs<int>() const;
template unsigned int BtLineEdit::getValueAs<unsigned int>() const;
template double BtLineEdit::getValueAs<double>() const;

void BtLineEdit::onLineChanged() {
   auto const myFieldType = this->getFieldType();
   qDebug() <<
      Q_FUNC_INFO << "this->fieldType=" << myFieldType << ", this->units=" << this->units <<
      ", forcedSystemOfMeasurement=" << this->getForcedSystemOfMeasurement() << ", forcedRelativeScale=" <<
      this->getForcedRelativeScale() << ", value=" << this->getWidgetText();

   if (!std::holds_alternative<Measurement::PhysicalQuantity>(myFieldType)) {
      qDebug() << Q_FUNC_INFO << "Not physical quantity";
      if (sender() == this) {
         emit textModified();
      }
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
      qDebug() << Q_FUNC_INFO << "Nothing changed; field holds" << this->getWidgetText();
      return;
   }

   this->textOrUnitsChanged(previousScaleInfo);

   if (sender() == this) {
      emit textModified();
   }

   return;
}

void BtLineEdit::setText(double amount) {
   this->setText(amount, this->defaultPrecision);
   return;
}

void BtLineEdit::setText(double amount, int precision) {
   this->setWidgetText(this->displayAmount(amount, precision));
   this->setDisplaySize();
   return;
}

void BtLineEdit::setText(NamedEntity * element) {
   this->setText(element, this->defaultPrecision);
   return;
}

void BtLineEdit::setText(NamedEntity * element, int precision) {
   QString display;

   char const * const propertyName = this->editField.toLatin1().constData();
   QVariant const propertyValue = element->property(propertyName);
   qDebug() <<
      Q_FUNC_INFO << "Read property" << this->editField << "(" << propertyName << ") of" << *element << "as" <<
      propertyValue;
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

void BtLineEdit::setText(QString amount) {
   this->setText(amount, this->defaultPrecision);
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
      this->setWidgetText(this->displayAmount(amt, precision));
   }

   this->setDisplaySize(force);
   return;
}

void BtLineEdit::setText(QVariant amount) {
   this->setText(amount, this->defaultPrecision);
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

BtGenericEdit       ::BtGenericEdit       (QWidget *parent) : BtLineEdit(parent, NonPhysicalQuantity::String                  , nullptr                                   ) { return; }
BtMassEdit          ::BtMassEdit          (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Mass          , &Measurement::Units::kilograms            ) { return; }
BtVolumeEdit        ::BtVolumeEdit        (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Volume        , &Measurement::Units::liters               ) { return; }
BtTimeEdit          ::BtTimeEdit          (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Time          , &Measurement::Units::minutes           , 3) { return; }
BtTemperatureEdit   ::BtTemperatureEdit   (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Temperature   , &Measurement::Units::celsius           , 1) { return; }
BtColorEdit         ::BtColorEdit         (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Color         , &Measurement::Units::srm                  ) { return; }
BtDensityEdit       ::BtDensityEdit       (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Density       , &Measurement::Units::sp_grav              ) { return; }
BtDiastaticPowerEdit::BtDiastaticPowerEdit(QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::DiastaticPower, &Measurement::Units::lintner              ) { return; }
BtAcidityEdit       ::BtAcidityEdit       (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Acidity       , &Measurement::Units::pH                   ) { return; }
BtBitternessEdit    ::BtBitternessEdit    (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Bitterness    , &Measurement::Units::ibu                  ) { return; }
BtCarbonationEdit   ::BtCarbonationEdit   (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Carbonation   , &Measurement::Units::carbonationVolumes   ) { return; }
BtConcentrationEdit ::BtConcentrationEdit (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Concentration , &Measurement::Units::partsPerMillion      ) { return; }
BtViscosityEdit     ::BtViscosityEdit     (QWidget *parent) : BtLineEdit(parent, Measurement::PhysicalQuantity::Viscosity     , &Measurement::Units::centipoise           ) { return; }
BtStringEdit        ::BtStringEdit        (QWidget *parent) : BtLineEdit(parent, NonPhysicalQuantity::String                  , nullptr                                   ) { return; }
BtPercentageEdit    ::BtPercentageEdit    (QWidget *parent) : BtLineEdit(parent, NonPhysicalQuantity::Percentage              , nullptr                                , 0) { return; }
BtDimensionlessEdit ::BtDimensionlessEdit (QWidget *parent) : BtLineEdit(parent, NonPhysicalQuantity::Dimensionless           , nullptr                                , 3) { return; }

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
