/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * WaterDialog.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
#include "WaterDialog.h"

#include <limits>

#include <Algorithms.h>
#include <QComboBox>
#include <QFont>
#include <QInputDialog>

#include "buttons/WaterButton.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/ColorMethods.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeUseOfWater.h"
#include "model/Salt.h"
#include "tableModels/RecipeAdjustmentSaltTableModel.h"
#include "tableModels/WaterTableModel.h"
#include "editors/WaterEditor.h"
#include "listModels/WaterListModel.h"
#include "sortFilterProxyModels/WaterSortFilterProxyModel.h"
#include "widgets/SmartDigitWidget.h"


//
// All of the pH calculations are taken from the work done by Kai Troester and published at
// http://braukaiser.com/wiki/index.php/Beer_color_to_mash_pH_(v2.0) with additional information being gleaned from the
// spreadsheet associated with that link.
//

namespace {
   // I've seen some confusion over this constant. 50 mEq/l is what Kai uses.
   double constexpr mEq = 50.0;
   // Ca grams per mole
   double constexpr Cagpm = 40.0;
   // Mg grams per mole
   double constexpr Mggpm = 24.30;
   // HCO3 grams per mole
   double constexpr HCO3gpm = 61.01;
   // CO3 grams per mole
   double constexpr CO3gpm = 60.01;

   // The pH of a beer with no color
   double constexpr nosrmbeer_ph = 5.6;
   // Magic constants Kai derives in the document above.
   double constexpr pHSlopeLight = 0.21;
   double constexpr pHSlopeDark  = 0.06;
}

