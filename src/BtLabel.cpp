/*
 * BtLabel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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
#include "BtLabel.h"

#include <QSettings>
#include <QDebug>
#include <QMouseEvent>

#include "measurement/Measurement.h"
#include "model/Style.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/OptionalHelpers.h"
#include "widgets/UnitAndScalePopUpMenu.h"

BtLabel::BtLabel(QWidget *parent,
                 BtFieldType fieldType) :
   QLabel{parent},
   fieldType{fieldType},
   btParent{parent},
   contextMenu{nullptr} {
   connect(this, &QWidget::customContextMenuRequested, this, &BtLabel::popContextMenu);
   return;
}

BtLabel::~BtLabel() = default;

void BtLabel::enterEvent([[maybe_unused]] QEvent * event) {
   this->textEffect(true);
   return;
}

void BtLabel::leaveEvent([[maybe_unused]] QEvent * event) {
   this->textEffect(false);
   return;
}

void BtLabel::mouseReleaseEvent (QMouseEvent * event) {
   // For the moment, we want left-click and right-click to have the same effect, so when we get a left-click event, we
   // send ourselves the right-click signal, which will then fire BtLabel::popContextMenu().
   emit this->QWidget::customContextMenuRequested(event->pos());
   return;
}

void BtLabel::textEffect(bool enabled) {
   QFont myFont = this->font();
   myFont.setUnderline(enabled);
   this->setFont(myFont);
   return;
}

void BtLabel::initializeSection() {
   if (!this->configSection.isEmpty()) {
      return;
   }

   // as much as I dislike it, dynamic properties can't be referenced on
   // initialization.
   QWidget * mybuddy = this->buddy();

   //
   // If the label has the configSection defined, use it
   // otherwise, if the paired field has a configSection, use it
   // otherwise, if the parent object has a configSection, use it
   // if all else fails, get the parent's object name
   //
   if (this->property("configSection").isValid()) {
      this->configSection = property("configSection").toString();
   } else if (mybuddy && mybuddy->property("configSection").isValid() ) {
      this->configSection = mybuddy->property("configSection").toString();
   } else if (this->btParent->property("configSection").isValid() ) {
      this->configSection = this->btParent->property("configSection").toString();
   } else {
      qWarning() << Q_FUNC_INFO << "this failed" << this;
      this->configSection = this->btParent->objectName();
   }
   return;
}

void BtLabel::initializeProperty() {

   if (!this->propertyName.isEmpty()) {
      return;
   }

   QWidget* mybuddy = this->buddy();
   if (this->property("editField").isValid()) {
      this->propertyName = this->property("editField").toString();
   } else if (mybuddy && mybuddy->property("editField").isValid()) {
      this->propertyName = mybuddy->property("editField").toString();
   } else {
      qWarning() << Q_FUNC_INFO  << "That failed miserably";
   }
   return;
}



void BtLabel::initializeMenu() {
   // If a context menu already exists, we need to delete it and recreate it.  We can't always reuse an existing menu
   // because the sub-menu for relative scale needs to change when a different unit system is selected.  (In theory we
   // could only recreate the context menu when a different unit system is selected, but that adds complication.)
   if (this->contextMenu) {
      // NB: Although the existing menu is "owned" by this->btParent, it is fine for us to delete it here.  The Qt
      // ownership in this context merely guarantees that this->btParent will, in its own destructor, delete the menu if
      // it still exists.
      delete this->contextMenu;
      this->contextMenu = nullptr;
   }

   std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement =
      Measurement::getForcedSystemOfMeasurementForField(this->propertyName, this->configSection);
   std::optional<Measurement::UnitSystem::RelativeScale> forcedRelativeScale =
      Measurement::getForcedRelativeScaleForField(this->propertyName, this->configSection);
   qDebug() <<
      Q_FUNC_INFO << "forcedSystemOfMeasurement=" << forcedSystemOfMeasurement << ", forcedRelativeScale=" <<
      forcedRelativeScale;

   if (!std::holds_alternative<Measurement::PhysicalQuantity>(this->fieldType)) {
      return;
   }

   Measurement::PhysicalQuantity physicalQuantity = std::get<Measurement::PhysicalQuantity>(this->fieldType);

   this->contextMenu = UnitAndScalePopUpMenu::create(this->btParent,
                                                     physicalQuantity,
                                                     forcedSystemOfMeasurement,
                                                     forcedRelativeScale);
   return;
}

void BtLabel::popContextMenu(const QPoint& point) {
   // For the moment, at least, we do not allow people to choose date formats per-field.  (Although you might want to
   // mix and match metric and imperial systems in certain circumstances, it's less clear that there's a benefit to
   // mixing and matching date formats.)
   if (!std::holds_alternative<Measurement::PhysicalQuantity>(this->fieldType)) {
      return;
   }

   QObject* calledBy = sender();
   if (calledBy == nullptr) {
      return;
   }

   QWidget * widgie = qobject_cast<QWidget*>(calledBy);
   if (widgie == nullptr) {
      return;
   }

   this->initializeProperty();
   this->initializeSection();
   this->initializeMenu();

   // Show the pop-up menu and get back whatever the user seleted
   QAction * invoked = this->contextMenu->exec(widgie->mapToGlobal(point));
   if (invoked == nullptr) {
      return;
   }

   // Save the current settings (which may come from system-wide defaults) for the signal below
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(this->fieldType));
   Measurement::PhysicalQuantity physicalQuantity = std::get<Measurement::PhysicalQuantity>(this->fieldType);
   PreviousScaleInfo previousScaleInfo{
      Measurement::getSystemOfMeasurementForField(this->propertyName, this->configSection, physicalQuantity),
      Measurement::getForcedRelativeScaleForField(this->propertyName, this->configSection)
   };

   // To make this all work, we need to set ogMin and ogMax when og is set etc
   QVector<QString> fieldsToSet;
   fieldsToSet.append(this->propertyName);
   if (this->propertyName == "og") {
      fieldsToSet.append(QString(*PropertyNames::Style::ogMin));
      fieldsToSet.append(QString(*PropertyNames::Style::ogMax));
   } else if (this->propertyName == "fg") {
      fieldsToSet.append(QString(*PropertyNames::Style::fgMin));
      fieldsToSet.append(QString(*PropertyNames::Style::fgMax));
   } else if (this->propertyName == "color_srm") {
      fieldsToSet.append(QString(*PropertyNames::Style::colorMin_srm));
      fieldsToSet.append(QString(*PropertyNames::Style::colorMax_srm));
   }

   // User will either have selected a SystemOfMeasurement or a UnitSystem::RelativeScale.  We can know which based on
   // whether it's the menu or the sub-menu that it came from.
   bool isTopMenu{invoked->parentWidget() == this->contextMenu};
   if (isTopMenu) {
      // It's the menu, so SystemOfMeasurement
      std::optional<Measurement::SystemOfMeasurement> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::SystemOfMeasurement>(*invoked);
      qDebug() << Q_FUNC_INFO << "Selected SystemOfMeasurement" << whatSelected;
      if (!whatSelected) {
         // Null means "Default", which means don't set a forced SystemOfMeasurement for this field
         for (auto field : fieldsToSet) {
            Measurement::setForcedSystemOfMeasurementForField(field, this->configSection, std::nullopt);
         }
      } else {
         for (auto field : fieldsToSet) {
            Measurement::setForcedSystemOfMeasurementForField(field, this->configSection, *whatSelected);
         }
      }
      // Choosing a forced SystemOfMeasurement resets any selection of forced RelativeScale
      for (auto field : fieldsToSet) {
         Measurement::setForcedRelativeScaleForField(field, this->configSection, std::nullopt);
      }

      //
      // Hmm. For the color fields, we want to include the ecb or srm in the label text here.
      //
      // Assert that we already bailed above for fields that aren't a PhysicalQuantity, so we know std::get won't throw
      // here.
      //
      Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(this->fieldType));
      if (Measurement::PhysicalQuantity::Color == std::get<Measurement::PhysicalQuantity>(this->fieldType)) {
         Measurement::UnitSystem const & disp =
            Measurement::getUnitSystemForField(this->propertyName,
                                               this->configSection,
                                               Measurement::PhysicalQuantity::Color);
         this->setText(tr("Color (%1)").arg(disp.unit()->name));
      }
   } else {
      // It's the sub-menu, so UnitSystem::RelativeScale
      std::optional<Measurement::UnitSystem::RelativeScale> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::UnitSystem::RelativeScale>(*invoked);
      qDebug() << Q_FUNC_INFO << "Selected RelativeScale" << whatSelected;
      if (!whatSelected) {
         // Null means "Default", which means don't set a forced RelativeScale for this field
         for (auto field : fieldsToSet) {
            Measurement::setForcedRelativeScaleForField(field, this->configSection, std::nullopt);
         }
      } else {
         for (auto field : fieldsToSet) {
            Measurement::setForcedRelativeScaleForField(field, this->configSection, *whatSelected);
         }
      }
   }

   // Remember, we need the original unit, not the new one.
   emit changedSystemOfMeasurementOrScale(previousScaleInfo);

   return;
}

BtColorLabel::BtColorLabel(QWidget *parent) :                   BtLabel(parent, Measurement::PhysicalQuantity::Color)          { return; }
BtDateLabel::BtDateLabel(QWidget *parent) :                     BtLabel(parent, NonPhysicalQuantity::Date)                     { return; }
BtDensityLabel::BtDensityLabel(QWidget *parent) :               BtLabel(parent, Measurement::PhysicalQuantity::Density)        { return; }
BtMassLabel::BtMassLabel(QWidget *parent) :                     BtLabel(parent, Measurement::PhysicalQuantity::Mass)           { return; }
BtMixedLabel::BtMixedLabel(QWidget *parent) :                   BtLabel(parent, Measurement::PhysicalQuantity::Mixed)          { return; }
BtTemperatureLabel::BtTemperatureLabel(QWidget *parent) :       BtLabel(parent, Measurement::PhysicalQuantity::Temperature)    { return; }
BtTimeLabel::BtTimeLabel(QWidget *parent) :                     BtLabel(parent, Measurement::PhysicalQuantity::Time)           { return; }
BtVolumeLabel::BtVolumeLabel(QWidget *parent) :                 BtLabel(parent, Measurement::PhysicalQuantity::Volume)         { return; }
BtDiastaticPowerLabel::BtDiastaticPowerLabel(QWidget *parent) : BtLabel(parent, Measurement::PhysicalQuantity::DiastaticPower) { return; }
