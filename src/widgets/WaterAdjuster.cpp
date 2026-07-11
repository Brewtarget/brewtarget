/*======================================================================================================================
 * widgets/WaterAdjuster.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
 =====================================================================================================================*/
#include "widgets/WaterAdjuster.h"

#include <limits>

#include <Algorithms.h>
#include <QButtonGroup>
#include <QComboBox>
#include <QFont>
#include <QInputDialog>
#include <QVector>

#include "MainWindow.h"
#include "catalogs/FermentableCatalog.h"
#include "catalogs/MiscCatalog.h"
#include "editors/WaterEditor.h"
#include "measurement/ColorMethods.h"
#include "measurement/PhysicalConstants.h"
#include "measurement/Unit.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionMisc.h"
#include "model/Water.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.h"
#include "qtModels/tableModels/RecipeAdditionFermentableTableModel.h"
#include "qtModels/tableModels/RecipeAdditionMiscTableModel.h"
#include "utils/VeriTable.h"
#include "widgets/SmartDigitWidget.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_WaterAdjuster.cpp"
#endif


//
// All the pH calculations are taken from the work done by Kai Troester and originally published at
// http://braukaiser.com/wiki/index.php/Beer_color_to_mash_pH_(v2.0) with additional information being gleaned from the
// spreadsheet associated with that link.
//


namespace {

   struct WaterVolumes {
      double const totalMashWater_l  ;
      double const roWaterInfuse_l   ;
      double const roWaterSparge_l   ;
      double const roWaterTotal_l    ;
      double const waterBaseTotal_l  ;
      double const waterBase_fraction;
      double const roWater_fraction  ;

      WaterVolumes(double const totalMashWater_l,
                   double const roWaterInfuse_l,
                   double const roWaterSparge_l) :
         totalMashWater_l  {totalMashWater_l},
         roWaterInfuse_l   {roWaterInfuse_l},
         roWaterSparge_l   {roWaterSparge_l},
         roWaterTotal_l    {roWaterInfuse_l + roWaterSparge_l},
         waterBaseTotal_l  {totalMashWater_l - roWaterTotal_l  },
         waterBase_fraction{waterBaseTotal_l / totalMashWater_l},
         roWater_fraction  {roWaterTotal_l / totalMashWater_l} {
         return;
      }
   };

   //
   // For water chemistry, we often need to think about ions and valences.
   //
   // In simple terms, the valence of an element is the number of hydrogen atoms that can combine with it when no other
   // elements are present (aka in a binary hydride).  (It can also be twice the number of oxygen atoms combining with
   // an element in its oxide or oxides.)
   //
   // When we're measuring ion concentration in a solution, a useful measure is "milliequivalents per liter", aka mEq/L.
   // This measures concentration of ions in a solution, taking into account the charge of the ions.   It's particularly
   // useful for solutions with electrolytes, where the chemical reactivity is dependent not only on the concentration
   // but also on the charge of the ions present.  We derive it as:
   //
   //    mEq/L = (C × V) ÷ MW
   //
   // where:
   //
   //    - C is the mass concentration in mg/L
   //    - MW is the molecular weight in g/mol
   //    - V is the valence of the substance
   //
   // Eg, suppose we have regular salt, NaCl, in mass concentration of 200 mg/L.  Molecular weight of NaCl is
   // 58.44 g/mol.  Valence is 1 (for Na⁺ or Cl⁻).  So, the ion concentration is (200 × 1) / 58.44 ≈ 3.4223 mEq/L.
   //
   // The explanation above skips over what an "equivalent" is.  If you wade through
   // https://en.wikipedia.org/wiki/Equivalent_(chemistry) and
   // https://byo.com/articles/understanding-residual-alkalinity-ph/, you can gather that an "equivalent" (or
   // "equivalent weight") is an archaic term for the amount of a substance needed to do one of the following:
   //    - react with or supply one mole of hydrogen ions (H+) in an acid–base reaction
   //    - react with or supply one mole of electrons in a redox reaction.
   // Thus, the number of equivalents of a given ion in a solution is equal to the number of moles of that ion
   // multiplied by its valence.  Eg, for a solution of 1 mole of NaCl and 1 mole of CaCl₂.  The solution has 1 mole or
   // 1 equiv Na⁺, 1 mole or 2 equiv Ca²⁺, and 3 mole or 3 equiv Cl⁻.
   //
   // You can get the equivalent for a substance by dividing its molecular weight by its valence (ie Eq = MW / V).  More
   // formally, for elements with a single well-defined valence:
   //
   //    Equivalent weight = atomic weight ÷ valence
   //
   // And more generally:
   //
   //    Equivalent weight = molar mass ÷ n
   //
   // where n is the number of electrons transferred, protons exchanged, or charge units involved in the reaction.
   //
   // If you've kept reading this far, you'll see that we can get "milliequivalents per liter" by dividing mass
   // concentration (in mg/L) by equivalent weight (EW):
   //
   //    mEq/L = C ÷ EW = C ÷ (MW ÷ V) = (C × V) ÷ MW
   //
   // In the original code here, there was an assumption that we only needed one EW for all our calculations and its
   // value was 50.0.  IDK if this is true, or close enough, but it's confusing.
   //
   // TODO: Refactor things to be a bit more explicit.
   //