WaterDialog::WaterDialog(QWidget* parent) :
   QDialog{parent},
   m_ppm_digits  {QVector<SmartDigitWidget*>{Water::ionStringMapping.size()} },
   m_total_digits{QVector<SmartDigitWidget*>{Salt::typeStringMapping.size()} },
   m_rec{nullptr},
   m_base{nullptr},
   m_target{nullptr},
   m_mashRO{0.0},
   m_spargeRO{0.0},
   m_total_grains{0.0},
   m_thickness{0.0} {

   setupUi(this);
   // initialize the two buttons and lists (I think)
   m_base_combo_list = new WaterListModel(baseProfileCombo);
   m_base_filter    = new WaterSortFilterProxyModel(baseProfileCombo);
   m_base_filter->setDynamicSortFilter(true);
   m_base_filter->setSortLocaleAware(true);
   m_base_filter->setSourceModel(m_base_combo_list);
   m_base_filter->sort(0);
   baseProfileCombo->setModel(m_base_filter);

   m_target_combo_list = new WaterListModel(targetProfileCombo);
   m_target_filter    = new WaterSortFilterProxyModel(targetProfileCombo);
   m_target_filter->setDynamicSortFilter(true);
   m_target_filter->setSortLocaleAware(true);
   m_target_filter->setSourceModel(m_target_combo_list);
   m_target_filter->sort(0);
   targetProfileCombo->setModel(m_target_filter);

   SMART_FIELD_INIT_FS(WaterDialog, label_ca  , btDigit_ca  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_cl  , btDigit_cl  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_hco3, btDigit_hco3, double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_mg  , btDigit_mg  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_na  , btDigit_na  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_so4 , btDigit_so4 , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_pH  , btDigit_ph  , double, Measurement::PhysicalQuantity::Acidity           , 1);

   SMART_FIELD_INIT_FS(WaterDialog, label_totalcacl2 , btDigit_totalcacl2 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_totalcaco3 , btDigit_totalcaco3 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_totalcaso4 , btDigit_totalcaso4 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_totalmgso4 , btDigit_totalmgso4 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_totalnacl  , btDigit_totalnacl  , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterDialog, label_totalnahco3, btDigit_totalnahco3, double, Measurement::PhysicalQuantity::Mass, 2);

   // not sure if this is better or worse, but we will try it out
   m_ppm_digits[static_cast<int>(Water::Ion::Ca)]   = btDigit_ca  ;
   m_ppm_digits[static_cast<int>(Water::Ion::Cl)]   = btDigit_cl  ;
   m_ppm_digits[static_cast<int>(Water::Ion::HCO3)] = btDigit_hco3;
   m_ppm_digits[static_cast<int>(Water::Ion::Mg)]   = btDigit_mg  ;
   m_ppm_digits[static_cast<int>(Water::Ion::Na)]   = btDigit_na  ;
   m_ppm_digits[static_cast<int>(Water::Ion::SO4)]  = btDigit_so4 ;

   m_total_digits[static_cast<int>(Salt::Type::CaCl2 )] = btDigit_totalcacl2 ;
   m_total_digits[static_cast<int>(Salt::Type::CaCO3 )] = btDigit_totalcaco3 ;
   m_total_digits[static_cast<int>(Salt::Type::CaSO4 )] = btDigit_totalcaso4 ;
   m_total_digits[static_cast<int>(Salt::Type::MgSO4 )] = btDigit_totalmgso4 ;
   m_total_digits[static_cast<int>(Salt::Type::NaCl  )] = btDigit_totalnacl  ;
   m_total_digits[static_cast<int>(Salt::Type::NaHCO3)] = btDigit_totalnahco3;

   // foreach( SmartDigitWidget* i, m_ppm_digits ) {
   for (int ii = 0; ii < Water::ionStringMapping.size(); ++ii) {
      m_ppm_digits[ii]->setLimits(0.0,1000.0);
      m_ppm_digits[ii]->setQuantity(0.0);
      m_ppm_digits[ii]->setMessages(tr("Too low for target profile."),
                                    tr("In range for target profile."),
                                    tr("Too high for target profile."));
   }
   // we can be a bit more specific with pH
   btDigit_ph->setLowLim(5.0);
   btDigit_ph->setHighLim(5.5);
   btDigit_ph->setQuantity(7.0);

   // since all the things are now digits, lets get the totals configured
   for (int ii = static_cast<int>(Salt::Type::CaCl2); ii < static_cast<int>(Salt::Type::NaHCO3); ++ii ) {
      m_total_digits[ii]->setConstantColor(SmartDigitWidget::ColorType::Black);
      m_total_digits[ii]->setQuantity(0.0);
   }
   // and now let's see what the table does.
   m_saltAdjustmentTableModel = new RecipeAdjustmentSaltTableModel(tableView_saltAdjustments);
   m_saltAdjustmentDelegate   = new RecipeAdjustmentSaltItemDelegate(tableView_saltAdjustments, *m_saltAdjustmentTableModel);
   tableView_saltAdjustments->setItemDelegate(m_saltAdjustmentDelegate);
   tableView_saltAdjustments->setModel(m_saltAdjustmentTableModel);

   m_base_editor   = new WaterEditor(this, "Base");
   m_target_editor = new WaterEditor(this, "Target");

   // all the signals
   connect(baseProfileCombo,   QOverload<int>::of(&QComboBox::activated), this, &WaterDialog::update_baseProfile  );
   connect(targetProfileCombo, QOverload<int>::of(&QComboBox::activated), this, &WaterDialog::update_targetProfile);

   connect(baseProfileButton,   &WaterButton::clicked, m_base_editor,   &QWidget::show);
   connect(targetProfileButton, &WaterButton::clicked, m_target_editor, &QWidget::show);

   connect(m_saltAdjustmentTableModel, &RecipeAdjustmentSaltTableModel::newTotals, this                      , &WaterDialog::newTotals   );
   connect(pushButton_addSalt        , &QAbstractButton::clicked                 , m_saltAdjustmentTableModel, &RecipeAdjustmentSaltTableModel::catchSalt);
   connect(pushButton_removeSalt     , &QAbstractButton::clicked                 , this                      , &WaterDialog::removeSalts );

   connect(spinBox_mashRO,   QOverload<int>::of(&QSpinBox::valueChanged), this, &WaterDialog::setMashRO  );
   connect(spinBox_spargeRO, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaterDialog::setSpargeRO);

   connect(buttonBox_save, &QDialogButtonBox::accepted, this, &WaterDialog::saveAndClose );
   connect(buttonBox_save, &QDialogButtonBox::rejected, this, &WaterDialog::clearAndClose);

   return;
}

WaterDialog::~WaterDialog() = default;

