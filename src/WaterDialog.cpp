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
#include <QMimeData>
#include <QFont>

#include "WaterDialog.h"
#include "WaterListModel.h"
#include "WaterSortFilterProxyModel.h"
#include "WaterButton.h"
#include "WaterEditor.h"
#include "BtTreeFilterProxyModel.h"
#include "BtTreeModel.h"
#include "WaterButton.h"
#include "SaltTableModel.h"
#include "brewtarget.h"
#include "database.h"
#include "mash.h"
#include "mashstep.h"
#include "salt.h"
#include "ColorMethods.h"

WaterDialog::WaterDialog(QWidget* parent) : QDialog(parent),
   recObs(nullptr),
   base(nullptr),
   target(nullptr),
   m_mashRO(0.0),
   m_spargeRO(0.0),
   m_total_grains(0.0),
   m_thickness(0.0)
{
   setupUi(this);

   // initialize the two buttons and lists (I think)
   baseListModel = new WaterListModel(baseProfileCombo);
   baseFilter    = new WaterSortFilterProxyModel(baseProfileCombo);
   baseFilter->setDynamicSortFilter(true);
   baseFilter->setSortLocaleAware(true);
   baseFilter->setSourceModel(baseListModel);
   baseFilter->sort(0);
   baseProfileCombo->setModel(baseFilter);

   targetListModel = new WaterListModel(targetProfileCombo);
   targetFilter    = new WaterSortFilterProxyModel(targetProfileCombo);
   targetFilter->setDynamicSortFilter(true);
   targetFilter->setSortLocaleAware(true);
   targetFilter->setSourceModel(targetListModel);
   targetFilter->sort(0);
   targetProfileCombo->setModel(targetFilter);

   rangeWidget_ca->setPrecision(1);
   rangeWidget_cl->setPrecision(1);
   rangeWidget_mg->setPrecision(1);
   rangeWidget_na->setPrecision(1);
   rangeWidget_hco3->setPrecision(1);
   rangeWidget_so4->setPrecision(1);

   // we can set pH up a little better than the rest
   rangeWidget_pH->setPrecision(2);
   rangeWidget_pH->setRange(2,10);
   rangeWidget_pH->setTickMarks(1,5);
   rangeWidget_pH->setValue(7.0);
   rangeWidget_pH->setPreferredRange(5.2,5.5);

   // and now let's see what the table does.
   saltTableModel = new SaltTableModel(tableView_salts);
   tableView_salts->setItemDelegate(new SaltItemDelegate(tableView_salts));
   tableView_salts->setModel(saltTableModel);

   baseProfileEdit = new WaterEditor(this);
   targetProfileEdit = new WaterEditor(this);

   // all the signals
   connect(baseProfileCombo, SIGNAL( activated(int)), this, SLOT(update_baseProfile(int)));
   connect(targetProfileCombo, SIGNAL( activated(int)), this, SLOT(update_targetProfile(int)));

   connect(baseProfileButton, &WaterButton::clicked, baseProfileEdit, &QWidget::show);
   connect(targetProfileButton, &WaterButton::clicked, targetProfileEdit, &QWidget::show);

   connect( saltTableModel,        &SaltTableModel::newTotals, this, &WaterDialog::newTotals);
   connect( pushButton_addSalt,    &QAbstractButton::clicked, saltTableModel, &SaltTableModel::catchSalt);
   connect( pushButton_removeSalt, &QAbstractButton::clicked, this, &WaterDialog::removeSalts);

   connect( spinBox_mashRO, SIGNAL(valueChanged(int)), this, SLOT(setMashRO(int)));
   connect( spinBox_spargeRO, SIGNAL(valueChanged(int)), this, SLOT(setSpargeRO(int)));

   connect( lineEdit_lacticAmount, &BtLineEdit::textModified, this, &WaterDialog::updateAcids);
   connect( lineEdit_acidAmount, &BtLineEdit::textModified, this, &WaterDialog::updateAcids);
   connect( lineEdit_H3PO4, &BtLineEdit::textModified, this, &WaterDialog::updateAcids);

   connect( label_acidAmount, &BtLabel::labelChanged, lineEdit_acidAmount, &BtLineEdit::lineChanged);
   connect( label_lacticAmount, &BtLabel::labelChanged, lineEdit_lacticAmount, &BtLineEdit::lineChanged);
   connect( label_H3PO4, &BtLabel::labelChanged, lineEdit_H3PO4, &BtLineEdit::lineChanged);


}

WaterDialog::~WaterDialog() {}

