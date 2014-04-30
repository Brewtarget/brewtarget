/*
 * BtTreeFilterProxyModel.cpp is part of Brewtarget, and is Copyright Mik
 * Firestone (mikfire@gmail.com), 2012-2013.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QDebug>

#include "brewtarget.h"
#include "BtTreeFilterProxyModel.h"
#include "BrewTargetTreeModel.h"
#include "BrewTargetTreeItem.h"

BtTreeFilterProxyModel::BtTreeFilterProxyModel(QObject *parent,BrewTargetTreeModel::TypeMasks mask ) 
: QSortFilterProxyModel(parent),
   treeMask(mask)
{
}

bool BtTreeFilterProxyModel::lessThan(const QModelIndex &left, 
                                         const QModelIndex &right) const
{

   BrewTargetTreeModel* model = qobject_cast<BrewTargetTreeModel*>(sourceModel());
   switch( treeMask )
   {
      case BrewTargetTreeModel::RECIPEMASK:
        return lessThanRecipe(model,left, right);
      case BrewTargetTreeModel::EQUIPMASK:
        return lessThanEquip(model,left, right);
      case BrewTargetTreeModel::FERMENTMASK:
        return lessThanFerment(model,left, right);
      case BrewTargetTreeModel::HOPMASK:
        return lessThanHop(model,left, right);
      case BrewTargetTreeModel::MISCMASK:
        return lessThanMisc(model,left, right);
      case BrewTargetTreeModel::YEASTMASK:
        return lessThanYeast(model,left, right);
      case BrewTargetTreeModel::STYLEMASK:
        return lessThanStyle(model,left, right);
      default:
        return lessThanRecipe(model,left, right);

    }
}

bool BtTreeFilterProxyModel::lessThanRecipe(BrewTargetTreeModel* model, const QModelIndex &left, const QModelIndex &right) const
{
   // This is a little awkward.
   if ( model->getType(left) == BrewTargetTreeItem::BREWNOTE ||
        model->getType(right) == BrewTargetTreeItem::BREWNOTE )
      return false;


   Recipe* leftRecipe  = model->getRecipe(left);
   Recipe* rightRecipe = model->getRecipe(right);

   switch(left.column())
   {
      case BrewTargetTreeItem::RECIPENAMECOL:
         return leftRecipe->name() < rightRecipe->name();
      case BrewTargetTreeItem::RECIPEBREWDATECOL:
         return leftRecipe->date() < rightRecipe->date();
      case BrewTargetTreeItem::RECIPESTYLECOL:
         if ( ! leftRecipe->style() )
            return true;
         else if ( ! rightRecipe->style() )
            return false;

         return leftRecipe->style()->name() < rightRecipe->style()->name();
   }
   // Default will be to just do a name sort. This doesn't likely make sense,
   // but it will prevent a lot of warnings.
   return leftRecipe->name() < rightRecipe->name();
}

bool BtTreeFilterProxyModel::lessThanEquip(BrewTargetTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Equipment* leftEquip = model->getEquipment(left);
   Equipment* rightEquip = model->getEquipment(right);


   switch(left.column())
   {
      case BrewTargetTreeItem::EQUIPMENTNAMECOL:
         return leftEquip->name() < rightEquip->name();
      case BrewTargetTreeItem::EQUIPMENTBOILTIMECOL:
         return leftEquip->boilTime_min() < rightEquip->boilTime_min();
   }
   return leftEquip->name() < rightEquip->name();
}

bool BtTreeFilterProxyModel::lessThanFerment(BrewTargetTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Fermentable* leftFerment = model->getFermentable(left);
   Fermentable* rightFerment = model->getFermentable(right);

   switch(left.column())
   {
      case BrewTargetTreeItem::FERMENTABLENAMECOL:
         return leftFerment->name() < rightFerment->name();
      case BrewTargetTreeItem::FERMENTABLETYPECOL:
         return leftFerment->type() < rightFerment->type();
      case BrewTargetTreeItem::FERMENTABLECOLORCOL:
         return leftFerment->color_srm() < rightFerment->color_srm();
   }
   return leftFerment->name() < rightFerment->name();
}

bool BtTreeFilterProxyModel::lessThanHop(BrewTargetTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Hop* leftHop = model->getHop(left);
   Hop* rightHop = model->getHop(right);


   switch(left.column())
   {
      case BrewTargetTreeItem::HOPNAMECOL:
         return leftHop->name() < rightHop->name();
      case BrewTargetTreeItem::HOPFORMCOL:
         return leftHop->form() < rightHop->form();
      case BrewTargetTreeItem::HOPUSECOL:
         return leftHop->use() < rightHop->use();
   }
   return leftHop->name() < rightHop->name();
}

bool BtTreeFilterProxyModel::lessThanMisc(BrewTargetTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Misc* leftMisc = model->getMisc(left);
   Misc* rightMisc = model->getMisc(right);


   switch(left.column())
   {
      case BrewTargetTreeItem::MISCNAMECOL:
         return leftMisc->name() < rightMisc->name();
      case BrewTargetTreeItem::MISCTYPECOL:
         return leftMisc->type() < rightMisc->type();
      case BrewTargetTreeItem::MISCUSECOL:
         return leftMisc->use() < rightMisc->use();
   }
   return leftMisc->name() < rightMisc->name();
}

bool BtTreeFilterProxyModel::lessThanYeast(BrewTargetTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Yeast* leftYeast = model->getYeast(left);
   Yeast* rightYeast = model->getYeast(right);


   switch(left.column())
   {
      case BrewTargetTreeItem::YEASTNAMECOL:
         return leftYeast->name() < rightYeast->name();
      case BrewTargetTreeItem::YEASTTYPECOL:
         return leftYeast->type() < rightYeast->type();
      case BrewTargetTreeItem::YEASTFORMCOL:
         return leftYeast->form() < rightYeast->form();
   }
   return leftYeast->name() < rightYeast->name();
}

bool BtTreeFilterProxyModel::lessThanStyle(BrewTargetTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Style* leftStyle = model->getStyle(left);
   Style* rightStyle = model->getStyle(right);


   switch(left.column())
   {
      case BrewTargetTreeItem::STYLENAMECOL:
         return leftStyle->name() < rightStyle->name();
      case BrewTargetTreeItem::STYLECATEGORYCOL:
         return leftStyle->category() < rightStyle->category();
      case BrewTargetTreeItem::STYLENUMBERCOL:
         return leftStyle->categoryNumber() < rightStyle->categoryNumber();
      case BrewTargetTreeItem::STYLELETTERCOL:
         return leftStyle->styleLetter() < rightStyle->styleLetter();
      case BrewTargetTreeItem::STYLEGUIDECOL:
         return leftStyle->styleGuide() < rightStyle->styleGuide();
   }
   return leftStyle->name() < rightStyle->name();
}

bool BtTreeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
   if ( !source_parent.isValid() )
      return true;

   const BrewTargetTreeModel* model = qobject_cast<const BrewTargetTreeModel*>(source_parent.model());
   QModelIndex child = model->index(source_row, 0, source_parent);
   BeerXMLElement* thing = model->getThing(child);

   return thing->display();

}
