/*
 * widgets/SmartLabel.cpp is part of Brewtarget, and is copyright the following authors 2009-2023:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "widgets/SmartLabel.h"

#include <QSettings>
#include <QDebug>
#include <QMouseEvent>
#include <QVector>

#include "Logging.h"
#include "measurement/Measurement.h"
#include "model/Style.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/OptionalHelpers.h"
#include "SmartField.h"
#include "widgets/UnitAndScalePopUpMenu.h"

// This private implementation class holds all private non-virtual members of SmartLabel
class SmartLabel::impl {
public:
   impl(SmartLabel & self,
        QWidget * parent) :
      m_self        {self   },
      m_initialised {false},
      m_editorName  {"Uninitialised m_editorName!" },
      m_labelName   {"Uninitialised m_labelName!"  },
      m_labelFqName {"Uninitialised m_labelFqName!"},
      m_typeInfo    {nullptr},
      m_parent      {parent },
      m_contextMenu {nullptr} {
      return;
   }

   ~impl() = default;

   void initializeMenu() {
      // TODO: Change this to a smart pointer
      //
      // If a context menu already exists, we need to delete it and recreate it.  We can't always reuse an existing menu
      // because the sub-menu for relative scale needs to change when a different unit system is selected.  (In theory we
      // could only recreate the context menu when a different unit system is selected, but that adds complication.)
      if (this->m_contextMenu) {
         // NB: Although the existing menu is "owned" by this->m_parent, it is fine for us to delete it here.  The Qt
         // ownership in this context merely guarantees that this->m_parent will, in its own destructor, delete the menu if
         // it still exists.
         delete this->m_contextMenu;
         this->m_contextMenu = nullptr;
      }

      auto forcedSystemOfMeasurement = this->m_self.getForcedSystemOfMeasurement();
      auto forcedRelativeScale       = this->m_self.getForcedRelativeScale();
      qDebug() <<
         Q_FUNC_INFO << "forcedSystemOfMeasurement=" << forcedSystemOfMeasurement << ", forcedRelativeScale=" <<
         forcedRelativeScale;

      if (std::holds_alternative<NonPhysicalQuantity>(*this->m_typeInfo->fieldType)) {
         return;
      }

      // Since fieldType is not NonPhysicalQuantity this cast should be safe
      Measurement::PhysicalQuantity const physicalQuantity =
         std::get<Measurement::PhysicalQuantity>(*this->m_typeInfo->fieldType);
      this->m_contextMenu = UnitAndScalePopUpMenu::create(this->m_parent,
                                                          physicalQuantity,
                                                          forcedSystemOfMeasurement,
                                                          forcedRelativeScale);
      return;
   }

   SmartLabel &             m_self       ;
   bool                     m_initialised;
   char const *             m_editorName ;
   char const *             m_labelName  ;
   char const *             m_labelFqName;
   TypeInfo const *         m_typeInfo   ;
   QWidget *                m_parent     ;
   QMenu *                  m_contextMenu;
};

SmartLabel::SmartLabel(QWidget * parent) :
   QLabel{parent},
   pimpl {std::make_unique<impl>(*this, parent)} {
   connect(this, &QWidget::customContextMenuRequested, this, &SmartLabel::popContextMenu);
   return;
}

SmartLabel::~SmartLabel() = default;

void SmartLabel::init(char const * const   editorName,
                      char const * const   labelName,
                      char const * const   labelFqName,
                      SmartField *         smartField,
                      TypeInfo     const & typeInfo) {
   qDebug() << Q_FUNC_INFO << labelFqName << ":" << typeInfo;

   this->pimpl->m_editorName  = editorName ;
   this->pimpl->m_labelName   = labelName  ;
   this->pimpl->m_labelFqName = labelFqName;
   this->pimpl->m_typeInfo    = &typeInfo  ;
///   if (smartField) {
///      this->pimpl->m_buddies.append(smartField);
///      this->setBuddy(smartField);
///   }
   this->pimpl->m_initialised = true;
   return;
}

[[nodiscard]] bool SmartLabel::isInitialised() const {
  return this->pimpl->m_initialised;
}

///SmartField & SmartLabel::getBuddy() const {
///   Q_ASSERT(this->pimpl->m_initialised);
///
///   // Call QLabel's built-in function to get the buddy
///   QWidget * buddy = this->buddy();
///
///   // We assert that it's a coding error for there not to be a buddy!
///   Q_ASSERT(buddy);
///
///   auto & smartBuddy = static_cast<SmartField &>(*buddy);
///
///   // We assert that the buddy was set via our init function.  (It's OK if it was also set directly via the QLabel
///   // buddy property.)
///   Q_ASSERT(this->pimpl->m_buddies.contains(&smartBuddy));
///
///   return smartBuddy;
///}

void SmartLabel::setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement) {
   SmartAmounts::setForcedSystemOfMeasurement(this->pimpl->m_editorName, this->pimpl->m_labelName, systemOfMeasurement);
   return;
}

void SmartLabel::setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> relativeScale) {
   SmartAmounts::setForcedRelativeScale(this->pimpl->m_editorName, this->pimpl->m_labelName, relativeScale);
   return;
}

std::optional<Measurement::SystemOfMeasurement> SmartLabel::getForcedSystemOfMeasurement() const {
   return SmartAmounts::getForcedSystemOfMeasurement(this->pimpl->m_editorName, this->pimpl->m_labelName);
}

std::optional<Measurement::UnitSystem::RelativeScale> SmartLabel::getForcedRelativeScale() const {
   return SmartAmounts::getForcedRelativeScale(this->pimpl->m_editorName, this->pimpl->m_labelName);
}

SmartAmounts::ScaleInfo SmartLabel::getScaleInfo() const {
   Q_ASSERT(this->pimpl->m_initialised);
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));
   return SmartAmounts::getScaleInfo(this->pimpl->m_editorName,
                                     this->pimpl->m_labelName,
                                     ConvertToPhysicalQuantities(*this->pimpl->m_typeInfo->fieldType));
}

Measurement::UnitSystem const & SmartLabel::getDisplayUnitSystem() const {
   Q_ASSERT(this->pimpl->m_initialised);
   // It's a coding error to call this for NonPhysicalQuantity, and we assert we never have a Mixed2PhysicalQuantities
   // for a SmartLabel that has no associated SmartField.
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));
   return SmartAmounts::getUnitSystem(this->pimpl->m_editorName,
                                      this->pimpl->m_labelName,
                                      std::get<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));
}

double SmartLabel::getAmountToDisplay(double const canonicalValue) const {
   Q_ASSERT(this->pimpl->m_initialised);
   // It's a coding error to call this for NonPhysicalQuantity, and we assert we never have a Mixed2PhysicalQuantities
   // for a SmartLabel that has no associated SmartField.
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));

   auto const & canonicalUnit{
      Measurement::Unit::getCanonicalUnit(std::get<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType))
   };
   return Measurement::amountDisplay(
      Measurement::Amount{canonicalValue, canonicalUnit},
      this->getForcedSystemOfMeasurement(),
      this->getForcedRelativeScale()
   );
}

QPair<double,double> SmartLabel::getRangeToDisplay(double const canonicalValueMin,
                                                   double const canonicalValueMax) const {
   // Only need next bit for debugging!
//   if (!this->pimpl->m_initialised) {
//      qCritical() << Q_FUNC_INFO << this;
//      qCritical().noquote() << Q_FUNC_INFO << this->pimpl->m_labelFqName << "Stack trace:" << Logging::getStackTrace();
//   }
   Q_ASSERT(this->pimpl->m_initialised);
   // It's a coding error to call this for NonPhysicalQuantity, and we assert we never have a Mixed2PhysicalQuantities
   // for a SmartLabel that has no associated SmartField.
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));
   auto const & canonicalUnit{
      Measurement::Unit::getCanonicalUnit(std::get<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType))
   };
   auto const forcedSystemOfMeasurement = this->getForcedSystemOfMeasurement();
   auto const forcedRelativeScale       = this->getForcedRelativeScale();
   return QPair<double, double>(
      Measurement::amountDisplay(Measurement::Amount{canonicalValueMin, canonicalUnit}, forcedSystemOfMeasurement, forcedRelativeScale),
      Measurement::amountDisplay(Measurement::Amount{canonicalValueMax, canonicalUnit}, forcedSystemOfMeasurement, forcedRelativeScale)
   );
}

void SmartLabel::enterEvent([[maybe_unused]] QEvent * event) {
   this->textEffect(true);
   return;
}

void SmartLabel::leaveEvent([[maybe_unused]] QEvent * event) {
   this->textEffect(false);
   return;
}

void SmartLabel::mouseReleaseEvent(QMouseEvent * event) {
   // For the moment, we want left-click and right-click to have the same effect, so when we get a left-click event, we
   // send ourselves the right-click signal, which will then fire SmartLabel::popContextMenu().
   emit this->QWidget::customContextMenuRequested(event->pos());
   return;
}

void SmartLabel::textEffect(bool enabled) {
   Q_ASSERT(this->pimpl->m_initialised);

   // If we are a label for a NonPhysicalQuantity, then we don't want the underline effect as there are no scale choices
   // for the user to make.
   if (std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType)) {
      return;
   }

   QFont myFont = this->font();
   myFont.setUnderline(enabled);
   this->setFont(myFont);
   return;
}

///void SmartLabel::initializeSection() {
///   if (!this->pimpl->m_configSection.isEmpty()) {
///      // We're already initialised
///      return;
///   }
///
///   //
///   // If the label has the pimpl->m_configSection defined, use it
///   // otherwise, if the paired field has a pimpl->m_configSection, use it
///   // otherwise, if the parent object has a pimpl->m_configSection, use it
///   // if all else fails, get the parent's object name
///   //
///   if (this->property(*PropertyNames::SmartField::configSection).isValid()) {
///      this->pimpl->m_configSection = this->property(*PropertyNames::SmartField::configSection).toString();
///      return;
///   }
///
///   // As much as I dislike it, dynamic properties can't be referenced on initialization.
///   QWidget const * mybuddy = this->buddy();
///   if (mybuddy && mybuddy->property(*PropertyNames::SmartField::configSection).isValid() ) {
///      this->pimpl->m_configSection = mybuddy->property(*PropertyNames::SmartField::configSection).toString();
///      return;
///   }
///
///   if (this->pimpl->m_parent->property(*PropertyNames::SmartField::configSection).isValid() ) {
///      this->pimpl->m_configSection =
///         this->pimpl->m_parent->property(*PropertyNames::SmartField::configSection).toString();
///      return;
///   }
///
///   qWarning() << Q_FUNC_INFO << "this failed" << this;
///   this->pimpl->m_configSection = this->pimpl->m_parent->objectName();
///   return;
///}
///
///void SmartLabel::initializeProperty() {
///
///   if (!this->pimpl->m_propertyName.isEmpty()) {
///      return;
///   }
///
///   QWidget* mybuddy = this->buddy();
///   if (this->property("editField").isValid()) {
///      this->pimpl->m_propertyName = this->property("editField").toString();
///   } else if (mybuddy && mybuddy->property("editField").isValid()) {
///      this->pimpl->m_propertyName = mybuddy->property("editField").toString();
///   } else {
///      qWarning() << Q_FUNC_INFO  << "That failed miserably";
///   }
///   return;
///}

void SmartLabel::popContextMenu(const QPoint& point) {
   Q_ASSERT(this->pimpl->m_initialised);

   // For the moment, at least, we do not allow people to choose date formats per-field.  (Although you might want to
   // mix and match metric and imperial systems in certain circumstances, it's less clear that there's a benefit to
   // mixing and matching date formats.)
   if (!std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType)) {
      return;
   }

   QObject * calledBy = this->sender();
   if (calledBy == nullptr) {
      return;
   }

   QWidget * widgie = qobject_cast<QWidget*>(calledBy);
   if (widgie == nullptr) {
      return;
   }

   this->pimpl->initializeMenu();

   // Show the pop-up menu and get back whatever the user seleted
   QAction * invoked = this->pimpl->m_contextMenu->exec(widgie->mapToGlobal(point));
   if (invoked == nullptr) {
      return;
   }

   // Save the current settings (which may come from system-wide defaults) for the signal below
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));
   SmartAmounts::ScaleInfo const previousScaleInfo = this->getScaleInfo();

   // User will either have selected a SystemOfMeasurement or a UnitSystem::RelativeScale.  We can know which based on
   // whether it's the menu or the sub-menu that it came from.
   bool isTopMenu{invoked->parentWidget() == this->pimpl->m_contextMenu};
   if (isTopMenu) {
      // It's the menu, so SystemOfMeasurement
      std::optional<Measurement::SystemOfMeasurement> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::SystemOfMeasurement>(*invoked);
      qDebug() << Q_FUNC_INFO << "Selected SystemOfMeasurement" << whatSelected;
      // This stores the new SystemOfMeasurement (if any), but modification of the field contents
      // won't happen until it receives the signal sent below.
      this->setForcedSystemOfMeasurement(whatSelected);
      // Choosing a forced SystemOfMeasurement resets any selection of forced RelativeScale
      this->setForcedRelativeScale(std::nullopt);
   } else {
      // It's the sub-menu, so UnitSystem::RelativeScale
      std::optional<Measurement::UnitSystem::RelativeScale> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::UnitSystem::RelativeScale>(*invoked);
      qDebug() << Q_FUNC_INFO << "Selected RelativeScale" << whatSelected;
      // This stores the new SystemOfMeasurement (if any), but modification of the field contents
      // won't happen until it receives the signal sent below.
      this->setForcedRelativeScale(whatSelected);
   }

   // Remember, we need the original unit, not the new one.
   //
   // Note that the changedSystemOfMeasurementOrScale signal can also be received by other slots than the
   // SmartField::lineChanged.  This is why we use a signal and why we include the SmartAmounts::ScaleInfo data (which
   // SmartField could work out itself).
   emit changedSystemOfMeasurementOrScale(previousScaleInfo);

   return;
}
