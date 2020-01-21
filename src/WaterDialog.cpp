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
   obsRec(nullptr)
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
   rangeWidget_bicarb->setPrecision(1);
   rangeWidget_sulfate->setPrecision(1);

   // setup the misc tree
   SaltTreeFilterProxyModel* sfilter = new SaltTreeFilterProxyModel(this);
   sfilter->setSourceModel( treeView_salts->model());
   treeView_salts->setModel(sfilter);
   sfilter->setDynamicSortFilter(true);
   treeView_salts->setFilter(sfilter);

   // and now let's see what the table does.
   saltTableModel = new SaltTableModel(tableView_salts);
   tableView_salts->setItemDelegate(new SaltItemDelegate(tableView_salts));
   tableView_salts->setModel(saltTableModel);
   // can I catch the dropped salts?
   tableView_salts->setAcceptDrops(true);
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

   obsRec = rec;
   Mash* mash = obsRec->mash();
   if ( mash == nullptr || mash->mashSteps().size() == 0 ) {
      Brewtarget::logW(QString("Can not set strike water chemistry without a mash"));
      return;
   }

   baseProfileButton->setRecipe(obsRec);
   targetProfileButton->setRecipe(obsRec);

   Water *base = nullptr;
   Water *target = nullptr;

   foreach( Water* w, obsRec->waters()) {
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

double WaterDialog::gypsum_to_calcium(Water* strike, double volume)
{

}  // 1g/L == 232 ppm
void WaterDialog::update_baseProfile(int selected)
{

   if ( obsRec == nullptr )
      return;

   QModelIndex proxyIdx(baseFilter->index(baseProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(baseFilter->mapToSource(proxyIdx));
   const Water* parent = baseListModel->at(sourceIdx.row());

   if ( parent ) {
      // this is in cache only until we say "ok"
      Water* child = new Water(*parent, true);
      child->setType(Water::BASE);

      baseProfileButton->setWater(child);

      rangeWidget_ca->setValue(child->calcium_ppm());
      rangeWidget_cl->setValue(child->chloride_ppm());
      rangeWidget_mg->setValue(child->magnesium_ppm());
      rangeWidget_na->setValue(child->sodium_ppm());
      rangeWidget_bicarb->setValue(child->bicarbonate_ppm());
      rangeWidget_sulfate->setValue(child->sulfate_ppm());
   }
}

void WaterDialog::update_targetProfile(int selected)
{

   if ( obsRec == nullptr )
      return;

   QModelIndex proxyIdx(targetFilter->index(targetProfileCombo->currentIndex(),0));
   QModelIndex sourceIdx(targetFilter->mapToSource(proxyIdx));
   Water* parent = targetListModel->at(sourceIdx.row());

   if ( parent ) {
      // this is in cache only until we say "ok"
      Water* child = new Water(*parent, true);
      child->setType(Water::TARGET);
      targetProfileButton->setWater(child);

      setSlider(rangeWidget_ca, child->calcium_ppm());
      setSlider(rangeWidget_cl, child->chloride_ppm());
      setSlider(rangeWidget_mg,child->magnesium_ppm());
      setSlider(rangeWidget_na, child->sodium_ppm());
      setSlider(rangeWidget_bicarb, child->bicarbonate_ppm());
      setSlider(rangeWidget_sulfate, child->sulfate_ppm());
   }
}

void WaterDialog::dropEvent(QDropEvent *dpEvent)
{
   const QMimeData* mData;
   int _type;
   QString name;
   int id;
   QList<Misc*> salty;

   qDebug() << "Caught a drop";
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
         qDebug() << Q_FUNC_INFO << _type << id << name;
         salty.append( Database::instance().misc(id) );
   }

   if ( salty.size() > 0 )
      addSalts(salty);
   // this is like "mischief managed"
   dpEvent->acceptProposedAction();
}

void WaterDialog::addSalts(QList<Misc*> dropped)
{
   qDebug() << "Entering " << Q_FUNC_INFO;
   if ( obsRec == nullptr )
      return;

   foreach( Misc *m, dropped ) {
      QString mName = m->name();

      Salt::Types sType = Salt::NONE;

      // Oh, this just got hard. We dropped MISC but I need salts.
      Salt* gaq = Database::instance().newSalt();
      gaq->setName(mName);
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
      obsRec->addSalt(gaq);
   }
}
void WaterDialog::dragEnterEvent(QDragEnterEvent *deEvent)
{
   if ( acceptMime.size() == 0 )
      acceptMime = tableView_salts->property("mimeAccepted").toString();

   qDebug() << "Drag enter event" << acceptMime;

   if (deEvent->mimeData()->hasFormat(acceptMime) )
      deEvent->acceptProposedAction();
}
