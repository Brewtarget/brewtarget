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

   if ( type & RECIPEMASK)
   {
      rootItem->insertChildren(items,1,BtTreeItem::RECIPE);
      connect( &(Database::instance()), SIGNAL(newRecipeSignal(Recipe*)),this, SLOT(recipeAdded(Recipe*)));
      connect( &(Database::instance()), SIGNAL(deletedRecipeSignal(Recipe*)),this, SLOT(recipeRemoved(Recipe*)));
      // Brewnotes need love too!
      connect( &(Database::instance()), SIGNAL(newBrewNoteSignal(BrewNote*)),this, SLOT(brewNoteAdded(BrewNote*)));
      connect( &(Database::instance()), SIGNAL(deletedBrewNoteSignal(BrewNote*)),this, SLOT(brewNoteRemoved(BrewNote*)));
   }

   if ( type & EQUIPMASK )
   {
      rootItem->insertChildren(items,1,BtTreeItem::EQUIPMENT);
      connect( &(Database::instance()), SIGNAL(newEquipmentSignal(Equipment*)),this, SLOT(equipmentAdded(Equipment*)));
      connect( &(Database::instance()), SIGNAL(deletedEquipmentSignal(Equipment*)),this, SLOT(equipmentRemoved(Equipment*)));
   }

   if ( type & FERMENTMASK )
   {
      rootItem->insertChildren(items,1,BtTreeItem::FERMENTABLE);
      connect( &(Database::instance()), SIGNAL(newFermentableSignal(Fermentable*)),this, SLOT(fermentableAdded(Fermentable*)));
      connect( &(Database::instance()), SIGNAL(deletedFermentableSignal(Fermentable*)),this, SLOT(fermentableRemoved(Fermentable*)));
   }

   if ( type & HOPMASK )
   {
      rootItem->insertChildren(items,1,BtTreeItem::HOP);
      connect( &(Database::instance()), SIGNAL(newHopSignal(Hop*)),this, SLOT(hopAdded(Hop*)));
      connect( &(Database::instance()), SIGNAL(deletedHopSignal(Hop*)),this, SLOT(hopRemoved(Hop*)));
   }

   if ( type & MISCMASK )
   {
      rootItem->insertChildren(items,1,BtTreeItem::MISC);
      connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)),this, SLOT(miscAdded(Misc*)));
      connect( &(Database::instance()), SIGNAL(deletedMiscSignal(Misc*)),this, SLOT(miscRemoved(Misc*)));
   }

   if ( type & YEASTMASK )
   {
      rootItem->insertChildren(items,1,BtTreeItem::YEAST);

      connect( &(Database::instance()), SIGNAL(newYeastSignal(Yeast*)),this, SLOT(yeastAdded(Yeast*)));
      connect( &(Database::instance()), SIGNAL(deletedYeastSignal(Yeast*)),this, SLOT(yeastRemoved(Yeast*)));
   }

   if ( type & STYLEMASK )
   {
      rootItem->insertChildren(items,1,BtTreeItem::STYLE);

      connect( &(Database::instance()), SIGNAL(newStyleSignal(Style*)),this, SLOT(styleAdded(Style*)));
      connect( &(Database::instance()), SIGNAL(deletedStyleSignal(Style*)),this, SLOT(styleRemoved(Style*)));
   }

   treeMask = type;
   parentTree = parent;
   loadTreeModel();
}

BtTreeModel::~BtTreeModel()
{
   delete rootItem;
}

BtTreeItem *BtTreeModel::getItem( const QModelIndex &index ) const
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
   
   BtTreeItem *pItem = getItem(parent);

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

   pItem = getItem(parent);
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

   cItem = getItem(index);

   if ( cItem == 0 )
      return QModelIndex();

   pItem = cItem->parent();

   if (pItem == rootItem || pItem == 0 )
      return QModelIndex();

   return createIndex(pItem->childNumber(),0,pItem);
}