void WaterDialog::updateAcids()
{
   // this only causes a recalc right now. I have no idea how I'm storing
   // the acids.
   newTotals();
}
void WaterDialog::setMashRO(int val)
{
   m_mashRO = val/100.0;
   if ( base ) base->setMashRO(m_mashRO);
   newTotals();
}

void WaterDialog::setSpargeRO(int val)
{
   m_spargeRO = val/100.0;
   if ( base ) base->setSpargeRO(m_spargeRO);
   newTotals();
}

void WaterDialog::setSlider( RangedSlider* slider, double data)
{
   slider->setPreferredRange( 0.95 * data, 1.05 * data);
   slider->setRange(0,data*2);
   slider->setTickMarks(data/4,4);
}

void WaterDialog::setRecipe(Recipe *rec)
{
   if ( rec == nullptr )
      return;

   recObs = rec;
   Mash* mash = recObs->mash();
   saltTableModel->observeRecipe(recObs);

   if ( mash == nullptr || mash->mashSteps().size() == 0 ) {
      Brewtarget::logW(QString("Can not set strike water chemistry without a mash"));
      return;
   }

   baseProfileButton->setRecipe(recObs);
   targetProfileButton->setRecipe(recObs);

   foreach( Water* w, recObs->waters()) {
      if (w->type() == Water::BASE )
         base = w;
      else if ( w->type() == Water::TARGET )
         target = w;
   }

   if ( base != nullptr ) {

      m_mashRO = base->mashRO();
      spinBox_mashRO->setValue( QVariant(m_mashRO*100).toInt());
      m_spargeRO = base->spargeRO();
      spinBox_spargeRO->setValue( QVariant(m_spargeRO*100).toInt());
      double modifier = 1 - ( m_mashRO * mash->totalInfusionAmount_l() + m_spargeRO * mash->totalSpargeAmount_l() ) / mash->totalMashWater_l();

      baseProfileButton->setWater(base);
      baseProfileEdit->setWater(base);
      rangeWidget_ca->setValue(modifier * base->calcium_ppm());
      rangeWidget_cl->setValue(modifier * base->chloride_ppm());
      rangeWidget_mg->setValue(modifier * base->magnesium_ppm());
      rangeWidget_na->setValue(modifier * base->sodium_ppm());
      rangeWidget_hco3->setValue(modifier * base->bicarbonate_ppm());
      rangeWidget_so4->setValue(modifier * base->sulfate_ppm());
   }
   if ( target != nullptr ) {
      targetProfileButton->setWater(target);
      targetProfileEdit->setWater(target);

      setSlider(rangeWidget_ca, target->calcium_ppm());
      setSlider(rangeWidget_cl, target->chloride_ppm());
      setSlider(rangeWidget_mg,target->magnesium_ppm());
      setSlider(rangeWidget_na, target->sodium_ppm());
      setSlider(rangeWidget_hco3, target->bicarbonate_ppm());
      setSlider(rangeWidget_so4, target->sulfate_ppm());
   }

   // oh. this sucks. I need that total mass first
   foreach( Fermentable *i, recObs->fermentables() ) {
      m_total_grains   += i->amount_kg();
   }
   // and then I need to use that to get this number.
   foreach( Fermentable *i, recObs->fermentables() ) {
      double lovi = ( i->color_srm() +0.6 ) / 1.35;
      m_weighted_colors   += (i->amount_kg()/m_total_grains)*lovi;
   }
   m_thickness = recObs->mash()->totalInfusionAmount_l()/m_total_grains;

   newTotals();
}

