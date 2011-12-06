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

#include "BrewTargetTreeItem.h"
#include "brewnote.h"
#include "brewtarget.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "recipe.h"
#include "misc.h"
#include "yeast.h"

bool operator==(BrewTargetTreeItem& lhs, BrewTargetTreeItem& rhs)
{
   // Things of different types are not equal
   if ( lhs.type != rhs.type )
      return false;

   return lhs.data(lhs.type,0) == rhs.data(rhs.type,0);
}

BrewTargetTreeItem::BrewTargetTreeItem(int type, BrewTargetTreeItem *parent)
   : parentItem(parent), thing(0)
{
   setType(type);
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
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::columnCount Bad column: %1").arg(type));
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
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::data Bad column: %1").arg(column));
         return QVariant();
    }
}

int BrewTargetTreeItem::childNumber() const
{
   if (parentItem)
      return parentItem->childItems.indexOf(const_cast<BrewTargetTreeItem*>(this));
   return 0;
}

void BrewTargetTreeItem::setData(int t, QObject* d)
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
   Recipe* recipe = qobject_cast<Recipe*>(thing);
   switch(column)
   {
        case RECIPENAMECOL:
         if (! thing)
            return QVariant(QObject::tr("Recipes"));
        else
            return QVariant(recipe->name());
         break;
        case RECIPEBREWDATECOL:
         if ( recipe )
            return QVariant(recipe->date());
         break;
        case RECIPESTYLECOL:
         if ( recipe )
            return QVariant(recipe->style()->name());
         break;
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::dataRecipe Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataEquipment(int column) 
{
   Equipment* kit = qobject_cast<Equipment*>(thing);
   switch(column)
   {
        case EQUIPMENTNAMECOL:
         if ( ! kit )
            return QVariant(QObject::tr("Equipment"));
         else
            return QVariant(kit->name());
        case EQUIPMENTBOILTIMECOL:
         if ( kit )
            return QVariant(kit->boilTime_min());
         break;
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::dataEquipment Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataFermentable(int column)
{
    Fermentable* ferm = qobject_cast<Fermentable*>(thing);
   switch(column)
   {
        case FERMENTABLENAMECOL:
         if ( ferm )
            return QVariant(ferm->name());
         else
            return QVariant(QObject::tr("Fermentables"));
        case FERMENTABLETYPECOL:
         if ( ferm )
            return QVariant(ferm->typeStringTr());
         break;
        case FERMENTABLECOLORCOL:
         if ( ferm )
            return QVariant(ferm->color_srm());
         break;
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::dataFermentable Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataHop(int column)
{
    Hop* hop = qobject_cast<Hop*>(thing);
   switch(column)
   {
      case HOPNAMECOL:
         if ( ! hop )
            return QVariant(QObject::tr("Hops"));
         else
            return QVariant(hop->name());
      case HOPFORMCOL:
         if ( hop )
            return QVariant(hop->formStringTr());
         break;
      case HOPUSECOL:
         if ( hop )
            return QVariant(hop->useStringTr());
         break;
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::dataHop Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataMisc(int column)
{
    Misc* misc = qobject_cast<Misc*>(thing);
   switch(column)
   {
      case MISCNAMECOL:
         if ( ! misc )
            return QVariant(QObject::tr("Miscellaneous"));
         else
            return QVariant(misc->name());
      case MISCTYPECOL:
         if ( misc )
            return QVariant(misc->typeStringTr());
         break;
      case MISCUSECOL:
         if ( misc )
            return QVariant(misc->useStringTr());
         break;
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::dataMisc Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataYeast(int column)
{
   Yeast* yeast = qobject_cast<Yeast*>(thing);
   switch(column)
   {
      case YEASTNAMECOL:
         if ( ! yeast )
            return QVariant(QObject::tr("Yeast"));
         else
            return QVariant(yeast->name());
      case YEASTTYPECOL:
         if ( yeast )
            return QVariant(yeast->typeStringTr());
         break;
      case YEASTFORMCOL:
         if ( yeast )
            return QVariant(yeast->formStringTr());
         break;
      default :
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("BrewTargetTreeItem::dataYeast Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BrewTargetTreeItem::dataBrewNote(int column)
{
   if ( ! thing )
      return QVariant();

   BrewNote* bNote = qobject_cast<BrewNote*>(thing);

   return bNote->brewDate_short();
}

void BrewTargetTreeItem::setType(int t)
{
    type = t;
}

Recipe* BrewTargetTreeItem::getRecipe()
{
    if ( type == RECIPE && thing )
        return qobject_cast<Recipe*>(thing);

    return 0;
}

Equipment* BrewTargetTreeItem::getEquipment()
{
    if ( type == EQUIPMENT )
       return qobject_cast<Equipment*>(thing);
    return 0;
}

Fermentable* BrewTargetTreeItem::getFermentable()
{
    if ( type == FERMENTABLE )
       return qobject_cast<Fermentable*>(thing);
    return 0;
}

Hop* BrewTargetTreeItem::getHop()
{
    if ( type == HOP ) 
       return qobject_cast<Hop*>(thing);
    return 0;
}

Misc* BrewTargetTreeItem::getMisc()
{
    if ( type == MISC ) 
       return qobject_cast<Misc*>(thing);
    return 0;
}

Yeast* BrewTargetTreeItem::getYeast()
{
    if ( type == YEAST ) 
       return qobject_cast<Yeast*>(thing);
    return 0;
}

BrewNote* BrewTargetTreeItem::getBrewNote()
{
    if ( type == BREWNOTE && thing ) 
       return qobject_cast<BrewNote*>(thing);

    return 0;
}