QModelIndex BtTreeModel::getFirst()
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

   BtTreeItem *item = getItem(index);
   if ( role == Qt::DecorationRole && index.column() == 0) 
   {
      if ( item->type() == BtTreeItem::FOLDER )
         return QIcon(":images/folder.png");
      else
         return QVariant();
   }

   return item->getData(index.column());
}

// This is much better, assuming the rest can be made to work
QVariant BtTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
      return QVariant();

   switch(treeMask)
   {
   case RECIPEMASK:
      return getRecipeHeader(section);
   case EQUIPMASK:
      return getEquipmentHeader(section);
   case FERMENTMASK:
      return getFermentableHeader(section);
   case HOPMASK:
      return getHopHeader(section);
   case MISCMASK:
      return getMiscHeader(section);
   case YEASTMASK:
      return getYeastHeader(section);
   case STYLEMASK:
      return getStyleHeader(section);
   case FOLDERMASK:
      return getFolderHeader(section);
   default: 
      return QVariant();
   }
}

QVariant BtTreeModel::getRecipeHeader(int section) const
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

QVariant BtTreeModel::getEquipmentHeader(int section) const
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

QVariant BtTreeModel::getFermentableHeader(int section) const
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

QVariant BtTreeModel::getHopHeader(int section) const
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

QVariant BtTreeModel::getMiscHeader(int section) const
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

QVariant BtTreeModel::getYeastHeader(int section) const
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

QVariant BtTreeModel::getStyleHeader(int section) const
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

QVariant BtTreeModel::getFolderHeader(int section) const
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

   BtTreeItem *pItem = getItem(parent);
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
   BtTreeItem *pItem = getItem(parent);
   bool success = true;
    
   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

/* All of the find methods are going to require rework so they go down folders
 * properly
 */
QModelIndex BtTreeModel::findRecipe(Recipe* rec, BtTreeItem* parent)
{
   BtTreeItem* pItem;
   QModelIndex pIndex;

   int i;

   if ( parent == NULL )
      pItem = rootItem->child(0);
   else
      pItem = parent;

   if (! rec || (treeMask & (RECIPEMASK|FOLDERMASK) ) == 0)
      return createIndex(0,0,pItem);

   // Recursion. Wonderful.
   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getRecipe() == rec )
         return createIndex(i,0,pItem->child(i));

      if ( pItem->child(i)->type() == BtTreeItem::FOLDER )
      {
         pIndex = findRecipe(rec,pItem->child(i));
         if ( pIndex.isValid() )
            return pIndex;
      }

   }
   return QModelIndex();
}

