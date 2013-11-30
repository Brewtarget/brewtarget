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
#include "BtTreeModel.h"
#include "BtTreeItem.h"

BtTreeFilterProxyModel::BtTreeFilterProxyModel(QObject *parent,BtTreeModel::TypeMasks mask ) 
: QSortFilterProxyModel(parent),
   treeMask(mask)
{
}

bool BtTreeFilterProxyModel::lessThan(const QModelIndex &left, 
                                         const QModelIndex &right) const
{

   BtTreeModel* model = qobject_cast<BtTreeModel*>(sourceModel());
   switch( treeMask )
   {
      case BtTreeModel::RECIPEMASK:
        return lessThanRecipe(model,left, right);
      case BtTreeModel::EQUIPMASK:
        return lessThanEquip(model,left, right);
      case BtTreeModel::FERMENTMASK:
        return lessThanFerment(model,left, right);
      case BtTreeModel::HOPMASK:
        return lessThanHop(model,left, right);
      case BtTreeModel::MISCMASK:
        return lessThanMisc(model,left, right);
      case BtTreeModel::YEASTMASK:
        return lessThanYeast(model,left, right);
      case BtTreeModel::STYLEMASK:
        return lessThanStyle(model,left, right);
      default:
        return lessThanRecipe(model,left, right);

    }
}

bool BtTreeFilterProxyModel::lessThanRecipe(BtTreeModel* model, const QModelIndex &left, const QModelIndex &right) const
{
   // This is a little awkward.
   if ( model->type(left) == BtTreeItem::BREWNOTE ||
        model->type(right) == BtTreeItem::BREWNOTE )
      return false;

   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::RECIPE)
   {
      BtFolder* leftFolder = model->getFolder(left);
      Recipe*  rightRecipe = model->getRecipe(right);

      return leftFolder->fullPath() < rightRecipe->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::RECIPE)
   {
      BtFolder* rightFolder = model->getFolder(right);
      Recipe*  leftRecipe = model->getRecipe(left);
      return leftRecipe->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->getFolder(right);
      BtFolder* leftFolder = model->getFolder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }


   Recipe* leftRecipe  = model->getRecipe(left);
   Recipe* rightRecipe = model->getRecipe(right);

   switch(left.column())
   {
      case BtTreeItem::RECIPENAMECOL:
         return leftRecipe->name() < rightRecipe->name();
      case BtTreeItem::RECIPEBREWDATECOL:
         return leftRecipe->date() < rightRecipe->date();
      case BtTreeItem::RECIPESTYLECOL:
         return leftRecipe->style()->name() < rightRecipe->style()->name();
   }
   // Default will be to just do a name sort. This doesn't likely make sense,
   // but it will prevent a lot of warnings.
   return leftRecipe->name() < rightRecipe->name();
}

bool BtTreeFilterProxyModel::lessThanEquip(BtTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Equipment* leftEquip = model->getEquipment(left);
   Equipment* rightEquip = model->getEquipment(right);


   switch(left.column())
   {
      case BtTreeItem::EQUIPMENTNAMECOL:
         return leftEquip->name() < rightEquip->name();
      case BtTreeItem::EQUIPMENTBOILTIMECOL:
         return leftEquip->boilTime_min() < rightEquip->boilTime_min();
   }
   return leftEquip->name() < rightEquip->name();
}

bool BtTreeFilterProxyModel::lessThanFerment(BtTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Fermentable* leftFerment = model->getFermentable(left);
   Fermentable* rightFerment = model->getFermentable(right);

   switch(left.column())
   {
      case BtTreeItem::FERMENTABLENAMECOL:
         return leftFerment->name() < rightFerment->name();
      case BtTreeItem::FERMENTABLETYPECOL:
         return leftFerment->type() < rightFerment->type();
      case BtTreeItem::FERMENTABLECOLORCOL:
         return leftFerment->color_srm() < rightFerment->color_srm();
   }
   return leftFerment->name() < rightFerment->name();
}

bool BtTreeFilterProxyModel::lessThanHop(BtTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Hop* leftHop = model->getHop(left);
   Hop* rightHop = model->getHop(right);


   switch(left.column())
   {
      case BtTreeItem::HOPNAMECOL:
         return leftHop->name() < rightHop->name();
      case BtTreeItem::HOPFORMCOL:
         return leftHop->form() < rightHop->form();
      case BtTreeItem::HOPUSECOL:
         return leftHop->use() < rightHop->use();
   }
   return leftHop->name() < rightHop->name();
}

bool BtTreeFilterProxyModel::lessThanMisc(BtTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Misc* leftMisc = model->getMisc(left);
   Misc* rightMisc = model->getMisc(right);


   switch(left.column())
   {
      case BtTreeItem::MISCNAMECOL:
         return leftMisc->name() < rightMisc->name();
      case BtTreeItem::MISCTYPECOL:
         return leftMisc->type() < rightMisc->type();
      case BtTreeItem::MISCUSECOL:
         return leftMisc->use() < rightMisc->use();
   }
   return leftMisc->name() < rightMisc->name();
}

bool BtTreeFilterProxyModel::lessThanYeast(BtTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Yeast* leftYeast = model->getYeast(left);
   Yeast* rightYeast = model->getYeast(right);


   switch(left.column())
   {
      case BtTreeItem::YEASTNAMECOL:
         return leftYeast->name() < rightYeast->name();
      case BtTreeItem::YEASTTYPECOL:
         return leftYeast->type() < rightYeast->type();
      case BtTreeItem::YEASTFORMCOL:
         return leftYeast->form() < rightYeast->form();
   }
   return leftYeast->name() < rightYeast->name();
}

bool BtTreeFilterProxyModel::lessThanStyle(BtTreeModel* model, const QModelIndex &left, 
                                         const QModelIndex &right) const
{
   Style* leftStyle = model->getStyle(left);
   Style* rightStyle = model->getStyle(right);


   switch(left.column())
   {
      case BtTreeItem::STYLENAMECOL:
         return leftStyle->name() < rightStyle->name();
      case BtTreeItem::STYLECATEGORYCOL:
         return leftStyle->category() < rightStyle->category();
      case BtTreeItem::STYLENUMBERCOL:
         return leftStyle->categoryNumber() < rightStyle->categoryNumber();
      case BtTreeItem::STYLELETTERCOL:
         return leftStyle->styleLetter() < rightStyle->styleLetter();
      case BtTreeItem::STYLEGUIDECOL:
         return leftStyle->styleGuide() < rightStyle->styleGuide();
   }
   return leftStyle->name() < rightStyle->name();
}

bool BtTreeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
   if ( !source_parent.isValid() )
      return true;

   const BtTreeModel* model = qobject_cast<const BtTreeModel*>(source_parent.model());

   QModelIndex child = model->index(source_row, 0, source_parent);

   if ( model->isFolder(child) ) 
      return true;

   BeerXMLElement* thing = model->getThing(child);

   return thing->display();

}
