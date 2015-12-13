/*
 * BtTreeItem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#include <QString>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QDateTime>
#include <QString>
#include <QObject>
#include <QVector>
#include <QDebug>

#include "BtTreeItem.h"
#include "brewnote.h"
#include "brewtarget.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "recipe.h"
#include "misc.h"
#include "yeast.h"
#include "style.h"
#include "BtFolder.h"

bool operator==(BtTreeItem& lhs, BtTreeItem& rhs)
{
   // Things of different types are not equal
   if ( lhs._type != rhs._type )
      return false;

   return lhs.data(lhs._type,0) == rhs.data(rhs._type,0);
}

BtTreeItem::BtTreeItem(int _type, BtTreeItem *parent)
   : parentItem(parent), _thing(0)
{
   setType(_type);
}

BtTreeItem::~BtTreeItem()
{
   qDeleteAll(childItems);
}

BtTreeItem* BtTreeItem::child(int number)
{
   if ( number < childItems.count() )
      return childItems.value(number);

   return 0;
}

BtTreeItem* BtTreeItem::parent()
{
   return parentItem;
}

int BtTreeItem::type()
{
    return _type;
}

int BtTreeItem::childCount() const
{
   return childItems.count();
}

int BtTreeItem::columnCount(int _type) const
{
    switch(_type)
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
        case STYLE:
            return STYLENUMCOLS;
        case BREWNOTE:
            return BREWNUMCOLS;
        case FOLDER:
            return FOLDERNUMCOLS;
        default:
         Brewtarget::logW( QString("BtTreeItem::columnCount Bad column: %1").arg(_type));
            return 0;
    }
            
}

QVariant BtTreeItem::data(int _type, int column)
{

   switch(_type)
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
      case STYLE:
         return dataStyle(column);
      case BREWNOTE:
         return dataBrewNote(column);
      case FOLDER:
         return dataFolder(column);
      default:
         Brewtarget::logW( QString("BtTreeItem::data Bad column: %1").arg(column));
         return QVariant();
    }
}

int BtTreeItem::childNumber() const
{
   if (parentItem)
      return parentItem->childItems.indexOf(const_cast<BtTreeItem*>(this));
   return 0;
}

void BtTreeItem::setData(int t, QObject* d)
{
   _thing = d;
   _type  = t;
}

QVariant BtTreeItem::data(int column)
{
   return data(type(),column);
}

bool BtTreeItem::insertChildren(int position, int count, int _type)
{
   if ( position < 0  || position > childItems.size())
      return false;

   for (int row = 0; row < count; ++row) {
      BtTreeItem *newItem = new BtTreeItem(_type, this);
      childItems.insert(position + row, newItem);
   }

   return true;
}

bool BtTreeItem::removeChildren(int position, int count)
{
   if ( position < 0 || position + count > childItems.count() )
      return false;

   for (int row = 0; row < count; ++row)
      delete childItems.takeAt(position);
      // FIXME: memory leak here. With delete, it's a concurrency/memory
      // access error, due to the fact that these pointers are floating around.
      //childItems.takeAt(position);

   return true;
}

QVariant BtTreeItem::dataRecipe( int column ) 
{
   Recipe* recipe = qobject_cast<Recipe*>(_thing);
   switch(column)
   {
        case RECIPENAMECOL:
         if (! _thing)
            return QVariant(QObject::tr("Recipes"));
        else
            return QVariant(recipe->name());
         break;
        case RECIPEBREWDATECOL:
         if ( recipe )
            return Brewtarget::displayDateUserFormated(recipe->date());
         break;
        case RECIPESTYLECOL:
         if ( recipe && recipe->style() )
            return QVariant(recipe->style()->name());
         break;
      default :
         Brewtarget::logW( QString("BtTreeItem::dataRecipe Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BtTreeItem::dataEquipment(int column) 
{
   Equipment* kit = qobject_cast<Equipment*>(_thing);
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
         Brewtarget::logW( QString("BtTreeItem::dataEquipment Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BtTreeItem::dataFermentable(int column)
{
    Fermentable* ferm = qobject_cast<Fermentable*>(_thing);
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
         Brewtarget::logW( QString("BtTreeItem::dataFermentable Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BtTreeItem::dataHop(int column)
{
    Hop* hop = qobject_cast<Hop*>(_thing);
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
         Brewtarget::logW( QString("BtTreeItem::dataHop Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BtTreeItem::dataMisc(int column)
{
    Misc* misc = qobject_cast<Misc*>(_thing);
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
         Brewtarget::logW( QString("BtTreeItem::dataMisc Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BtTreeItem::dataYeast(int column)
{
   Yeast* yeast = qobject_cast<Yeast*>(_thing);
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
         Brewtarget::logW( QString("BtTreeItem::dataYeast Bad column: %1").arg(column));
   }
   return QVariant();
}

QVariant BtTreeItem::dataBrewNote(int column)
{
   if ( ! _thing )
      return QVariant();

   BrewNote* bNote = qobject_cast<BrewNote*>(_thing);

   return bNote->brewDate_short();
}

QVariant BtTreeItem::dataStyle(int column)
{
   Style* style = qobject_cast<Style*>(_thing);

   if ( ! style && column == STYLENAMECOL )
   {
      return QVariant(QObject::tr("Style"));
   }
   else if ( style )
   {
      switch(column)
      {
         case STYLENAMECOL:
               return QVariant(style->name());
         case STYLECATEGORYCOL:
            return QVariant(style->category());
         case STYLENUMBERCOL:
               return QVariant(style->categoryNumber());
         case STYLELETTERCOL:
               return QVariant(style->styleLetter());
         case STYLEGUIDECOL:
               return QVariant(style->styleGuide());
         default :
            Brewtarget::logW( QString("BtTreeItem::dataStyle Bad column: %1").arg(column));
      }
   }
   return QVariant();
}

QVariant BtTreeItem::dataFolder(int column)
{
   BtFolder* folder = qobject_cast<BtFolder*>(_thing);


   if ( ! folder && column == FOLDERNAMECOL )
      return QVariant(QObject::tr("Folder"));

   if ( ! folder ) 
      return QVariant(QObject::tr("Folder"));
   else if ( column == FOLDERNAMECOL )
      return QVariant( folder->name() );

   return QVariant();
}

void BtTreeItem::setType(int t)
{
    _type = t;
}

Recipe* BtTreeItem::recipe()
{
    if ( _type == RECIPE && _thing )
        return qobject_cast<Recipe*>(_thing);

    return 0;
}

Equipment* BtTreeItem::equipment()
{
    if ( _type == EQUIPMENT )
       return qobject_cast<Equipment*>(_thing);
    return 0;
}

Fermentable* BtTreeItem::fermentable()
{
    if ( _type == FERMENTABLE )
       return qobject_cast<Fermentable*>(_thing);
    return 0;
}

Hop* BtTreeItem::hop()
{
    if ( _type == HOP ) 
       return qobject_cast<Hop*>(_thing);
    return 0;
}

Misc* BtTreeItem::misc()
{
    if ( _type == MISC ) 
       return qobject_cast<Misc*>(_thing);
    return 0;
}

Yeast* BtTreeItem::yeast()
{
    if ( _type == YEAST ) 
       return qobject_cast<Yeast*>(_thing);
    return 0;
}

BrewNote* BtTreeItem::brewNote()
{
    if ( _type == BREWNOTE && _thing ) 
       return qobject_cast<BrewNote*>(_thing);

    return 0;
}

Style* BtTreeItem::style()
{
    if ( _type == STYLE && _thing ) 
       return qobject_cast<Style*>(_thing);

    return 0;
}

BtFolder* BtTreeItem::folder()
{
    if ( _type == FOLDER && _thing ) 
       return qobject_cast<BtFolder*>(_thing);

    return 0;
}

BeerXMLElement* BtTreeItem::thing()
{
    if ( _thing )
        return qobject_cast<BeerXMLElement*>(_thing);

    return 0;
}

QString BtTreeItem::name()
{
   if ( ! _thing )
      return QString();
   switch(_type)
   {
      case RECIPE:
         return qobject_cast<Recipe*>(_thing)->name();
      case EQUIPMENT:
         return qobject_cast<Equipment*>(_thing)->name();
      case FERMENTABLE:
         return qobject_cast<Fermentable*>(_thing)->name();
      case FOLDER:
         return qobject_cast<BtFolder*>(_thing)->name();
      case HOP:
         return qobject_cast<Hop*>(_thing)->name();
      case MISC:
         return qobject_cast<Misc*>(_thing)->name();
      case STYLE:
         return qobject_cast<Style*>(_thing)->name();
      case YEAST:
         return qobject_cast<Yeast*>(_thing)->name();
   }
   return QString();
}
