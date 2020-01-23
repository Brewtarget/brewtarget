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

#include "WaterDialog.h"
#include "WaterListModel.h"
#include "WaterSortFilterProxyModel.h"
#include "BtTreeFilterProxyModel.h"
#include "BtTreeModel.h"
#include "WaterButton.h"
#include "SaltTableModel.h"
#include "brewtarget.h"
#include "database.h"
#include "mash.h"
#include "mashstep.h"
#include "salt.h"

WaterDialog::WaterDialog(QWidget* parent) : QDialog(parent),
   recObs(nullptr),
   base(nullptr),
   target(nullptr),
   m_mashRO(0.0),
   m_spargeRO(0.0)
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
   connect(baseProfileCombo, SIGNAL( activated(int)), this, SLOT(update_baseProfile(int)));

   targetListModel = new WaterListModel(targetProfileCombo);
   targetFilter    = new WaterSortFilterProxyModel(targetProfileCombo);
   targetFilter->setDynamicSortFilter(true);
   targetFilter->setSortLocaleAware(true);
   targetFilter->setSourceModel(targetListModel);
   targetFilter->sort(0);
   targetProfileCombo->setModel(targetFilter);
   connect(targetProfileCombo, SIGNAL( activated(int)), this, SLOT(update_targetProfile(int)));

   rangeWidget_ca->setPrecision(1);
   rangeWidget_cl->setPrecision(1);
   rangeWidget_mg->setPrecision(1);
   rangeWidget_na->setPrecision(1);
   rangeWidget_hco3->setPrecision(1);
   rangeWidget_so4->setPrecision(1);

   // and now let's see what the table does.
   saltTableModel = new SaltTableModel(tableView_salts);
   tableView_salts->setItemDelegate(new SaltItemDelegate(tableView_salts));
   tableView_salts->setModel(saltTableModel);

   connect( saltTableModel,        &SaltTableModel::newTotals, this, &WaterDialog::newTotals);
   connect( pushButton_addSalt,    &QAbstractButton::clicked, saltTableModel, &SaltTableModel::catchSalt);
   connect( pushButton_removeSalt, &QAbstractButton::clicked, this, &WaterDialog::removeSalts);

   connect( spinBox_mashRO, SIGNAL(valueChanged(int)), this, SLOT(setMashRO(int)));
   connect( spinBox_spargeRO, SIGNAL(valueChanged(int)), this, SLOT(setSpargeRO(int)));

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
      rangeWidget_ca->setValue(modifier * base->calcium_ppm());
      rangeWidget_cl->setValue(modifier * base->chloride_ppm());
      rangeWidget_mg->setValue(modifier * base->magnesium_ppm());
      rangeWidget_na->setValue(modifier * base->sodium_ppm());
      rangeWidget_hco3->setValue(modifier * base->bicarbonate_ppm());
      rangeWidget_so4->setValue(modifier * base->sulfate_ppm());
   }
   if ( target != nullptr ) {
      targetProfileButton->setWater(target);
      setSlider(rangeWidget_ca, target->calcium_ppm());
      setSlider(rangeWidget_cl, target->chloride_ppm());
      setSlider(rangeWidget_mg,target->magnesium_ppm());
      setSlider(rangeWidget_na, target->sodium_ppm());
      setSlider(rangeWidget_hco3, target->bicarbonate_ppm());
      setSlider(rangeWidget_so4, target->sulfate_ppm());
   }
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

   // the total_* method return the numerator, we supply the denominator and include the base
   // water ppm. I still need to make this care about the %RO.

   if ( base != nullptr ) {

      // I hope this is right. All this 'rithmetic is making me head hurt.
      // If we are watering down (hahaha) the base water with RO, this should adjust the salts properly.
      double modifier = 1 - ( (m_mashRO * mash->totalInfusionAmount_l()) + (m_spargeRO * mash->totalSpargeAmount_l())) / allTheWaters;

      rangeWidget_ca->setValue( saltTableModel->total_Ca() / allTheWaters + modifier * base->calcium_ppm());
      rangeWidget_mg->setValue( saltTableModel->total_Mg() / allTheWaters + modifier * base->magnesium_ppm());
      rangeWidget_na->setValue( saltTableModel->total_Na() / allTheWaters + modifier * base->sodium_ppm());
      rangeWidget_cl->setValue( saltTableModel->total_Cl() / allTheWaters + modifier * base->chloride_ppm());
      rangeWidget_hco3->setValue( saltTableModel->total_HCO3() / allTheWaters + modifier * base->bicarbonate_ppm());
      rangeWidget_so4->setValue( saltTableModel->total_SO4() / allTheWaters + modifier * base->sulfate_ppm());

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
