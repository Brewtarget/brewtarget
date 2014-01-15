/*
 * BtTreeModel.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <Qt>
#include <QObject>
#include <QStringBuilder>

#include "brewtarget.h"
#include "BtTreeItem.h"
#include "BtTreeModel.h"
#include "BtTreeView.h"
#include "database.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "recipe.h"
#include "yeast.h"
#include "brewnote.h"
#include "style.h"

BtTreeModel::BtTreeModel(BtTreeView *parent, TypeMasks type)
   : QAbstractItemModel(parent)
{
   // Initialize the tree structure
   int items = 0;
   rootItem = new BtTreeItem();

   switch (type)
   {
      case RECIPEMASK:
         rootItem->insertChildren(items,1,BtTreeItem::RECIPE);
         connect( &(Database::instance()), SIGNAL(newRecipeSignal(Recipe*)),this, SLOT(elementAdded(Recipe*)));
         connect( &(Database::instance()), SIGNAL(deletedRecipeSignal(Recipe*)),this, SLOT(elementRemoved(Recipe*)));
         // Brewnotes need love too!
         connect( &(Database::instance()), SIGNAL(newBrewNoteSignal(BrewNote*)),this, SLOT(elementAdded(BrewNote*)));
         connect( &(Database::instance()), SIGNAL(deletedBrewNoteSignal(BrewNote*)),this, SLOT(elementRemoved(BrewNote*)));
         _type = BtTreeItem::RECIPE;
         _mimeType = "application/x-brewtarget-recipe";
         break;
      case EQUIPMASK:
         rootItem->insertChildren(items,1,BtTreeItem::EQUIPMENT);
         connect( &(Database::instance()), SIGNAL(newEquipmentSignal(Equipment*)),this, SLOT(elementAdded(Equipment*)));
         connect( &(Database::instance()), SIGNAL(deletedEquipmentSignal(Equipment*)),this, SLOT(elementRemoved(Equipment*)));
         _type = BtTreeItem::EQUIPMENT;
         _mimeType = "application/x-brewtarget-recipe";
         break;
      case FERMENTMASK:
         rootItem->insertChildren(items,1,BtTreeItem::FERMENTABLE);
         connect( &(Database::instance()), SIGNAL(newFermentableSignal(Fermentable*)),this, SLOT(elementAdded(Fermentable*)));
         connect( &(Database::instance()), SIGNAL(deletedFermentableSignal(Fermentable*)),this, SLOT(elementRemoved(Fermentable*)));
         _type = BtTreeItem::FERMENTABLE;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case HOPMASK:
         rootItem->insertChildren(items,1,BtTreeItem::HOP);
         connect( &(Database::instance()), SIGNAL(newHopSignal(Hop*)),this, SLOT(elementAdded(Hop*)));
         connect( &(Database::instance()), SIGNAL(deletedHopSignal(Hop*)),this, SLOT(elementRemoved(Hop*)));
         _type = BtTreeItem::HOP;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case MISCMASK:
         rootItem->insertChildren(items,1,BtTreeItem::MISC);
         connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)),this, SLOT(elementAdded(Misc*)));
         connect( &(Database::instance()), SIGNAL(deletedMiscSignal(Misc*)),this, SLOT(elementRemoved(Misc*)));
         _type = BtTreeItem::MISC;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case STYLEMASK:
         rootItem->insertChildren(items,1,BtTreeItem::STYLE);
         connect( &(Database::instance()), SIGNAL(newStyleSignal(Style*)),this, SLOT(elementAdded(Style*)));
         connect( &(Database::instance()), SIGNAL(deletedStyleSignal(Style*)),this, SLOT(elementRemoved(Style*)));
         _type = BtTreeItem::STYLE;
         _mimeType = "application/x-brewtarget-recipe";
         break;
      case YEASTMASK:
         rootItem->insertChildren(items,1,BtTreeItem::YEAST);
         connect( &(Database::instance()), SIGNAL(newYeastSignal(Yeast*)),this, SLOT(elementAdded(Yeast*)));
         connect( &(Database::instance()), SIGNAL(deletedYeastSignal(Yeast*)),this, SLOT(elementRemoved(Yeast*)));
         _type = BtTreeItem::YEAST;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      default:
         Brewtarget::logW(QString("Invalid treemask: %1").arg(type));
   }

   treeMask = type;
   parentTree = parent;
   loadTreeModel();
}

BtTreeModel::~BtTreeModel()
{
   delete rootItem;
}

BtTreeItem *BtTreeModel::item( const QModelIndex &index ) const
{
   if ( index.isValid())
   {
      BtTreeItem *item = static_cast<BtTreeItem*>(index.internalPointer());
      if (item)
         return item;
   }

   return rootItem;
}

int BtTreeModel::rowCount(const QModelIndex &parent) const
{
   if (! parent.isValid())
      return rootItem->childCount();
   
   BtTreeItem *pItem = item(parent);

   return pItem->childCount();
}

int BtTreeModel::columnCount( const QModelIndex &parent) const
{
   switch(treeMask)
   {
   case RECIPEMASK:
      return BtTreeItem::RECIPENUMCOLS;
   case EQUIPMASK:
      return BtTreeItem::EQUIPMENTNUMCOLS;
   case FERMENTMASK:
      return BtTreeItem::FERMENTABLENUMCOLS;
   case HOPMASK:
      return BtTreeItem::HOPNUMCOLS;
   case MISCMASK:
      return BtTreeItem::MISCNUMCOLS;
   case YEASTMASK:
      return BtTreeItem::YEASTNUMCOLS;
   case STYLEMASK:
      return BtTreeItem::STYLENUMCOLS;
   default:
      return 0;
   }
   // Backwards compatibility. This MUST be fixed before the code goes live.
   return BtTreeItem::RECIPENUMCOLS;
}

Qt::ItemFlags BtTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return Qt::ItemIsDropEnabled;

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QModelIndex BtTreeModel::index( int row, int column, const QModelIndex &parent) const
{
   BtTreeItem *pItem, *cItem;

   if ( parent.isValid() && parent.column() != 0)
      return QModelIndex();

   pItem = item(parent);
   cItem = pItem->child(row);

   if (cItem)
      return createIndex(row,column,cItem);
   else
      return QModelIndex();
}

QModelIndex BtTreeModel::parent(const QModelIndex &index) const
{
   BtTreeItem *pItem, *cItem;

   if (!index.isValid())
      return QModelIndex();

   cItem = item(index);

   if ( cItem == 0 )
      return QModelIndex();

   pItem = cItem->parent();

   if (pItem == rootItem || pItem == 0 )
      return QModelIndex();

   return createIndex(pItem->childNumber(),0,pItem);
}

QModelIndex BtTreeModel::first()
{
   QModelIndex parent;
   BtTreeItem* pItem; 

   // get the first item in the list, which is the place holder
   pItem = rootItem->child(0);
   if ( pItem->childCount() > 0 )
      return createIndex(0,0,pItem->child(0));

   return QModelIndex();
}

QVariant BtTreeModel::data(const QModelIndex &index, int role) const
{
   int maxColumns;

   switch(treeMask)
   {
   case RECIPEMASK:
      maxColumns = BtTreeItem::RECIPENUMCOLS;
      break;
   case EQUIPMASK:
      maxColumns = BtTreeItem::EQUIPMENTNUMCOLS;
      break;
   case FERMENTMASK:
      maxColumns = BtTreeItem::FERMENTABLENUMCOLS;
      break;
   case HOPMASK:
      maxColumns = BtTreeItem::HOPNUMCOLS;
      break;
   case MISCMASK:
      maxColumns = BtTreeItem::MISCNUMCOLS;
      break;
   case YEASTMASK:
      maxColumns = BtTreeItem::YEASTNUMCOLS;
      break;
   case STYLEMASK:
      maxColumns = BtTreeItem::STYLENUMCOLS;
      break;
   case FOLDERMASK:
      maxColumns = BtTreeItem::FOLDERNUMCOLS;
      break;
   default:
      // Backwards compatibility. This MUST be fixed prior to releasing the code
      maxColumns = BtTreeItem::RECIPENUMCOLS;
   }

   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns)
      return QVariant();

   if ( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::DecorationRole)
      return QVariant();

   BtTreeItem* itm = item(index);
   if ( role == Qt::DecorationRole && index.column() == 0) 
   {
      if ( itm->type() == BtTreeItem::FOLDER )
         return QIcon(":images/folder.png");
      else
         return QVariant();
   }

   return itm->data(index.column());
}

// This is much better, assuming the rest can be made to work
QVariant BtTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
      return QVariant();

   switch(treeMask)
   {
   case RECIPEMASK:
      return recipeHeader(section);
   case EQUIPMASK:
      return equipmentHeader(section);
   case FERMENTMASK:
      return fermentableHeader(section);
   case HOPMASK:
      return hopHeader(section);
   case MISCMASK:
      return miscHeader(section);
   case YEASTMASK:
      return yeastHeader(section);
   case STYLEMASK:
      return styleHeader(section);
   case FOLDERMASK:
      return folderHeader(section);
   default: 
      return QVariant();
   }
}

QVariant BtTreeModel::recipeHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::RECIPENAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::RECIPEBREWDATECOL:
      return QVariant(tr("Brew Date"));
   case BtTreeItem::RECIPESTYLECOL:
      return QVariant(tr("Style"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BtTreeModel::getRecipeHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BtTreeModel::equipmentHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::EQUIPMENTNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::EQUIPMENTBOILTIMECOL:
      return QVariant(tr("Boil Time"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BtTreeModel::getEquipmentHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BtTreeModel::fermentableHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::FERMENTABLENAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::FERMENTABLECOLORCOL:
      return QVariant(tr("Color"));
   case BtTreeItem::FERMENTABLETYPECOL:
      return QVariant(tr("Type"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BtTreeModel::getFermentableHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BtTreeModel::hopHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::HOPNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::HOPFORMCOL:
      return QVariant(tr("Type"));
   case BtTreeItem::HOPUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BtTreeModel::getHopHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BtTreeModel::miscHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::MISCNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::MISCTYPECOL:
      return QVariant(tr("Type"));
   case BtTreeItem::MISCUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BtTreeModel::getMiscHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BtTreeModel::yeastHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::YEASTNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::YEASTTYPECOL:
      return QVariant(tr("Type"));
   case BtTreeItem::YEASTFORMCOL:
      return QVariant(tr("Form"));
   }

   Brewtarget::logW( QString("BtTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

QVariant BtTreeModel::styleHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::STYLENAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::STYLECATEGORYCOL:
      return QVariant(tr("Category"));
   case BtTreeItem::STYLENUMBERCOL:
      return QVariant(tr("Number"));
   case BtTreeItem::STYLELETTERCOL:
      return QVariant(tr("Letter"));
   case BtTreeItem::STYLEGUIDECOL:
      return QVariant(tr("Guide"));
   }

   Brewtarget::logW( QString("BtTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

QVariant BtTreeModel::folderHeader(int section) const
{
   switch(section) 
   {
      case BtTreeItem::FOLDERNAMECOL:
         return QVariant(tr("Name"));
      case BtTreeItem::FOLDERPATHCOL:
         return QVariant(tr("PATH"));
      case BtTreeItem::FOLDERFULLCOL:
         return QVariant(tr("FULLPATH"));
   }

   Brewtarget::logW( QString("BtTreeModel::getFolderHeader Bad column: %1").arg(section) );
   return QVariant();
}

bool BtTreeModel::insertRow(int row, const QModelIndex &parent, QObject* victim, int victimType )
{
   if ( ! parent.isValid() )
      return false;

   BtTreeItem *pItem = item(parent);
   int type = pItem->type();

   bool success = true;

   beginInsertRows(parent,row,row);
   success = pItem->insertChildren(row,1,type);
   if ( victim && success ) 
   {
      type = victimType == -1 ? type : victimType;
      BtTreeItem* added = pItem->child(row);
      added->setData(type, victim);
   }
   endInsertRows();

   return success;
}

bool BtTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   BtTreeItem *pItem = item(parent);
   bool success = true;
    
   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

// One find method for all things. This .. is nice
QModelIndex BtTreeModel::findElement(BeerXMLElement* thing, BtTreeItem* parent)
{
   BtTreeItem* pItem;
   QModelIndex pIndex;

   int i;

   if ( parent == NULL )
      pItem = rootItem->child(0);
   else
      pItem = parent;

   if (! thing )
      return createIndex(0,0,pItem);

   // Recursion. Wonderful.
   for(i=0; i < pItem->childCount(); ++i)
   {
      // If we've found what we are looking for, return
      if ( pItem->child(i)->thing() == thing )
         return createIndex(i,0,pItem->child(i));

      // If we have a folder, or we are looking for a brewnote and have a
      // recipe in hand, recurse
      if ( pItem->child(i)->type() == BtTreeItem::FOLDER ||
           (qobject_cast<BrewNote*>(thing) && pItem->child(i)->type() == BtTreeItem::RECIPE ) )
      {
         return findElement(thing,pItem->child(i));
      }
   }
   return QModelIndex();
}

void BtTreeModel::addFolder(QString name) 
{
   QModelIndex ndx = findFolder(name, rootItem->child(0), true, "");
   QModelIndex pInd = parent(ndx);
}

// This signature simply creates a BtFolder object and then calls the other
// form for renameFolder
void BtTreeModel::renameFolder(BtFolder* victim, QString newName)
{
   BtFolder* topPath = new BtFolder();
   topPath->setfullPath(newName);
   renameFolder(victim,topPath);
}

void BtTreeModel::renameFolder(BtFolder* victim, BtFolder* topPath)
{
   QModelIndex ndx = findFolder(victim->fullPath(), 0, false);
   BtTreeItem* start = item(ndx);
   int i;

   
   // Ok. We have a start and an index.
   for (i=0; i < start->childCount(); ++i)
   {
      BtTreeItem* next = start->child(i);
      // If a folder, recurse. We need ot change the parent folder a little,
      // just so we know where we are moving the leaf nodes to
      if ( next->type() == BtTreeItem::FOLDER ) 
      {
         BtFolder* newTop = new BtFolder();
         newTop->setfullPath(QString("%1/%2").arg(topPath->fullPath()).arg(victim->name()));
         renameFolder(next->folder(),newTop);
      }
      else // Leafnode
      {
         BeerXMLElement* thg = next->thing();

         QString fPath;
         // An edge case when dropping the folder on the root window
         if ( ! topPath ) 
           fPath = QString("/%1").arg(victim->name());
         else
           fPath = QString("%1/%2").arg(topPath->fullPath()).arg(victim->name());

         thg->setFolder(fPath);

      }
   }
   // Last thing is to remove the folder. It will be fascinating to see how
   // this recurses
   ndx = findFolder(victim->fullPath(), 0, false);
   QModelIndex pInd = parent(ndx);

   i = start->childNumber();
   removeRows(i, 1, pInd); 

}

QModelIndex BtTreeModel::createFolderTree( QStringList dirs, BtTreeItem* parent, QString pPath)
{
   BtTreeItem* pItem = parent;

   // Start the loop. We are going to return ndx at the end, so it needs to
   // be preserved
   QModelIndex ndx = createIndex(pItem->childCount(),0,pItem);

   // Need to call this because we are adding different things with different
   // column counts. Just using the rowsAboutToBeAdded throws ugly errors and
   // then a sigsegv
   emit layoutAboutToBeChanged();
   foreach ( QString cur, dirs )
   {
      QString fPath;
      BtFolder* temp = new BtFolder();
      int i;

      // If the parent item is a folder, use its full path
      if ( pItem->type() == BtTreeItem::FOLDER )
         fPath = pItem->folder()->fullPath() % "/" % cur;
      else
         fPath = pPath % "/" % cur; // If it isn't we need the parent path

      fPath.replace(QRegExp("//"), "/");

      // Set the full path, which will set the name and the path
      temp->setfullPath(fPath);
      i = pItem->childCount();

      // Insert the item into the tree
      insertRow(i, ndx, temp, BtTreeItem::FOLDER);

      // Set the parent item to point to the newly created tree
      pItem = pItem->child(i);

      // And this for the return
      ndx = createIndex(pItem->childCount(),0,pItem);
   }
   emit layoutChanged();

   // May K&R have mercy on my soul
   return ndx;
}

/* This one will be a bit twisted, because of the way I want it to work. name
 * will be a string like "/a/b/c". This method will need to split that into
 * [a,b,c], pop the first element off, "a", and look in the parent for a folder
 * with the same name.
 *
 * if the folder is found
 *   and there are things left in the array, recurse using join("/", [a,b])
 *   and the array is empty, we've found the folder return the QModelIndex
 *
 * if the folder is not found, start creating and inserting them. When done,
 * return the QModelIndex to the lowest item
 *
 * Consider renaming this? findFolder should do just that and no more. 
 * Yeah, I should
 * 
 */
