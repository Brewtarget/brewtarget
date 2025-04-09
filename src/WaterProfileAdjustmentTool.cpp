/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * WaterProfileAdjustmentTool.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "WaterProfileAdjustmentTool.h"

#include <limits>

#include <Algorithms.h>
#include <QButtonGroup>
#include <QComboBox>
#include <QFont>
#include <QInputDialog>
#include <QVector>

#include "buttons/WaterButton.h"
#include "catalogs/SaltCatalog.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/ColorMethods.h"
#include "measurement/Unit.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeUseOfWater.h"
#include "model/Salt.h"
#include "model/Water.h"
#include "qtModels/tableModels/RecipeAdjustmentSaltTableModel.h"
#include "qtModels/tableModels/WaterTableModel.h"
#include "editors/WaterEditor.h"
#include "qtModels/listModels/WaterListModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdjustmentSaltSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.h"
#include "utils/VeriTable.h"
#include "widgets/SmartDigitWidget.h"
#include "MainWindow.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_WaterDialog.cpp"
#endif


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

// This private implementation class holds all private non-virtual members of WaterProfileAdjustmentTool
class WaterProfileAdjustmentTool::impl {
public:
   //
   // These mini structs help us write code to iterate over all the water ion displays or all the salt displays.
   // NOTE that it is intentional that the digitWidget member is a reference to a pointer.  The pointers in question
   // (eg btDigit_ca, btDigit_totalcacl2, etc from waterProfileAdjustmentTool.ui) get initialised _after_ the call
   // to the impl constructor (by setupUi), so taking a copy of the pointer is no good.
   //
   struct WaterIonDigitInfo {
      Water::Ion ion;
      SmartDigitWidget * & digitWidget;
   };

   struct SaltDigitInfo {
      Salt::Type type;
      SmartDigitWidget * & digitWidget;
   };

