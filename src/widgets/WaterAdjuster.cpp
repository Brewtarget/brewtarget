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

#include <algorithm>
#include <cmath>
#include <limits>

#include <QButtonGroup>
#include <QComboBox>
#include <QFont>
#include <QInputDialog>
#include <QSignalBlocker>
#include <QUndoStack>
#include <QVector>

#include "MainWindow.h"
#include "WaterAutoAdjuster.h"
#include "calculations/ColorMethods.h"
#include "calculations/Algorithms.h"
#include "catalogs/FermentableCatalog.h"
#include "catalogs/MiscCatalog.h"
#include "database/ObjectStoreWrapper.h"
#include "editors/WaterEditor.h"
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
#include "undoRedo/Undoable.h"
#include "undoRedo/UndoableAutoAdjust.h"
#include "utils/VeriTable.h"
#include "widgets/SmartDigitWidget.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_WaterAdjuster.cpp"
#endif


//
// The mash pH and ion calculations are now in WaterAutoAdjuster, so that what we show here agrees with what Auto-Adjust
// calculates.
//

namespace {

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
   // See WaterAutoAdjuster for where this is used.
   //

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

   //! Position on the undo stack just after our last auto-adjust, so we know when it's been undone, or -1 if none
   int m_autoAdjustUndoIndex = -1;

   //! The tool tip for the target mash pH spin box from the .ui file, to which we sometimes add more information
   QString m_targetMashPhToolTip;
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

   // Enabled once we have a recipe -- see setRecipe
   for (QWidget * widget : std::initializer_list<QWidget *>{this->pushButton_autoAdjust,
                                                            this->checkBox_keptInSync,
                                                            this->checkBox_targetMashPh,
                                                            this->doubleSpinBox_targetMashPh}) {
      widget->setEnabled(false);
   }

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

   connect(pushButton_autoAdjust     , &QAbstractButton::clicked , this, &WaterAdjuster::autoAdjust        );
   connect(checkBox_keptInSync       , &QAbstractButton::toggled , this, &WaterAdjuster::keptInSyncToggled );
   connect(checkBox_targetMashPh     , &QAbstractButton::toggled , this, &WaterAdjuster::targetMashPhChanged);
   connect(doubleSpinBox_targetMashPh, &QDoubleSpinBox::valueChanged, this, &WaterAdjuster::targetMashPhChanged);
   //
   // Undo and redo can turn keep in sync on and off (see UndoableAutoAdjust).  Also, if our last auto-adjust has been
   // undone, the message about it no longer applies.
   //
   connect(&Undoable::getStack(), &QUndoStack::indexChanged, this, [this](int const index) {
      if (this->pimpl->m_autoAdjustUndoIndex >= 0 && index < this->pimpl->m_autoAdjustUndoIndex) {
         this->showAutoAdjustMessage(tr("Auto-Adjust undone."));
         this->pimpl->m_autoAdjustUndoIndex = -1;
      }
      this->refreshAutoAdjustControls();
      return;
   });

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

   if (this->pimpl->m_rec) {
      this->disconnect(this->pimpl->m_rec, nullptr, this, nullptr);
   }
   this->pimpl->m_rec = rec;
   // Changes to eg the grain bill or the mash affect the predicted mash pH and ion concentrations
   this->connect(this->pimpl->m_rec, &NamedEntity::changed, this, &WaterAdjuster::recipeChanged);
   this->showAutoAdjustMessage("");
   this->pimpl->m_autoAdjustUndoIndex = -1;
   this->refreshAutoAdjustControls();

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
   if (!this->pimpl->m_rec) {
      return;
   }

   std::optional<WaterAutoAdjuster::Prediction> const prediction = WaterAutoAdjuster::predict(*this->pimpl->m_rec);
   if (!prediction) {
      qDebug() << Q_FUNC_INFO << "Cannot set water chemistry without water in the mash";
      return;
   }

   for (auto const & [ion, digitWidget] : this->pimpl->m_waterIonDisplays) {
      double const ionTotal_ppm = prediction->ions_ppm.value(ion, 0.0);
      digitWidget->setQuantity(ionTotal_ppm);
      this->pimpl->m_actual.setIonConcentration_ppm(ion, ionTotal_ppm);
   }