   //
   //
   // So you divide a mass concentration (in
   // mg/L) by this number to convert it to mEq/L.
   //

   // I've seen some confusion over this constant. 50 mEq/l is what Kai uses.
   double constexpr mEq = 50.0;

   // The pH of a beer with no color
   double constexpr noColorBeer_ph = 5.6;
   // Magic constants Kai derives in the document above.
   double constexpr pHSlopeLight = 0.21;
   double constexpr pHSlopeDark  = 0.06;

   auto const seriesNameBase  {WaterEditor::tr("Base Profile" )};
   auto const seriesNameTarget{WaterEditor::tr("Target Profile")};
   auto const seriesNameActual{WaterEditor::tr("Actual Profile")};

   auto constexpr colorForBase       {QColorConstants::Svg::lightslategrey};
   auto constexpr colorForTarget     {QColorConstants::Svg::dodgerblue    };
   auto constexpr colorForActual     {QColorConstants::Svg::green         };
   auto constexpr colorForWaterAgents{QColorConstants::Svg::blueviolet    };
   auto constexpr colorForAcidMalts  {QColorConstants::Svg::darkgoldenrod };

   auto constexpr styleForBase  {Qt::SolidLine};
   auto constexpr styleForTarget{Qt::SolidLine};
   auto constexpr styleForActual{Qt::DashLine };

}

// This private implementation class holds all private non-virtual members of WaterAdjuster
class WaterAdjuster::impl {
public:
   //
   // These mini structs help us write code to iterate over all the water ion displays or all the salt displays.
   // NOTE that it is intentional that the digitWidget member is a reference to a pointer.  The pointers in question
   // (eg btDigit_ca, btDigit_totalcacl2, etc from waterAdjuster.ui) get initialised _after_ the call to the impl
   // constructor (by setupUi), so taking a copy of the pointer is no good.
   //
   struct WaterIonDigitInfo {
      Water::MineralIon ion;
      SmartDigitWidget * & digitWidget;
   };

   struct WaterAdjustmentDigitInfo {
      Misc::WaterAgentType type;
      SmartDigitWidget * & digitWidget;
   };