QModelIndex BtTreeModel::findFolder( QString name, BtTreeItem* parent, bool create, QString pPath )
{
   BtTreeItem* pItem;
   QStringList dirs;
   QString current;
   int i;

   pItem = parent ? parent : rootItem->child(0);

   // I am assuming asking me to find an empty name means find the root of the
   // tree.
   if ( name.isEmpty() )
      return createIndex(0,0,pItem);

   dirs = name.split("/", QString::SkipEmptyParts);

   current = dirs.takeFirst();

   for ( i = 0; i < pItem->childCount(); ++i )
   {
      BtTreeItem* temp = pItem->child(i);
      // The parent has a folder

      if ( temp->type() == BtTreeItem::FOLDER )
      {
         QString fullPath = pPath % "/" % current;
         BtFolder* fold = temp->folder();

         fullPath.replace(QRegExp("//"), "/");
         // The folder name matches the part we are looking at
         if ( fold->isFolder(fullPath) )
         {
            if ( dirs.isEmpty() ) 
               return createIndex(i,0,temp);
            else
               return findFolder(dirs.join("/"), temp, create, fullPath);
         }
      }
   }
   // If we get here, we found no match.

   // If we are supposed to create something, then lets get busy
   if ( create )
   {
      // push the current dir back on the stack
      dirs.prepend(current);
      // And start with the madness
      return createFolderTree( dirs, pItem, pPath);
   }

   // If we weren't supposed to create, we drop to here and return an empty
   // index. 
   return QModelIndex();
}

