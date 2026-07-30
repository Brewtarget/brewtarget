/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionMiscTableModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Luke Vincent <luke.r.vincent@gmail.com>
 *   • Markus Mårtensson <mackan.90@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Tim Payne <swstim@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "qtModels/tableModels/RecipeAdditionMiscTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/StockPurchase.h"
#include "model/Mash.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionMiscTableModel.cpp"
#endif

COLUMN_INFOS(
   RecipeAdditionMiscTableModel,
   //
   // Note that for Name, we want the name of the contained Misc, not the name of the RecipeAdditionMisc
   //
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Name          , PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,         // "Name"
                                                                        PropertyNames::NamedEntity::name      }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Type          , PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,         // "Type"
                                                                        PropertyNames::Misc::type             }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Amount        , PropertyNames::IngredientAmount::amount             ),         // "Amount"
   TABLE_MODEL_HEADER(RecipeAdditionMisc, AmountType    , PropertyNames::IngredientAmount::amount, Misc::validMeasures), // "Amount Type"
   // Total inventory is read-only, so there is intentionally no TotalInventoryType column
   TABLE_MODEL_HEADER(RecipeAdditionMisc, TotalInventory, PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,         // "Inventory"
                                                                        PropertyNames::Ingredient::totalInventory}, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Stage         , PropertyNames::RecipeAddition::stage                     ),    // "Stage"
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Time          , PropertyNames::RecipeAddition::addAtTime_mins            ),    // "Time"
)

RecipeAdditionMiscTableModel::RecipeAdditionMiscTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{parent, editable},
   TableModelBase<RecipeAdditionMiscTableModel, RecipeAdditionMisc>{},
   showIBUs(false) {
   this->m_rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionMiscTableModel::contextMenu);
   connect(&ObjectStoreTyped<StockPurchaseMisc>::getInstance(), &ObjectStoreTyped<StockPurchaseMisc>::signalPropertyChanged, this,
           &RecipeAdditionMiscTableModel::changedInventory);
   return;
}

RecipeAdditionMiscTableModel::~RecipeAdditionMiscTableModel() = default;

void RecipeAdditionMiscTableModel::added  ([[maybe_unused]] std::shared_ptr<RecipeAdditionMisc> const item) {
   auto const misc = item->misc();
   if (misc->type() == Misc::Type::WaterAgent && misc->waterAgentType()) {
      emit newTotals();
   }
   return;
}
void RecipeAdditionMiscTableModel::removed([[maybe_unused]] std::shared_ptr<RecipeAdditionMisc> const item) {
   // Logic is the same as for added, so don't repeat ourselves here
   this->added(item);
   return;
}
void RecipeAdditionMiscTableModel::modified(std::shared_ptr<RecipeAdditionMisc> const item) {
   // Logic is the same as for added, so don't repeat ourselves here
   this->added(item);
   return;
}

void RecipeAdditionMiscTableModel::updateTotals() {
   return;
}

void RecipeAdditionMiscTableModel::setShowIBUs(bool var) {
   showIBUs = var;
   return;
}

QVariant RecipeAdditionMiscTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

QVariant RecipeAdditionMiscTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return ColumnOwnerTraits<RecipeAdditionMiscTableModel>::getColumnLabel(section);
   }
   if (showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      QList<double> ibus = recObs->IBUs();

      if (ibus.size() > section) {
         return QVariant(QString("%L1 IBU").arg(ibus.at(section), 0, 'f', 1));
      }
   }
   return QVariant();
}

bool RecipeAdditionMiscTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

template<Water::MineralIon ion> double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm() const {
   double concentrationPerLiter_massConcPpm = 0.0;
   for (auto const & miscAddition : this->m_rows) {
      if (auto const misc = miscAddition->misc();
          misc->type() == Misc::Type::WaterAgent && misc->waterAgentType()) {
         auto const additionAmount = miscAddition->amount();
         //
         // If the amount is a volume, we are assuming that 1 liter of it weighs 1 kilogram.
         //
         // Our canonical unit for weight is kg, so multiply by 1000 here to go from kg to g
         //
         double const additionMass_g = 1000.0 * additionAmount.quantity;
         double const concentrationPerGramPerLiter_massConcPpm =
            Misc::concentrationPerGramPerLiter_massConcPpm<ion>(*misc->waterAgentType());
         concentrationPerLiter_massConcPpm += additionMass_g * concentrationPerGramPerLiter_massConcPpm;
      }
   }
   return concentrationPerLiter_massConcPpm;
}
// Instantiate the above template function for the types that are going to use it
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Bicarbonate>() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Calcium    >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Carbonate  >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Chloride   >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Copper     >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Iron       >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Magnesium  >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Manganese  >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Nitrate    >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Nitrite    >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Phosphate  >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Potassium  >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Sodium     >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Sulfate    >() const;
template double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm<Water::MineralIon::Zinc       >() const;

double RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm(Water::MineralIon const ion) const {
   switch(ion) {
      case Water::MineralIon::Bicarbonate: return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Bicarbonate>();
      case Water::MineralIon::Calcium    : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Calcium    >();
      case Water::MineralIon::Carbonate  : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Carbonate  >();
      case Water::MineralIon::Chloride   : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Chloride   >();
      case Water::MineralIon::Copper     : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Copper     >();
      case Water::MineralIon::Iron       : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Iron       >();
      case Water::MineralIon::Magnesium  : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Magnesium  >();
      case Water::MineralIon::Manganese  : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Manganese  >();
      case Water::MineralIon::Nitrate    : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Nitrate    >();
      case Water::MineralIon::Nitrite    : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Nitrite    >();
      case Water::MineralIon::Phosphate  : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Phosphate  >();
      case Water::MineralIon::Potassium  : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Potassium  >();
      case Water::MineralIon::Sodium     : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Sodium     >();
      case Water::MineralIon::Sulfate    : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Sulfate    >();
      case Water::MineralIon::Zinc       : return this->concentrationPerLiter_massConcPpm<Water::MineralIon::Zinc       >();
         // NB: No default case as we want compiler to warn us if we missed a possibility above
   }
   return 0.0;
}

Measurement::Amount RecipeAdditionMiscTableModel::total(Misc::WaterAgentType const waterAgentType) const {
   Measurement::Amount totalAmount{Misc::suggestedMeasureFor(waterAgentType), 0.0};
   for (auto const & miscAddition : this->m_rows) {
      if (auto const misc = miscAddition->misc();
          misc->type() == Misc::Type::WaterAgent && misc->waterAgentType() == waterAgentType) {
         Measurement::Amount const waterAgentAmount = miscAddition->amount().toCanonical();
         // Normally leave this commented out as otherwise generates too much logging
//         qDebug() << Q_FUNC_INFO << waterAgentAmount << "of" << misc << "to add to" << totalAmount;
         // .:TBD:. For the moment, we are assuming that mass and volume are interchangeable, which isn't great.  But at
         //         least let's log a warning when we do it.
         if (waterAgentAmount.unit != totalAmount.unit) {
            //
            // If we didn't yet add anything to our running total, we just assume the first amount we find is measured
            // in the physical quantity we want.
            //
            // Although we wouldn't normally compare doubles using ==, I think it's OK in checking whether we yet added
            // anything to zero.
            //
            if (0.0 == totalAmount.quantity) {
               qDebug() <<
                  Q_FUNC_INFO << "First water agent addition found was" << waterAgentAmount << "of" << misc <<
                  "so changing units from" << totalAmount.unit;
               totalAmount.unit = waterAgentAmount.unit;
            } else {
               qWarning() <<
                  Q_FUNC_INFO << "Adding" << waterAgentAmount << "of" << misc << "to a total of" << totalAmount <<
                  "involves implicit assumption that units are interchangeable";
            }

         }
         totalAmount.quantity += waterAgentAmount.quantity;
         qDebug() << Q_FUNC_INFO << "Total now" << totalAmount;
      }
   }
   return totalAmount;
}

double RecipeAdditionMiscTableModel::totalAcid_kg(Misc::WaterAgentType const waterAgentType) const {
   constexpr double H3PO4_density  = 1.685;
   constexpr double lactic_density = 1.2;

   // .:TODO:. There are assumptions in here about measurement being by weight or by volume.  We should check or assert
   //          these.
   double ret = 0.0;
   for (auto const & miscAddition : this->m_rows) {
      if (auto const misc = miscAddition->misc();
          misc->type() == Misc::Type::WaterAgent && misc->waterAgentType() == waterAgentType) {

         double const mult  = 1000.0;
         auto const waterAgentAcid_pct = misc->waterAgentPercentAcid();
         if (waterAgentAcid_pct) {
            if (waterAgentType == Misc::WaterAgentType::Other) {
               ret += 1000.0 * miscAddition->amount().quantity * *waterAgentAcid_pct;
            } else if (waterAgentType == Misc::WaterAgentType::LacticAcid) {
               // Lactic acid isn't quite so easy
               double const density = *waterAgentAcid_pct/88.0 * (lactic_density - 1.0) + 1.0;
               double const lactic_wgt = 1000.0 * miscAddition->amount().quantity * mult * density;
               ret += (*waterAgentAcid_pct/100.0) * lactic_wgt;
            } else if (waterAgentType == Misc::WaterAgentType::PhosphoricAcid) {
               double const density = *waterAgentAcid_pct/85.0 * (H3PO4_density - 1.0) + 1.0;
               double const H3PO4_wgt = 1000.0 * miscAddition->amount().quantity * density;
               ret += (*waterAgentAcid_pct/100.0) * H3PO4_wgt;
            }
         }
      }
   }
   return ret;
}

// Insert the boilerplate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionMisc, recipeAdditionMisc, PropertyNames::Recipe::miscAdditions)
//=============================================== CLASS RecipeAdditionMiscItemDelegate ================================================

// Insert the boilerplate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionMisc)