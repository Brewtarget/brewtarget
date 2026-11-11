/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/UnitAndScalePopUpMenu.cpp is part of Brewtarget, and is copyright the following authors 2012-2026:
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "widgets/UnitAndScalePopUpMenu.h"

#include <QActionGroup>
#include <QApplication>
#include <QDebug>

#include "measurement/Measurement.h"
#include "measurement/SystemOfMeasurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "utils/TypeInfo.h"

namespace {
   /**
    * \brief Used by \c UnitAndScalePopUpMenu::create to add an item to a menu
    *
    *        We store the whole \c UnitAndScalePopUpMenu::Selection in the item's data.  This means that, when the user
    *        picks an item, we know not just its value but also which (sub-)menu it came from, without having to look at
    *        the menu hierarchy or names.  It also avoids having to encode "Default" (ie \c std::nullopt) as some special
    *        int value.
    *
    * \param menu The menu to which we are adding an item/action
    * \param text The translated descriptive text for the item/action
    * \param data The data to return/store if this item/action is clicked by the user
    * \param currentVal The data for the currently selected item/action
    * \param actionGroup The (exclusive) group for the items in \c menu.  Each (sub-)menu needs its own group, otherwise
    *                    only one item in the whole menu hierarchy could be checked.
    */
   template<typename T>
   void generateAction(QMenu & menu,
                       QString const & text,
                       std::optional<T> const data,
                       std::optional<T> const currentVal,
                       QActionGroup & actionGroup) {
      // Using new here is OK as menu will own this QAction
      QAction * action = new QAction(&menu);

      action->setText(text);
      action->setData(QVariant::fromValue(UnitAndScalePopUpMenu::Selection{data}));
      action->setCheckable(true);
      action->setChecked(currentVal == data);
      actionGroup.addAction(action);

      menu.addAction(action);
      return;
   }

   /**
    * \brief Make a new action group for \c menu.  We are OK to use raw pointers here because \c menu will own the
    *        new \c QActionGroup and its destructor will destroy it.
    */
   QActionGroup & newActionGroup(QMenu & menu) {
      return *new QActionGroup{&menu};
   }

   /**
    * \brief Add a sub-menu to allow the user to override the number of decimal places shown in a field.  (See comment
    *        on physicalQuantityToUserSpecifiedDecimalPlaces in measurement/Measurement.cpp for the hierarchy of
    *        preferences for number of decimal places shown.)
    */
   void addDecimalPlacesSubMenu(QMenu & menu, std::optional<Measurement::NumDecimalPlaces> const forcedDecimalPlaces) {
      // As in UnitAndScalePopUpMenu::create, menu owns the sub-menu, so it's fine to use new here
      QMenu & decimalPlacesSubMenu = *new QMenu(&menu);
      QActionGroup & actionGroup = newActionGroup(decimalPlacesSubMenu);
      generateAction(decimalPlacesSubMenu,
                     QApplication::translate("UnitAndScalePopUpMenu", "Default"),
                     std::optional<Measurement::NumDecimalPlaces>{std::nullopt},
                     forcedDecimalPlaces,
                     actionGroup);
      for (int numDecimalPlaces = 0; numDecimalPlaces <= Measurement::NumDecimalPlaces::maxValue; ++numDecimalPlaces) {
         generateAction(decimalPlacesSubMenu,
                        QString::number(numDecimalPlaces),
                        std::optional<Measurement::NumDecimalPlaces>{Measurement::NumDecimalPlaces{numDecimalPlaces}},
                        forcedDecimalPlaces,
                        actionGroup);
      }
      decimalPlacesSubMenu.setTitle(QApplication::translate("UnitAndScalePopUpMenu", "Decimal Places"));
      menu.addMenu(&decimalPlacesSubMenu);
      return;
   }
}

bool UnitAndScalePopUpMenu::isAvailableFor(TypeInfo const & typeInfo) {
   // It's a coding error to call this for a field with no type
   Q_ASSERT(typeInfo.fieldType);
   if (!std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
      return true;
   }
   NonPhysicalQuantity const nonPhysicalQuantity = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);
   // GetUltimateDefaultDecimalPlaces returns std::nullopt for NonPhysicalQuantity values that don't have decimal places
   return typeInfo.typeIndex == TypeInfo::Index::Double &&
          GetUltimateDefaultDecimalPlaces(nonPhysicalQuantity).has_value();
}

std::unique_ptr<QMenu> UnitAndScalePopUpMenu::create(
   QWidget * parent,
   [[maybe_unused]] NonPhysicalQuantity const nonPhysicalQuantity,
   std::optional<Measurement::NumDecimalPlaces> const forcedDecimalPlaces
) {
   // It's a coding error to ask for a menu for something that can't have decimal places
   Q_ASSERT(GetUltimateDefaultDecimalPlaces(nonPhysicalQuantity).has_value());
   std::unique_ptr<QMenu> menu = std::make_unique<QMenu>(parent);
   addDecimalPlacesSubMenu(*menu, forcedDecimalPlaces);
   return menu;
}

std::optional<UnitAndScalePopUpMenu::Selection> UnitAndScalePopUpMenu::exec(QMenu & menu,
                                                                             QPoint const & globalPosition) {
   // This shows the menu and returns when the user either selects something or dismisses the menu
   QAction const * invoked = menu.exec(globalPosition);
   if (!invoked) {
      return std::nullopt;
   }

   QVariant const data = invoked->data();
   // It's a coding error if the action wasn't created by generateAction
   Q_ASSERT(data.canConvert<UnitAndScalePopUpMenu::Selection>());
   auto const selection = data.value<UnitAndScalePopUpMenu::Selection>();
   qDebug() << Q_FUNC_INFO << "Selected item" << invoked->text() << "from menu" << selection.index();
   return selection;
}

