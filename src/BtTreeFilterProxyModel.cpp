/*
 * BtTreeFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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
      case BtTreeModel::WATERMASK:
        return lessThanWater(model,left, right);
      default:
        return lessThanRecipe(model,left, right);

    }
}

bool BtTreeFilterProxyModel::lessThanRecipe(BtTreeModel* model, const QModelIndex &left, const QModelIndex &right) const
{
   // This is a little awkward.
   if ( model->type(left) == BtTreeItem::BREWNOTE ||
        model->type(right) == BtTreeItem::BREWNOTE ) {
      BrewNote *leftBn = model->brewNote(left);
      BrewNote *rightBn = model->brewNote(right);

      return leftBn->brewDate() < rightBn->brewDate();
   }

   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::RECIPE)
   {
      BtFolder* leftFolder = model->folder(left);
      Recipe*  rightRecipe = model->recipe(right);

      return leftFolder->fullPath() < rightRecipe->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::RECIPE)
   {
      BtFolder* rightFolder = model->folder(right);
      Recipe*  leftRecipe = model->recipe(left);
      return leftRecipe->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }


   Recipe* leftRecipe  = model->recipe(left);
   Recipe* rightRecipe = model->recipe(right);

   switch(left.column())
   {
      case BtTreeItem::RECIPENAMECOL:
         return leftRecipe->name() < rightRecipe->name();
      case BtTreeItem::RECIPEBREWDATECOL:
         return leftRecipe->date() < rightRecipe->date();
      case BtTreeItem::RECIPESTYLECOL:
         if ( ! leftRecipe->style() )
            return true;
         else if ( ! rightRecipe->style() )
            return false;
         else
            return leftRecipe->style()->name() < rightRecipe->style()->name();
   }
   // Default will be to just do a name sort. This doesn't likely make sense,
   // but it will prevent a lot of warnings.
   return leftRecipe->name() < rightRecipe->name();
}

bool BtTreeFilterProxyModel::lessThanEquip(BtTreeModel* model, const QModelIndex &left,
                                         const QModelIndex &right) const
{
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::EQUIPMENT)
   {
      BtFolder* leftFolder = model->folder(left);
      Equipment*  rightEquipment = model->equipment(right);

      return leftFolder->fullPath() < rightEquipment->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::EQUIPMENT)
   {
      BtFolder* rightFolder = model->folder(right);
      Equipment*  leftEquipment = model->equipment(left);
      return leftEquipment->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Equipment* leftEquip = model->equipment(left);
   Equipment* rightEquip = model->equipment(right);


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
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::FERMENTABLE)
   {
      BtFolder* leftFolder = model->folder(left);
      Fermentable*  rightFermentable = model->fermentable(right);

      return leftFolder->fullPath() < rightFermentable->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FERMENTABLE)
   {
      BtFolder* rightFolder = model->folder(right);
      Fermentable*  leftFermentable = model->fermentable(left);
      return leftFermentable->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Fermentable* leftFerment = model->fermentable(left);
   Fermentable* rightFerment = model->fermentable(right);

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
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::HOP)
   {
      BtFolder* leftFolder = model->folder(left);
      Hop*  rightHop = model->hop(right);

      return leftFolder->fullPath() < rightHop->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::HOP)
   {
      BtFolder* rightFolder = model->folder(right);
      Hop*  leftHop = model->hop(left);
      return leftHop->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Hop* leftHop = model->hop(left);
   Hop* rightHop = model->hop(right);


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
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::MISC)
   {
      BtFolder* leftFolder = model->folder(left);
      Misc*  rightMisc = model->misc(right);

      return leftFolder->fullPath() < rightMisc->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::MISC)
   {
      BtFolder* rightFolder = model->folder(right);
      Misc*  leftMisc = model->misc(left);
      return leftMisc->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Misc* leftMisc = model->misc(left);
   Misc* rightMisc = model->misc(right);

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

bool BtTreeFilterProxyModel::lessThanStyle(BtTreeModel* model, const QModelIndex &left,
                                         const QModelIndex &right) const
{
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::STYLE)
   {
      BtFolder* leftFolder = model->folder(left);
      Style*  rightStyle = model->style(right);

      return leftFolder->fullPath() < rightStyle->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::STYLE)
   {
      BtFolder* rightFolder = model->folder(right);
      Style*  leftStyle = model->style(left);
      return leftStyle->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Style* leftStyle = model->style(left);
   Style* rightStyle = model->style(right);


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

bool BtTreeFilterProxyModel::lessThanYeast(BtTreeModel* model, const QModelIndex &left,
                                         const QModelIndex &right) const
{
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::YEAST)
   {
      BtFolder* leftFolder = model->folder(left);
      Yeast*  rightYeast = model->yeast(right);

      return leftFolder->fullPath() < rightYeast->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::YEAST)
   {
      BtFolder* rightFolder = model->folder(right);
      Yeast*  leftYeast = model->yeast(left);
      return leftYeast->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Yeast* leftYeast = model->yeast(left);
   Yeast* rightYeast = model->yeast(right);


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

bool BtTreeFilterProxyModel::lessThanWater(BtTreeModel* model, const QModelIndex &left,
                                         const QModelIndex &right) const
{
   // As the models get more complex, so does the sort algorithm
   if ( model->type(left) == BtTreeItem::FOLDER && model->type(right) == BtTreeItem::WATER)
   {
      BtFolder* leftFolder = model->folder(left);
      Water*  rightWater = model->water(right);

      return leftFolder->fullPath() < rightWater->name();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::WATER)
   {
      BtFolder* rightFolder = model->folder(right);
      Water*  leftWater = model->water(left);
      return leftWater->name() < rightFolder->fullPath();
   }
   else if (model->type(right) == BtTreeItem::FOLDER && model->type(left) == BtTreeItem::FOLDER)
   {
      BtFolder* rightFolder = model->folder(right);
      BtFolder* leftFolder = model->folder(left);
      return leftFolder->fullPath() < rightFolder->fullPath();
   }

   Water*  leftWater = model->water(left);
   Water* rightWater = model->water(right);


   switch(left.column())
   {
      case BtTreeItem::WATERNAMECOL:
         return leftWater->name() < rightWater->name();
      case BtTreeItem::WATERpHCOL:
         return leftWater->ph() < rightWater->ph();
      case BtTreeItem::WATERHCO3COL:
         return leftWater->bicarbonate_ppm() < rightWater->bicarbonate_ppm();
      case BtTreeItem::WATERSO4COL:
         return leftWater->sulfate_ppm() < rightWater->sulfate_ppm();
      case BtTreeItem::WATERCLCOL:
         return leftWater->chloride_ppm() < rightWater->chloride_ppm();
      case BtTreeItem::WATERNACOL:
         return leftWater->sodium_ppm() < rightWater->sodium_ppm();
      case BtTreeItem::WATERMGCOL:
         return leftWater->magnesium_ppm() < rightWater->magnesium_ppm();
      case BtTreeItem::WATERCACOL:
         return leftWater->calcium_ppm() < rightWater->calcium_ppm();
   }
   return leftWater->name() < rightWater->name();
}

bool BtTreeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
   if ( !source_parent.isValid() )
      return true;

   const BtTreeModel* model = qobject_cast<const BtTreeModel*>(source_parent.model());

   QModelIndex child = model->index(source_row, 0, source_parent);

   // We shouldn't get here, but if we cannot find the row in the parent,
   // don't display the item.
   if ( ! child.isValid() )
      return false;

   if ( model->isFolder(child) )
      return true;

   Ingredient* thing = model->thing(child);

   return thing->display();

}