QList<BeerXMLElement*> BtTreeModel::elements()
{
   QList<BeerXMLElement*> elements;
   switch(treeMask)
   {
   case RECIPEMASK:
      foreach( BeerXMLElement* elem, Database::instance().recipes() )
         elements.append(elem);
      break;
   case EQUIPMASK:
      foreach( BeerXMLElement* elem, Database::instance().equipments() )
         elements.append(elem);
      break;
   case FERMENTMASK:
      foreach( BeerXMLElement* elem, Database::instance().fermentables() )
         elements.append(elem);
      break;
   case HOPMASK:
      foreach( BeerXMLElement* elem, Database::instance().hops() )
         elements.append(elem);
      break;
   case MISCMASK:
      foreach( BeerXMLElement* elem, Database::instance().miscs() )
         elements.append(elem);
      break;
   case YEASTMASK:
      foreach( BeerXMLElement* elem, Database::instance().yeasts() )
         elements.append(elem);
      break;
   case STYLEMASK:
      foreach( BeerXMLElement* elem, Database::instance().styles() )
         elements.append(elem);
      break;
   default:
      Brewtarget::logW(QString("Invalid treemask: %1").arg(treeMask));
   }
   return elements;
}

void BtTreeModel::loadTreeModel(QString propName)
{
   int i;

   QModelIndex ndxLocal;
   BtTreeItem* local = rootItem->child(0);
   QList<BeerXMLElement*> elems = elements();

   foreach( BeerXMLElement* elem, elems )
   {

      if (! elem->folder().isEmpty() )
      {
         ndxLocal = findFolder( elem->folder(), rootItem->child(0), true, "/" );
         local = item(ndxLocal);
         i = local->childCount();
      }
      else
      {
         local = rootItem->child(0);
         i = local->childCount();
         ndxLocal = createIndex(i,0,local);
      }

      insertRow(i,ndxLocal,elem,_type);

      // If we have brewnotes, set them up here.
      if ( treeMask & RECIPEMASK )
         addBrewNoteSubTree(qobject_cast<Recipe*>(elem),i,local);
      
      observeElement(elem);
   }
}

