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
   recObs(nullptr)
{
   setupUi(this);
   base = nullptr;
   target = nullptr;

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
   rangeWidget_bicarb->setPrecision(1);
   rangeWidget_sulfate->setPrecision(1);

   // setup the misc tree
   SaltTreeFilterProxyModel* sfilter = new SaltTreeFilterProxyModel(this);
   sfilter->setSourceModel( treeView_salts->model());
   treeView_salts->setModel(sfilter);
   sfilter->setDynamicSortFilter(true);
   treeView_salts->setFilter(sfilter);

   // and now let's see what the table does.
   saltTableModel = new SaltTableModel(tableView_salts,this);
   tableView_salts->setItemDelegate(new SaltItemDelegate(tableView_salts));
   tableView_salts->setModel(saltTableModel);
   // can I catch the dropped salts?
   tableView_salts->setAcceptDrops(true);

   connect( saltTableModel, &SaltTableModel::newTotals, this, &WaterDialog::newTotals);
}

WaterDialog::~WaterDialog() {}

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
      baseProfileButton->setWater(base);
      rangeWidget_ca->setValue(base->calcium_ppm());
      rangeWidget_cl->setValue(base->chloride_ppm());
      rangeWidget_mg->setValue(base->magnesium_ppm());
      rangeWidget_na->setValue(base->sodium_ppm());
      rangeWidget_bicarb->setValue(base->bicarbonate_ppm());
      rangeWidget_sulfate->setValue(base->sulfate_ppm());
   }
   if ( target != nullptr ) {
      targetProfileButton->setWater(target);
      setSlider(rangeWidget_ca, target->calcium_ppm());
      setSlider(rangeWidget_cl, target->chloride_ppm());
      setSlider(rangeWidget_mg,target->magnesium_ppm());
      setSlider(rangeWidget_na, target->sodium_ppm());
      setSlider(rangeWidget_bicarb, target->bicarbonate_ppm());
      setSlider(rangeWidget_sulfate, target->sulfate_ppm());
   }
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

      setSlider(rangeWidget_ca,      target->calcium_ppm());
      setSlider(rangeWidget_cl,      target->chloride_ppm());
      setSlider(rangeWidget_mg,      target->magnesium_ppm());
      setSlider(rangeWidget_na,      target->sodium_ppm());
      setSlider(rangeWidget_bicarb,  target->bicarbonate_ppm());
      setSlider(rangeWidget_sulfate, target->sulfate_ppm());
   }
}

void WaterDialog::newTotals()
{
   if ( ! recObs || ! recObs->mash() )
      return;

   double allTheWaters = recObs->mash()->totalMashWater_l();

   if ( qFuzzyCompare(allTheWaters,0.0) ) {
      Brewtarget::logW(QString("How did you get this far with no mash water?"));
      return;
   }
   // Two major things need to happen here:
   //   o the totals need to be updated
   //   o the slides need to be updated
   lineEdit_totalcacl->setText(saltTableModel->total(Salt::CACL2));
   lineEdit_totalcaco3->setText(saltTableModel->total(Salt::CACO3));
   lineEdit_totalcaso4->setText(saltTableModel->total(Salt::CASO4));
   lineEdit_totalmgso4->setText(saltTableModel->total(Salt::MGSO4));
   lineEdit_totalnacl->setText(saltTableModel->total(Salt::NACL));
   lineEdit_totalnahco3->setText(saltTableModel->total(Salt::NAHCO3));

   // the total_* method return the numerator, we supply the denominator and include the base
   // water ppm. I still need to make this care about the %RO.

   if ( base != nullptr ) {
      rangeWidget_ca->setValue( saltTableModel->total_Ca() / allTheWaters + base->calcium_ppm());
      rangeWidget_mg->setValue( saltTableModel->total_Mg() / allTheWaters + base->magnesium_ppm());
      rangeWidget_sulfate->setValue( saltTableModel->total_SO4() / allTheWaters + base->sulfate_ppm());
      rangeWidget_na->setValue( saltTableModel->total_Na() / allTheWaters + base->sodium_ppm());
      rangeWidget_cl->setValue( saltTableModel->total_Cl() / allTheWaters + base->chloride_ppm());
      rangeWidget_bicarb->setValue( saltTableModel->total_HCO3() / allTheWaters + base->bicarbonate_ppm());

      qDebug() << "grams of caso4 =" << saltTableModel->total(Salt::MGSO4);
      qDebug() << "total water =" << allTheWaters;
      qDebug() << "total_ca = " << saltTableModel->total_Ca();
      qDebug() << "total_so4 = " << saltTableModel->total_SO4();
   }
   else {
      rangeWidget_ca->setValue( saltTableModel->total_Ca() / allTheWaters );
      rangeWidget_mg->setValue( saltTableModel->total_Mg() / allTheWaters );
      rangeWidget_sulfate->setValue( saltTableModel->total_SO4() / allTheWaters );
      rangeWidget_na->setValue( saltTableModel->total_Na() / allTheWaters );
      rangeWidget_cl->setValue( saltTableModel->total_Cl() / allTheWaters );
      rangeWidget_bicarb->setValue( saltTableModel->total_HCO3() / allTheWaters );

   }

}

void WaterDialog::dropEvent(QDropEvent *dpEvent)
{
   const QMimeData* mData;
   int _type;
   QString name;
   int id;
   QList<Misc*> salty;

   if ( acceptMime.size() == 0 )
      acceptMime = tableView_salts->property("mimeAccepted").toString();

   if ( ! dpEvent->mimeData()->hasFormat(acceptMime)) {
      return;
   }

   mData = dpEvent->mimeData();
   QByteArray itemData = mData->data(acceptMime);
   QDataStream dStream(&itemData, QIODevice::ReadOnly);

   while ( ! dStream.atEnd() ) {
      dStream >> _type >> id >> name;
      if ( _type == BtTreeItem::MISC )
         salty.append( Database::instance().misc(id) );
   }

   if ( salty.size() > 0 )
      addSalts(salty);
   // this is like "mischief managed"
   dpEvent->acceptProposedAction();
}

void WaterDialog::addSalts(QList<Misc*> dropped)
{
   QList<Salt*> caught;
   if ( recObs == nullptr )
      return;

   foreach( Misc *m, dropped ) {
      QString mName = m->name();

      Salt::Types sType = Salt::NONE;

      // Oh, this just got hard. We dropped MISC but I need salts.
      Salt* gaq = new Salt(m->name(), true);
      if (mName == QString("Calcium Chloride") )
         sType = Salt::CACL2;
      else if (mName == QString("Calcium Carbonate") )
         sType = Salt::CACO3;
      else if (mName == QString("Gypsum"))
         sType = Salt::CASO4;
      else if (mName == QString("Epsom Salt") )
         sType = Salt::MGSO4;
      else if ( mName == QString("Kosher Salt") )
         sType = Salt::NACL;
      else if ( mName == QString("Baking Soda") )
         sType = Salt::NAHCO3;
      else
         Brewtarget::logW( QString("Unknown brewing salt %1").arg(mName));
      gaq->setType(sType);
      gaq->setAddTo(Salt::BOTH);
      gaq->m_misc_id = m->key();
      caught.append(gaq);
   }
   if ( caught.size() > 0 ) {
      emit droppedSalts(caught);
   }
}
void WaterDialog::dragEnterEvent(QDragEnterEvent *deEvent)
{
   if ( acceptMime.size() == 0 )
      acceptMime = tableView_salts->property("mimeAccepted").toString();

   if (deEvent->mimeData()->hasFormat(acceptMime) )
      deEvent->acceptProposedAction();
}