   /**
    * Constructor
    */
   impl(WaterProfileAdjustmentTool & self) :
      m_self {self},
      m_saltAdditionsVeriTable{},
      m_waterIonDisplays{
         {
            {Water::Ion::Ca  , m_self.btDigit_ca  },
            {Water::Ion::Cl  , m_self.btDigit_cl  },
            {Water::Ion::HCO3, m_self.btDigit_hco3},
            {Water::Ion::Mg  , m_self.btDigit_mg  },
            {Water::Ion::Na  , m_self.btDigit_na  },
            {Water::Ion::SO4 , m_self.btDigit_so4 },
         }
      },
      m_saltDisplays{
         {
            {Salt::Type::CaCl2 , m_self.btDigit_totalcacl2 },
            {Salt::Type::CaCO3 , m_self.btDigit_totalcaco3 },
            {Salt::Type::CaSO4 , m_self.btDigit_totalcaso4 },
            {Salt::Type::MgSO4 , m_self.btDigit_totalmgso4 },
            {Salt::Type::NaCl  , m_self.btDigit_totalnacl  },
            {Salt::Type::NaHCO3, m_self.btDigit_totalnahco3},
         }
      } {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    *
    */
   void setDigits() {
      if (!this->m_target) {
         return;
      }

      for (auto & waterIonDisplay : this->m_waterIonDisplays) {
         double const ppm = this->m_target->ppm(waterIonDisplay.ion);
         double const min_ppm = ppm * 0.95;
         double const max_ppm = ppm * 1.05;
         waterIonDisplay.digitWidget->setLimits(min_ppm, max_ppm);
         waterIonDisplay.digitWidget->setMessages(tr("Minimum expected concentration is %1 ppm").arg(min_ppm),
                                                  tr("In range for target profile."),
                                                  tr("Maximum expected concentration is %1 ppm").arg(max_ppm));
      }

      // oddly, pH doesn't change with the target water
      return;
   }

   /**
    * \brief Calculates the residual alkalinity of the mash water.
    */
   double calculateRA() const {
      double residual = 0.0;
      if (this->m_base) {

         double base_alk = ( 1.0 - this->m_base->mashRo_pct().value_or(0.0) ) * this->m_base->alkalinity_ppm().value_or(0.0);
         if (!this->m_base->alkalinityAsHCO3()) {
            base_alk = 1.22 * base_alk;
         }
         residual = base_alk/61;
      }

      return residual;
   }

   /**
    * \brief Calculates the theoretical distilled water mash pH. I make some
    *        rather rash assumptions about a crystal v roasted malt.
    */
   double calculateGristpH() {
      double gristPh = nosrmbeer_ph;
      double pHAdjustment = 0.0;

      if ( this->m_rec && this->m_rec->fermentableAdditions().size() ) {

         double platoRatio = 1/Measurement::Units::plato.fromCanonical(this->m_rec->og());
         double color = this->m_rec->color_srm();
         double colorFromGrain = 0.0;

         for (auto const & fermentableAddition : this->m_rec->fermentableAdditions() ) {
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
                     colorFromGrain = (fermentableAddition->amount().quantity / this->m_total_grains ) * lovi;
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
         double colorRatio = colorFromGrain/this->m_weighted_colors;
         pHAdjustment = platoRatio * ( pHSlopeLight * (1-colorRatio) + pHSlopeDark * colorRatio) *color;

         gristPh = gristPh - pHAdjustment;
      }
      return gristPh;
   }

   /**
    * \brief Calculates the pH of the base water caused by any Ca or Mg
    *        including figuring out the residual alkalinity.
    */
   double calculateSaltpH() {
      if (!this->m_rec || !this->m_rec->mash()) {
         return 0.0;
      }

      auto mash = this->m_rec->mash();
      double const allTheWaters = mash->totalMashWater_l();

      double const modifier = 1 - ( (this->m_mashRO * mash->totalInfusionAmount_l()) + (this->m_spargeRO * mash->totalSpargeAmount_l())) / allTheWaters;

      // I have no idea where the 2 comes from, but Kai did it. I wish I knew why
      // we get the initial numbers from the base water
      double const cappm = modifier * this->m_base->calcium_ppm()/Cagpm * 2;
      double const mgppm = modifier * this->m_base->magnesium_ppm()/Mggpm * 2;

      // I need mass of the salts, and all the previous math gave me
      // ppm. Multiplying by the water volume gives me the mass
      // The 3.5 and 7 come from Paul Kohlbach's work from the 1940's.
      double const totalDelta = (this->calculateRA() - cappm/3.5 - mgppm/7) * this->m_rec->mash()->totalInfusionAmount_l();
      // note: The referenced paper says the formula is
      // gristpH + strikepH * thickness/mEq. I could never get that to work.
      // the spreadsheet gave me this formula, and  it works much better.
      return totalDelta/this->m_thickness/mEq;
   }

   /**
    * \brief Calculates the pH delta caused by any salt additions.
    */
   double calculateAddedSaltpH() {

      // We need the value from the salt table model, because we need all the
      // added salts, but not the base.
      double ca   = this->m_saltAdditionsVeriTable.m_tableModel->total_Ca()/Cagpm * 2;
      double mg   = this->m_saltAdditionsVeriTable.m_tableModel->total_Mg()/Mggpm * 2;
      double hco3 = this->m_saltAdditionsVeriTable.m_tableModel->total_HCO3()/HCO3gpm;
      double co3  = this->m_saltAdditionsVeriTable.m_tableModel->total_CO3()/CO3gpm;

      // The 61 is another magic number from Kai. Sigh
      // unlike previous calculations, I am getting a mass here so I do not
      // need to convert from mg/L
      double totalDelta = 0.0 - ca/3.5 - mg/7 + (hco3+co3)/61;
      return totalDelta/this->m_thickness/mEq;
   }

   /**
    * \brief Calculates the pH adjustment caused by lactic acid, H3PO4 and/or acid malts
    */
   double calculateAcidpH() {
      double const H3PO4_gpm = 98;
      double const lactic_gpm = 90;
      double totalDelta = 0.0;

      double const lactic_amt   = this->m_saltAdditionsVeriTable.m_tableModel->totalAcidWeight(Salt::Type::LacticAcid);
      double const acidmalt_amt = this->m_saltAdditionsVeriTable.m_tableModel->totalAcidWeight(Salt::Type::AcidulatedMalt);
      double const H3PO4_amt    = this->m_saltAdditionsVeriTable.m_tableModel->totalAcidWeight(Salt::Type::H3PO4);

      if ( lactic_amt + acidmalt_amt > 0.0 ) {
         totalDelta += 1000 * (lactic_amt + acidmalt_amt) / lactic_gpm;
      }
      if ( H3PO4_amt > 0.0 ) {
         totalDelta += 1000 * H3PO4_amt / H3PO4_gpm;
      }

      return totalDelta/mEq/this->m_thickness;
   }

   /**
    * \brief This figures out the expected mash pH. It really just calls all the other pieces to get those calculations
    *        and then sums them all up.
    */
   double calculateMashpH() {
      double mashpH = 0.0;

      if (this->m_rec && this->m_rec->fermentableAdditions().size()) {
         double gristpH   = this->calculateGristpH();
         double basepH    = this->calculateSaltpH();
         double saltpH    = this->calculateAddedSaltpH();
         double acids     = this->calculateAcidpH();

         // qDebug() << "basepH =" << basepH << "gristph =" << gristpH << "saltpH =" << saltpH << "acids =" << acids;
         // residual alkalinity is handled by basepH
         mashpH = basepH + gristpH + saltpH - acids;
      }

      return mashpH;
   }

   //============================================ Member variables for impl ============================================
   WaterProfileAdjustmentTool &     m_self;
   VeriTable<RecipeAdjustmentSalt>  m_saltAdditionsVeriTable;
   QVector<WaterIonDigitInfo> const m_waterIonDisplays;
   QVector<    SaltDigitInfo> const m_saltDisplays;
   WaterListModel *                 m_base_combo_list   = nullptr;
   WaterListModel *                 m_target_combo_list = nullptr;
   WaterEditor *                    m_base_editor       = nullptr;
   WaterEditor *                    m_target_editor     = nullptr;
   Recipe *                         m_rec               = nullptr;
   std::shared_ptr<Water>           m_base              = nullptr;
   std::shared_ptr<Water>           m_target            = nullptr;
   double                           m_mashRO            = 0.0;
   double                           m_spargeRO          = 0.0;
   double                           m_total_grains      = 0.0;
   double                           m_thickness         = 0.0;
   double                           m_weighted_colors   = 0.0;
   WaterSortFilterProxyModel *      m_base_filter       = nullptr;
   WaterSortFilterProxyModel *      m_target_filter     = nullptr;
};

WaterProfileAdjustmentTool::WaterProfileAdjustmentTool(QWidget* parent) :
   QDialog{parent},
   pimpl{std::make_unique<impl>(*this)}{

   setupUi(this);
   // initialize the two buttons and lists (I think)
   this->pimpl->m_base_combo_list = new WaterListModel(baseProfileCombo);
   this->pimpl->m_base_filter    = new WaterSortFilterProxyModel(baseProfileCombo);
   this->pimpl->m_base_filter->setDynamicSortFilter(true);
   this->pimpl->m_base_filter->setSortLocaleAware(true);
   this->pimpl->m_base_filter->setSourceModel(this->pimpl->m_base_combo_list);
   this->pimpl->m_base_filter->sort(0);
   baseProfileCombo->setModel(this->pimpl->m_base_filter);

   this->pimpl->m_target_combo_list = new WaterListModel(targetProfileCombo);
   this->pimpl->m_target_filter    = new WaterSortFilterProxyModel(targetProfileCombo);
   this->pimpl->m_target_filter->setDynamicSortFilter(true);
   this->pimpl->m_target_filter->setSortLocaleAware(true);
   this->pimpl->m_target_filter->setSourceModel(this->pimpl->m_target_combo_list);
   this->pimpl->m_target_filter->sort(0);
   targetProfileCombo->setModel(this->pimpl->m_target_filter);

   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_ca  , btDigit_ca  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_cl  , btDigit_cl  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_hco3, btDigit_hco3, double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_mg  , btDigit_mg  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_na  , btDigit_na  , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_so4 , btDigit_so4 , double, Measurement::PhysicalQuantity::MassFractionOrConc, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_pH  , btDigit_ph  , double, Measurement::PhysicalQuantity::Acidity           , 1);

   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_totalcacl2 , btDigit_totalcacl2 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_totalcaco3 , btDigit_totalcaco3 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_totalcaso4 , btDigit_totalcaso4 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_totalmgso4 , btDigit_totalmgso4 , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_totalnacl  , btDigit_totalnacl  , double, Measurement::PhysicalQuantity::Mass, 2);
   SMART_FIELD_INIT_FS(WaterProfileAdjustmentTool, label_totalnahco3, btDigit_totalnahco3, double, Measurement::PhysicalQuantity::Mass, 2);