   // If there's a target mash pH, we show whether we're close to it, otherwise we use the range most brewers aim for
   std::optional<double> const target_pH = this->pimpl->m_rec->targetMashAcidity_pH();
   if (target_pH) {
      this->btDigit_ph->setLowLim (*target_pH - 0.1);
      this->btDigit_ph->setHighLim(*target_pH + 0.1);
   } else {
      this->btDigit_ph->setLowLim (5.2);
      this->btDigit_ph->setHighLim(5.6);
   }
   this->btDigit_ph->setQuantity(prediction->mash_pH.value_or(0.0));

   this->radarChart->replot();
   return;
}

void WaterAdjuster::recipeChanged(QMetaProperty prop, [[maybe_unused]] QVariant val) {
   QString const propName = prop.name();
   if (propName == *PropertyNames::Recipe::waterKeptInSync ||
       propName == *PropertyNames::Recipe::targetMashAcidity_pH) {
      this->refreshAutoAdjustControls();
   }
   this->newTotals();
   return;
}

void WaterAdjuster::showAutoAdjustMessage(QString const & heading, QStringList const & details) {
   //
   // The one-line summary goes under the Auto-Adjust button, and the details go in the Auto-Adjust Notes tab, as there
   // can be quite a lot of them, and they would otherwise make the top of the display rather crowded.
   //
   // NB: We don't put the number of notes in the tab title, as it then doesn't fit alongside the other tabs
   if (details.isEmpty()) {
      this->label_autoAdjustInfo->setText(heading.toHtmlEscaped());
      this->textBrowser_autoAdjustNotes->clear();
      return;
   }

   this->label_autoAdjustInfo->setText(
      QString{"%1 %2"}.arg(heading.toHtmlEscaped(),
                           tr("See the <i>Auto-Adjust Notes</i> tab below for details (%n item(s)).", "", details.size()))
   );
   QString notes = QString{"<p>%1</p><ul>"}.arg(heading.toHtmlEscaped());
   for (auto const & detail : details) {
      notes += QString{"<li>%1</li>"}.arg(detail.toHtmlEscaped());
   }
   notes += "</ul>";
   this->textBrowser_autoAdjustNotes->setHtml(notes);
   return;
}

void WaterAdjuster::refreshAutoAdjustControls() {
   Recipe const * const recipe = this->pimpl->m_rec;
   bool const enabled = recipe && !recipe->locked();
   for (QWidget * widget : std::initializer_list<QWidget *>{this->pushButton_autoAdjust,
                                                            this->checkBox_keptInSync,
                                                            this->checkBox_targetMashPh}) {
      widget->setEnabled(enabled);
   }

   std::optional<double> const target_pH = recipe ? recipe->targetMashAcidity_pH() : std::nullopt;
   {
      QSignalBlocker const blockKeptInSync{this->checkBox_keptInSync       };
      QSignalBlocker const blockTargetPh  {this->checkBox_targetMashPh     };
      QSignalBlocker const blockSpinBox   {this->doubleSpinBox_targetMashPh};
      this->checkBox_keptInSync  ->setChecked(recipe && recipe->waterKeptInSync());
      this->checkBox_targetMashPh->setChecked(target_pH.has_value());
      //
      // If the recipe doesn't have a target, but its mash has a pH set on one of its steps, show that, so it's what we
      // use if the user ticks the box.  (We never write anything back to the mash -- see
      // WaterAutoAdjuster::suggestedTargetMashPh.)
      //
      std::optional<double> const suggested_pH =
         (recipe && !target_pH) ? WaterAutoAdjuster::suggestedTargetMashPh(*recipe) : std::nullopt;
      if (target_pH) {
         this->doubleSpinBox_targetMashPh->setValue(*target_pH);
      } else if (suggested_pH) {
         this->doubleSpinBox_targetMashPh->setValue(*suggested_pH);
      }
      if (this->pimpl->m_targetMashPhToolTip.isNull()) {
         this->pimpl->m_targetMashPhToolTip = this->doubleSpinBox_targetMashPh->toolTip();
      }
      this->doubleSpinBox_targetMashPh->setToolTip(
         suggested_pH ? this->pimpl->m_targetMashPhToolTip + "\n\n" +
                        tr("The value shown is the pH set on the recipe's mash.") :
                        this->pimpl->m_targetMashPhToolTip
      );
   }
   this->doubleSpinBox_targetMashPh->setEnabled(enabled && target_pH.has_value());
   return;
}

