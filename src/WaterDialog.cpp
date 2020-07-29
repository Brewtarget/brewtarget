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
   recObs(nullptr),
   base(nullptr),
   target(nullptr),
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

   // not sure if this is better or worse, but we will try it out
   m_ppm_digits.append(btDigit_ca);
   m_ppm_digits.append(btDigit_cl);
   m_ppm_digits.append(btDigit_hco3);
   m_ppm_digits.append(btDigit_mg);
   m_ppm_digits.append(btDigit_na);
   m_ppm_digits.append(btDigit_so4);

   m_total_digits.append(btDigit_totalcaco3);
   m_total_digits.append(btDigit_totalcacl2);
   m_total_digits.append(btDigit_totalcaso4);
   m_total_digits.append(btDigit_totalmgso4);
   m_total_digits.append(btDigit_totalnacl);
   m_total_digits.append(btDigit_totalnahco3);

   foreach( BtDigitWidget* i, m_ppm_digits ) {
      i->setLimits(0.0,1000.0);
      i->display(0.0, 1);
      i->setMessages(msgs);
   }
   // as before, we can be a bit more specific with pH
   btDigit_ph->setLowLim(5.0);
   btDigit_ph->setHighLim(5.5);
   btDigit_ph->display(7.0,1);

   // since all the things are now digits, lets get the totals configured
   foreach( BtDigitWidget* i, m_total_digits ) {
      i->setConstantColor(BtDigitWidget::BLACK);
      i->display(0,1);
   }
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

   connect( buttonBox_save, &QDialogButtonBox::accepted, this, &WaterDialog::saveAndClose);
   connect( buttonBox_save, &QDialogButtonBox::rejected, this, &WaterDialog::clearAndClose);

}

WaterDialog::~WaterDialog() {}

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

void WaterDialog::setDigits(Water* target)
{
   if ( target == nullptr )
      return;

   btDigit_ca->setLimits(target->calcium_ppm() * 0.95,target->calcium_ppm() * 1.05);
   btDigit_cl->setLimits(target->chloride_ppm() * 0.95,target->chloride_ppm() * 1.05);
   btDigit_hco3->setLimits(target->bicarbonate_ppm() * 0.95,target->bicarbonate_ppm() * 1.05);
   btDigit_mg->setLimits(target->magnesium_ppm() * 0.95,target->magnesium_ppm() * 1.05);
   btDigit_na->setLimits(target->sodium_ppm() * 0.95,target->sodium_ppm() * 1.05);
   btDigit_so4->setLimits(target->sulfate_ppm() * 0.95,target->sulfate_ppm() * 1.05);

   // oddly, pH doesn't change with the target water
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

   // I need these numbers before we set the ranges
   foreach( Fermentable *i, recObs->fermentables() ) {
      m_total_grains   += i->amount_kg();
   }

   foreach( Fermentable *i, recObs->fermentables() ) {
      double lovi = ( i->color_srm() +0.6 ) / 1.35;
      m_weighted_colors   += (i->amount_kg()/m_total_grains)*lovi;
   }
   m_thickness = recObs->mash()->totalInfusionAmount_l()/m_total_grains;

   if ( base != nullptr ) {

      m_mashRO = base->mashRO();
      spinBox_mashRO->setValue( QVariant(m_mashRO*100).toInt());
      m_spargeRO = base->spargeRO();
      spinBox_spargeRO->setValue( QVariant(m_spargeRO*100).toInt());

      baseProfileButton->setWater(base);
      baseProfileEdit->setWater(base);
      // all of the magic to set the sliders happens in newTotals(). So don't do it twice
   }
   if ( target != nullptr  && target != base ) {
      targetProfileButton->setWater(target);
      targetProfileEdit->setWater(target);

      setDigits(target);
   }
   newTotals();

}