   for (auto & waterIonDisplay : this->pimpl->m_waterIonDisplays) {
      waterIonDisplay.digitWidget->setLimits(0.0, 1000.0);
      waterIonDisplay.digitWidget->setQuantity(0.0);
      waterIonDisplay.digitWidget->setMessages(tr("Too low for target profile." ),
                                               tr("In range for target profile."),
                                               tr("Too high for target profile."));
   }
   // we can be a bit more specific with pH
   this->btDigit_ph->setLowLim(5.0);
   this->btDigit_ph->setHighLim(5.5);
   this->btDigit_ph->setQuantity(7.0);

   // since all the things are now digits, lets get the totals configured
   for (auto & saltDisplay : this->pimpl->m_saltDisplays) {
      saltDisplay.digitWidget->setConstantColor(SmartDigitWidget::ColorType::Black);
      saltDisplay.digitWidget->setQuantity(0.0);
   }

   this->pimpl->m_base_editor   = new WaterEditor(this, "Base");
   this->pimpl->m_target_editor = new WaterEditor(this, "Target");

   MainWindow & mainWindow{MainWindow::instance()};

   this->pimpl->m_saltAdditionsVeriTable.setup(this->tableView_saltAdjustments, &mainWindow.getEditor<Salt>());
   this->pimpl->m_saltAdditionsVeriTable.setSortColumn(RecipeAdjustmentSaltTableModel::ColumnIndex::Name);