void WaterDialog::setMashRO(int val) {
   m_mashRO = val/100.0;
   if ( m_base ) m_base->setMashRo_pct(m_mashRO);
   newTotals();
   return;
}

void WaterDialog::setSpargeRO(int val) {
   m_spargeRO = val/100.0;
   if ( m_base ) m_base->setSpargeRo_pct(m_spargeRO);
   newTotals();
   return;
}

void WaterDialog::setDigits() {
   if (!this->m_target) {
      return;
   }

   for (int ii = 0; ii < Water::ionStringMapping.size(); ++ii) {
      double ppm = this->m_target->ppm(static_cast<Water::Ion>(ii));
      double min_ppm = ppm * 0.95;
      double max_ppm = ppm * 1.05;
      m_ppm_digits[ii]->setLimits(min_ppm,max_ppm);
      m_ppm_digits[ii]->setMessages(tr("Minimum expected concentration is %1 ppm").arg(min_ppm),
                                   tr("In range for target profile."),
                                   tr("Maximum expected concentration is %1 ppm").arg(max_ppm));
   }

   // oddly, pH doesn't change with the target water
   return;
}

void WaterDialog::setRecipe(Recipe *rec) {
   if (!rec) {
      return;
   }

   this->m_rec = rec;
   auto mash = this->m_rec->mash();
   m_saltAdjustmentTableModel->observeRecipe(this->m_rec);

   if (!mash || mash->mashSteps().size() == 0 ) {
      qWarning() << QString("Cannot set water chemistry without a mash");
      return;
   }

   baseProfileButton->setRecipe(this->m_rec);
   targetProfileButton->setRecipe(this->m_rec);

   for (auto waterUse : this->m_rec->waterUses()) {
      auto water = ObjectStoreWrapper::getSharedFromRaw(waterUse->water());
      if (water->type() == Water::Type::Base) {
         qDebug() << Q_FUNC_INFO << "Base Water" << *water;
         this->m_base = water;
      } else if (water->type() == Water::Type::Target) {
         qDebug() << Q_FUNC_INFO << "Target Water" << *water;
         this->m_target = water;
      }
   }

   // I need these numbers before we set the ranges
   for (auto const & fermentableAddition : m_rec->fermentableAdditions() ) {
      // .:TBD:. This almost certainly needs some refinement
      switch (fermentableAddition->fermentable()->type()) {
         case Fermentable::Type::Grain:
         case Fermentable::Type::Extract:
         case Fermentable::Type::Dry_Extract:
            if (fermentableAddition->getMeasure() == Measurement::PhysicalQuantity::Mass) {
               m_total_grains += fermentableAddition->amount().quantity;
            }
            break;
         case Fermentable::Type::Sugar:
         case Fermentable::Type::Other_Adjunct:
         case Fermentable::Type::Fruit:
         case Fermentable::Type::Juice:
         case Fermentable::Type::Honey:
            // For the moment, at least, assume these types of fermentables do not affect color.  .:TBD:. This is
            // probably wrong!
            break;
      }
   }

   // Now we've got m_total_grains, we need to loop over fermentable again
   for (auto const & fermentableAddition : m_rec->fermentableAdditions() ) {
      // .:TBD:. This almost certainly needs some refinement
      switch (fermentableAddition->fermentable()->type()) {
         case Fermentable::Type::Grain:
         case Fermentable::Type::Extract:
         case Fermentable::Type::Dry_Extract:
            if (fermentableAddition->getMeasure() == Measurement::PhysicalQuantity::Mass) {
               double lovi = (fermentableAddition->fermentable()->color_srm() +0.6 ) / 1.35;
               m_weighted_colors   += (fermentableAddition->amount().quantity/m_total_grains)*lovi;
            }
            break;
         case Fermentable::Type::Sugar:
         case Fermentable::Type::Other_Adjunct:
         case Fermentable::Type::Fruit:
         case Fermentable::Type::Juice:
         case Fermentable::Type::Honey:
            // For the moment, at least, assume these types of fermentables do not affect color.  .:TBD:. This is
            // probably wrong!
            break;
      }
   }

   m_thickness = m_rec->mash()->totalInfusionAmount_l()/m_total_grains;

   if (this->m_base) {

      m_mashRO = this->m_base->mashRo_pct().value_or(0.0);
      spinBox_mashRO->setValue( QVariant(m_mashRO * 100).toInt());
      m_spargeRO = this->m_base->spargeRo_pct().value_or(0.0);
      spinBox_spargeRO->setValue( QVariant(m_spargeRO * 100).toInt());

      baseProfileButton->setWater(this->m_base);
      m_base_editor->setWater(this->m_base);
      // all of the magic to set the sliders happens in newTotals(). So don't do it twice
   }
   if (this->m_target && this->m_target != this->m_base) {
      targetProfileButton->setWater(this->m_target);
      m_target_editor->setWater(this->m_target);

      this->setDigits();
   }
   newTotals();

   return;
}

