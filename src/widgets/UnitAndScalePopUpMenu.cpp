/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/UnitAndScalePopUpMenu.cpp is part of Brewtarget, and is copyright the following authors 2012-2023:
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

#include <QApplication>
#include <QDebug>

#include "measurement/Measurement.h"
#include "measurement/SystemOfMeasurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"

namespace {
   /**
    * \brief Used by \c generateAction to put \c std::optional<Measurement::SystemOfMeasurement> or
    *        \c std::optional<Measurement::UnitSystem::RelativeScale> into a QVariant
    */
   template<typename T>
   QVariant toQVariant(std::optional<T> data) {
      //
      // We could use a null-value QVariant to represent "default", eg by constructing QVariant(QVariant::Int) but it's
      // a bit risky as, if you forget to check, an int QVariant will convert null to 0.  Eg operator== returns true
      // for the test QVariant(QVariant::Int) == QVariant(0) because a null state QVariant "if accessed will return a
      // default constructed value of the type".
      //
      // So, instead, we use a negative value int to mean "default" and assert that none of the enums we're using here
      // takes a negative value.
      //
      // Either way, users of UnitAndScalePopUpMenu don't need to know or worry about how data is stored in menu items.
      //
      if (data) {
         int raw = static_cast<int>(*data);
         Q_ASSERT(raw >= 0);
         return QVariant(raw);
      }
      return QVariant(-1);
   }

   /**
    * \brief Used by UnitAndScalePopUpMenu constructor
    *
    * \param menu The menu to which we are adding an item/action
    * \param text The translated descriptive text for the item/action
    * \param data The data to return/store if this item/action is clicked by the user
    * \param currentVal The data for the currently selected item/action
    * \param actionGroup
    */
   template<typename T>
   void generateAction(QMenu * menu,
                       QString text,
                       std::optional<T> data,
                       std::optional<T> currentVal,
                       QActionGroup* actionGroup) {
      QAction * action = new QAction(menu);

      action->setText(text);
      action->setData(toQVariant(data));
      action->setCheckable(true);
      action->setChecked(currentVal == data);
      if (actionGroup) {
         actionGroup->addAction(action);
      }

      menu->addAction(action);
      return;
   }
}

template<typename T>
std::optional<T> UnitAndScalePopUpMenu::dataFromQAction(QAction const & action) {
   QVariant const data = action.data();
   Q_ASSERT(data.type() == QVariant::Int);
   int raw = data.toInt();
   if (raw < 0) {
      qDebug() << Q_FUNC_INFO << "Raw" << raw << "= null";
      return std::nullopt;
   }
   qDebug() << Q_FUNC_INFO << "Raw" << raw << "=" << static_cast<T>(raw);
   return static_cast<T>(raw);
}
//
// Instantiate the above template function for the types that are going to use it
// (This is just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template std::optional<Measurement::SystemOfMeasurement> UnitAndScalePopUpMenu::dataFromQAction(QAction const & action);
template std::optional<Measurement::UnitSystem::RelativeScale> UnitAndScalePopUpMenu::dataFromQAction(QAction const & action);

std::unique_ptr<QMenu> UnitAndScalePopUpMenu::create(QWidget * parent,
                                                     Measurement::PhysicalQuantities physicalQuantities,
                                                     std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement,
                                                     std::optional<Measurement::UnitSystem::RelativeScale> forcedRelativeScale) {
   std::unique_ptr<QMenu> menu = std::make_unique<QMenu>(parent);

   // We are OK to use raw pointers here because menu will own actionGroup and menu's destructor will destroy it.  Since
   // we want actionGroup and menu to have the same lifetime, there is no mileage in adding a smart pointer wrapper.
   QActionGroup * actionGroup = new QActionGroup{menu.get()};

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
   auto unitSystems = Measurement::UnitSystem::getUnitSystems(physicalQuantity);
   if (unitSystems.size() > 1) {
      generateAction(menu.get(),
                     QApplication::translate("UnitAndScalePopUpMenu", "Default"),
                     std::optional<Measurement::SystemOfMeasurement>{std::nullopt},
                     forcedSystemOfMeasurement,
                     actionGroup);
      for (auto system : unitSystems) {
         generateAction(menu.get(),
                        Measurement::getDisplayName(system->systemOfMeasurement),
                        std::optional<Measurement::SystemOfMeasurement>{system->systemOfMeasurement},
                        forcedSystemOfMeasurement,
                        actionGroup);
      }
   }

   // Don't even think about a scale menu for "mixed" as there isn't a sensible way to combine the Mass and Volume
   // scales (or the MassConcentration and VolumeConcentration ones)!
   if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(physicalQuantities)) {
      return menu;
   }

   // If the UnitSystem/SystemOfMeasurement currently used to display the field has more than one Unit, allow the user
   // to select a forced Unit for the scale
   Measurement::UnitSystem const & unitSystem{
      forcedSystemOfMeasurement ?
         Measurement::UnitSystem::getInstance(*forcedSystemOfMeasurement, physicalQuantity) :
         Measurement::getDisplayUnitSystem(physicalQuantity)
   };
   auto relativeScales = unitSystem.getRelativeScales();
   if (relativeScales.size() > 1) {
      QMenu * subMenu = new QMenu(menu.get());
      generateAction(subMenu,
                     QApplication::translate("UnitAndScalePopUpMenu", "Default"),
                     std::optional<Measurement::UnitSystem::RelativeScale>{std::nullopt},
                     forcedRelativeScale,
                     actionGroup);
      for (auto scale : relativeScales) {
         generateAction(subMenu,
                        unitSystem.scaleUnit(scale)->name,
                        std::optional<Measurement::UnitSystem::RelativeScale>{scale},
                        forcedRelativeScale,
                        actionGroup);
      }
      subMenu->setTitle(QApplication::translate("UnitSystem", "Scale"));
      menu->addMenu(subMenu);
   }

   return menu;
}