void WaterDialog::update_baseProfile(int selected)
{
   Q_UNUSED(selected)
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

   Q_UNUSED(selected)
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

      setDigits(target);
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
   //   o the digits need to be updated

   btDigit_totalcacl2->display( Brewtarget::amountDisplay( saltTableModel->total(Salt::CACL2), Units::kilograms ), 2 );
   btDigit_totalcaco3 ->display( Brewtarget::amountDisplay( saltTableModel->total(Salt::CACO3), Units::kilograms), 2 );
   btDigit_totalcaso4->display( Brewtarget::amountDisplay( saltTableModel->total(Salt::CASO4), Units::kilograms), 2 );
   btDigit_totalmgso4->display( Brewtarget::amountDisplay( saltTableModel->total(Salt::MGSO4), Units::kilograms), 2 );
   btDigit_totalnacl->display( Brewtarget::amountDisplay( saltTableModel->total(Salt::NACL), Units::kilograms), 2 );
   btDigit_totalnahco3->display( Brewtarget::amountDisplay( saltTableModel->total(Salt::NAHCO3), Units::kilograms), 2 );

   // the total_* method return the numerator, we supply the denominator and
   // include the base water ppm. The confusing math generates an adjustment
   // for the base water that depends the %RO in the mash and sparge water

   if ( base != nullptr ) {

      // I hope this is right. All this 'rithmetic is making me head hurt.
      double modifier = 1 - ( (m_mashRO * mash->totalInfusionAmount_l()) + (m_spargeRO * mash->totalSpargeAmount_l())) / allTheWaters;

      btDigit_ca->display( saltTableModel->total_Ca() / allTheWaters + modifier * base->calcium_ppm());
      btDigit_mg->display( saltTableModel->total_Mg() / allTheWaters + modifier * base->magnesium_ppm());
      btDigit_na->display( saltTableModel->total_Na() / allTheWaters + modifier * base->sodium_ppm());
      btDigit_cl->display( saltTableModel->total_Cl() / allTheWaters + modifier * base->chloride_ppm());
      btDigit_hco3->display( saltTableModel->total_HCO3() / allTheWaters + modifier * base->bicarbonate_ppm());
      btDigit_so4->display( saltTableModel->total_SO4() / allTheWaters + modifier * base->sulfate_ppm());
      btDigit_ph->display( calculateMashpH() );

   }
   else {
      btDigit_ca->display( saltTableModel->total_Ca() / allTheWaters );
      btDigit_mg->display( saltTableModel->total_Mg() / allTheWaters );
      btDigit_na->display( saltTableModel->total_Na() / allTheWaters );
      btDigit_cl->display( saltTableModel->total_Cl() / allTheWaters );
      btDigit_hco3->display( saltTableModel->total_HCO3() / allTheWaters );
      btDigit_so4->display( saltTableModel->total_SO4() / allTheWaters );

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
   if ( base ) {

      double base_alk = ( 1.0 - base->mashRO() ) * base->alkalinity();
      if ( ! base->alkalinityAsHCO3() ) {
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
   if ( ! recObs || ! recObs->mash() )
      return 0.0;

   Mash* mash = recObs->mash();
   double allTheWaters = mash->totalMashWater_l();

   double modifier = 1 - ( (m_mashRO * mash->totalInfusionAmount_l()) + (m_spargeRO * mash->totalSpargeAmount_l())) / allTheWaters;

   // I have no idea where the 2 comes from, but Kai did it. I wish I knew why
   // we get the initial numbers from the base water
   double cappm = modifier * base->calcium_ppm()/Cagpm * 2;
   double mgppm = modifier * base->magnesium_ppm()/Mggpm * 2;

   // I need mass of the salts, and all the previous math gave me
   // ppm. Multiplying by the water volume gives me the mass
   // The 3.5 and 7 come from Paul Kohlbach's work from the 1940's.
   double totalDelta = (calculateRA() - cappm/3.5 - mgppm/7) * recObs->mash()->totalInfusionAmount_l();
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
   double ca = saltTableModel->total_Ca()/Cagpm * 2;
   double mg = saltTableModel->total_Mg()/Mggpm * 2;
   double hco3 = saltTableModel->total_HCO3()/HCO3gpm;
   double co3 = saltTableModel->total_CO3()/CO3gpm;

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

   double lactic_amt   = saltTableModel->totalAcidWeight(Salt::LACTIC);
   double acidmalt_amt = saltTableModel->totalAcidWeight(Salt::ACIDMLT);
   double H3PO4_amt    = saltTableModel->totalAcidWeight(Salt::H3PO4);

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

   if ( recObs && recObs->fermentables().size() ) {
      double gristpH   = calculateGristpH();
      double basepH    = calculateSaltpH();
      double saltpH    = calculateAddedSaltpH();
      double acids     = calculateAcidpH();

      // qDebug() << "gristph =" << gristpH << "strike =" << strikepH << "acids =" << acids;
      // residual alkalinity is handled by basepH
      mashpH = basepH + gristpH + saltpH - acids;
   }

   return mashpH;
}

void WaterDialog::saveAndClose()
{
   saltTableModel->saveAndClose();
   if ( base != nullptr && base->cacheOnly() ) {
      Database::instance().insertWater(base);
      Database::instance().addToRecipe(recObs,base,true);
   }
   if ( target != nullptr && target->cacheOnly() ) {
      Database::instance().insertWater(target);
      Database::instance().addToRecipe(recObs,target,true);
   }

   setVisible(false);
}

void WaterDialog::clearAndClose()
{
   setVisible(false);
}