QModelIndex BtTreeModel::findEquipment(Equipment* kit)
{
   BtTreeItem* pItem = rootItem->child(0);
   int i;

   if (! kit || (treeMask & EQUIPMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getEquipment() == kit )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BtTreeModel::findFermentable(Fermentable* ferm)
{
   BtTreeItem* pItem = rootItem->child(0);
   int i;

   if (! ferm || (treeMask & FERMENTMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getFermentable() == ferm )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BtTreeModel::findHop(Hop* hop)
{
   BtTreeItem* pItem = rootItem->child(0);
   int i;

   if (! hop || (treeMask & HOPMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getHop() == hop )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BtTreeModel::findMisc(Misc* misc)
{
   BtTreeItem* pItem = rootItem->child(0);
   int i;

   if (! misc || (treeMask & MISCMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getMisc() == misc )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BtTreeModel::findYeast(Yeast* yeast)
{
   BtTreeItem* pItem = rootItem->child(0);
   int i;

   if (! yeast || (treeMask & YEASTMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getYeast() == yeast )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BtTreeModel::findStyle(Style* style)
{
   BtTreeItem* pItem = rootItem->child(0);
   int i;

   if (! style || (treeMask & STYLEMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getStyle() == style )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

/* Important lesson here.  When building the index, the pointer needs to be to
 * the child's parent item, as understood by the model.  Not the pointer to
 * the actual object (e.g., the BrewNote) or the recipe, or the brewnote's
 * place in the recipe.
 */
QModelIndex BtTreeModel::findBrewNote(BrewNote* bNote)
{
   if (! bNote )
      return QModelIndex();

   // Get the brewnote's parent
   Recipe *parent = Database::instance().getParentRecipe(bNote);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   BtTreeItem* pItem = getItem(pInd);


   QList<BrewNote*> notes = parent->brewNotes();
   int i = notes.indexOf(bNote);
   if( i > 0 )
      return createIndex(i,0,pItem->child(i));
   else
      return QModelIndex();
}

void BtTreeModel::addFolder(QString name) 
{
   // emit layoutAboutToBeChanged();

   QModelIndex ndx = findFolder(name, rootItem->child(0), true, "");
   QModelIndex pInd = parent(ndx);

   // emit layoutChanged();
}

void BtTreeModel::renameFolder(BtFolder* victim, QString newName, QString oldPath)
{
   QModelIndex ndx = findFolder(victim->fullPath(), 0, false);
   BtTreeItem* start = getItem(ndx);
   QString replacePath = oldPath.isEmpty() ? victim->fullPath() : oldPath;
   int i;

   // Ok. We have a start and an index.
   for (i=0; i < start->childCount(); ++i)
   {
      BtTreeItem* next = start->child(i);
      // If a folder, recurse
      if ( next->type() == BtTreeItem::FOLDER ) 
         renameFolder(next->getFolder(),newName,replacePath);
      else // Leafnode
      {
         layoutAboutToBeChanged();

         QString fPath = next->getThing()->folder();
   
         fPath.replace(QRegExp(replacePath),newName);
         next->getThing()->setFolder(fPath);

         // Remove it
         removeRows(i, 1, ndx); 

         // Find the new parent
         QModelIndex indx = findFolder(fPath, rootItem->child(0), true);
         insertRow(i,indx,next->getThing(),next->type());

         layoutChanged();
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
         fPath = pItem->getFolder()->fullPath() % "/" % cur;
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
 */
QModelIndex BtTreeModel::findFolder( QString name, BtTreeItem* parent, bool create, QString pPath )
{
   BtTreeItem* pItem;
   QStringList dirs;
   QString current;
   int i;

   pItem = parent ? parent : rootItem->child(0);

   dirs = name.split("/", QString::SkipEmptyParts);

   current = dirs.takeFirst();

   for ( i = 0; i < pItem->childCount(); ++i )
   {
      BtTreeItem* temp = pItem->child(i);
      // The parent has a folder

      if ( temp->type() == BtTreeItem::FOLDER )
      {
         QString fullPath = pPath % "/" % current;
         BtFolder* fold = temp->getFolder();

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

void BtTreeModel::loadTreeModel(QString propName)
{
   int i;

   bool loadAll = (propName == "");
  
   if ( (treeMask & RECIPEMASK ) && 
        (loadAll || propName == "recipes" ) )
   {  
      QModelIndex ndxLocal;
      BtTreeItem* local = rootItem->child(0);
      QList<Recipe*> recipes = Database::instance().recipes();

      foreach( Recipe* rec, recipes )
      {
         if (! rec->folder().isEmpty() )
         {
            ndxLocal = findFolder( rec->folder(), rootItem->child(0), true, "/" );
            local = getItem(ndxLocal);
            i = local->childCount();
         }
         else
         {
            local = rootItem->child(0);
            i = local->childCount();
            ndxLocal = createIndex(i,0,local);
         }

         insertRow(i,ndxLocal,rec,BtTreeItem::RECIPE);

         // If we have brewnotes, set them up here.
         addBrewNoteSubTree(rec,i,local);
         
         observeRecipe(rec);
      }
   }

   if ( (treeMask & EQUIPMASK) &&
      (loadAll || propName == "equipments") )
   {
      BtTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Equipment*> equipments = Database::instance().equipments();
      
      i = 0;
      foreach( Equipment* equipment, equipments )
      {
         insertRow(i, ndxLocal, equipment);
         observeEquipment(equipment);
         ++i;
      }
   }

   if ( (treeMask & FERMENTMASK) &&
      (loadAll || propName == "fermentables") )
   {
      BtTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Fermentable*> ferms = Database::instance().fermentables();

      i = 0;
      foreach( Fermentable* ferm, ferms )
      {
         insertRow(i,ndxLocal,ferm);
         observeFermentable( ferm );
         ++i;
      }
   }

   if ( (treeMask & HOPMASK) &&
      (loadAll || propName == "hops") )
   {
      BtTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Hop*> hops = Database::instance().hops();

      i = 0;
      foreach( Hop* hop, hops )
      {
         insertRow(i,ndxLocal, hop);
         observeHop(hop);
         ++i;
      }
   }

   if ( (treeMask & MISCMASK) &&
      (loadAll || propName == "miscs") )
   {
      BtTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Misc*> miscs = Database::instance().miscs();

      i = 0;
      foreach( Misc* misc, miscs )
      {
         insertRow(i,ndxLocal,misc);
         observeMisc(misc);
         ++i;
      }
   }

   if ( (treeMask & YEASTMASK) &&
      (loadAll || propName == "yeasts") )
   {
      BtTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Yeast*> yeasts = Database::instance().yeasts();

      i = 0;
      foreach( Yeast* yeast, yeasts )
      {
        insertRow(i,ndxLocal,yeast);
        observeYeast(yeast);
        ++i;
      }
   }

   if ( (treeMask & STYLEMASK) &&
      (loadAll || propName == "styles") )
   {
      BtTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Style*> styles = Database::instance().styles();

      i = 0;
      foreach( Style* style, styles )
      {
        insertRow(i++,ndxLocal,style);
        observeStyle(style);
      }
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
      observeBrewNote(note);
      ++j;
   }
}

void BtTreeModel::unloadTreeModel(QString propName)
{
   int breadth;
   QModelIndex parent;

   bool unloadAll = (propName=="");
   
   if ( (treeMask & RECIPEMASK) &&
        (unloadAll || propName == "recipes"))
   {
      parent = createIndex(BtTreeItem::RECIPE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & EQUIPMASK) &&
      (unloadAll || propName == "equipments"))
   {
      parent = createIndex(BtTreeItem::EQUIPMENT,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & FERMENTMASK) &&
      (unloadAll || propName == "fermentables"))
   {
      parent = createIndex(BtTreeItem::FERMENTABLE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & HOPMASK) &&
      (unloadAll || propName == "hops"))
   {
      parent = createIndex(BtTreeItem::HOP,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & MISCMASK) &&
      (unloadAll || propName == "miscs"))
   {
      parent = createIndex(BtTreeItem::MISC,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & YEASTMASK) &&
      (unloadAll || propName == "yeasts"))
   {
      parent = createIndex(BtTreeItem::YEAST,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & STYLEMASK) &&
      (unloadAll || propName == "styles"))
   {
      parent = createIndex(BtTreeItem::STYLE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
}

Recipe* BtTreeModel::getRecipe(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getRecipe();
}

Equipment* BtTreeModel::getEquipment(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getEquipment();
}

Fermentable* BtTreeModel::getFermentable(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getFermentable();
}

Hop* BtTreeModel::getHop(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getHop();
}

Misc* BtTreeModel::getMisc(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getMisc();
}

Yeast* BtTreeModel::getYeast(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getYeast();
}

Style* BtTreeModel::getStyle(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getStyle();
}

BrewNote* BtTreeModel::getBrewNote(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getBrewNote();
}

BtFolder* BtTreeModel::getFolder(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getFolder();
}

BeerXMLElement* BtTreeModel::getThing(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);

   return item->getThing();
}

bool BtTreeModel::isRecipe(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::RECIPE;
}

bool BtTreeModel::isEquipment(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::EQUIPMENT;
}

bool BtTreeModel::isFermentable(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::FERMENTABLE;
}

bool BtTreeModel::isHop(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::HOP;
}

bool BtTreeModel::isMisc(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::MISC;
}

bool BtTreeModel::isYeast(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::YEAST;
}

bool BtTreeModel::isStyle(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::STYLE;
}

bool BtTreeModel::isBrewNote(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::BREWNOTE;
}

bool BtTreeModel::isFolder(const QModelIndex &index) const
{
   BtTreeItem* item = getItem(index);
   return item->type() == BtTreeItem::FOLDER;
}

int BtTreeModel::type(const QModelIndex &index)
{
   BtTreeItem* item = getItem(index);
   return item->type();
}

QString BtTreeModel::name(const QModelIndex &idx)
{
   BtTreeItem* item = getItem(idx);

   return item->name();

}

int BtTreeModel::getMask()
{
   return treeMask;
}

void BtTreeModel::equipmentChanged()
{
   Equipment* d = qobject_cast<Equipment*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findEquipment(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::fermentableChanged()
{
   Fermentable* d = qobject_cast<Fermentable*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findFermentable(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::hopChanged()
{
   Hop* d = qobject_cast<Hop*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findHop(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::miscChanged()
{
   Misc* d = qobject_cast<Misc*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findMisc(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::recipeChanged()
{
   Recipe* d = qobject_cast<Recipe*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findRecipe(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::yeastChanged()
{
   Yeast* d = qobject_cast<Yeast*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findYeast(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::styleChanged()
{
   Style* d = qobject_cast<Style*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findStyle(d);
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
   Recipe* test = qobject_cast<Recipe*>(sender());
   if ( ! test )
      return;

   // Find it.
   QModelIndex ndx = findRecipe(test);
   QModelIndex pIndex = parent(ndx); // Get the parent
   int i = getItem(ndx)->childNumber();

   // Remove it
   removeRows(i, 1, pIndex); 

   // Find the new parent
   ndx = findFolder(test->folder(), rootItem->child(0), false);
   BtTreeItem* local = getItem(ndx);
   i = local->childCount();

   insertRow(i,ndx,test,BtTreeItem::RECIPE);

   // If we have brewnotes, set them up here.
   addBrewNoteSubTree(test,i,local);

   return;
}

void BtTreeModel::brewNoteChanged()
{
   BrewNote* d = qobject_cast<BrewNote*>(sender());
   if( !d )
      return;
   
   QModelIndex ndxLeft = findBrewNote(d);
   if( ! ndxLeft.isValid() )
      return;
   
   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

void BtTreeModel::equipmentAdded(Equipment* victim)
{
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeEquipment(victim);
}

void BtTreeModel::equipmentRemoved(Equipment* victim)
{
   QModelIndex index = findEquipment(victim);
   QModelIndex pIndex = parent(index);

   if ( index.isValid() )
   {
      removeRows(index.row(), 1, pIndex );
      disconnect( victim, 0, this, 0 );
   }
}

void BtTreeModel::fermentableAdded(Fermentable* victim)
{
   // This is an import edge case. Things are being added to the db that are
   // marked not display. Don't do anything if they are not display
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeFermentable(victim);
}

void BtTreeModel::fermentableRemoved(Fermentable* victim)
{
   QModelIndex index = findFermentable(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void BtTreeModel::hopAdded(Hop* victim)
{
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeHop(victim);
}

void BtTreeModel::hopRemoved(Hop* victim)
{
   QModelIndex index = findHop(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void BtTreeModel::miscAdded(Misc* victim)
{
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeMisc(victim);
}

void BtTreeModel::miscRemoved(Misc* victim)
{
   QModelIndex index = findMisc(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

// I can no longer assume the parent is at 0,0. 
void BtTreeModel::recipeAdded(Recipe* victim)
{
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeRecipe(victim);
}

void BtTreeModel::recipeRemoved(Recipe* victim)
{
   QModelIndex index = findRecipe(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void BtTreeModel::yeastAdded(Yeast* victim)
{
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeYeast(victim);
}

void BtTreeModel::yeastRemoved(Yeast* victim)
{
   QModelIndex index = findYeast(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void BtTreeModel::styleAdded(Style* victim)
{
   if ( ! victim->display() ) 
      return;

   BtTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeStyle(victim);
}

void BtTreeModel::styleRemoved(Style* victim)
{
   QModelIndex index = findStyle(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

// BrewNotes get no respect, but they get signals. They also get mighty
// confusing
void BtTreeModel::brewNoteAdded(BrewNote* victim)
{
   // Get the brewnote's parent. We can't find it in the tree, because we
   // haven't inserted it yet.
   Recipe *parent = Database::instance().getParentRecipe(victim);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   BtTreeItem* pItem = getItem(pInd);

   int breadth = pItem->childCount();
   insertRow(breadth,pInd,victim,BtTreeItem::BREWNOTE);
   observeBrewNote(victim);
}

// deleting them is worse. Unfortunately, the blasted brewnote is deleted by
// the time we get this signal. So we have to rebuild the entire list.
void BtTreeModel::brewNoteRemoved(BrewNote* victim)
{
   // Get the brewnote's parent recipe
   Recipe *parent = Database::instance().getParentRecipe(victim);
   // Find that recipe's index in the tree
   QModelIndex parentInd = findRecipe(parent);
   // and get the treeItem
   BtTreeItem* parentItem = getItem(parentInd);

   disconnect( victim, 0, this, 0 );
   
   // If the tree item has children -- brewnotes -- remove them all
   if ( parentItem->childCount() )
      removeRows(0,parentItem->childCount(),parentInd);
   
   QList<BrewNote*> brewNotes = parent->brewNotes();
   for (int j=0; j < brewNotes.size(); ++j)
      insertRow(j,parentInd,brewNotes[j],BtTreeItem::BREWNOTE);
}

void BtTreeModel::observeEquipment(Equipment* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(equipmentChanged()) );
}

void BtTreeModel::observeFermentable(Fermentable* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(fermentableChanged()) );
}

void BtTreeModel::observeHop(Hop* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(hopChanged()) );
}

void BtTreeModel::observeMisc(Misc* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(miscChanged()) );
}

void BtTreeModel::observeRecipe(Recipe* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(recipeChanged()) );
   connect( d, SIGNAL(changedFolder(QString)), this, SLOT(folderChanged(QString)));
}

void BtTreeModel::observeYeast(Yeast* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(yeastChanged()) );
}

void BtTreeModel::observeStyle(Style* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(styleChanged()) );
}

void BtTreeModel::observeBrewNote(BrewNote* d)
{
   connect( d, SIGNAL(brewDateChanged(QDateTime)), this, SLOT(brewNoteChanged()) );
}  


// DRAG AND DROP STUFF

bool BtTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
   QByteArray encodedData = data->data("application/x-brewtarget-recipe");
   QDataStream stream( &encodedData, QIODevice::ReadOnly);
   int _type, id;
   QList<int> droppedIds;
   QString target = ""; 
    
   if ( isFolder(parent) )
   {
      target = getFolder(parent)->fullPath();
   }
   else 
   {
      BeerXMLElement* thing = getThing(parent);
      target = thing->folder();
      if ( target.size() == 0 )
         return false;
   }
 
   // If we couldn't figure out where we dropped it, bug out 
   if ( target.size() == 0 )
      return false;
      
   // pull the ids from the stream
   while( !stream.atEnd() )
   {
      QString text;
      stream >> _type >> id;
      Recipe* rec = Database::instance().recipe(id);
      rec->setFolder(target);
   }

   return true;

QStringList BtTreeModel::mimeTypes() const
{
   QStringList types;
   types << "application/x-brewtarget-recipe";

   return types;
}

Qt::DropActions BtTreeModel::supportedDropActions() const
{
   return Qt::CopyAction | Qt::MoveAction;
}