   // all the signals
   connect(baseProfileCombo,   QOverload<int>::of(&QComboBox::activated), this, &WaterProfileAdjustmentTool::update_baseProfile  );
   connect(targetProfileCombo, QOverload<int>::of(&QComboBox::activated), this, &WaterProfileAdjustmentTool::update_targetProfile);

   connect(baseProfileButton,   &WaterButton::clicked, this->pimpl->m_base_editor,   &QWidget::show);
   connect(targetProfileButton, &WaterButton::clicked, this->pimpl->m_target_editor, &QWidget::show);

   connect(this->pimpl->m_saltAdditionsVeriTable.m_tableModel.get(),
           &RecipeAdjustmentSaltTableModel::newTotals,
           this,
           &WaterProfileAdjustmentTool::newTotals   );
   connect(pushButton_addSalt        , &QAbstractButton::clicked, &mainWindow.getCatalog<Salt>(), &QWidget::show  );
   connect(pushButton_removeSalt     , &QAbstractButton::clicked, this                          , &WaterProfileAdjustmentTool::removeSalts );

   connect(spinBox_mashRO,   QOverload<int>::of(&QSpinBox::valueChanged), this, &WaterProfileAdjustmentTool::setMashRO  );
   connect(spinBox_spargeRO, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaterProfileAdjustmentTool::setSpargeRO);

   connect(buttonBox_save, &QDialogButtonBox::accepted, this, &WaterProfileAdjustmentTool::saveAndClose );
   connect(buttonBox_save, &QDialogButtonBox::rejected, this, &WaterProfileAdjustmentTool::clearAndClose);

   return;
}

WaterProfileAdjustmentTool::~WaterProfileAdjustmentTool() = default;

void WaterProfileAdjustmentTool::setMashRO(int val) {
   this->pimpl->m_mashRO = val/100.0;
   if (this->pimpl->m_base) {
      this->pimpl->m_base->setMashRo_pct(this->pimpl->m_mashRO);
   }
   this->newTotals();
   return;
}

void WaterProfileAdjustmentTool::setSpargeRO(int val) {
   this->pimpl->m_spargeRO = val/100.0;
   if (this->pimpl->m_base) {
      this->pimpl->m_base->setSpargeRo_pct(this->pimpl->m_spargeRO);
   }
   newTotals();
   return;
}

