/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartLabel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
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
#include "widgets/SmartField.h"
#include "widgets/UnitAndScalePopUpMenu.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_SmartLabel.cpp"

// This private implementation class holds all private non-virtual members of SmartLabel
class SmartLabel::impl {
public:
   impl(SmartLabel & self,
        QWidget * parent) :
      m_self        {self   },
      m_initialised {false},
      m_labelFqName {"Uninitialised m_labelFqName!"},
      m_settings    {nullptr},
      m_parent      {parent } {
      return;
   }

   ~impl() = default;

   SmartLabel &     m_self       ;
   bool             m_initialised;
   char const *     m_labelFqName;
   std::unique_ptr<SmartAmountSettings> m_settings;
   QWidget *        m_parent     ;
};

SmartLabel::SmartLabel(QWidget * parent) :
   QLabel{parent},
   SmartBase<SmartLabel>{},
   pimpl {std::make_unique<impl>(*this, parent)} {
   connect(this, &QWidget::customContextMenuRequested, this, &SmartLabel::popContextMenu);
   return;
}

SmartLabel::~SmartLabel() = default;

void SmartLabel::init(char const * const   editorName,
                      char const * const   labelName,
                      char const * const   labelFqName,
                      [[maybe_unused]] SmartField *         smartField,
                      TypeInfo     const & typeInfo) {
//   qDebug() << Q_FUNC_INFO << labelFqName << ":" << typeInfo;

   this->pimpl->m_labelFqName = labelFqName;
   this->pimpl->m_settings    = std::make_unique<SmartAmountSettings>(editorName, labelName, typeInfo, nullptr);
   this->pimpl->m_initialised = true;
   return;
}

[[nodiscard]] bool SmartLabel::isInitialised() const {
  return this->pimpl->m_initialised;
}

[[nodiscard]] SmartAmountSettings const & SmartLabel::settings() const {
   Q_ASSERT(this->pimpl->m_initialised);
   Q_ASSERT(this->pimpl->m_settings);
   return *this->pimpl->m_settings.get();
}

void SmartLabel::correctEnteredText(SmartAmounts::ScaleInfo previousScaleInfo) {
   emit changedSystemOfMeasurementOrScale(previousScaleInfo);
   return;
}

double SmartLabel::getAmountToDisplay(double const canonicalValue) const {
   Q_ASSERT(this->pimpl->m_initialised);
   // It's a coding error to call this for NonPhysicalQuantity, and we assert we never have a Mixed2PhysicalQuantities
   // for a SmartLabel that has no associated SmartField.
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->getTypeInfo().fieldType));

   auto const & canonicalUnit{
      Measurement::Unit::getCanonicalUnit(std::get<Measurement::PhysicalQuantity>(*this->getTypeInfo().fieldType))
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
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->getTypeInfo().fieldType));
   auto const & canonicalUnit{
      Measurement::Unit::getCanonicalUnit(std::get<Measurement::PhysicalQuantity>(*this->getTypeInfo().fieldType))
   };
   auto const forcedSystemOfMeasurement = this->getForcedSystemOfMeasurement();
   auto const forcedRelativeScale       = this->getForcedRelativeScale();
   return QPair<double, double>(
      Measurement::amountDisplay(Measurement::Amount{canonicalValueMin, canonicalUnit}, forcedSystemOfMeasurement, forcedRelativeScale),
      Measurement::amountDisplay(Measurement::Amount{canonicalValueMax, canonicalUnit}, forcedSystemOfMeasurement, forcedRelativeScale)
   );
}

void SmartLabel::enterEvent([[maybe_unused]] QEnterEvent * event) {
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
   // Uncomment the next command for diagnosing asserts!
//   qDebug().noquote() << Q_FUNC_INFO << "Text:" << this->text() << ", Stack trace:" << Logging::getStackTrace();
   Q_ASSERT(this->pimpl->m_initialised);

   // If we are a label for a NonPhysicalQuantity, then we don't want the underline effect as there are no scale choices
   // for the user to make.
   if (std::holds_alternative<NonPhysicalQuantity>(*this->getTypeInfo().fieldType)) {
      return;
   }

   QFont myFont = this->font();
   myFont.setUnderline(enabled);
   this->setFont(myFont);
   return;
}

void SmartLabel::popContextMenu(const QPoint& point) {
   Q_ASSERT(this->pimpl->m_initialised);

   // For the moment, at least, we do not allow people to choose date formats per-field.  (Although you might want to
   // mix and match metric and imperial systems in certain circumstances, it's less clear that there's a benefit to
   // mixing and matching date formats.)
   if (std::holds_alternative<NonPhysicalQuantity>(*this->getTypeInfo().fieldType)) {
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

   //
   // We always make a new context menu.  It's simpler as we couldn't always reuse an existing menu because the
   // sub-menu for relative scale needs to change when a different unit system is selected.
   //
   // NB: Although the existing menu is "owned" by this->m_parent, it is fine for us (or rather the smart pointer) to
   // delete it here.  The Qt ownership in this context merely guarantees that this->m_parent will, in its own
   // destructor, delete the menu if it still exists.
   //

   auto forcedSystemOfMeasurement = this->getForcedSystemOfMeasurement();
   auto forcedRelativeScale       = this->getForcedRelativeScale();
   qDebug() <<
      Q_FUNC_INFO << this->pimpl->m_labelFqName << "forcedSystemOfMeasurement=" << forcedSystemOfMeasurement <<
      ", forcedRelativeScale=" << forcedRelativeScale;

   auto const physicalQuantity = this->getPhysicalQuantity();
   std::unique_ptr<QMenu> menu = UnitAndScalePopUpMenu::create(this->pimpl->m_parent,
                                                               physicalQuantity,
                                                               forcedSystemOfMeasurement,
                                                               forcedRelativeScale);

   // If the pop-up menu has no entries, then we can bail out here
   if (menu->actions().size() == 0) {
      qDebug() << Q_FUNC_INFO << "Nothing to show for" << this->getPhysicalQuantity();
   }

   // Show the pop-up menu and get back whatever the user seleted
   QAction * invoked = menu->exec(widgie->mapToGlobal(point));
   if (invoked == nullptr) {
      return;
   }

   // Save the current settings (which may come from system-wide defaults) for the signal below
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->getTypeInfo().fieldType));
   SmartAmounts::ScaleInfo const previousScaleInfo = this->getScaleInfo();

   // User will either have selected a SystemOfMeasurement or a UnitSystem::RelativeScale.  We can know which based on
   // whether it's the menu or the sub-menu that it came from.
   bool isTopMenu{invoked->parent() == menu.get()};
   if (isTopMenu) {
      // It's the menu, so SystemOfMeasurement
      std::optional<Measurement::SystemOfMeasurement> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::SystemOfMeasurement>(*invoked);
      qDebug() << Q_FUNC_INFO << this->pimpl->m_labelFqName << "Selected SystemOfMeasurement" << whatSelected;
      // This stores the new SystemOfMeasurement (if any), but modification of the field contents
      // won't happen until it receives the signal sent below.
      this->setForcedSystemOfMeasurement(whatSelected);
      // Choosing a forced SystemOfMeasurement resets any selection of forced RelativeScale
      this->setForcedRelativeScale(std::nullopt);
   } else {
      // It's the sub-menu, so UnitSystem::RelativeScale
      std::optional<Measurement::UnitSystem::RelativeScale> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::UnitSystem::RelativeScale>(*invoked);
      qDebug() << Q_FUNC_INFO << this->pimpl->m_labelFqName << "Selected RelativeScale" << whatSelected;
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