void BtTreeModel::addBrewNoteSubTree(Recipe* rec, int i, BtTreeItem* parent)
{
   QList<BrewNote*> notes = rec->brewNotes();
   BtTreeItem* temp = parent->child(i);

   int j = 0;

   foreach( BrewNote* note, notes )
   {
      insertRow(j, createIndex(i,0,temp), note, BtTreeItem::BREWNOTE);
      observeElement(note);
      ++j;
   }
}

/*
   I think this method is no longer used or required. I am disabling for now.
   maf

void BtTreeModel::unloadTreeModel(QString propName)
{
   int breadth;
   QModelIndex parent;

   bool unloadAll = (propName=="");
   
   if ( unloadAll )
   {
      parent = createIndex(0,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
}
*/

Recipe* BtTreeModel::recipe(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->recipe();
}

Equipment* BtTreeModel::equipment(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->equipment();
}

Fermentable* BtTreeModel::fermentable(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->fermentable();
}

Hop* BtTreeModel::hop(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->hop();
}

Misc* BtTreeModel::misc(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->misc();
}

Yeast* BtTreeModel::yeast(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->yeast();
}

Style* BtTreeModel::style(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->style();
}

BrewNote* BtTreeModel::brewNote(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->brewNote();
}

BtFolder* BtTreeModel::folder(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->folder();
}