void WaterDialog::update_baseProfile(int selected)
{

   if ( recObs == nullptr )
      return;

   QModelIndex proxyIdx(baseFilter->index(baseProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(baseFilter->mapToSource(proxyIdx));
   const Water* parent = baseListModel->at(sourceIdx.row());

   if ( parent ) {
      // this is in cache only until we say "ok"
      base = new Water(*parent, true);
      base->setType(Water::BASE);

      baseProfileButton->setWater(base);
      baseProfileEdit->setWater(base);
      newTotals();

   }
}

void WaterDialog::update_targetProfile(int selected)
{

   if ( recObs == nullptr )
      return;

   QModelIndex proxyIdx(targetFilter->index(targetProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(targetFilter->mapToSource(proxyIdx));
   Water* parent = targetListModel->at(sourceIdx.row());

   if ( parent ) {
      // this is in cache only until we say "ok"
      target = new Water(*parent, true);
      target->setType(Water::TARGET);
      targetProfileButton->setWater(target);
      targetProfileEdit->setWater(target);

      setSlider(rangeWidget_ca,   target->calcium_ppm());
      setSlider(rangeWidget_cl,   target->chloride_ppm());
      setSlider(rangeWidget_mg,   target->magnesium_ppm());
      setSlider(rangeWidget_na,   target->sodium_ppm());
      setSlider(rangeWidget_hco3, target->bicarbonate_ppm());
      setSlider(rangeWidget_so4,  target->sulfate_ppm());
   }
}

void WaterDialog::newTotals()
{
   if ( ! recObs || ! recObs->mash() )
      return;

   Mash* mash = recObs->mash();
   double allTheWaters = mash->totalMashWater_l();

   if ( qFuzzyCompare(allTheWaters,0.0) ) {
      Brewtarget::logW(QString("How did you get this far with no mash water?"));
      return;
   }
   // Two major things need to happen here:
   //   o the totals need to be updated
   //   o the slides need to be updated
   lineEdit_totalcacl2->setText(saltTableModel->total(Salt::CACL2));
   lineEdit_totalcaco3->setText(saltTableModel->total(Salt::CACO3));
   lineEdit_totalcaso4->setText(saltTableModel->total(Salt::CASO4));
   lineEdit_totalmgso4->setText(saltTableModel->total(Salt::MGSO4));
   lineEdit_totalnacl->setText(saltTableModel->total(Salt::NACL));
   lineEdit_totalnahco3->setText(saltTableModel->total(Salt::NAHCO3));

   // the total_* method return the numerator, we supply the denominator and
   // include the base water ppm. The confusing math generates an adjustment
   // for the base water that depends the %RO in the mash and sparge water

   if ( base != nullptr ) {

      // I hope this is right. All this 'rithmetic is making me head hurt.
      double modifier = 1 - ( (m_mashRO * mash->totalInfusionAmount_l()) + (m_spargeRO * mash->totalSpargeAmount_l())) / allTheWaters;

      rangeWidget_ca->setValue( saltTableModel->total_Ca() / allTheWaters + modifier * base->calcium_ppm());
      rangeWidget_mg->setValue( saltTableModel->total_Mg() / allTheWaters + modifier * base->magnesium_ppm());
      rangeWidget_na->setValue( saltTableModel->total_Na() / allTheWaters + modifier * base->sodium_ppm());
      rangeWidget_cl->setValue( saltTableModel->total_Cl() / allTheWaters + modifier * base->chloride_ppm());
      rangeWidget_hco3->setValue( saltTableModel->total_HCO3() / allTheWaters + modifier * base->bicarbonate_ppm());
      rangeWidget_so4->setValue( saltTableModel->total_SO4() / allTheWaters + modifier * base->sulfate_ppm());
      rangeWidget_pH->setValue( calculateMashpH() );

   }
   else {
      rangeWidget_ca->setValue( saltTableModel->total_Ca() / allTheWaters );
      rangeWidget_mg->setValue( saltTableModel->total_Mg() / allTheWaters );
      rangeWidget_na->setValue( saltTableModel->total_Na() / allTheWaters );
      rangeWidget_cl->setValue( saltTableModel->total_Cl() / allTheWaters );
      rangeWidget_hco3->setValue( saltTableModel->total_HCO3() / allTheWaters );
      rangeWidget_so4->setValue( saltTableModel->total_SO4() / allTheWaters );

   }
}

void WaterDialog::removeSalts()
{
   QModelIndexList selected = tableView_salts->selectionModel()->selectedIndexes();
   QList<int> deadSalts;

   foreach( QModelIndex i, selected) {
      deadSalts.append( i.row() );
   }
   saltTableModel->removeSalts(deadSalts);
}


double WaterDialog::calculateRA() const
{
   double residual = 0.0;
   if ( base ) {

      double base_alk = ( 1.0 - base->mashRO() ) * base->alkalinity();
      if ( ! base->alkalinityAsHCO3() ) {
         base_alk = 1.22 * base_alk;
      }
      residual = base_alk/61;
   }

   return residual;
}

// All of the pH calculations are taken from the work done by Kai Troester and
// published at
// http://braukaiser.com/wiki/index.php/Beer_color_to_mash_pH_(v2.0)
// with additional information being gleaned from the spreadsheet associated
// with that link.

const double mEq = 50.0;

//! brief Calculates the pH delta caused by any Ca or Mg either in the base
//! water or the salt additions.
double WaterDialog::calculateSaltpH()
{
   // Ca grams per mole
   const double Cagpm = 40.0;
   // Mg grams per mole
   const double Mggpm = 24.30;

   // I have no idea where the 2 comes from, but Kai does it.
   // We need the value from the range widget (which is already in ppm)
   // as that will deal with both the base water and the salt additions.
   double cappm = rangeWidget_ca->value()/Cagpm * 2;
   double mgppm = rangeWidget_mg->value()/Mggpm * 2;

   // totalDelta gives me a value in mEq/l. Multiplied by the mash,
   // it will give me the mEq which is what is needed.
   double totalDelta = calculateRA() - cappm/3.5 - mgppm/7;
   return totalDelta * recObs->mash()->totalInfusionAmount_l();
}

double WaterDialog::calculateAcidpH()
{
   const double H3PO4_density = 1.685;
   const double lactic_density = 1.2;
   const double H3PO4_gpm = 98;
   const double lactic_gpm = 90;

   double totalDelta = 0.0;
   // this is cheating, but  the formula are meant to work on grams and mL,
   // not liters and  kilograms.
   double lactic_amt   = 1000 * lineEdit_lacticAmount->toSI();
   double acidmalt_amt = 1000 * lineEdit_acidAmount->toSI();
   double H3PO4_amt    = 1000 * lineEdit_H3PO4->toSI();

   if ( lactic_amt + acidmalt_amt > 0.0 ) {
      // from malt is easy
      double from_malt = acidmalt_amt * spinBox_acidity->value()/100.0;

      // from lactic acid is not
      double density = spinBox_lacticConcentration->value()/88 * (lactic_density - 1) + 1;
      double lactic_wgt = lactic_amt * density;
      double from_acid  = (spinBox_lacticConcentration->value()/100.0) * lactic_wgt;

      totalDelta += 1000 * (from_malt + from_acid) / lactic_gpm;
   }
   if ( H3PO4_amt > 0.0 ) {
      double density = spinBox_H3PO4->value()/85 * (H3PO4_density - 1) + 1;
      double H3PO4_wgt = H3PO4_amt * density;
      double from_H3PO4 = (spinBox_H3PO4->value()/100.0) * H3PO4_wgt;

      totalDelta += 1000 * from_H3PO4 / H3PO4_gpm;
   }

   return totalDelta/mEq/m_thickness;
}

//! \brief Calculates the theoretical DI mash pH. There is some confusion over
//! the value for the no SRM beer. The spreadsheet says 5.7, the documentation
//! suggests 5.6 is more accurate.
double WaterDialog::calculateGristpH()
{
   const double nosrmbeer_ph = 5.6;
   const double pHSlopeLight = 0.21;
   const double pHSlopeDark  = 0.06;

   double gristPh = nosrmbeer_ph;
   double pHAdjustment = 0.0;

   if ( recObs && recObs->fermentables().size() ) {
      PlatoUnit* convert = new PlatoUnit;

      double platoRatio = 1/convert->fromSI(recObs->og());
      double color = recObs->color_srm();
      double colorFromGrain = 0.0;

      foreach( Fermentable *i, recObs->fermentables() ) {
         // I am counting anything that doesn't have diastatic
         // power as a roasted/crystal malt. I am sure my assumption will
         // haunt me later, but I have no way of knowing what kind of malt
         // (base, crystal, roasted) this is.
         if ( i->diastaticPower_lintner() < 1 ) {
            double lovi = ( i->color_srm() + 0.6)/1.35;
            colorFromGrain = ( i->amount_kg() / m_total_grains ) * lovi;
          }
      }
      double colorRatio = colorFromGrain/m_weighted_colors;
      pHAdjustment = platoRatio * ( pHSlopeLight * (1-colorRatio) + pHSlopeDark * colorRatio) *color;

      gristPh = gristPh - pHAdjustment;
   }
   return gristPh;
}

//! \brief This figures out the expected mash pH. I am unsure about the
//! 50 mEq/(kg*Ph). I have seen 32, 36 and 50. This is the value in the
//! spreadsheet.
double WaterDialog::calculateMashpH()
{
   double mashpH = 0.0;

   if ( recObs && recObs->fermentables().size() ) {
      double gristpH   = calculateGristpH();
      double strikepH  = calculateSaltpH();
      double acids     = calculateAcidpH();

      // note: The referenced paper says the formula is
      // gristpH + strikepH * thickness/mEq. I could never get that to work.
      // the spreadsheet gave me this formula, and  it works much better.
      mashpH = gristpH + strikepH/m_thickness/mEq - acids;
   }

   return mashpH;
}