   /**
    * Constructor
    */
   explicit impl(WaterAdjuster & self) :
      m_self {self},
      m_miscAdditionsVeriTable{},
      m_fermentableAdditionsVeriTable{},
      m_waterIonDisplays{
         {
            {Water::MineralIon::Calcium    , m_self.btDigit_ca  },
            {Water::MineralIon::Chloride   , m_self.btDigit_cl  },
            {Water::MineralIon::Bicarbonate, m_self.btDigit_hco3},
            {Water::MineralIon::Magnesium  , m_self.btDigit_mg  },
            {Water::MineralIon::Sodium     , m_self.btDigit_na  },
            {Water::MineralIon::Sulfate    , m_self.btDigit_so4 },
         }
      } {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   void setWaterBase(std::shared_ptr<Water> const & profile) {
      // TBD: At the moment, the UI doesn't let you unset a profile after you set it
      if (profile) {
         this->m_base = profile;
         qDebug() << Q_FUNC_INFO << "Set base to" << *this->m_base;
         this->m_editorForBase->setEditItem(this->m_base);
         this->m_self.radarChart->addSeries(seriesNameBase, *this->m_base, colorForBase, styleForBase);
         this->m_self.radarChart->replot();
         this->m_self.newTotals();
      }
      return;
   }

   void setWaterTarget(std::shared_ptr<Water> const & profile) {
      // TBD: At the moment, the UI doesn't let you unset a profile after you set it
      if (profile) {
         this->m_target = profile;
         qDebug() << Q_FUNC_INFO << "Set target to" << *this->m_target;
         this->m_editorForTarget->setEditItem(this->m_target);
         this->m_self.radarChart->addSeries(seriesNameTarget, *this->m_target, colorForTarget, styleForTarget);
         this->m_self.radarChart->replot();
         this->m_self.newTotals();
         this->setDigits();
      }
      return;
   }

   /**
    *
    */
   void setDigits() {
      if (!this->m_target) {
         return;
      }

      for (auto const & [ion, digitWidget] : this->m_waterIonDisplays) {
         double const ppm = *this->m_target->ionConcentration_ppm(ion);
         double const min_ppm = ppm * 0.95;
         double const max_ppm = ppm * 1.05;
         digitWidget->setLimits(min_ppm, max_ppm);
         digitWidget->setMessages(tr("Minimum expected concentration is %1 ppm").arg(min_ppm),
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

         double base_alk = ( 1.0 - this->m_rec->roWaterMash_pct()) * this->m_base->alkalinity_ppm().value_or(0.0);
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
      double gristPh = noColorBeer_ph;
      double pHAdjustment = 0.0;

      if ( this->m_rec && this->m_rec->fermentableAdditions().size() ) {

         double const platoRatio = 1/Measurement::Units::plato.fromCanonical(this->m_rec->og());
         double const color = this->m_rec->color_srm();
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
                     colorFromGrain = (fermentableAddition->amount().quantity / this->m_totalGrains_kg ) * lovi;
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
         double const colorRatio = colorFromGrain/this->m_weightedColor_lovibond;
         pHAdjustment = platoRatio * ( pHSlopeLight * (1-colorRatio) + pHSlopeDark * colorRatio) *color;

         gristPh = gristPh - pHAdjustment;
      }
      return gristPh;
   }

   /**
    * \brief Calculates the pH of the base water caused by any Ca or Mg
    *        including figuring out the residual alkalinity.
    */
   double calculateWaterAdjustmentpH(WaterVolumes const & waters) {
      if (!this->m_rec || !this->m_rec->mash()) {
         return 0.0;
      }

      // I have no idea where the 2 comes from, but Kai did it. I wish I knew why
      // we get the initial numbers from the base water
      double const ca_ppm =
         this->m_base ? waters.waterBase_fraction * this->m_base->calcium_ppm  ()/MolarMass::Calcium * 2 : 0.0;
      double const mg_ppm =
         this->m_base ? waters.waterBase_fraction * this->m_base->magnesium_ppm()/MolarMass::Magnesium * 2 : 0.0;

      // I need mass of the salts, and all the previous math gave me
      // ppm. Multiplying by the water volume gives me the mass
      // The 3.5 and 7 come from Paul Kohlbach's work from the 1940's.
      double const totalDelta = (this->calculateRA() - ca_ppm/3.5 - mg_ppm/7) * this->m_rec->mash()->totalInfusionAmount_l();
      // note: The referenced paper says the formula is
      // gristpH + strikepH * thickness/mEq. I could never get that to work.
      // the spreadsheet gave me this formula, and  it works much better.
      return totalDelta/this->m_thickness_LKg/mEq;
   }

   /**
    * \brief Calculates the pH delta caused by any salt additions.
    */
   double calculateAddedWaterAdjustmentpH() {

      // We need the value from the salt table model, because we need all the
      // added salts, but not the base.
      double const ca   = this->m_miscAdditionsVeriTable.m_tableModel->concentrationPerLiter_massConcPpm<Water::MineralIon::Calcium>    ()/MolarMass::Calcium * 2.0;
      double const mg   = this->m_miscAdditionsVeriTable.m_tableModel->concentrationPerLiter_massConcPpm<Water::MineralIon::Magnesium>  ()/MolarMass::Magnesium * 2.0;
      double const hco3 = this->m_miscAdditionsVeriTable.m_tableModel->concentrationPerLiter_massConcPpm<Water::MineralIon::Bicarbonate>()/MolarMass::Bicarbonate;
      double const co3  = this->m_miscAdditionsVeriTable.m_tableModel->concentrationPerLiter_massConcPpm<Water::MineralIon::Carbonate>  ()/MolarMass::Carbonate;

      // The 61 is another magic number from Kai. Sigh
      // unlike previous calculations, I am getting a mass here so I do not
      // need to convert from mg/L
      double const totalDelta = 0.0 - ca/3.5 - mg/7 + (hco3+co3)/61.0;
      return totalDelta/this->m_thickness_LKg/mEq;
   }

   /**
    * \brief Calculates the pH adjustment caused by lactic acid, H3PO4 and/or acid malts
    */
   double calculateAcidpH() {
      double const H3PO4_gpm = 98.0;
      double const lactic_gpm = 90.0;
      double totalDelta = 0.0;

      double const lactic_kg   = this->m_miscAdditionsVeriTable.m_tableModel->totalAcid_kg(Misc::WaterAgentType::LacticAcid);
      double const lacticFromMalt_kg = this->m_fermentableAdditionsVeriTable.m_tableModel->totalLacticAcid_kg();

      double const phosphoric_kg    = this->m_miscAdditionsVeriTable.m_tableModel->totalAcid_kg(Misc::WaterAgentType::PhosphoricAcid);

      if ( lactic_kg + lacticFromMalt_kg > 0.0 ) {
         totalDelta += 1000.0 * (lactic_kg + lacticFromMalt_kg) / lactic_gpm;
      }
      if ( phosphoric_kg > 0.0 ) {
         totalDelta += 1000.0 * phosphoric_kg / H3PO4_gpm;
      }

      return totalDelta/mEq/this->m_thickness_LKg;
   }

   /**
    * \brief This figures out the expected mash pH. It really just calls all the other pieces to get those calculations
    *        and then sums them all up.
    */
   double calculateMashpH(WaterVolumes const & waters) {
      if (this->m_rec && this->m_rec->fermentableAdditions().size()) {
         double const grist_pH   = this->calculateGristpH();
         double const base_pH    = this->calculateWaterAdjustmentpH(waters);
         double const salt_pH    = this->calculateAddedWaterAdjustmentpH();
         double const acids     = this->calculateAcidpH();

         // qDebug() << "base_pH =" << base_pH << "grist_pH =" << grist_pH << "salt_pH =" << salt_pH << "acids =" << acids;
         // residual alkalinity is handled by base_pH
         return base_pH + grist_pH + salt_pH - acids;
      }

      return 0.0;
   }

   //============================================ Member variables for impl ============================================
   WaterAdjuster &         m_self;
   VeriTable<RecipeAdditionMisc>        m_miscAdditionsVeriTable;
   VeriTable<RecipeAdditionFermentable> m_fermentableAdditionsVeriTable;


   QVector<WaterIonDigitInfo> const  m_waterIonDisplays;
   WaterEditor *                     m_editorForBase     = nullptr;
   WaterEditor *                     m_editorForTarget   = nullptr;
   Recipe *                          m_rec               = nullptr;

   std::shared_ptr<Water>            m_base              = nullptr;
   std::shared_ptr<Water>            m_target            = nullptr;
   // This is a temporary calculated water profile that doesn't get stored in the DB.  It's just a convenient way to
   // pass data to the radar chart.
   Water                             m_actual            {};

   double m_roWaterMash_pct        = 0.0;
   double m_roWaterSparge_pct      = 0.0;
   double m_totalGrains_kg         = 0.0;
   double m_thickness_LKg          = 0.0;
   double m_weightedColor_lovibond = 0.0;
};

WaterAdjuster::WaterAdjuster(QWidget* parent) :
   QWidget{parent},
   pimpl{std::make_unique<impl>(*this)}{

   setupUi(this);

   this->combo_waterBase->init();
   this->combo_waterTarget->init();

   SMART_FIELD_INIT_FS(WaterAdjuster, label_ca  , btDigit_ca  , double, Measurement::PhysicalQuantity::MassFractionOrConcentration, 2);
   SMART_FIELD_INIT_FS(WaterAdjuster, label_cl  , btDigit_cl  , double, Measurement::PhysicalQuantity::MassFractionOrConcentration, 2);
   SMART_FIELD_INIT_FS(WaterAdjuster, label_hco3, btDigit_hco3, double, Measurement::PhysicalQuantity::MassFractionOrConcentration, 2);
   SMART_FIELD_INIT_FS(WaterAdjuster, label_mg  , btDigit_mg  , double, Measurement::PhysicalQuantity::MassFractionOrConcentration, 2);
   SMART_FIELD_INIT_FS(WaterAdjuster, label_na  , btDigit_na  , double, Measurement::PhysicalQuantity::MassFractionOrConcentration, 2);
   SMART_FIELD_INIT_FS(WaterAdjuster, label_so4 , btDigit_so4 , double, Measurement::PhysicalQuantity::MassFractionOrConcentration, 2);
   SMART_FIELD_INIT_FS(WaterAdjuster, label_pH  , btDigit_ph  , double, Measurement::PhysicalQuantity::Acidity           , 1);

   this->label_baseProfile  ->setStyleSheet(QString("background-color : %1;").arg(colorForBase  .name()));
   this->label_targetProfile->setStyleSheet(QString("background-color : %1;").arg(colorForTarget.name()));
   this->groupbox_actual    ->setStyleSheet(QString("QGroupBox::title { background-color : %1; }").arg(colorForActual.name()));

   this->radarChart->init(
      tr("PPM"),
      50,
      {
         {PropertyNames::Water::calcium_ppm    , Water::ionDisplayNames[Water::MineralIon::Calcium    ]},
         {PropertyNames::Water::bicarbonate_ppm, Water::ionDisplayNames[Water::MineralIon::Bicarbonate]},
         {PropertyNames::Water::sulfate_ppm    , Water::ionDisplayNames[Water::MineralIon::Sulfate    ]},
         {PropertyNames::Water::chloride_ppm   , Water::ionDisplayNames[Water::MineralIon::Chloride   ]},
         {PropertyNames::Water::sodium_ppm     , Water::ionDisplayNames[Water::MineralIon::Sodium     ]},
         {PropertyNames::Water::magnesium_ppm  , Water::ionDisplayNames[Water::MineralIon::Magnesium  ]}
      }
   );

   this->radarChart->addSeries(seriesNameActual, this->pimpl->m_actual, colorForActual, styleForActual);

   for (const auto & [ion, digitWidget] : this->pimpl->m_waterIonDisplays) {
      digitWidget->setLimits(0.0, 1000.0);
      digitWidget->setQuantity(0.0);
      this->pimpl->m_actual.setIonConcentration_ppm(ion, 0.0);
      digitWidget->setMessages(tr("Too low for target profile." ),
                               tr("In range for target profile."),
                               tr("Too high for target profile."));
   }
   // we can be a bit more specific with pH
   this->btDigit_ph->setLowLim(5.0);
   this->btDigit_ph->setHighLim(5.5);
   this->btDigit_ph->setQuantity(7.0);

   this->pimpl->m_editorForBase   = new WaterEditor(this, "Base");
   this->pimpl->m_editorForTarget = new WaterEditor(this, "Target");



   return;
}

WaterAdjuster::~WaterAdjuster() = default;

void WaterAdjuster::init() {
   MainWindow const & mainWindow{MainWindow::instance()};

   this->pimpl->m_miscAdditionsVeriTable.setup(this->tableView_waterAgents, &mainWindow.getEditor<Misc>());
   this->pimpl->m_miscAdditionsVeriTable.setSortColumn(RecipeAdditionMiscTableModel::ColumnIndex::Name);

   this->pimpl->m_fermentableAdditionsVeriTable.setup(this->tableView_acidMalts, &mainWindow.getEditor<Fermentable>());
   this->pimpl->m_fermentableAdditionsVeriTable.setSortColumn(RecipeAdditionFermentableTableModel::ColumnIndex::Name);

   this->pimpl->m_miscAdditionsVeriTable.m_sortFilterProxyModel->setAdditionalFilter(
      [](RecipeAdditionMisc const & miscAddition) -> bool {
         return miscAddition.misc()->type() == Misc::Type::WaterAgent;
      }
   );

   this->pimpl->m_fermentableAdditionsVeriTable.m_sortFilterProxyModel->setAdditionalFilter(
      [](RecipeAdditionFermentable const & fermentableAddition) -> bool {
         return fermentableAddition.fermentable()->lacticAcidByWeight_pct().has_value();
      }
   );

   // all the signals
   connect(combo_waterBase  , QOverload<int>::of(&QComboBox::activated)  , this, &WaterAdjuster::updateBaseProfile  );
   connect(combo_waterTarget, QOverload<int>::of(&QComboBox::activated)  , this, &WaterAdjuster::updateTargetProfile);
   connect(spinBox_mashRO   , QOverload<int>::of(&QSpinBox::valueChanged), this, &WaterAdjuster::updateRoWaterMash_pct  );
   connect(spinBox_spargeRO , QOverload<int>::of(&QSpinBox::valueChanged), this, &WaterAdjuster::updateRoWaterSparge_pct);

   connect(pushButton_editBaseProfile,   &QPushButton::clicked, this->pimpl->m_editorForBase,   &QWidget::show);
   connect(pushButton_editTargetProfile, &QPushButton::clicked, this->pimpl->m_editorForTarget, &QWidget::show);

   connect(this->pimpl->m_miscAdditionsVeriTable.m_tableModel.get(),
           &RecipeAdditionMiscTableModel::newTotals,
           this,
           &WaterAdjuster::newTotals   );
   connect(pushButton_addWaterAgent   , &QAbstractButton::clicked, &mainWindow.getCatalog<Misc>(), &QWidget::show  );
   connect(pushButton_removeWaterAgent, &QAbstractButton::clicked, this                          , &WaterAdjuster::removeSelectedWaterAdjustments );

// ¥¥¥¥¥¥ TODO: Make this work!
//   connect(this->pimpl->m_fermentableAdditionsVeriTable.m_tableModel.get(),
//           &RecipeAdditionFermentableTableModel::newTotals,
//           this,
//           &WaterAdjuster::newTotals   );
   connect(pushButton_addAcidMalt   , &QAbstractButton::clicked, &mainWindow.getCatalog<Fermentable>(), &QWidget::show  );
   connect(pushButton_removeAcidMalt, &QAbstractButton::clicked, this                                 , &WaterAdjuster::removeSelectedAcidMalts );

   return;
}

void WaterAdjuster::updateRoWaterMash_pct(int const val) {
   this->pimpl->m_roWaterMash_pct = val;
   if (this->pimpl->m_rec) {
      this->pimpl->m_rec->setRoWaterMash_pct(this->pimpl->m_roWaterMash_pct);
   }
   this->newTotals();
   return;
}

void WaterAdjuster::updateRoWaterSparge_pct(int const val) {
   this->pimpl->m_roWaterSparge_pct = val;
   if (this->pimpl->m_rec) {
      this->pimpl->m_rec->setRoWaterSparge_pct(this->pimpl->m_roWaterSparge_pct);
   }
   this->newTotals();
   return;
}

void WaterAdjuster::setRecipe(Recipe * rec) {
   if (!rec) {
      return;
   }

   this->pimpl->m_rec = rec;

   auto const mash = this->pimpl->m_rec->mash();
   this->pimpl->       m_miscAdditionsVeriTable.m_tableModel->observeRecipe(this->pimpl->m_rec);
   this->pimpl->m_fermentableAdditionsVeriTable.m_tableModel->observeRecipe(this->pimpl->m_rec);

   if (!mash || mash->mashSteps().size() == 0 ) {
      qWarning() << QString("Cannot set water chemistry without a mash");
      return;
   }

   auto const waterBase   = this->pimpl->m_rec->waterBase();
   this->combo_waterBase->setItem(waterBase);
   this->pimpl->setWaterBase(waterBase);
   auto const waterTarget = this->pimpl->m_rec->waterTarget();
   this->combo_waterTarget->setItem(waterTarget);
   this->pimpl->setWaterTarget(waterTarget);

   this->pimpl->m_roWaterMash_pct = this->pimpl->m_rec->roWaterMash_pct();
   spinBox_mashRO->setValue( QVariant(this->pimpl->m_roWaterMash_pct).toInt());
   this->pimpl->m_roWaterSparge_pct = this->pimpl->m_rec->roWaterSparge_pct();
   spinBox_spargeRO->setValue( QVariant(this->pimpl->m_roWaterSparge_pct).toInt());

   // I need these numbers before we set the ranges
   for (auto const & fermentableAddition : this->pimpl->m_rec->fermentableAdditions() ) {
      // .:TBD:. This almost certainly needs some refinement
      switch (fermentableAddition->fermentable()->type()) {
         case Fermentable::Type::Grain:
         case Fermentable::Type::Extract:
         case Fermentable::Type::Dry_Extract:
            if (fermentableAddition->getMeasure() == Measurement::PhysicalQuantity::Mass) {
               this->pimpl->m_totalGrains_kg += fermentableAddition->amount().quantity;
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
               this->pimpl->m_weightedColor_lovibond   += (fermentableAddition->amount().quantity/this->pimpl->m_totalGrains_kg)*lovi;
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

   this->pimpl->m_thickness_LKg = this->pimpl->m_rec->mash()->totalInfusionAmount_l()/this->pimpl->m_totalGrains_kg;


   if (this->pimpl->m_base) {

      this->pimpl->m_editorForBase->setEditItem(this->pimpl->m_base);
      // all of the magic to set the sliders happens in newTotals(). So don't do it twice
   }
   if (this->pimpl->m_target && this->pimpl->m_target != this->pimpl->m_base) {
      this->pimpl->m_editorForTarget->setEditItem(this->pimpl->m_target);

      this->pimpl->setDigits();
   }
   newTotals();

   return;
}

void WaterAdjuster::updateBaseProfile(int const selected) const {
   Q_UNUSED(selected)
   if (!this->pimpl->m_rec) {
      return;
   }

   auto const waterBase{this->combo_waterBase->getItem()};
   this->pimpl->m_rec->setWaterBase(waterBase);
   this->pimpl->setWaterBase(waterBase);

   return;
}

void WaterAdjuster::updateTargetProfile(int const selected) const {
   Q_UNUSED(selected)
   if (!this->pimpl->m_rec) {
      return;
   }

   auto const waterTarget{this->combo_waterTarget->getItem()};
   this->pimpl->m_rec->setWaterTarget(waterTarget);
   this->pimpl->setWaterTarget(waterTarget);

   return;
}

void WaterAdjuster::newTotals() {
   if (!this->pimpl->m_rec || !this->pimpl->m_rec->mash()) {
      qDebug() << Q_FUNC_INFO << "¥¥¥¥¥¥¥¥¥¥ Bailing ¥¥¥¥¥¥¥¥¥¥¥";
      return;
   }

   // Two major things need to happen here:
   //   o the totals need to be updated
   //   o the digits need to be updated

   auto const mash = this->pimpl->m_rec->mash();
   if (qFuzzyCompare(mash->totalMashWater_l(), 0.0)) {
      qWarning() << Q_FUNC_INFO << "Cannot set strike water chemistry without a mash";
      return;
   }
   WaterVolumes const waters{mash->totalMashWater_l(),
                             (this->pimpl->m_roWaterMash_pct / 100.0) * mash->totalInfusionAmount_l(),
                             (this->pimpl->m_roWaterSparge_pct / 100.0) * mash->totalSpargeAmount_l()};

   for (auto const & [ion, digitWidget] : this->pimpl->m_waterIonDisplays) {
      //
      // If the base water is, say, a quarter of all the water then, without any other additions, the total ion
      // concentration in all the water is also a quarter of what it is in the base water.
      //
      // If no base water is specified then we assume it is RO water -- ie with no ions.
      //
      double const ionFromBaseWater_ppm =
         this->pimpl->m_base ? waters.waterBase_fraction * *this->pimpl->m_base->ionConcentration_ppm(ion) : 0.0;

      //
      // RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm gives us the concentration of an ion if the
      // all the Misc additions are dissolved in 1 liter of water, so we have to divide it by the total number of
      // liters of water.
      //
      double const ionFromWaterAgents_ppm =
         this->pimpl->m_miscAdditionsVeriTable.m_tableModel->concentrationPerLiter_massConcPpm(ion) / waters.totalMashWater_l;

      double const ionTotal_ppm = ionFromWaterAgents_ppm + ionFromBaseWater_ppm;
      digitWidget->setQuantity(ionTotal_ppm);
      this->pimpl->m_actual.setIonConcentration_ppm(ion, ionTotal_ppm);
   }

   btDigit_ph->setQuantity(this->pimpl->calculateMashpH(waters));

   this->radarChart->replot();
   return;
}

void WaterAdjuster::removeSelectedWaterAdjustments() {
   this->pimpl->m_miscAdditionsVeriTable.removeSelected();
   return;
}

void WaterAdjuster::removeSelectedAcidMalts() {
   this->pimpl->m_fermentableAdditionsVeriTable.removeSelected();
   return;
}