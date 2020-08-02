/*
 * WaterDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <limits>
#include <Algorithms.h>
#include <QComboBox>
#include <QFont>
#include <QInputDialog>

#include "WaterDialog.h"
#include "WaterListModel.h"
#include "WaterSortFilterProxyModel.h"
#include "WaterButton.h"
#include "WaterEditor.h"
#include "WaterTableModel.h"
#include "WaterButton.h"
// #include "SaltEditor.h"
#include "SaltTableModel.h"
#include "WaterTableModel.h"
#include "BtDigitWidget.h"
#include "brewtarget.h"
#include "database.h"
#include "fermentable.h"
#include "mash.h"
#include "mashstep.h"
#include "salt.h"
#include "ColorMethods.h"

WaterDialog::WaterDialog(QWidget* parent) : QDialog(parent),
   m_ppm_digits( QVector<BtDigitWidget*>(Water::numIons) ),
   m_total_digits( QVector<BtDigitWidget*>(Salt::numTypes) ),
   m_rec(nullptr),
   m_base(nullptr),
   m_target(nullptr),
   m_mashRO(0.0),
   m_spargeRO(0.0),
   m_total_grains(0.0),
   m_thickness(0.0)
{
   QStringList msgs = QStringList() << tr("Too low for target profile.")
                                    << tr("In range for target profile.")
                                    << tr("Too high for target profile.");

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

   // not sure if this is better or worse, but we will try it out
   m_ppm_digits[Water::Ca]   = btDigit_ca;
   m_ppm_digits[Water::Cl]   = btDigit_cl;
   m_ppm_digits[Water::HCO3] = btDigit_hco3;
   m_ppm_digits[Water::Mg]   = btDigit_mg;
   m_ppm_digits[Water::Na]   = btDigit_na;
   m_ppm_digits[Water::SO4]  = btDigit_so4;

   m_total_digits[Salt::CACL2] = btDigit_totalcacl2;
   m_total_digits[Salt::CACO3] = btDigit_totalcaco3;
   m_total_digits[Salt::CASO4] = btDigit_totalcaso4;
   m_total_digits[Salt::MGSO4] = btDigit_totalmgso4;
   m_total_digits[Salt::NACL] = btDigit_totalnacl;
   m_total_digits[Salt::NAHCO3] = btDigit_totalnahco3;

   // foreach( BtDigitWidget* i, m_ppm_digits ) {
   for(int i = 0; i < Water::numIons; ++i ) {
      m_ppm_digits[i]->setLimits(0.0,1000.0);
      m_ppm_digits[i]->setText(0.0, 1);
      m_ppm_digits[i]->setMessages(msgs);
   }
   // we can be a bit more specific with pH
   btDigit_ph->setLowLim(5.0);
   btDigit_ph->setHighLim(5.5);
   btDigit_ph->display(7.0,1);

   // since all the things are now digits, lets get the totals configured
   for (int i = Salt::CACL2; i < Salt::NAHCO3; ++i ) {
      m_total_digits[i]->setConstantColor(BtDigitWidget::BLACK);
      m_total_digits[i]->setText(0.0,1);
   }
   // and now let's see what the table does.
   m_salt_table_model = new SaltTableModel(tableView_salts);
   m_salt_delegate    = new SaltItemDelegate(tableView_salts);
   tableView_salts->setItemDelegate(m_salt_delegate);
   tableView_salts->setModel(m_salt_table_model);

   m_base_editor = new WaterEditor(this);
   m_target_editor = new WaterEditor(this);

   // all the signals
   connect(baseProfileCombo,   SIGNAL( activated(int)), this, SLOT(update_baseProfile(int)));
   connect(targetProfileCombo, SIGNAL( activated(int)), this, SLOT(update_targetProfile(int)));

   connect(baseProfileButton,   &WaterButton::clicked, m_base_editor,   &QWidget::show);
   connect(targetProfileButton, &WaterButton::clicked, m_target_editor, &QWidget::show);

   connect( m_salt_table_model,    &SaltTableModel::newTotals, this, &WaterDialog::newTotals);
   connect( pushButton_addSalt,    &QAbstractButton::clicked,  m_salt_table_model, &SaltTableModel::catchSalt);
   connect( pushButton_removeSalt, &QAbstractButton::clicked,  this, &WaterDialog::removeSalts);

   connect( spinBox_mashRO, SIGNAL(valueChanged(int)),   this, SLOT(setMashRO(int)));
   connect( spinBox_spargeRO, SIGNAL(valueChanged(int)), this, SLOT(setSpargeRO(int)));

   connect( buttonBox_save, &QDialogButtonBox::accepted, this, &WaterDialog::saveAndClose);
   connect( buttonBox_save, &QDialogButtonBox::rejected, this, &WaterDialog::clearAndClose);

}

WaterDialog::~WaterDialog() {}

void WaterDialog::setMashRO(int val)
{
   m_mashRO = val/100.0;
   if ( m_base ) m_base->setMashRO(m_mashRO);
   newTotals();
}

void WaterDialog::setSpargeRO(int val)
{
   m_spargeRO = val/100.0;
   if ( m_base ) m_base->setSpargeRO(m_spargeRO);
   newTotals();
}

void WaterDialog::setDigits(Water* target)
{
   if ( target == nullptr )
      return;

   for(int i = 0; i < Water::numIons; ++i ) {
      double ppm = target->ppm(static_cast<Water::Ions>(i));
      double min_ppm = ppm * 0.95;
      double max_ppm = ppm * 1.05;
      QStringList msgs = QStringList() 
         << tr("Minimum expected concentration is %1 ppm").arg(min_ppm)
         << tr("In range for target profile.")
         << tr("Maximum expected concentration is %1 ppm").arg(max_ppm);
      m_ppm_digits[i]->setLimits(min_ppm,max_ppm);
      m_ppm_digits[i]->setMessages(msgs);
   }

   // oddly, pH doesn't change with the target water
}

void WaterDialog::setRecipe(Recipe *rec)
{
   if ( rec == nullptr )
      return;

   m_rec = rec;
   Mash* mash = m_rec->mash();
   m_salt_table_model->observeRecipe(m_rec);
   m_salt_delegate->observeRecipe(m_rec);

   if ( mash == nullptr || mash->mashSteps().size() == 0 ) {
      Brewtarget::logW(QString("Can not set water chemistry without a mash"));
      return;
   }

   baseProfileButton->setRecipe(m_rec);
   targetProfileButton->setRecipe(m_rec);

   foreach( Water* w, m_rec->waters()) {
      if (w->type() == Water::BASE )
         m_base = w;
      else if ( w->type() == Water::TARGET )
         m_target = w;
   }

   // I need these numbers before we set the ranges
   foreach( Fermentable *i, m_rec->fermentables() ) {
      m_total_grains   += i->amount_kg();
   }

   foreach( Fermentable *i, m_rec->fermentables() ) {
      double lovi = ( i->color_srm() +0.6 ) / 1.35;
      m_weighted_colors   += (i->amount_kg()/m_total_grains)*lovi;
   }
   m_thickness = m_rec->mash()->totalInfusionAmount_l()/m_total_grains;

   if ( m_base != nullptr ) {

      m_mashRO = m_base->mashRO();
      spinBox_mashRO->setValue( QVariant(m_mashRO*100).toInt());
      m_spargeRO = m_base->spargeRO();
      spinBox_spargeRO->setValue( QVariant(m_spargeRO*100).toInt());

      baseProfileButton->setWater(m_base);
      m_base_editor->setWater(m_base);
      // all of the magic to set the sliders happens in newTotals(). So don't do it twice
   }
   if ( m_target != nullptr  && m_target != m_base ) {
      targetProfileButton->setWater(m_target);
      m_target_editor->setWater(m_target);

      setDigits(m_target);
   }
   newTotals();

}

void WaterDialog::update_baseProfile(int selected)
{
   Q_UNUSED(selected)
   if ( m_rec == nullptr )
      return;

   QModelIndex proxyIdx(m_base_filter->index(baseProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(m_base_filter->mapToSource(proxyIdx));
   const Water* parent = m_base_combo_list->at(sourceIdx.row());

   if ( parent ) {
      // this is in cache only until we say "ok"
      m_base = new Water(*parent, true);
      m_base->setType(Water::BASE);

      baseProfileButton->setWater(m_base);
      m_base_editor->setWater(m_base);
      newTotals();

   }
}

void WaterDialog::update_targetProfile(int selected)
{

   Q_UNUSED(selected)
   if ( m_rec == nullptr )
      return;

   QModelIndex proxyIdx(m_target_filter->index(targetProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(m_target_filter->mapToSource(proxyIdx));
   Water* parent = m_target_combo_list->at(sourceIdx.row());

   if ( parent ) {
      // this is in cache only until we say "ok"
      m_target = new Water(*parent, true);
      m_target->setType(Water::TARGET);
      targetProfileButton->setWater(m_target);
      m_target_editor->setWater(m_target);

      setDigits(m_target);
   }
}

void WaterDialog::newTotals()
{
   if ( ! m_rec || ! m_rec->mash() )
      return;

   Mash* mash = m_rec->mash();
   double allTheWaters = mash->totalMashWater_l();

   if ( qFuzzyCompare(allTheWaters,0.0) ) {
      Brewtarget::logW(QString("Can not set strike water chemistry without a mash"));
      return;
   }
   // Two major things need to happen here:
   //   o the totals need to be updated
   //   o the digits need to be updated

   for (int i = Salt::CACL2; i < Salt::LACTIC; ++i ) {
      Salt::Types type = static_cast<Salt::Types>(i);
      m_total_digits[i]->setText(m_salt_table_model->total(type), 2);
   }

   // the total_* method return the numerator, we supply the denominator and
   // include the base water ppm. The confusing math generates an adjustment
   // for the base water that depends the %RO in the mash and sparge water

   if ( m_base != nullptr ) {
      // 'd' means 'diluted'. They make calculating the modifier readable
      double dInfuse = m_mashRO * mash->totalInfusionAmount_l();
      double dSparge = m_spargeRO * mash->totalSpargeAmount_l();

      // I hope this is right. All this 'rithmetic is making me head hurt.
      double modifier = 1.0 - (dInfuse + dSparge) / allTheWaters;

      for (int i = 0; i < Water::numIons; ++i ) {
         Water::Ions ion = static_cast<Water::Ions>(i);
         double mPPM = modifier * m_base->ppm(ion);
         m_ppm_digits[i]->setText( m_salt_table_model->total(ion) / allTheWaters + mPPM, 0 );
                                   
      }
      btDigit_ph->setText( calculateMashpH(), 2 );

   }
   else {
      for (int i = 0; i < Water::numIons; ++i ) {
         Water::Ions ion = static_cast<Water::Ions>(i);
         m_ppm_digits[i]->setText( m_salt_table_model->total(ion) / allTheWaters, 0 );
      }
   }
}

void WaterDialog::removeSalts()
{
   QModelIndexList selected = tableView_salts->selectionModel()->selectedIndexes();
   QList<int> deadSalts;

   foreach( QModelIndex i, selected) {
      deadSalts.append( i.row() );
   }
   m_salt_table_model->removeSalts(deadSalts);
}

// All of the pH calculations are taken from the work done by Kai Troester and
// published at
// http://braukaiser.com/wiki/index.php/Beer_color_to_mash_pH_(v2.0)
// with additional information being gleaned from the spreadsheet associated
// with that link.

// I've seen some confusion over this constant. 50 mEq/l is what Kai uses.
const double mEq = 50.0;
// Ca grams per mole
const double Cagpm = 40.0;
// Mg grams per mole
const double Mggpm = 24.30;
// HCO3 grams per mole
const double HCO3gpm = 61.01;
// CO3 grams per mole
const double CO3gpm = 60.01;

// The pH of a beer with no color
const double nosrmbeer_ph = 5.6;
// Magic constants Kai derives in the document above.
const double pHSlopeLight = 0.21;
const double pHSlopeDark  = 0.06;

//! \brief Calcuates the residual alkalinity of the mash water.
double WaterDialog::calculateRA() const
{
   double residual = 0.0;
   if ( m_base ) {

      double base_alk = ( 1.0 - m_base->mashRO() ) * m_base->alkalinity();
      if ( ! m_base->alkalinityAsHCO3() ) {
         base_alk = 1.22 * base_alk;
      }
      residual = base_alk/61;
   }

   return residual;
}


//! \brief Calculates the pH of the base water caused by any Ca or Mg
//! including figuring out the residual alkalinity.
double WaterDialog::calculateSaltpH()
{
   if ( ! m_rec || ! m_rec->mash() )
      return 0.0;

   Mash* mash = m_rec->mash();
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
double WaterDialog::calculateAddedSaltpH()
{

   // We need the value from the salt table model, because we need all the
   // added salts, but not the base.
   double ca = m_salt_table_model->total_Ca()/Cagpm * 2;
   double mg = m_salt_table_model->total_Mg()/Mggpm * 2;
   double hco3 = m_salt_table_model->total_HCO3()/HCO3gpm;
   double co3 = m_salt_table_model->total_CO3()/CO3gpm;

   // The 61 is another magic number from Kai. Sigh
   // unlike previous calculations, I am getting a mass here so I do not
   // need to convert from mg/L
   double totalDelta = 0.0 - ca/3.5 - mg/7 + (hco3+co3)/61;
   return totalDelta/m_thickness/mEq;
}

//! \brief Calculates the pH adjustment caused by lactic acid, H3PO4 and/or acid
//! malts
double WaterDialog::calculateAcidpH()
{
   const double H3PO4_gpm = 98;
   const double lactic_gpm = 90;
   double totalDelta = 0.0;

   double lactic_amt   = m_salt_table_model->totalAcidWeight(Salt::LACTIC);
   double acidmalt_amt = m_salt_table_model->totalAcidWeight(Salt::ACIDMLT);
   double H3PO4_amt    = m_salt_table_model->totalAcidWeight(Salt::H3PO4);

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
double WaterDialog::calculateGristpH()
{
   double gristPh = nosrmbeer_ph;
   double pHAdjustment = 0.0;

   if ( m_rec && m_rec->fermentables().size() ) {
      PlatoUnit* convert = new PlatoUnit;

      double platoRatio = 1/convert->fromSI(m_rec->og());
      double color = m_rec->color_srm();
      double colorFromGrain = 0.0;

      foreach( Fermentable *i, m_rec->fermentables() ) {
         // I am counting anything that doesn't have diastatic
         // power as a roasted/crystal malt. I am sure my assumption will
         // haunt me later, but I have no way of knowing what kind of malt
         // (base, crystal, roasted) this is.
         if ( i->diastaticPower_lintner() < 1 ) {
            double lovi = 19.0;
            if ( i->color_srm() <= 120 ) {
               lovi = ( i->color_srm() + 0.6)/1.35;
            }
            colorFromGrain = ( i->amount_kg() / m_total_grains ) * lovi;
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
double WaterDialog::calculateMashpH()
{
   double mashpH = 0.0;

   if ( m_rec && m_rec->fermentables().size() ) {
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

void WaterDialog::saveAndClose()
{
   m_salt_table_model->saveAndClose();
   if ( m_base != nullptr && m_base->cacheOnly() ) {
      Database::instance().insertWater(m_base);
      Database::instance().addToRecipe(m_rec,m_base,true);
   }
   if ( m_target != nullptr && m_target->cacheOnly() ) {
      Database::instance().insertWater(m_target);
      Database::instance().addToRecipe(m_rec,m_target,true);
   }

   setVisible(false);
}

void WaterDialog::clearAndClose()
{
   setVisible(false);
}
