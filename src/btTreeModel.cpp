/*
 * btTreeModel.cpp is part of Brewtarget and was written by Mik
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

#include "brewtarget.h"
#include "btTreeItem.h"
#include "btTreeModel.h"
#include "btTreeView.h"
#include "database.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "recipe.h"
#include "yeast.h"
#include "brewnote.h"
#include "style.h"

btTreeModel::btTreeModel(btTreeView *parent, TypeMasks type)
   : QAbstractItemModel(parent)
{
   // Initialize the tree structure
   int items = 0;

   rootItem = new btTreeItem();

   if ( type & RECIPEMASK)
   {
      rootItem->insertChildren(items,1,btTreeItem::RECIPE);
      connect( &(Database::instance()), SIGNAL(newRecipeSignal(Recipe*)),this, SLOT(recipeAdded(Recipe*)));
      connect( &(Database::instance()), SIGNAL(deletedRecipeSignal(Recipe*)),this, SLOT(recipeRemoved(Recipe*)));
      // Brewnotes need love too!
      connect( &(Database::instance()), SIGNAL(newBrewNoteSignal(BrewNote*)),this, SLOT(brewNoteAdded(BrewNote*)));
      connect( &(Database::instance()), SIGNAL(deletedBrewNoteSignal(BrewNote*)),this, SLOT(brewNoteRemoved(BrewNote*)));
   }

   if ( type & EQUIPMASK )
   {
      rootItem->insertChildren(items,1,btTreeItem::EQUIPMENT);
      connect( &(Database::instance()), SIGNAL(newEquipmentSignal(Equipment*)),this, SLOT(equipmentAdded(Equipment*)));
      connect( &(Database::instance()), SIGNAL(deletedEquipmentSignal(Equipment*)),this, SLOT(equipmentRemoved(Equipment*)));
   }

   if ( type & FERMENTMASK )
   {
      rootItem->insertChildren(items,1,btTreeItem::FERMENTABLE);
      connect( &(Database::instance()), SIGNAL(newFermentableSignal(Fermentable*)),this, SLOT(fermentableAdded(Fermentable*)));
      connect( &(Database::instance()), SIGNAL(deletedFermentableSignal(Fermentable*)),this, SLOT(fermentableRemoved(Fermentable*)));
   }

   if ( type & HOPMASK )
   {
      rootItem->insertChildren(items,1,btTreeItem::HOP);
      connect( &(Database::instance()), SIGNAL(newHopSignal(Hop*)),this, SLOT(hopAdded(Hop*)));
      connect( &(Database::instance()), SIGNAL(deletedHopSignal(Hop*)),this, SLOT(hopRemoved(Hop*)));
   }

   if ( type & MISCMASK )
   {
      rootItem->insertChildren(items,1,btTreeItem::MISC);
      connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)),this, SLOT(miscAdded(Misc*)));
      connect( &(Database::instance()), SIGNAL(deletedMiscSignal(Misc*)),this, SLOT(miscRemoved(Misc*)));
   }

   if ( type & YEASTMASK )
   {
      rootItem->insertChildren(items,1,btTreeItem::YEAST);

      connect( &(Database::instance()), SIGNAL(newYeastSignal(Yeast*)),this, SLOT(yeastAdded(Yeast*)));
      connect( &(Database::instance()), SIGNAL(deletedYeastSignal(Yeast*)),this, SLOT(yeastRemoved(Yeast*)));
   }

   if ( type & STYLEMASK )
   {
      rootItem->insertChildren(items,1,btTreeItem::STYLE);

      connect( &(Database::instance()), SIGNAL(newStyleSignal(Style*)),this, SLOT(styleAdded(Style*)));
      connect( &(Database::instance()), SIGNAL(deletedStyleSignal(Style*)),this, SLOT(styleRemoved(Style*)));
   }

   treeMask = type;
   parentTree = parent;
   loadTreeModel();
}

btTreeModel::~btTreeModel()
{
   delete rootItem;
}

btTreeItem *btTreeModel::getItem( const QModelIndex &index ) const
{
   if ( index.isValid())
   {
      btTreeItem *item = static_cast<btTreeItem*>(index.internalPointer());
      if (item)
         return item;
   }

   return rootItem;
}

int btTreeModel::rowCount(const QModelIndex &parent) const
{
   if (! parent.isValid())
      return rootItem->childCount();
   
   btTreeItem *pItem = getItem(parent);

   return pItem->childCount();
}

int btTreeModel::columnCount( const QModelIndex &parent) const
{
   switch(treeMask)
   {
   case RECIPEMASK:
      return btTreeItem::RECIPENUMCOLS;
   case EQUIPMASK:
      return btTreeItem::EQUIPMENTNUMCOLS;
   case FERMENTMASK:
      return btTreeItem::FERMENTABLENUMCOLS;
   case HOPMASK:
      return btTreeItem::HOPNUMCOLS;
   case MISCMASK:
      return btTreeItem::MISCNUMCOLS;
   case YEASTMASK:
      return btTreeItem::YEASTNUMCOLS;
   case STYLEMASK:
      return btTreeItem::STYLENUMCOLS;
   default:
      return 0;
   }
   // Backwards compatibility. This MUST be fixed before the code goes live.
   return btTreeItem::RECIPENUMCOLS;
}

Qt::ItemFlags btTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return 0;

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex btTreeModel::index( int row, int column, const QModelIndex &parent) const
{
   btTreeItem *pItem, *cItem;

   if ( parent.isValid() && parent.column() != 0)
      return QModelIndex();

   pItem = getItem(parent);
   cItem = pItem->child(row);

   if (cItem)
      return createIndex(row,column,cItem);
   else
      return QModelIndex();
}

QModelIndex btTreeModel::parent(const QModelIndex &index) const
{
   btTreeItem *pItem, *cItem;

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

QModelIndex btTreeModel::getFirst()
{
   QModelIndex parent;
   btTreeItem* pItem; 

   // get the first item in the list, which is the place holder
   pItem = rootItem->child(0);
   if ( pItem->childCount() > 0 )
      return createIndex(0,0,pItem->child(0));

   return QModelIndex();
}

QVariant btTreeModel::data(const QModelIndex &index, int role) const
{
   int maxColumns;

   switch(treeMask)
   {
   case RECIPEMASK:
      maxColumns = btTreeItem::RECIPENUMCOLS;
      break;
   case EQUIPMASK:
      maxColumns = btTreeItem::EQUIPMENTNUMCOLS;
      break;
   case FERMENTMASK:
      maxColumns = btTreeItem::FERMENTABLENUMCOLS;
      break;
   case HOPMASK:
      maxColumns = btTreeItem::HOPNUMCOLS;
      break;
   case MISCMASK:
      maxColumns = btTreeItem::MISCNUMCOLS;
      break;
   case YEASTMASK:
      maxColumns = btTreeItem::YEASTNUMCOLS;
      break;
   case STYLEMASK:
      maxColumns = btTreeItem::STYLENUMCOLS;
      break;
   case FOLDERMASK:
      maxColumns = btTreeItem::FOLDERNUMCOLS;
      break;
   default:
      // Backwards compatibility. This MUST be fixed prior to releasing the code
      maxColumns = btTreeItem::RECIPENUMCOLS;
   }

   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns)
      return QVariant();

   if ( role != Qt::DisplayRole && role != Qt::EditRole)
      return QVariant();

   btTreeItem *item = getItem(index);
   return item->getData(index.column());
}

// This is much better, assuming the rest can be made to work
QVariant btTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant btTreeModel::getRecipeHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::RECIPENAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::RECIPEBREWDATECOL:
      return QVariant(tr("Brew Date"));
   case btTreeItem::RECIPESTYLECOL:
      return QVariant(tr("Style"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("btTreeModel::getRecipeHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant btTreeModel::getEquipmentHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::EQUIPMENTNAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::EQUIPMENTBOILTIMECOL:
      return QVariant(tr("Boil Time"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("btTreeModel::getEquipmentHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant btTreeModel::getFermentableHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::FERMENTABLENAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::FERMENTABLECOLORCOL:
      return QVariant(tr("Color"));
   case btTreeItem::FERMENTABLETYPECOL:
      return QVariant(tr("Type"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("btTreeModel::getFermentableHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant btTreeModel::getHopHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::HOPNAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::HOPFORMCOL:
      return QVariant(tr("Type"));
   case btTreeItem::HOPUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("btTreeModel::getHopHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant btTreeModel::getMiscHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::MISCNAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::MISCTYPECOL:
      return QVariant(tr("Type"));
   case btTreeItem::MISCUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("btTreeModel::getMiscHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant btTreeModel::getYeastHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::YEASTNAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::YEASTTYPECOL:
      return QVariant(tr("Type"));
   case btTreeItem::YEASTFORMCOL:
      return QVariant(tr("Form"));
   }

   Brewtarget::logW( QString("btTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

QVariant btTreeModel::getStyleHeader(int section) const
{
   switch(section)
   {
   case btTreeItem::STYLENAMECOL:
      return QVariant(tr("Name"));
   case btTreeItem::STYLECATEGORYCOL:
      return QVariant(tr("Category"));
   case btTreeItem::STYLENUMBERCOL:
      return QVariant(tr("Number"));
   case btTreeItem::STYLELETTERCOL:
      return QVariant(tr("Letter"));
   case btTreeItem::STYLEGUIDECOL:
      return QVariant(tr("Guide"));
   }

   Brewtarget::logW( QString("btTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

QVariant btTreeModel::getFolderHeader(int section) const
{
   switch(section) 
   {
      case btTreeItem::FOLDERNAMECOL:
         return QVariant(tr("Name"));
      case btTreeItem::FOLDERPATHCOL:
         return QVariant(tr("PATH"));
      case btTreeItem::FOLDERFULLCOL:
         return QVariant(tr("FULLPATH"));
   }

   Brewtarget::logW( QString("btTreeModel::getFolderHeader Bad column: %1").arg(section) );
   return QVariant();
}

bool btTreeModel::insertRow(int row, const QModelIndex &parent, QObject* victim, int victimType )
{
   if ( ! parent.isValid() )
      return false;

   btTreeItem *pItem = getItem(parent);
   int type = pItem->getType();

   bool success = true;

   beginInsertRows(parent, row, row );
   success = pItem->insertChildren(row, 1, type);
   if ( victim && success ) 
   {
      type = victimType == -1 ? type : victimType;
      btTreeItem* added = pItem->child(row);
      added->setData(type, victim);
   }
   endInsertRows();

   return success;
}

bool btTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   btTreeItem *pItem = getItem(parent);
   bool success = true;
    
   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

/* All of the find methods are going to require rework so they go down folders
 * properly
 */
