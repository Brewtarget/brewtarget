/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/UnitAndScalePopUpMenu.h is part of Brewtarget, and is copyright the following authors 2012-2024:
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
#ifndef WIDGETS_UNITANDSCALEPOPUPMENU_H
#define WIDGETS_UNITANDSCALEPOPUPMENU_H
#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

#include <QMenu>
#include <QPoint>

#include "measurement/Measurement.h"
#include "measurement/SystemOfMeasurement.h"
#include "measurement/UnitSystem.h"

namespace UnitAndScalePopUpMenu {
   /**
    * \brief Creates a \c QMenu (possibly with a sub-menu) for specifying/changing a "forced" \c SystemOfMeasurement
    *        (and thus \c UnitSystem) and/or \c RelativeScale used to display a particular UI field.  This allows it to
    *        be different from the global default (eg so the user can specify a particular weight field is shown in
    *        metric even though they have set US Customary as the default for weight fields).
    *
    *        It also allows the user to select whether to override the number of decimal places displayed in this field.
    *        (Strictly speaking we should have renamed the namespace to UnitScaleAndPrecisionPopUpMenu when we added
    *        this feature, but that's starting to get a bit of a mouthful.)
    *
    *        Note that the \c UnitAndScalePopUpMenu itself has no knowledge of the field; it is for the creator to
    *        manage the effects of menu choices on how the field is displayed.
    *
    * \param parent the Qt Widget that is to "own" the newly-created \c QMenu, or \c nullptr if there is none
    * \param physicalQuantities the physical quantity (or quantities) of the value(s) in the UI field.  If there is more
    *                           than one physical quantity here, it means we can measure in more than one way, eg by
    *                           Mass or by Volume
    * \param forcedSystemOfMeasurement the current \c SystemOfMeasurement, if any, specified for the UI field.  (If none
    *                                  is specified then the global default is used.)
    * \param forcedRelativeScale the forced scale, if any, for displaying the field.  (NB: Should always be
    *                            \c std::nullopt if \c physicalQuantities is \c Measurement::ChoiceOfPhysicalQuantity.)
    * \param forcedDecimalPlaces if set, the number of decimal places to use when displaying this field
    *
    * \return New \c QMenu "owned" by \c parent, but see comment in \c widgets/SmartLabel.cpp for why we return
    *         unique pointer so that caller really owns the object -- essentially the returned object typically has a
    *         much shorter lifetime than the parent.
    */
   std::unique_ptr<QMenu> create(QWidget * parent,
                                 Measurement::PhysicalQuantities physicalQuantities,
                                 std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement,
                                 std::optional<Measurement::UnitSystem::RelativeScale> forcedRelativeScale,
                                 std::optional<Measurement::NumDecimalPlaces> forcedDecimalPlaces);

   /**
    * \brief What the user selected from a menu created by \c create.  The alternative held tells you which (sub-)menu
    *        the selection came from.  Each alternative is a \c std::optional because \c std::nullopt means the user
    *        chose "Default" (ie to remove any forced setting).
    */
   using Selection = std::variant<std::optional<Measurement::SystemOfMeasurement      >,
                                  std::optional<Measurement::UnitSystem::RelativeScale>,
                                  std::optional<Measurement::NumDecimalPlaces         >>;

   /**
    * \brief Shows a menu created by \c create and returns what the user selected
    *
    * \param menu
    * \param globalPosition Where to show the menu, in global coordinates
    *
    * \return \c std::nullopt if the user dismissed the menu without selecting anything
    */
   [[nodiscard]] std::optional<Selection> exec(QMenu & menu, QPoint const & globalPosition);

   /**
    * \brief Anything we can apply a \c Selection to -- in practice \c SmartLabel and \c ColumnInfo
    */
   template<class T>
   concept CanHaveForcedDisplaySettings = requires (T & target) {
      target.setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement      >{});
      target.setForcedRelativeScale      (std::optional<Measurement::UnitSystem::RelativeScale>{});
      target.setForcedDecimalPlaces      (std::optional<Measurement::NumDecimalPlaces         >{});
   };

   /**
    * \brief Stores the user's \c Selection via the relevant "forced" setter on \c target.  Note that this only stores
    *        the new setting; it is for the caller to redisplay the field(s) concerned.
    */
   template<CanHaveForcedDisplaySettings T>
   void applySelection(Selection const & selection, T & target) {
      std::visit(
         [&target](auto const & selected) {
            using SelectedType = std::decay_t<decltype(selected)>;
            if constexpr (std::is_same_v<SelectedType, std::optional<Measurement::SystemOfMeasurement>>) {
               target.setForcedSystemOfMeasurement(selected);
               // Any forced RelativeScale relates to the old UnitSystem, so choosing a forced SystemOfMeasurement
               // resets it.
               target.setForcedRelativeScale(std::nullopt);
            } else if constexpr (std::is_same_v<SelectedType, std::optional<Measurement::UnitSystem::RelativeScale>>) {
               target.setForcedRelativeScale(selected);
            } else {
               static_assert(std::is_same_v<SelectedType, std::optional<Measurement::NumDecimalPlaces>>);
               target.setForcedDecimalPlaces(selected);
            }
            return;
         },
         selection
      );
      return;
   }
}

#endif