void WaterProfileAdjustmentTool::setRecipe(Recipe *rec) {
   if (!rec) {
      return;
   }

   this->pimpl->m_rec = rec;
   auto mash = this->pimpl->m_rec->mash();
   this->pimpl->m_saltAdditionsVeriTable.m_tableModel->observeRecipe(this->pimpl->m_rec);

   if (!mash || mash->mashSteps().size() == 0 ) {
      qWarning() << QString("Cannot set water chemistry without a mash");
      return;
   }

   baseProfileButton->setRecipe(this->pimpl->m_rec);
   targetProfileButton->setRecipe(this->pimpl->m_rec);

   for (auto waterUse : this->pimpl->m_rec->waterUses()) {
      Water * waterRaw = waterUse->water();
      if (waterRaw) {
         auto water = ObjectStoreWrapper::getSharedFromRaw(waterRaw);
         if (water->type() == Water::Type::Base) {
            qDebug() << Q_FUNC_INFO << "Base Water" << *water;
            this->pimpl->m_base = water;
         } else if (water->type() == Water::Type::Target) {
            qDebug() << Q_FUNC_INFO << "Target Water" << *water;
            this->pimpl->m_target = water;
         }
      }
   }

   // I need these numbers before we set the ranges
   for (auto const & fermentableAddition : this->pimpl->m_rec->fermentableAdditions() ) {
      // .:TBD:. This almost certainly needs some refinement
      switch (fermentableAddition->fermentable()->type()) {
         case Fermentable::Type::Grain:
         case Fermentable::Type::Extract:
         case Fermentable::Type::Dry_Extract:
            if (fermentableAddition->getMeasure() == Measurement::PhysicalQuantity::Mass) {
               this->pimpl->m_total_grains += fermentableAddition->amount().quantity;
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

   // Now we've got this->pimpl->m_total_grains, we need to loop over fermentable again
   for (auto const & fermentableAddition : this->pimpl->m_rec->fermentableAdditions() ) {
      // .:TBD:. This almost certainly needs some refinement
      switch (fermentableAddition->fermentable()->type()) {
         case Fermentable::Type::Grain:
         case Fermentable::Type::Extract:
         case Fermentable::Type::Dry_Extract:
            if (fermentableAddition->getMeasure() == Measurement::PhysicalQuantity::Mass) {
               double lovi = (fermentableAddition->fermentable()->color_srm() +0.6 ) / 1.35;
               this->pimpl->m_weighted_colors   += (fermentableAddition->amount().quantity/this->pimpl->m_total_grains)*lovi;
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

   this->pimpl->m_thickness = this->pimpl->m_rec->mash()->totalInfusionAmount_l()/this->pimpl->m_total_grains;

   if (this->pimpl->m_base) {

      this->pimpl->m_mashRO = this->pimpl->m_base->mashRo_pct().value_or(0.0);
      spinBox_mashRO->setValue( QVariant(this->pimpl->m_mashRO * 100).toInt());
      this->pimpl->m_spargeRO = this->pimpl->m_base->spargeRo_pct().value_or(0.0);
      spinBox_spargeRO->setValue( QVariant(this->pimpl->m_spargeRO * 100).toInt());

      baseProfileButton->setWater(this->pimpl->m_base);
      this->pimpl->m_base_editor->setEditItem(this->pimpl->m_base);
      // all of the magic to set the sliders happens in newTotals(). So don't do it twice
   }
   if (this->pimpl->m_target && this->pimpl->m_target != this->pimpl->m_base) {
      targetProfileButton->setWater(this->pimpl->m_target);
      this->pimpl->m_target_editor->setEditItem(this->pimpl->m_target);

      this->pimpl->setDigits();
   }
   newTotals();

   return;
}

void WaterProfileAdjustmentTool::update_baseProfile(int selected) {
   Q_UNUSED(selected)
   if (!this->pimpl->m_rec) {
      return;
   }

   QModelIndex proxyIdx(this->pimpl->m_base_filter->index(baseProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(this->pimpl->m_base_filter->mapToSource(proxyIdx));
   Water const * parent = this->pimpl->m_base_combo_list->at(sourceIdx.row());
   if (parent) {
      // The copy constructor won't copy the key (aka database ID), so the new object will be in-memory only until we
      // explicitly insert it in the Object Store (which will be done if/when it is added to the Recipe).  Note
      // however that we do need to ensure the link to the "parent" water is not lost - hence the call to makeChild().
      this->pimpl->m_base = std::make_shared<Water>(*parent);
      this->pimpl->m_base->makeChild(*parent);
      this->pimpl->m_base->setType(Water::Type::Base);
      qDebug() << Q_FUNC_INFO << "Made base child" << *this->pimpl->m_base << "from parent" << parent;

      baseProfileButton->setWater(this->pimpl->m_base);
      this->pimpl->m_base_editor->setEditItem(this->pimpl->m_base);
      this->newTotals();
   }
   return;
}

void WaterProfileAdjustmentTool::update_targetProfile(int selected) {
   Q_UNUSED(selected)
   if (!this->pimpl->m_rec) {
      return;
   }

   QModelIndex proxyIdx(this->pimpl->m_target_filter->index(targetProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(this->pimpl->m_target_filter->mapToSource(proxyIdx));
   Water* parent = this->pimpl->m_target_combo_list->at(sourceIdx.row());

   if (parent) {
      // Comment above for copy of this->pimpl->m_base applies equally here
      this->pimpl->m_target = std::make_shared<Water>(*parent);
      this->pimpl->m_target->makeChild(*parent);
      this->pimpl->m_target->setType(Water::Type::Target);
      qDebug() << Q_FUNC_INFO << "Made target child" << *this->pimpl->m_target << "from parent" << parent;

      targetProfileButton->setWater(this->pimpl->m_target);
      this->pimpl->m_target_editor->setEditItem(this->pimpl->m_target);

      this->pimpl->setDigits();
   }
   return;
}

void WaterProfileAdjustmentTool::newTotals() {
   if (!this->pimpl->m_rec || !this->pimpl->m_rec->mash()) {
      return;
   }

   auto mash = this->pimpl->m_rec->mash();
   double const allTheWaters = mash->totalMashWater_l();

   if (qFuzzyCompare(allTheWaters, 0.0)) {
      qWarning() << Q_FUNC_INFO << "Cannot set strike water chemistry without a mash";
      return;
   }

   // Two major things need to happen here:
   //   o the totals need to be updated
   //   o the digits need to be updated

   for (auto & saltDisplay : this->pimpl->m_saltDisplays) {
      // RecipeAdjustmentSaltTableModel::total does all the work for us
      Measurement::Amount const total = this->pimpl->m_saltAdditionsVeriTable.m_tableModel->total(saltDisplay.type);
      qDebug() << Q_FUNC_INFO << "Total for" << saltDisplay.type << "is" << total;
      saltDisplay.digitWidget->setAmount(total);
   }

   // the total_* method return the numerator, we supply the denominator and
   // include the base water ppm. The confusing math generates an adjustment
   // for the base water that depends the %RO in the mash and sparge water

   if (this->pimpl->m_base) {
      // 'd' means 'diluted'. They make calculating the modifier readable
      double const dInfuse = this->pimpl->m_mashRO * mash->totalInfusionAmount_l();
      double const dSparge = this->pimpl->m_spargeRO * mash->totalSpargeAmount_l();

      // I hope this is right. All this 'rithmetic is making me head hurt.
      double modifier = 1.0 - (dInfuse + dSparge) / allTheWaters;

      for (auto & waterIonDisplay : this->pimpl->m_waterIonDisplays) {
         double const mPPM = modifier * this->pimpl->m_base->ppm(waterIonDisplay.ion);
         waterIonDisplay.digitWidget->setQuantity(
            this->pimpl->m_saltAdditionsVeriTable.m_tableModel->total(waterIonDisplay.ion) / allTheWaters + mPPM
         );
      }

      btDigit_ph->setQuantity(this->pimpl->calculateMashpH());

   } else {
      for (auto & waterIonDisplay : this->pimpl->m_waterIonDisplays) {
         waterIonDisplay.digitWidget->setQuantity(
            this->pimpl->m_saltAdditionsVeriTable.m_tableModel->total(waterIonDisplay.ion) / allTheWaters
         );
      }
   }
   return;
}

void WaterProfileAdjustmentTool::removeSalts() {
   this->pimpl->m_saltAdditionsVeriTable.removeSelected();
   return;
}

void WaterProfileAdjustmentTool::saveAndClose() {
   this->pimpl->m_saltAdditionsVeriTable.m_tableModel->saveAndClose();

   // TODO: For the moment we are not saving RecipeUseOfWater to the Recipe because we first need to decide what it
   //       means!

//   if (this->pimpl->m_base && this->pimpl->m_base->key() < 0) {
//      // Recipe will take care of adding to the relevant object store
//      auto waterUse = std::make_shared<RecipeUseOfWater>(tr("Use of %1").arg(this->pimpl->m_base->name()),
//                                                         this->pimpl->m_rec->key(),
//                                                         this->pimpl->m_base->key());
//      this->pimpl->m_rec->addAddition(waterUse);
//   }
//   if (this->pimpl->m_target && this->pimpl->m_target->key() < 0) {
//      // Recipe will take care of adding to the relevant object store
//      auto waterUse = std::make_shared<RecipeUseOfWater>(tr("Use of %1").arg(this->pimpl->m_target->name()),
//                                                         this->pimpl->m_rec->key(),
//                                                         this->pimpl->m_target->key());
//      this->pimpl->m_rec->addAddition(waterUse);
//   }

   this->setVisible(false);
   return;
}

void WaterProfileAdjustmentTool::clearAndClose() {
   this->setVisible(false);
   return;
}