void WaterDialog::update_baseProfile(int selected) {
   Q_UNUSED(selected)
   if (!this->m_rec) {
      return;
   }

   QModelIndex proxyIdx(m_base_filter->index(baseProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(m_base_filter->mapToSource(proxyIdx));
   Water const * parent = m_base_combo_list->at(sourceIdx.row());
   if (parent) {
      // The copy constructor won't copy the key (aka database ID), so the new object will be in-memory only until we
      // explicitly insert it in the Object Store (which will be done if/when it is added to the Recipe).  Note
      // however that we do need to ensure the link to the "parent" water is not lost - hence the call to makeChild().
      this->m_base = std::make_shared<Water>(*parent);
      this->m_base->makeChild(*parent);
      this->m_base->setType(Water::Type::Base);
      qDebug() << Q_FUNC_INFO << "Made base child" << *this->m_base << "from parent" << parent;

      baseProfileButton->setWater(this->m_base);
      m_base_editor->setWater(this->m_base);
      newTotals();
   }
   return;
}

void WaterDialog::update_targetProfile(int selected) {
   Q_UNUSED(selected)
   if (!this->m_rec) {
      return;
   }

   QModelIndex proxyIdx(m_target_filter->index(targetProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(m_target_filter->mapToSource(proxyIdx));
   Water* parent = m_target_combo_list->at(sourceIdx.row());

   if (parent) {
      // Comment above for copy of m_base applies equally here
      this->m_target = std::make_shared<Water>(*parent);
      this->m_target->makeChild(*parent);
      this->m_target->setType(Water::Type::Target);
      qDebug() << Q_FUNC_INFO << "Made target child" << *this->m_target << "from parent" << parent;

      targetProfileButton->setWater(this->m_target);
      m_target_editor->setWater(this->m_target);

      this->setDigits();
   }
   return;
}

void WaterDialog::newTotals() {
   if (!this->m_rec || !this->m_rec->mash()) {
      return;
   }

   auto mash = m_rec->mash();
   double allTheWaters = mash->totalMashWater_l();

   if ( qFuzzyCompare(allTheWaters,0.0) ) {
      qWarning() << QString("Can not set strike water chemistry without a mash");
      return;
   }
   // Two major things need to happen here:
   //   o the totals need to be updated
   //   o the digits need to be updated

   // .:TBD:. It seems the ordering of Salt::Type is important.  Would be good to decouple this!
   for (int ii = static_cast<int>(Salt::Type::CaCl2); ii < static_cast<int>(Salt::Type::LacticAcid); ++ii ) {
      Salt::Type type = static_cast<Salt::Type>(ii);
      m_total_digits[ii]->setQuantity(m_saltAdjustmentTableModel->total(type));
   }

   // the total_* method return the numerator, we supply the denominator and
   // include the base water ppm. The confusing math generates an adjustment
   // for the base water that depends the %RO in the mash and sparge water

   if (this->m_base) {
      // 'd' means 'diluted'. They make calculating the modifier readable
      double dInfuse = m_mashRO * mash->totalInfusionAmount_l();
      double dSparge = m_spargeRO * mash->totalSpargeAmount_l();

      // I hope this is right. All this 'rithmetic is making me head hurt.
      double modifier = 1.0 - (dInfuse + dSparge) / allTheWaters;

      for (int ii = 0; ii < Water::ionStringMapping.size(); ++ii) {
         Water::Ion ion = static_cast<Water::Ion>(ii);
         double mPPM = modifier * this->m_base->ppm(ion);
         m_ppm_digits[ii]->setQuantity( m_saltAdjustmentTableModel->total(ion) / allTheWaters + mPPM);

      }
      btDigit_ph->setQuantity(calculateMashpH());

   } else {
      for (int ii = 0; ii < Water::ionStringMapping.size(); ++ii) {
         Water::Ion ion = static_cast<Water::Ion>(ii);
         m_ppm_digits[ii]->setQuantity( m_saltAdjustmentTableModel->total(ion) / allTheWaters);
      }
   }
   return;
}

void WaterDialog::removeSalts() {
   QModelIndexList selected = tableView_saltAdjustments->selectionModel()->selectedIndexes();

   for (QModelIndex ii : selected) {
      m_saltAdjustmentTableModel->remove(ii);
   }
   return;
}

//! \brief Calcuates the residual alkalinity of the mash water.
double WaterDialog::calculateRA() const {
   double residual = 0.0;
   if (this->m_base) {

      double base_alk = ( 1.0 - m_base->mashRo_pct().value_or(0.0) ) * m_base->alkalinity_ppm().value_or(0.0);
      if (!m_base->alkalinityAsHCO3()) {
         base_alk = 1.22 * base_alk;
      }
      residual = base_alk/61;
   }

   return residual;
}


//! \brief Calculates the pH of the base water caused by any Ca or Mg
//! including figuring out the residual alkalinity.
double WaterDialog::calculateSaltpH() {
   if (!this->m_rec || !this->m_rec->mash()) {
      return 0.0;
   }

   auto mash = m_rec->mash();
   double allTheWaters = mash->totalMashWater_l();

   double modifier = 1 - ( (m_mashRO * mash->totalInfusionAmount_l()) + (m_spargeRO * mash->totalSpargeAmount_l())) / allTheWaters;

   // I have no idea where the 2 comes from, but Kai did it. I wish I knew why
   // we get the initial numbers from the base water
   double cappm = modifier * m_base->calcium_ppm()/Cagpm * 2;
   double mgppm = modifier * m_base->magnesium_ppm()/Mggpm * 2;

   // I need mass of the salts, and all the previous math gave me
   // ppm. Multiplying by the water volume gives me the mass
   // The 3.5 and 7 come from Paul Kohlbach's work from the 1940's.
   double totalDelta = (calculateRA() - cappm/3.5 - mgppm/7) * m_rec->mash()->totalInfusionAmount_l();
   // note: The referenced paper says the formula is
   // gristpH + strikepH * thickness/mEq. I could never get that to work.
   // the spreadsheet gave me this formula, and  it works much better.
   return totalDelta/m_thickness/mEq;
}

//! \brief Calculates the pH delta caused by any salt additions.
double WaterDialog::calculateAddedSaltpH() {

   // We need the value from the salt table model, because we need all the
   // added salts, but not the base.
   double ca   = this->m_saltAdjustmentTableModel->total_Ca()/Cagpm * 2;
   double mg   = this->m_saltAdjustmentTableModel->total_Mg()/Mggpm * 2;
   double hco3 = this->m_saltAdjustmentTableModel->total_HCO3()/HCO3gpm;
   double co3  = this->m_saltAdjustmentTableModel->total_CO3()/CO3gpm;

   // The 61 is another magic number from Kai. Sigh
   // unlike previous calculations, I am getting a mass here so I do not
   // need to convert from mg/L
   double totalDelta = 0.0 - ca/3.5 - mg/7 + (hco3+co3)/61;
   return totalDelta/m_thickness/mEq;
}

//! \brief Calculates the pH adjustment caused by lactic acid, H3PO4 and/or acid
//! malts
double WaterDialog::calculateAcidpH() {
   const double H3PO4_gpm = 98;
   const double lactic_gpm = 90;
   double totalDelta = 0.0;

   double lactic_amt   = this->m_saltAdjustmentTableModel->totalAcidWeight(Salt::Type::LacticAcid);
   double acidmalt_amt = this->m_saltAdjustmentTableModel->totalAcidWeight(Salt::Type::AcidulatedMalt);
   double H3PO4_amt    = this->m_saltAdjustmentTableModel->totalAcidWeight(Salt::Type::H3PO4);

   if ( lactic_amt + acidmalt_amt > 0.0 ) {
      totalDelta += 1000 * (lactic_amt + acidmalt_amt) / lactic_gpm;
   }
   if ( H3PO4_amt > 0.0 ) {
      totalDelta += 1000 * H3PO4_amt / H3PO4_gpm;
   }

   return totalDelta/mEq/m_thickness;
}

//! \brief Calculates the theoretical distilled water mash pH. I make some
//! rather rash assumptions about a crystal v roasted malt.
double WaterDialog::calculateGristpH() {
   double gristPh = nosrmbeer_ph;
   double pHAdjustment = 0.0;

   if ( m_rec && m_rec->fermentableAdditions().size() ) {

      double platoRatio = 1/Measurement::Units::plato.fromCanonical(m_rec->og());
      double color = m_rec->color_srm();
      double colorFromGrain = 0.0;

      for (auto const & fermentableAddition : m_rec->fermentableAdditions() ) {
         switch (fermentableAddition->fermentable()->type()) {
            case Fermentable::Type::Grain:
            case Fermentable::Type::Extract:
            case Fermentable::Type::Dry_Extract:
               // I am counting anything that doesn't have diastatic
               // power as a roasted/crystal malt. I am sure my assumption will
               // haunt me later, but I have no way of knowing what kind of malt
               // (base, crystal, roasted) this is.
               if (fermentableAddition->fermentable()->diastaticPower_lintner() < 1 ) {
                  double lovi = 19.0;
                  if (fermentableAddition->fermentable()->color_srm() <= 120 ) {
                     lovi = (fermentableAddition->fermentable()->color_srm() + 0.6)/1.35;
               }
                  colorFromGrain = (fermentableAddition->amount().quantity / m_total_grains ) * lovi;
               }
               break;
            case Fermentable::Type::Sugar:
            case Fermentable::Type::Other_Adjunct:
            case Fermentable::Type::Fruit:
            case Fermentable::Type::Juice:
            case Fermentable::Type::Honey:
               // For the moment, at least, assume these types of fermentables do not affect color.  .:TBD:. This is
               // probably wrong!
               break;
         }
      }
      double colorRatio = colorFromGrain/m_weighted_colors;
      pHAdjustment = platoRatio * ( pHSlopeLight * (1-colorRatio) + pHSlopeDark * colorRatio) *color;

      gristPh = gristPh - pHAdjustment;
   }
   return gristPh;
}

//! \brief This figures out the expected mash pH. It really just calls
//! all the other pieces to get those calculations and then sums them
//! all up.
double WaterDialog::calculateMashpH() {
   double mashpH = 0.0;

   if ( m_rec && m_rec->fermentableAdditions().size() ) {
      double gristpH   = calculateGristpH();
      double basepH    = calculateSaltpH();
      double saltpH    = calculateAddedSaltpH();
      double acids     = calculateAcidpH();

      // qDebug() << "basepH =" << basepH << "gristph =" << gristpH << "saltpH =" << saltpH << "acids =" << acids;
      // residual alkalinity is handled by basepH
      mashpH = basepH + gristpH + saltpH - acids;
   }

   return mashpH;
}

void WaterDialog::saveAndClose() {
   this->m_saltAdjustmentTableModel->saveAndClose();
   if (this->m_base && this->m_base->key() < 0) {
      // Recipe will take care of adding to the relevant object store
      auto waterUse = std::make_shared<RecipeUseOfWater>(tr("Use of %1").arg(this->m_base->name()),
                                                         this->m_rec->key(),
                                                         this->m_base->key());
      this->m_rec->addAddition(waterUse);
   }
   if (this->m_target && this->m_target->key() < 0) {
      // Recipe will take care of adding to the relevant object store
      auto waterUse = std::make_shared<RecipeUseOfWater>(tr("Use of %1").arg(this->m_target->name()),
                                                         this->m_rec->key(),
                                                         this->m_target->key());
      this->m_rec->addAddition(waterUse);
   }

   setVisible(false);
   return;
}

void WaterDialog::clearAndClose() {
   setVisible(false);
   return;
}
