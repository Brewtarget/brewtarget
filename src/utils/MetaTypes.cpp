/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/MetaTypes.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "utils/MetaTypes.h"

#include <memory>
#include <optional>

#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/Equipment.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeAdjustmentSalt.h"
#include "model/RecipeUseOfWater.h"
#include "model/Style.h"

void registerMetaTypes() {
   //
   // Not all of these are needed, but it's simpler to register everything we mention in the header.
   //
   qRegisterMetaType<std::optional<bool        >          >();
   qRegisterMetaType<std::optional<double      >          >();
   qRegisterMetaType<std::optional<int         >          >();
   qRegisterMetaType<std::optional<QDate       >          >();
   qRegisterMetaType<std::optional<QString     >          >();
   qRegisterMetaType<std::optional<unsigned int>          >();

   qRegisterMetaType<Measurement::Amount                  >();
   qRegisterMetaType<std::optional<Measurement::Amount>   >();
   qRegisterMetaType<Measurement::PhysicalQuantity        >();
   qRegisterMetaType<Measurement::ChoiceOfPhysicalQuantity>();
   qRegisterMetaType<Measurement::Unit const *            >();

   //
   // In theory we don't need these, as the Q_DECLARE_METATYPE declarations in model/*.h should suffice.  In practice,
   // adding them got rid of "QMetaProperty::read: Unable to handle unregistered datatype" errors.  One day we should
   // get to the bottom of what's going on here, but, for now, it feels pretty low priority.
   //
   // If you get a compile error here, it may be because you didn't include the necessary "model/" header file
   //
   qRegisterMetaType<QList<std::shared_ptr<BoilStep                 >>>();
   qRegisterMetaType<QList<std::shared_ptr<FermentationStep         >>>();
   qRegisterMetaType<QList<std::shared_ptr<MashStep                 >>>();
   qRegisterMetaType<QList<std::shared_ptr<RecipeAdditionFermentable>>>();
   qRegisterMetaType<QList<std::shared_ptr<RecipeAdditionHop        >>>();
   qRegisterMetaType<QList<std::shared_ptr<RecipeAdditionMisc       >>>();
   qRegisterMetaType<QList<std::shared_ptr<RecipeAdditionYeast      >>>();
   qRegisterMetaType<QList<std::shared_ptr<RecipeAdjustmentSalt     >>>();
   qRegisterMetaType<QList<std::shared_ptr<RecipeUseOfWater         >>>();

   qRegisterMetaType<std::shared_ptr<Boil            >>();
   qRegisterMetaType<std::shared_ptr<Equipment       >>();
   qRegisterMetaType<std::shared_ptr<Fermentation    >>();
   qRegisterMetaType<std::shared_ptr<FermentationStep>>();
   qRegisterMetaType<std::shared_ptr<Mash            >>();
   qRegisterMetaType<std::shared_ptr<Style           >>();

   return;
}