QModelIndex btTreeModel::findRecipe(Recipe* rec, btTreeItem* parent)
{
   btTreeItem* pItem;
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

      if ( pItem->child(i)->getType() == btTreeItem::FOLDER )
      {
         pIndex = findRecipe(rec,pItem->child(i));
         if ( pIndex.isValid() )
            return pIndex;
      }

   }
   return QModelIndex();
}

QModelIndex btTreeModel::findEquipment(Equipment* kit)
{
   btTreeItem* pItem = rootItem->child(0);
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

QModelIndex btTreeModel::findFermentable(Fermentable* ferm)
{
   btTreeItem* pItem = rootItem->child(0);
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

QModelIndex btTreeModel::findHop(Hop* hop)
{
   btTreeItem* pItem = rootItem->child(0);
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

QModelIndex btTreeModel::findMisc(Misc* misc)
{
   btTreeItem* pItem = rootItem->child(0);
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

QModelIndex btTreeModel::findYeast(Yeast* yeast)
{
   btTreeItem* pItem = rootItem->child(0);
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

QModelIndex btTreeModel::findStyle(Style* style)
{
   btTreeItem* pItem = rootItem->child(0);
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
QModelIndex btTreeModel::findBrewNote(BrewNote* bNote)
{
   if (! bNote )
      return QModelIndex();

   // Get the brewnote's parent
   Recipe *parent = Database::instance().getParentRecipe(bNote);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   btTreeItem* pItem = getItem(pInd);


   QList<BrewNote*> notes = parent->brewNotes();
   int i = notes.indexOf(bNote);
   if( i > 0 )
      return createIndex(i,0,pItem->child(i));
   else
      return QModelIndex();
}

void btTreeModel::loadTreeModel(QString propName)
{
   int i,j;
   btTreeItem* temp;

   bool loadAll = (propName == "");
  
   if ( (treeMask & RECIPEMASK ) && 
        (loadAll || propName == "recipes" ) )
   {  
      btTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);

      // test code. Create a folder and try to shove everything underneath it
      btFolder* toplevel = new btFolder;

      toplevel->setName("topLevel");
      toplevel->setPath("/");
      toplevel->setfullPath("/toplevel");

      // Insert the folder into the tree.
      i = 0;
      insertRow(i,ndxLocal,toplevel, btTreeItem::FOLDER);
      temp = local->child(i);
      ndxLocal = createIndex(i,0,temp);
      local = temp;

      QList<Recipe*> recipes = Database::instance().recipes();
      
      foreach( Recipe* rec, recipes )
      {
         insertRow(i,ndxLocal,rec,btTreeItem::RECIPE);
         temp = local->child(i);

         // If we have brewnotes, set them up here.
         QList<BrewNote*> notes = rec->brewNotes();
         j = 0;
         foreach( BrewNote* note, notes )
         {
            insertRow(j, createIndex(i,0,temp), note, btTreeItem::BREWNOTE);
            observeBrewNote(note);
            ++j;
         }
         
         observeRecipe(rec);
         ++i;
      }
   }

   if ( (treeMask & EQUIPMASK) &&
      (loadAll || propName == "equipments") )
   {
      btTreeItem* local = rootItem->child(0);
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
      btTreeItem* local = rootItem->child(0);
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
      btTreeItem* local = rootItem->child(0);
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
      btTreeItem* local = rootItem->child(0);
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
      btTreeItem* local = rootItem->child(0);
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
      btTreeItem* local = rootItem->child(0);
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

void btTreeModel::unloadTreeModel(QString propName)
{
   int breadth;
   QModelIndex parent;

   bool unloadAll = (propName=="");
   
   if ( (treeMask & RECIPEMASK) &&
        (unloadAll || propName == "recipes"))
   {
      parent = createIndex(btTreeItem::RECIPE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & EQUIPMASK) &&
      (unloadAll || propName == "equipments"))
   {
      parent = createIndex(btTreeItem::EQUIPMENT,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & FERMENTMASK) &&
      (unloadAll || propName == "fermentables"))
   {
      parent = createIndex(btTreeItem::FERMENTABLE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & HOPMASK) &&
      (unloadAll || propName == "hops"))
   {
      parent = createIndex(btTreeItem::HOP,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & MISCMASK) &&
      (unloadAll || propName == "miscs"))
   {
      parent = createIndex(btTreeItem::MISC,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & YEASTMASK) &&
      (unloadAll || propName == "yeasts"))
   {
      parent = createIndex(btTreeItem::YEAST,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & STYLEMASK) &&
      (unloadAll || propName == "styles"))
   {
      parent = createIndex(btTreeItem::STYLE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
}

Recipe* btTreeModel::getRecipe(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getRecipe();
}

Equipment* btTreeModel::getEquipment(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getEquipment();
}

Fermentable* btTreeModel::getFermentable(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getFermentable();
}

Hop* btTreeModel::getHop(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getHop();
}

Misc* btTreeModel::getMisc(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getMisc();
}

Yeast* btTreeModel::getYeast(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getYeast();
}

Style* btTreeModel::getStyle(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getStyle();
}

BrewNote* btTreeModel::getBrewNote(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getBrewNote();
}

BeerXMLElement* btTreeModel::getThing(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);

   return item->getThing();
}

bool btTreeModel::isRecipe(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::RECIPE;
}

bool btTreeModel::isEquipment(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::EQUIPMENT;
}

bool btTreeModel::isFermentable(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::FERMENTABLE;
}

bool btTreeModel::isHop(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::HOP;
}

bool btTreeModel::isMisc(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::MISC;
}

bool btTreeModel::isYeast(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::YEAST;
}

bool btTreeModel::isStyle(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::STYLE;
}

bool btTreeModel::isBrewNote(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::BREWNOTE;
}

bool btTreeModel::isFolder(const QModelIndex &index) const
{
   btTreeItem* item = getItem(index);
   return item->getType() == btTreeItem::FOLDER;
}

int btTreeModel::getType(const QModelIndex &index)
{
   btTreeItem* item = getItem(index);
   return item->getType();
}

int btTreeModel::getMask()
{
   return treeMask;
}

void btTreeModel::equipmentChanged()
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

void btTreeModel::fermentableChanged()
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

void btTreeModel::hopChanged()
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

void btTreeModel::miscChanged()
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

void btTreeModel::recipeChanged()
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

void btTreeModel::yeastChanged()
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

void btTreeModel::styleChanged()
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

void btTreeModel::brewNoteChanged()
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

void btTreeModel::equipmentAdded(Equipment* victim)
{
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeEquipment(victim);
}

void btTreeModel::equipmentRemoved(Equipment* victim)
{
   QModelIndex index = findEquipment(victim);
   QModelIndex pIndex = parent(index);

   if ( index.isValid() )
   {
      removeRows(index.row(), 1, pIndex );
      disconnect( victim, 0, this, 0 );
   }
}

void btTreeModel::fermentableAdded(Fermentable* victim)
{
   // This is an import edge case. Things are being added to the db that are
   // marked not display. Don't do anything if they are not display
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeFermentable(victim);
}

void btTreeModel::fermentableRemoved(Fermentable* victim)
{
   QModelIndex index = findFermentable(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void btTreeModel::hopAdded(Hop* victim)
{
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeHop(victim);
}

void btTreeModel::hopRemoved(Hop* victim)
{
   QModelIndex index = findHop(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void btTreeModel::miscAdded(Misc* victim)
{
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeMisc(victim);
}

void btTreeModel::miscRemoved(Misc* victim)
{
   QModelIndex index = findMisc(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

// I can no longer assume the parent is at 0,0. 
void btTreeModel::recipeAdded(Recipe* victim)
{
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeRecipe(victim);
}

void btTreeModel::recipeRemoved(Recipe* victim)
{
   QModelIndex index = findRecipe(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void btTreeModel::yeastAdded(Yeast* victim)
{
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeYeast(victim);
}

void btTreeModel::yeastRemoved(Yeast* victim)
{
   QModelIndex index = findYeast(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

void btTreeModel::styleAdded(Style* victim)
{
   if ( ! victim->display() ) 
      return;

   btTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeStyle(victim);
}

void btTreeModel::styleRemoved(Style* victim)
{
   QModelIndex index = findStyle(victim);
   QModelIndex pIndex = parent(index);

   removeRows(index.row(),1,pIndex);
   disconnect( victim, 0, this, 0 );
}

// BrewNotes get no respect, but they get signals. They also get mighty
// confusing
void btTreeModel::brewNoteAdded(BrewNote* victim)
{
   // Get the brewnote's parent
   Recipe *parent = Database::instance().getParentRecipe(victim);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   btTreeItem* pItem = getItem(pInd);

   int breadth = pItem->childCount();
   insertRow(breadth,pInd,victim,btTreeItem::BREWNOTE);
   observeBrewNote(victim);
}

// deleting them is worse. Unfortunately, the blasted brewnote is deleted by
// the time we get this signal. So we have to rebuild the entire list.
void btTreeModel::brewNoteRemoved(BrewNote* victim)
{
   // Get the brewnote's parent recipe
   Recipe *parent = Database::instance().getParentRecipe(victim);
   // Find that recipe's index in the tree
   QModelIndex parentInd = findRecipe(parent);
   // and get the treeItem
   btTreeItem* parentItem = getItem(parentInd);

   disconnect( victim, 0, this, 0 );
   
   // If the tree item has children -- brewnotes -- remove them all
   if ( parentItem->childCount() )
      removeRows(0,parentItem->childCount(),parentInd);
   
   QList<BrewNote*> brewNotes = parent->brewNotes();
   for (int j=0; j < brewNotes.size(); ++j)
      insertRow(j,parentInd,brewNotes[j],btTreeItem::BREWNOTE);
}

void btTreeModel::observeEquipment(Equipment* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(equipmentChanged()) );
}

void btTreeModel::observeFermentable(Fermentable* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(fermentableChanged()) );
}

void btTreeModel::observeHop(Hop* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(hopChanged()) );
}

void btTreeModel::observeMisc(Misc* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(miscChanged()) );
}

void btTreeModel::observeRecipe(Recipe* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(recipeChanged()) );
}

void btTreeModel::observeYeast(Yeast* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(yeastChanged()) );
}

void btTreeModel::observeStyle(Style* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(styleChanged()) );
}

void btTreeModel::observeBrewNote(BrewNote* d)
{
   connect( d, SIGNAL(brewDateChanged(QDateTime)), this, SLOT(brewNoteChanged()) );
}  
