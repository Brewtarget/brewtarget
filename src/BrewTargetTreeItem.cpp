/*
 * BrewTargetTreeItem.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#include <QString>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QDateTime>
#include <QString>
#include <QObject>
#include <QVector>

#include "brewtarget.h"
#include "recipe.h"
#include "BrewTargetTreeItem.h"

bool operator==(BrewTargetTreeItem& lhs, BrewTargetTreeItem& rhs)
{
   // Things of different types are not equal
   if ( lhs.type != rhs.type )
      return false;

   return lhs.data(lhs.type,0) == rhs.data(rhs.type,0);
}

BrewTargetTreeItem::BrewTargetTreeItem(int type, BrewTargetTreeItem *parent)
{
   parentItem = parent;
   setType(type);
   thing = 0;
}

BrewTargetTreeItem::~BrewTargetTreeItem()
{
   qDeleteAll(childItems);
}

BrewTargetTreeItem* BrewTargetTreeItem::child(int number)
{
   if ( number < childItems.count() )
      return childItems.value(number);

   return 0;
}

BrewTargetTreeItem* BrewTargetTreeItem::parent()
{
   return parentItem;
}

int BrewTargetTreeItem::getType()
{
    return type;
}

int BrewTargetTreeItem::childCount() const
{
   return childItems.count();
}

int BrewTargetTreeItem::columnCount(int type) const
{
    switch(type)
    {
        case RECIPE:
            return RECIPENUMCOLS;
        case EQUIPMENT:
            return EQUIPMENTNUMCOLS;
        case FERMENTABLE:
            return FERMENTABLENUMCOLS;
        case HOP:
            return HOPNUMCOLS;
        case MISC:
            return MISCNUMCOLS;
        case YEAST:
            return YEASTNUMCOLS;
        case BREWNOTE:
            return BREWNUMCOLS;
        default:
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(type));
            return 0;
    }
            
}

QVariant BrewTargetTreeItem::data(int type, int column)
{

   switch(type)
   {
      case RECIPE:
         return dataRecipe(column);
      case EQUIPMENT:
         return dataEquipment(column);
      case FERMENTABLE:
         return dataFermentable(column);
      case HOP:
         return dataHop(column);
      case MISC:
         return dataMisc(column);
      case YEAST:
         return dataYeast(column);
      case BREWNOTE:
         return dataBrewNote(column);
      default:
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
         return QVariant();
    }
}

int BrewTargetTreeItem::childNumber() const
{
   if (parentItem)
      return parentItem->childItems.indexOf(const_cast<BrewTargetTreeItem*>(this));
   return 0;
}

void BrewTargetTreeItem::setData(int t, void* d)
{
   thing = d;
   type  = t;
}

QVariant BrewTargetTreeItem::getData(int column)
{
   return data(getType(),column);
}

bool BrewTargetTreeItem::insertChildren(int position, int count, int type)
{
   int i;
   if ( position < 0  || position > childItems.size())
      return false;

   for(i=0; i < count; ++i)
   {
      BrewTargetTreeItem *newItem = new BrewTargetTreeItem(type,this);
      childItems.insert(position+i,newItem);
   }

   return true;
}

bool BrewTargetTreeItem::removeChildren(int position, int count)
{
   if ( position < 0 || position + count > childItems.count() )
      return false;

   for (int row = 0; row < count; ++row)
      delete childItems.takeAt(position);

   return true;
}

QVariant BrewTargetTreeItem::dataRecipe( int column ) 
{
   Recipe* recipe = static_cast<Recipe*>(thing);
   switch(column)
   {
        case RECIPENAMECOL:
         if (! thing)
            return QVariant(QObject::tr("Recipes"));
         else
            return QVariant(recipe->getName());
        case RECIPEBREWDATECOL:
         if ( recipe )
            return QVariant(recipe->getDate());
        case RECIPESTYLECOL:
         if ( recipe )
            return QVariant(recipe->getStyle()->getName());
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataEquipment(int column) 
{
    Equipment* kit = static_cast<Equipment*>(thing);
   switch(column)
   {
        case EQUIPMENTNAMECOL:
         if ( ! kit )
            return QVariant(QObject::tr("Equipment"));
         else
            return QVariant(kit->getName());
        case EQUIPMENTBOILTIMECOL:
         if ( kit )
            return QVariant(kit->getBoilTime_min());
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataFermentable(int column)
{
    Fermentable* ferm = static_cast<Fermentable*>(thing);
   switch(column)
   {
        case FERMENTABLENAMECOL:
         if ( ferm )
            return QVariant(ferm->getName());
         else
            return QVariant(QObject::tr("Fermentables"));
        case FERMENTABLETYPECOL:
         if ( ferm )
            return QVariant(ferm->getTypeStringTr());
        case FERMENTABLECOLORCOL:
         if ( ferm )
            return QVariant(ferm->getColor_srm());
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataHop(int column)
{
    Hop* hop = static_cast<Hop*>(thing);
   switch(column)
   {
      case HOPNAMECOL:
         if ( ! hop )
            return QVariant(QObject::tr("Hops"));
         else
            return QVariant(hop->getName());
      case HOPFORMCOL:
         if ( hop )
            return QVariant(hop->getFormStringTr());
      case HOPUSECOL:
         if ( hop )
            return QVariant(hop->getUseStringTr());
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataMisc(int column)
{
    Misc* misc = static_cast<Misc*>(thing);
   switch(column)
   {
      case MISCNAMECOL:
         if ( ! misc )
            return QVariant(QObject::tr("Miscellaneous"));
         else
            return QVariant(misc->getName());
      case MISCTYPECOL:
         if ( misc )
            return QVariant(misc->getTypeStringTr());
      case MISCUSECOL:
         if ( misc )
            return QVariant(misc->getUseStringTr());
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataYeast(int column)
{
   Yeast* yeast = static_cast<Yeast*>(thing);
   switch(column)
   {
      case YEASTNAMECOL:
         if ( ! yeast )
            return QVariant(QObject::tr("Yeast"));
         else
            return QVariant(yeast->getName());
      case YEASTTYPECOL:
         if ( yeast )
            return QVariant(yeast->getTypeStringTr());
      case YEASTFORMCOL:
         if ( yeast )
            return QVariant(yeast->getFormStringTr());
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataBrewNote(int column)
{
   if ( ! thing )
      return QVariant();

   BrewNote* bNote = static_cast<BrewNote*>(thing);

   return bNote->getBrewDate_short();
}

void BrewTargetTreeItem::setType(int t)
{
    type = t;
}

Recipe* BrewTargetTreeItem::getRecipe()
{
    if ( type == RECIPE && thing )
        return static_cast<Recipe*>(thing);

    return 0;
}

Equipment* BrewTargetTreeItem::getEquipment()
{
    if ( type == EQUIPMENT )
        return static_cast<Equipment*>(thing);
    return 0;
}

Fermentable* BrewTargetTreeItem::getFermentable()
{
    if ( type == FERMENTABLE )
        return static_cast<Fermentable*>(thing);
    return 0;
}

Hop* BrewTargetTreeItem::getHop()
{
    if ( type == HOP ) 
        return static_cast<Hop*>(thing);
    return 0;
}

Misc* BrewTargetTreeItem::getMisc()
{
    if ( type == MISC ) 
        return static_cast<Misc*>(thing);
    return 0;
}

Yeast* BrewTargetTreeItem::getYeast()
{
    if ( type == YEAST ) 
        return static_cast<Yeast*>(thing);
    return 0;
}

BrewNote* BrewTargetTreeItem::getBrewNote()
{
    if ( type == BREWNOTE && thing ) 
       return static_cast<BrewNote*>(thing);

    return 0;
}