BeerXMLElement* BtTreeModel::thing(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);

   return _item->thing();
}

bool BtTreeModel::isRecipe(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::RECIPE;
}

bool BtTreeModel::isEquipment(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::EQUIPMENT;
}

bool BtTreeModel::isFermentable(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::FERMENTABLE;
}

bool BtTreeModel::isHop(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::HOP;
}

bool BtTreeModel::isMisc(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::MISC;
}

bool BtTreeModel::isYeast(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::YEAST;
}

bool BtTreeModel::isStyle(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::STYLE;
}

bool BtTreeModel::isBrewNote(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::BREWNOTE;
}

bool BtTreeModel::isFolder(const QModelIndex &index) const
{
   BtTreeItem* _item = item(index);
   return _item->type() == BtTreeItem::FOLDER;
}

int BtTreeModel::type(const QModelIndex &index)
{
   BtTreeItem* _item = item(index);
   return _item->type();
}

QString BtTreeModel::name(const QModelIndex &idx)
{
   BtTreeItem* _item = item(idx);

   return _item->name();

}

int BtTreeModel::mask()
{
   return treeMask;
}

void BtTreeModel::elementChanged()
{
   BeerXMLElement* d = qobject_cast<BeerXMLElement*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findElement(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

// The actual magic shouldn't be hard. Once we trap the signal, find the
// recipe, remove it from the parent and add it to the target folder.
// The uncomfortable part is how to make this more generic? I do not want to
// write this code once for every type. Like I have so many times already.
void BtTreeModel::folderChanged(QString name)
{
   BeerXMLElement* test = qobject_cast<BeerXMLElement*>(sender());
   if ( ! test )
      return;

   // Find it.
   QModelIndex ndx = findElement(test);
   QModelIndex pIndex = parent(ndx); // Get the parent
   int i = item(ndx)->childNumber();

   // Remove it
   removeRows(i, 1, pIndex); 

   // Find the new parent
   ndx = findFolder(test->folder(), rootItem->child(0), true);
   BtTreeItem* local = item(ndx);
   i = local->childCount();

   insertRow(i,ndx,test,_type);

   // If we have brewnotes, set them up here.
   if ( treeMask & RECIPEMASK )
      addBrewNoteSubTree(qobject_cast<Recipe*>(test),i,local);

   return;
}


/* I don't like this part, but Qt's signal/slot mechanism are pretty
 * simplistic and do a string compare on signatures. Each one of these one
 * liners is required to give the right signature and to be able to call
 * addElement() properly
 */
void BtTreeModel::elementAdded(Recipe* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(Equipment* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(Fermentable* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(Hop* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(Misc* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(Style* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(Yeast* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementAdded(BrewNote* victim) { elementAdded(qobject_cast<BeerXMLElement*>(victim)); }

// I guess this isn't too bad. Better than this same function copied 7 times
void BtTreeModel::elementAdded(BeerXMLElement* victim)
{
   QModelIndex pIdx;
   int lType = _type;

   if ( ! victim->display() ) 
      return;

   if ( qobject_cast<BrewNote*>(victim) )
   {
      pIdx = findElement(Database::instance().getParentRecipe(qobject_cast<BrewNote*>(victim)));
      lType = BtTreeItem::BREWNOTE;
   }
   else
      pIdx = createIndex(0,0,rootItem->child(0));

   if ( ! pIdx.isValid() )
      return;

   int breadth = rowCount(pIdx);

   insertRow(breadth,pIdx,victim,lType);
   observeElement(victim);
}

void BtTreeModel::elementRemoved(Recipe* victim)      { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(Equipment* victim)   { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(Fermentable* victim) { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(Hop* victim)         { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(Misc* victim)        { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(Style* victim)       { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(Yeast* victim)       { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }
void BtTreeModel::elementRemoved(BrewNote* victim)    { elementRemoved(qobject_cast<BeerXMLElement*>(victim)); }

void BtTreeModel::elementRemoved(BeerXMLElement* victim)
{
   QModelIndex index = findElement(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void BtTreeModel::observeElement(BeerXMLElement* d)
{
   if ( qobject_cast<BrewNote*>(d) )
      connect( d, SIGNAL(brewDateChanged(QDateTime)), this, SLOT(elementChanged()) );
   else 
   {
      connect( d, SIGNAL(changedName(QString)), this, SLOT(elementChanged()) );
      connect( d, SIGNAL(changedFolder(QString)), this, SLOT(folderChanged(QString)));
   }
}

// DRAG AND DROP STUFF

// As of 01-10-2014, I need to test a grop with both items and folders. I
// suspect it will break in the most horrible fashions.
bool BtTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, 
                               int row, int column, const QModelIndex &parent)
{
   QByteArray encodedData;

   if ( data->hasFormat(_mimeType) )
      encodedData = data->data(_mimeType);
   else if ( data->hasFormat("application/x-brewtarget-folder") )
      encodedData = data->data("application/x-brewtarget-folder");
   else
      return false; // Don't know what we got, but we don't want it


   QDataStream stream( &encodedData, QIODevice::ReadOnly);
   int oType, id;
   QList<int> droppedIds;
   QString target = ""; 
   QString name = "";

   if ( ! parent.isValid() )
      return false;

   if ( isFolder(parent) )
      target = folder(parent)->fullPath();
   else 
   {
      BeerXMLElement* _thing = thing(parent);

      // Did you know there's a space between elements in a tree, and you can
      // actually drop things there? If somebody drops something there, don't
      // do anything
      if ( ! _thing ) 
         return false;

      target = _thing->folder();
   }
 
   // Pull the stream apart and do that which needs done. Late binding ftw!
   while( !stream.atEnd() )
   {
      QString text;
      stream >> oType >> id >> name;
      BeerXMLElement* elem;
      switch(oType) 
      {
         case BtTreeItem::RECIPE:
            elem = Database::instance().recipe(id);
            break;
         case BtTreeItem::EQUIPMENT:
            elem = Database::instance().equipment(id);
            break;
         case BtTreeItem::FERMENTABLE:
            elem = Database::instance().fermentable(id);
            break;
         case BtTreeItem::HOP:
            elem = Database::instance().hop(id);
            break;
         case BtTreeItem::MISC:
            elem = Database::instance().misc(id);
            break;
         case BtTreeItem::STYLE:
            elem = Database::instance().style(id);
            break;
         case BtTreeItem::YEAST:
            elem = Database::instance().yeast(id);
            break;
         case BtTreeItem::FOLDER:
            break;
         default:
            return false;
      }

      if ( oType != BtTreeItem::FOLDER ) 
         elem->setFolder(target);
      else 
      {
         // I need the actual folder object that got dropped.
         BtFolder* victim = new BtFolder;
         victim->setfullPath(name);

         BtFolder* topPath = new BtFolder;
         if (! target.isEmpty() )
            topPath->setfullPath(target);

         renameFolder(victim, topPath);
      }
   }

   return true;
}

QStringList BtTreeModel::mimeTypes() const
{
   QStringList types;
   // accept whatever type we like, and folders
   types << _mimeType << "application/x-brewtarget-folder";

   return types;
}

Qt::DropActions BtTreeModel::supportedDropActions() const
{
   return Qt::CopyAction | Qt::MoveAction;
}