void WaterAdjuster::targetMashPhChanged() {
   if (!this->pimpl->m_rec) {
      return;
   }
   // As with the RO percentages, this takes effect straight away.  If keep in sync is on, it will trigger an adjustment.
   std::optional<double> const target_pH =
      this->checkBox_targetMashPh->isChecked() ? std::optional<double>{this->doubleSpinBox_targetMashPh->value()} :
                                                 std::nullopt;
   this->pimpl->m_rec->setTargetMashAcidity_pH(target_pH);
   this->doubleSpinBox_targetMashPh->setEnabled(target_pH.has_value());
   return;
}

void WaterAdjuster::keptInSyncToggled(bool const checked) {
   Recipe * const recipe = this->pimpl->m_rec;
   if (!recipe) {
      // Check box should have been disabled
      return;
   }
   if (checked) {
      this->doAutoAdjust(true);
   } else {
      Undoable::doOrRedoUpdate(*recipe,
                               TYPE_INFO(Recipe, waterKeptInSync),
                               false,
                               tr("Turn off keeping water agents in sync"));
   }
   // In case the change wasn't made
   this->refreshAutoAdjustControls();
   return;
}

void WaterAdjuster::autoAdjust() {
   this->doAutoAdjust(false);
   return;
}

void WaterAdjuster::doAutoAdjust(bool const turnOnKeepInSync) {
   Recipe * const recipe = this->pimpl->m_rec;
   if (!recipe) {
      // Button should have been disabled
      qWarning() << Q_FUNC_INFO << "No recipe";
      return;
   }

   auto showMessage = [this](QString const & heading, QStringList const & details = {}) {
      this->showAutoAdjustMessage(heading, details);
      return;
   };

   //
   // Make all the changes (including turning on keep in sync if that's what we're doing) a single undoable step.  See
   // UndoableAutoAdjust for how undo interacts with keep in sync.  We hold a shared pointer to the recipe so the command
   // remains valid for as long as it's on the undo stack.
   //
   std::shared_ptr<Recipe> const recipePtr = ObjectStoreWrapper::getSharedFromRaw(recipe);
   QString const description = turnOnKeepInSync ? tr("Turn on keeping water agents in sync") :
                                                  tr("Auto-adjust water agents");
   auto command = std::make_unique<UndoableAutoAdjust>(
      description,
      [recipePtr]() { return recipePtr->waterKeptInSync(); },
      [recipePtr](bool const val) { recipePtr->setWaterKeptInSync(val); return; }
   );
   if (turnOnKeepInSync) {
      new SimpleUndoableUpdate(*recipe, TYPE_INFO(Recipe, waterKeptInSync), true, description, command.get());
   }

   WaterAutoAdjuster::Preparation const preparation = WaterAutoAdjuster::prepare(*recipe, *command);
   if (!preparation.ok()) {
      if (turnOnKeepInSync) {
         // We still turn on keep in sync, as the user asked for it.  The adjustment will happen when the problem is fixed.
         Undoable::doOrRedoUpdate(command.release());
         this->pimpl->m_autoAdjustUndoIndex = Undoable::getStack().index();
         showMessage(tr("Could not adjust the water agents yet: %1").arg(preparation.result.error));
      } else {
         showMessage(tr("Could not adjust the water agents, so nothing has been changed: %1").arg(preparation.result.error));
      }
      return;
   }

   bool const changed = command->childCount() > 0;
   if (changed) {
      Undoable::doOrRedoUpdate(command.release());
      this->pimpl->m_autoAdjustUndoIndex = Undoable::getStack().index();
   }

   // The table model will normally have told us about the changes, but it doesn't hurt to make sure
   this->newTotals();

   WaterAutoAdjuster::Result const & result = preparation.result;
   QStringList details = result.warnings;
   details.append(preparation.notes);
   QString const what = recipe->waterTarget() ? tr("target profile \"%1\"").arg(recipe->waterTarget()->name()) :
                                                tr("target mash pH");
   QString heading;
   if (!changed) {
      heading = tr("The water agents already match %1 as closely as they can.").arg(what);
   } else if (result.warnings.isEmpty()) {
      heading = tr("Water agents adjusted to match %1.").arg(what);
   } else {
      heading = tr("Water agents adjusted for %1, but some things need checking.").arg(what);
   }
   if (result.predictedMash_pH) {
      heading += " " + tr("Predicted mash pH is %1.").arg(QString::number(*result.predictedMash_pH, 'f', 2));
   }
   showMessage(heading, details);
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