std::unique_ptr<QMenu> UnitAndScalePopUpMenu::create(
   QWidget * parent,
   Measurement::PhysicalQuantities const physicalQuantities,
   std::optional<Measurement::SystemOfMeasurement      > const forcedSystemOfMeasurement,
   std::optional<Measurement::UnitSystem::RelativeScale> const forcedRelativeScale,
   std::optional<Measurement::NumDecimalPlaces         > const forcedDecimalPlaces
) {
   std::unique_ptr<QMenu> menu = std::make_unique<QMenu>(parent);

   //
   // In most circumstances, for a given PhysicalQuantity, it would be simplest to largely ignore SystemOfMeasurement
   // and deal only with UnitSystem and Unit.  This is because, aside from  the exception we are about to discuss, there
   // is a one-to-one correspondence between UnitSystem  and the pair (SystemOfMeasurement, PhysicalQuantity).  So, for
   // any particular field holding a given PhysicalQuantity, offering the user a choice of SystemOfMeasurement implies
   // the corresponding choice of UnitSystem.  Equally, a choice of RelativeScale corresponds to this UnitSystem and
   // UnitSystem + RelativeScale gives us a Unit.
   //
   // However, we have to handle the special case of ChoiceOfPhysicalQuantity, which means the user has the choice to
   // measure two different ways, eg by mass or by volume, on a per-item basis.  This is useful because, eg some Misc
   // ingredients are best measured by volume and others by mass.  Similarly, dry yeast is probably measured by mass
   // whereas wet yeast is usually measured by volume.
   //
   // For ChoiceOfPhysicalQuantity, where PhysicalQuantity varies per-item between two possibilities (eg Mass and
   // Volume), the choice of SystemOfMeasurement is going to imply a different UnitSystem per-item depending on, eg
   // Misc::amountIsWeight, Yeast::amountIsWeight, etc for that item.  So this is why we select/store
   // SystemOfMeasurement rather than UnitSystem.  We nonetheless get to SystemOfMeasurement via UnitSystem because it
   // is the latter class that holds all the various relations.
   //
   // Also, in the case of ChoiceOfPhysicalQuantity, it doesn't make sense to offer the user a choice of RelativeScale,
   // so we suppress that option.
   //

   // If we have > 1 UnitSystem/SystemOfMeasurement for the PhysicalQuantity then we want the user to be able to select
   // between them.  NOTE for physicalQuantities == ChoiceOfPhysicalQuantity we currently assume that it does not matter
   // which of the two PhysicalQuantity values we go via to get the result.  This is true for Mass & Volume (because
   // they share UnitSystems) and for MassConcentration & VolumeConcentration (because they each only have one
   // UnitSystem).  If we find cases where this is not true, then we'd need to rethink the UI here a bit.
   Measurement::PhysicalQuantity const physicalQuantity =
      std::holds_alternative<Measurement::PhysicalQuantity>(physicalQuantities) ?
         std::get<Measurement::PhysicalQuantity>(physicalQuantities) :
         Measurement::defaultPhysicalQuantity(std::get<Measurement::ChoiceOfPhysicalQuantity>(physicalQuantities));
   if (auto const unitSystems = Measurement::UnitSystem::getUnitSystems(physicalQuantity);
       unitSystems.size() > 1) {
      QActionGroup & actionGroup = newActionGroup(*menu);
      generateAction(*menu,
                     QApplication::translate("UnitAndScalePopUpMenu", "Default"),
                     std::optional<Measurement::SystemOfMeasurement>{std::nullopt},
                     forcedSystemOfMeasurement,
                     actionGroup);
      for (auto const system : unitSystems) {
         generateAction(*menu,
                        Measurement::getDisplayName(system->systemOfMeasurement),
                        std::optional<Measurement::SystemOfMeasurement>{system->systemOfMeasurement},
                        forcedSystemOfMeasurement,
                        actionGroup);
      }
   }

   // If the UnitSystem/SystemOfMeasurement currently used to display the field has more than one Unit, allow the user
   // to select a forced Unit for the scale.
   //
   // But don't even think about a scale menu for "mixed" as there isn't a sensible way to combine the Mass and Volume
   // scales (or the MassConcentration and VolumeConcentration ones)!  (We do still want the decimal places sub-menu
   // below though.)
   Measurement::UnitSystem const & unitSystem{
      forcedSystemOfMeasurement ?
         Measurement::UnitSystem::getInstance(*forcedSystemOfMeasurement, physicalQuantity) :
         Measurement::getDisplayUnitSystem(physicalQuantity)
   };
   if (auto const relativeScales = unitSystem.getRelativeScales();
       std::holds_alternative<Measurement::PhysicalQuantity>(physicalQuantities) && relativeScales.size() > 1) {
      QMenu & scalesSubMenu = *new QMenu(menu.get());
      QActionGroup & actionGroup = newActionGroup(scalesSubMenu);
      generateAction(scalesSubMenu,
                     QApplication::translate("UnitAndScalePopUpMenu", "Default"),
                     std::optional<Measurement::UnitSystem::RelativeScale>{std::nullopt},
                     forcedRelativeScale,
                     actionGroup);
      for (auto scale : relativeScales) {
         generateAction(scalesSubMenu,
                        unitSystem.scaleUnit(scale)->getDisplayAbbreviation(),
                        std::optional<Measurement::UnitSystem::RelativeScale>{scale},
                        forcedRelativeScale,
                        actionGroup);
      }
      scalesSubMenu.setTitle(QApplication::translate("UnitSystem", "Scale"));
      menu->addMenu(&scalesSubMenu);
   }

   // Allow the user to override the number of decimal places shown in this field
   addDecimalPlacesSubMenu(*menu, forcedDecimalPlaces);

   return menu;
}