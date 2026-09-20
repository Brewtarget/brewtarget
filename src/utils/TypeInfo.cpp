/*======================================================================================================================
 * utils/TypeInfo.cpp is part of Brewtarget, and is copyright the following authors 2023-2026:
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
 =====================================================================================================================*/
#include "utils/TypeInfo.h"

#include "measurement/ColorMethods.h"
#include "model/BoilStep.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/RecipeAddition.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/StockUse.h"
#include "model/Style.h"
#include "model/Yeast.h"

class Boil;
class BrewLog;
class Equipment;
class Fermentation;
class FermentationStep;
class Mash;
class Recipe;
class RecipeAdditionFermentable;
class RecipeAdditionYeast;
class Water;

namespace PropertyNames::None {
   BtStringConst const none{"none"};
}

//
// Specialisations for TypeInfo::indexOf have to be outside the struct/class definition
//
template<> TypeInfo::Index TypeInfo::indexOf<bool                                             >() { return Index::Bool                                ; }
template<> TypeInfo::Index TypeInfo::indexOf<double                                           >() { return Index::Double                              ; }
template<> TypeInfo::Index TypeInfo::indexOf<int                                              >() { return Index::Int                                 ; }
template<> TypeInfo::Index TypeInfo::indexOf<unsigned int                                     >() { return Index::UnsignedInt                         ; }
template<> TypeInfo::Index TypeInfo::indexOf<CurrencyAmount                                   >() { return Index::CurrencyAmount                      ; }
template<> TypeInfo::Index TypeInfo::indexOf<QDate                                            >() { return Index::QDate                               ; }
template<> TypeInfo::Index TypeInfo::indexOf<QString                                          >() { return Index::QString                             ; }
template<> TypeInfo::Index TypeInfo::indexOf<Measurement::Amount                              >() { return Index::MeasurementAmount                   ; }
template<> TypeInfo::Index TypeInfo::indexOf<BoilStep::ChillingType                           >() { return Index::BoilStepChillingType                ; }
template<> TypeInfo::Index TypeInfo::indexOf<ColorMethods::ColorFormula                       >() { return Index::ColorFormula                        ; }
template<> TypeInfo::Index TypeInfo::indexOf<Empty                                            >() { return Index::Empty                               ; }
template<> TypeInfo::Index TypeInfo::indexOf<Fermentable *                                    >() { return Index::FermentablePtr                      ; }
template<> TypeInfo::Index TypeInfo::indexOf<Fermentable::GrainGroup                          >() { return Index::FermentableGrainGroup               ; }
template<> TypeInfo::Index TypeInfo::indexOf<Fermentable::Type                                >() { return Index::FermentableType                     ; }
template<> TypeInfo::Index TypeInfo::indexOf<Hop *                                            >() { return Index::HopPtr                              ; }
template<> TypeInfo::Index TypeInfo::indexOf<Hop::Form                                        >() { return Index::HopForm                             ; }
template<> TypeInfo::Index TypeInfo::indexOf<Hop::Type                                        >() { return Index::HopType                             ; }
template<> TypeInfo::Index TypeInfo::indexOf<IbuMethods::IbuFormula                           >() { return Index::IbuMethodsIbuFormula                ; }
template<> TypeInfo::Index TypeInfo::indexOf<MashStep::Type                                   >() { return Index::MashStepType                        ; }
template<> TypeInfo::Index TypeInfo::indexOf<Measurement::PhysicalQuantity                    >() { return Index::MeasurementPhysicalQuantity         ; }
template<> TypeInfo::Index TypeInfo::indexOf<Measurement::Unit const *                        >() { return Index::MeasurementUnit                     ; }
template<> TypeInfo::Index TypeInfo::indexOf<Misc *                                           >() { return Index::MiscPtr                             ; }
template<> TypeInfo::Index TypeInfo::indexOf<Misc::Type                                       >() { return Index::MiscType                            ; }
template<> TypeInfo::Index TypeInfo::indexOf<Misc::WaterAgentType                             >() { return Index::MiscWaterAgentType                  ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<BoilStep>                 >>() { return Index::QListOfSptrBoilStep                 ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<BrewLog>                  >>() { return Index::QListOfSptrBrewLog                  ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<FermentationStep>         >>() { return Index::QListOfSptrFermentationStep         ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<MashStep>                 >>() { return Index::QListOfSptrMashStep                 ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<RecipeAdditionFermentable>>>() { return Index::QListOfSptrRecipeAdditionFermentable; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<RecipeAdditionHop>        >>() { return Index::QListOfSptrRecipeAdditionHop        ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<RecipeAdditionMisc>       >>() { return Index::QListOfSptrRecipeAdditionMisc       ; }
template<> TypeInfo::Index TypeInfo::indexOf<QList<std::shared_ptr<RecipeAdditionYeast>      >>() { return Index::QListOfSptrRecipeAdditionYeast      ; }
template<> TypeInfo::Index TypeInfo::indexOf<Recipe::Type                                     >() { return Index::RecipeType                          ; }
template<> TypeInfo::Index TypeInfo::indexOf<RecipeAddition::Stage                            >() { return Index::RecipeAdditionStage                 ; }
template<> TypeInfo::Index TypeInfo::indexOf<RecipeAdditionHop::Use                           >() { return Index::RecipeAdditionHopUse                ; }
template<> TypeInfo::Index TypeInfo::indexOf<RecipeAdditionMisc::Use                          >() { return Index::RecipeAdditionMiscUse               ; }
template<> TypeInfo::Index TypeInfo::indexOf<StockUse::Reason                                 >() { return Index::StockUseReason                      ; }
template<> TypeInfo::Index TypeInfo::indexOf<Style::Type                                      >() { return Index::StyleType                           ; }
template<> TypeInfo::Index TypeInfo::indexOf<Yeast *                                          >() { return Index::YeastPtr                            ; }
template<> TypeInfo::Index TypeInfo::indexOf<Yeast::Flocculation                              >() { return Index::YeastFlocculation                   ; }
template<> TypeInfo::Index TypeInfo::indexOf<Yeast::Form                                      >() { return Index::YeastForm                           ; }
template<> TypeInfo::Index TypeInfo::indexOf<Yeast::Type                                      >() { return Index::YeastType                           ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Boil            >                >() { return Index::SptrBoil                            ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<BrewLog         >                >() { return Index::SptrBrewLog                         ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Equipment       >                >() { return Index::SptrEquipment                       ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Fermentation    >                >() { return Index::SptrFermentation                    ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<FermentationStep>                >() { return Index::SptrFermentationStep                ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Mash            >                >() { return Index::SptrMash                            ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Recipe          >                >() { return Index::SptrRecipe                          ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Style           >                >() { return Index::SptrStyle                           ; }
template<> TypeInfo::Index TypeInfo::indexOf<std::shared_ptr<Water           >                >() { return Index::SptrWater                           ; }

QString TypeInfo::indexName(Index const index) {
   switch (index) {
      case TypeInfo::Index::Bool             : return "Bool"             ;
      case TypeInfo::Index::Double           : return "Double"           ;
      case TypeInfo::Index::Int              : return "Int"              ;
      case TypeInfo::Index::UnsignedInt      : return "UnsignedInt"      ;
      case TypeInfo::Index::BoilStepChillingType                : return "BoilStepChillingType"                ;
      case TypeInfo::Index::ColorFormula                        : return "ColorFormula"                        ;
      case TypeInfo::Index::CurrencyAmount                      : return "CurrencyAmount"                      ;
      case TypeInfo::Index::Empty                               : return "Empty"                               ;
      case TypeInfo::Index::FermentableGrainGroup               : return "FermentableGrainGroup"               ;
      case TypeInfo::Index::FermentablePtr                      : return "FermentablePtr"                      ;
      case TypeInfo::Index::FermentableType                     : return "FermentableType"                     ;
      case TypeInfo::Index::HopForm                             : return "HopForm"                             ;
      case TypeInfo::Index::HopPtr                              : return "HopPtr"                              ;
      case TypeInfo::Index::HopType                             : return "HopType"                             ;
      case TypeInfo::Index::IbuMethodsIbuFormula                : return "IbuMethodsIbuFormula"                ;
      case TypeInfo::Index::MashStepType                        : return "MashStepType"                        ;
      case TypeInfo::Index::MeasurementAmount                   : return "MeasurementAmount"                   ;
      case TypeInfo::Index::MeasurementPhysicalQuantity         : return "MeasurementPhysicalQuantity"         ;
      case TypeInfo::Index::MeasurementUnit                     : return "MeasurementUnit"                     ;
      case TypeInfo::Index::MiscPtr                             : return "MiscPtr"                             ;
      case TypeInfo::Index::MiscType                            : return "MiscType"                            ;
      case TypeInfo::Index::MiscWaterAgentType                  : return "MiscWaterAgentType"                  ;
      case TypeInfo::Index::QDate                               : return "QDate"                               ;
      case TypeInfo::Index::QListOfSptrBoilStep                 : return "QListOfSptrBoilStep"                 ;
      case TypeInfo::Index::QListOfSptrBrewLog                  : return "QListOfSptrBrewLog"                  ;
      case TypeInfo::Index::QListOfSptrFermentationStep         : return "QListOfSptrFermentationStep"         ;
      case TypeInfo::Index::QListOfSptrMashStep                 : return "QListOfSptrMashStep"                 ;
      case TypeInfo::Index::QListOfSptrRecipeAdditionFermentable: return "QListOfSptrRecipeAdditionFermentable";
      case TypeInfo::Index::QListOfSptrRecipeAdditionHop        : return "QListOfSptrRecipeAdditionHop"        ;
      case TypeInfo::Index::QListOfSptrRecipeAdditionMisc       : return "QListOfSptrRecipeAdditionMisc"       ;
      case TypeInfo::Index::QListOfSptrRecipeAdditionYeast      : return "QListOfSptrRecipeAdditionYeast"      ;
      case TypeInfo::Index::QString                             : return "QString"                             ;
      case TypeInfo::Index::RecipeAdditionHopUse                : return "RecipeAdditionHopUse"                ;
      case TypeInfo::Index::RecipeAdditionMiscUse               : return "RecipeAdditionMiscUse"               ;
      case TypeInfo::Index::RecipeAdditionStage                 : return "RecipeAdditionStage"                 ;
      case TypeInfo::Index::RecipeType                          : return "RecipeType"                          ;
      case TypeInfo::Index::SptrBoil                            : return "SptrBoil"                            ;
      case TypeInfo::Index::SptrBrewLog                         : return "SptrBrewLog"                         ;
      case TypeInfo::Index::SptrEquipment                       : return "SptrEquipment"                       ;
      case TypeInfo::Index::SptrFermentation                    : return "SptrFermentation"                    ;
      case TypeInfo::Index::SptrFermentationStep                : return "SptrFermentationStep"                ;
      case TypeInfo::Index::SptrMash                            : return "SptrMash"                            ;
      case TypeInfo::Index::SptrRecipe                          : return "SptrRecipe"                          ;
      case TypeInfo::Index::SptrStyle                           : return "SptrStyle"                           ;
      case TypeInfo::Index::SptrWater                           : return "SptrWater"                           ;
      case TypeInfo::Index::StockUseReason                      : return "StockUseReason"                      ;
      case TypeInfo::Index::StyleType                           : return "StyleType"                           ;
      case TypeInfo::Index::YeastFlocculation                   : return "YeastFlocculation"                   ;
      case TypeInfo::Index::YeastForm                           : return "YeastForm"                           ;
      case TypeInfo::Index::YeastPtr                            : return "YeastPtr"                            ;
      case TypeInfo::Index::YeastType                           : return "YeastType"                           ;

      // No default as we want compiler to warn us if we missed a case above
   }
   Q_UNREACHABLE();
}

bool TypeInfo::isEnum() const {
   if (this->classification == TypeInfo::Classification::RequiredEnum ||
       this->classification == TypeInfo::Classification::OptionalEnum) {
      return true;
   }
   return false;
}

bool TypeInfo::isOptional() const {
   if (this->classification == TypeInfo::Classification::OptionalEnum ||
       this->classification == TypeInfo::Classification::OptionalOther) {
      return true;
   }
   return false;
}

bool TypeInfo::isReadOnly() const {
   return this->access == TypeInfo::Access::ReadOnly;
}