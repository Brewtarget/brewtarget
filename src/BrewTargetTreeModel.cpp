/*
 * BrewTargetTreeModel.cpp is part of Brewtarget and was written by Mik
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
#include "BrewTargetTreeItem.h"
#include "BrewTargetTreeModel.h"
#include "BrewTargetTreeView.h"
#include "database.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "recipe.h"
#include "yeast.h"
#include "brewnote.h"
#include "style.h"

BrewTargetTreeModel::BrewTargetTreeModel(BrewTargetTreeView *parent, TypeMasks type)
   : QAbstractItemModel(parent)
{
   // Initialize the tree structure
   int items = 0;

   rootItem = new BrewTargetTreeItem();

   if ( type & RECIPEMASK)
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::RECIPE);
      connect( &(Database::instance()), SIGNAL(newRecipeSignal(Recipe*)),this, SLOT(recipeAdded(Recipe*)));
      connect( &(Database::instance()), SIGNAL(deletedRecipeSignal(Recipe*)),this, SLOT(recipeRemoved(Recipe*)));
      // Brewnotes need love too!
      connect( &(Database::instance()), SIGNAL(newBrewNoteSignal(BrewNote*)),this, SLOT(brewNoteAdded(BrewNote*)));
      connect( &(Database::instance()), SIGNAL(deletedBrewNoteSignal(BrewNote*)),this, SLOT(brewNoteRemoved(BrewNote*)));
   }

   if ( type & EQUIPMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::EQUIPMENT);
      connect( &(Database::instance()), SIGNAL(newEquipmentSignal(Equipment*)),this, SLOT(equipmentAdded(Equipment*)));
      connect( &(Database::instance()), SIGNAL(deletedEquipmentSignal(Equipment*)),this, SLOT(equipmentRemoved(Equipment*)));
   }

   if ( type & FERMENTMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::FERMENTABLE);
      connect( &(Database::instance()), SIGNAL(newFermentableSignal(Fermentable*)),this, SLOT(fermentableAdded(Fermentable*)));
      connect( &(Database::instance()), SIGNAL(deletedFermentableSignal(Fermentable*)),this, SLOT(fermentableRemoved(Fermentable*)));
   }

   if ( type & HOPMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::HOP);
      connect( &(Database::instance()), SIGNAL(newHopSignal(Hop*)),this, SLOT(hopAdded(Hop*)));
      connect( &(Database::instance()), SIGNAL(deletedHopSignal(Hop*)),this, SLOT(hopRemoved(Hop*)));
   }

   if ( type & MISCMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::MISC);
      connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)),this, SLOT(miscAdded(Misc*)));
      connect( &(Database::instance()), SIGNAL(deletedMiscSignal(Misc*)),this, SLOT(miscRemoved(Misc*)));
   }

   if ( type & YEASTMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::YEAST);

      connect( &(Database::instance()), SIGNAL(newYeastSignal(Yeast*)),this, SLOT(yeastAdded(Yeast*)));
      connect( &(Database::instance()), SIGNAL(deletedYeastSignal(Yeast*)),this, SLOT(yeastRemoved(Yeast*)));
   }

   if ( type & STYLEMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::STYLE);

      connect( &(Database::instance()), SIGNAL(newStyleSignal(Style*)),this, SLOT(styleAdded(Style*)));
      connect( &(Database::instance()), SIGNAL(deletedStyleSignal(Style*)),this, SLOT(styleRemoved(Style*)));
   }

   treeMask = type;
   parentTree = parent;
   loadTreeModel();
}

BrewTargetTreeModel::~BrewTargetTreeModel()
{
   delete rootItem;
}

BrewTargetTreeItem *BrewTargetTreeModel::getItem( const QModelIndex &index ) const
{
   if ( index.isValid())
   {
      BrewTargetTreeItem *item = static_cast<BrewTargetTreeItem*>(index.internalPointer());
      if (item)
         return item;
   }

   return rootItem;
}

int BrewTargetTreeModel::rowCount(const QModelIndex &parent) const
{
   if (! parent.isValid())
      return rootItem->childCount();
   
   BrewTargetTreeItem *pItem = getItem(parent);

   return pItem->childCount();
}

int BrewTargetTreeModel::columnCount( const QModelIndex &parent) const
{
   switch(treeMask)
   {
   case RECIPEMASK:
      return BrewTargetTreeItem::RECIPENUMCOLS;
   case EQUIPMASK:
      return BrewTargetTreeItem::EQUIPMENTNUMCOLS;
   case FERMENTMASK:
      return BrewTargetTreeItem::FERMENTABLENUMCOLS;
   case HOPMASK:
      return BrewTargetTreeItem::HOPNUMCOLS;
   case MISCMASK:
      return BrewTargetTreeItem::MISCNUMCOLS;
   case YEASTMASK:
      return BrewTargetTreeItem::YEASTNUMCOLS;
   case STYLEMASK:
      return BrewTargetTreeItem::STYLENUMCOLS;
   default:
      return 0;
   }
   // Backwards compatibility. This MUST be fixed before the code goes live.
   return BrewTargetTreeItem::RECIPENUMCOLS;
}

Qt::ItemFlags BrewTargetTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return 0;

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex BrewTargetTreeModel::index( int row, int column, const QModelIndex &parent) const
{
   BrewTargetTreeItem *pItem, *cItem;

   if ( parent.isValid() && parent.column() != 0)
      return QModelIndex();

   pItem = getItem(parent);
   cItem = pItem->child(row);

   if (cItem)
      return createIndex(row,column,cItem);
   else
      return QModelIndex();
}

QModelIndex BrewTargetTreeModel::parent(const QModelIndex &index) const
{
   BrewTargetTreeItem *pItem, *cItem;

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

QModelIndex BrewTargetTreeModel::getFirst()
{
   QModelIndex parent;
   BrewTargetTreeItem* pItem; 

   // get the first item in the list, which is the place holder
   pItem = rootItem->child(0);
   if ( pItem->childCount() > 0 )
      return createIndex(0,0,pItem->child(0));

   return QModelIndex();
}

QVariant BrewTargetTreeModel::data(const QModelIndex &index, int role) const
{
   int maxColumns;

   switch(treeMask)
   {
   case RECIPEMASK:
      maxColumns = BrewTargetTreeItem::RECIPENUMCOLS;
      break;
   case EQUIPMASK:
      maxColumns = BrewTargetTreeItem::EQUIPMENTNUMCOLS;
      break;
   case FERMENTMASK:
      maxColumns = BrewTargetTreeItem::FERMENTABLENUMCOLS;
      break;
   case HOPMASK:
      maxColumns = BrewTargetTreeItem::HOPNUMCOLS;
      break;
   case MISCMASK:
      maxColumns = BrewTargetTreeItem::MISCNUMCOLS;
      break;
   case YEASTMASK:
      maxColumns = BrewTargetTreeItem::YEASTNUMCOLS;
      break;
   case STYLEMASK:
      maxColumns = BrewTargetTreeItem::STYLENUMCOLS;
      break;
   default:
      // Backwards compatibility. This MUST be fixed prior to releasing the code
      maxColumns = BrewTargetTreeItem::RECIPENUMCOLS;
   }

   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns)
      return QVariant();

   if ( role != Qt::DisplayRole && role != Qt::EditRole)
      return QVariant();

   BrewTargetTreeItem *item = getItem(index);
   return item->getData(index.column());
}

// This is much better, assuming the rest can be made to work
QVariant BrewTargetTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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
   default: // This needs to be fixed later
      return getRecipeHeader(section);

   }
}

QVariant BrewTargetTreeModel::getRecipeHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::RECIPENAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::RECIPEBREWDATECOL:
      return QVariant(tr("Brew Date"));
   case BrewTargetTreeItem::RECIPESTYLECOL:
      return QVariant(tr("Style"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getRecipeHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getEquipmentHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::EQUIPMENTNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::EQUIPMENTBOILTIMECOL:
      return QVariant(tr("Boil Time"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getEquipmentHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getFermentableHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::FERMENTABLENAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::FERMENTABLECOLORCOL:
      return QVariant(tr("Color"));
   case BrewTargetTreeItem::FERMENTABLETYPECOL:
      return QVariant(tr("Type"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getFermentableHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getHopHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::HOPNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::HOPFORMCOL:
      return QVariant(tr("Type"));
   case BrewTargetTreeItem::HOPUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getHopHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getMiscHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::MISCNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::MISCTYPECOL:
      return QVariant(tr("Type"));
   case BrewTargetTreeItem::MISCUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getMiscHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getYeastHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::YEASTNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::YEASTTYPECOL:
      return QVariant(tr("Type"));
   case BrewTargetTreeItem::YEASTFORMCOL:
      return QVariant(tr("Form"));
   }

   Brewtarget::logW( QString("BrewTargetTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

QVariant BrewTargetTreeModel::getStyleHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::STYLENAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::STYLECATEGORYCOL:
      return QVariant(tr("Category"));
   case BrewTargetTreeItem::STYLENUMBERCOL:
      return QVariant(tr("Number"));
   case BrewTargetTreeItem::STYLELETTERCOL:
      return QVariant(tr("Letter"));
   case BrewTargetTreeItem::STYLEGUIDECOL:
      return QVariant(tr("Guide"));
   }

   Brewtarget::logW( QString("BrewTargetTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

bool BrewTargetTreeModel::insertRow(int row, const QModelIndex &parent, QObject* victim, int victimType )
{
   if ( ! parent.isValid() )
      return false;

   BrewTargetTreeItem *pItem = getItem(parent);
   int type = pItem->getType();

   bool success = true;

   beginInsertRows(parent, row, row );
   success = pItem->insertChildren(row, 1, type);
   if ( victim && success ) 
   {
      type = victimType == -1 ? type : victimType;
      BrewTargetTreeItem* added = pItem->child(row);
      added->setData(type, victim);
   }
   endInsertRows();

   return success;
}

bool BrewTargetTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   BrewTargetTreeItem *pItem = getItem(parent);
   bool success = true;
    
   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

QModelIndex BrewTargetTreeModel::findRecipe(Recipe* rec)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
   int i;

   if (! rec || (treeMask & RECIPEMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getRecipe() == rec )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findEquipment(Equipment* kit)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
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

QModelIndex BrewTargetTreeModel::findFermentable(Fermentable* ferm)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
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

QModelIndex BrewTargetTreeModel::findHop(Hop* hop)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
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

QModelIndex BrewTargetTreeModel::findMisc(Misc* misc)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
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

QModelIndex BrewTargetTreeModel::findYeast(Yeast* yeast)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
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

QModelIndex BrewTargetTreeModel::findStyle(Style* style)
{
   BrewTargetTreeItem* pItem = rootItem->child(0);
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
QModelIndex BrewTargetTreeModel::findBrewNote(BrewNote* bNote)
{
   if (! bNote )
      return QModelIndex();

   // Get the brewnote's parent
   Recipe *parent = Database::instance().getParentRecipe(bNote);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   BrewTargetTreeItem* pItem = getItem(pInd);


   QList<BrewNote*> notes = parent->brewNotes();
   int i = notes.indexOf(bNote);
   if( i > 0 )
      return createIndex(i,0,pItem->child(i));
   else
      return QModelIndex();
}

void BrewTargetTreeModel::loadTreeModel(QString propName)
{
   int i,j;
   BrewTargetTreeItem* temp;

   bool loadAll = (propName == "");
  
   if ( (treeMask & RECIPEMASK ) && 
        (loadAll || propName == "recipes" ) )
   {  
      BrewTargetTreeItem* local = rootItem->child(0);
      QModelIndex ndxLocal = createIndex(0,0,local);
      QList<Recipe*> recipes = Database::instance().recipes();
      
      i = 0;
      foreach( Recipe* rec, recipes )
      {
         insertRow(i,ndxLocal,rec);
         temp = local->child(i);

         // If we have brewnotes, set them up here.
         QList<BrewNote*> notes = rec->brewNotes();
         j = 0;
         foreach( BrewNote* note, notes )
         {
            insertRow(j, createIndex(i,0,temp), note, BrewTargetTreeItem::BREWNOTE);
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
      BrewTargetTreeItem* local = rootItem->child(0);
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
      BrewTargetTreeItem* local = rootItem->child(0);
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
      BrewTargetTreeItem* local = rootItem->child(0);
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
      BrewTargetTreeItem* local = rootItem->child(0);
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
      BrewTargetTreeItem* local = rootItem->child(0);
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
      BrewTargetTreeItem* local = rootItem->child(0);
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

void BrewTargetTreeModel::unloadTreeModel(QString propName)
{
   int breadth;
   QModelIndex parent;

   bool unloadAll = (propName=="");
   
   if ( (treeMask & RECIPEMASK) &&
        (unloadAll || propName == "recipes"))
   {
      parent = createIndex(BrewTargetTreeItem::RECIPE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & EQUIPMASK) &&
      (unloadAll || propName == "equipments"))
   {
      parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & FERMENTMASK) &&
      (unloadAll || propName == "fermentables"))
   {
      parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & HOPMASK) &&
      (unloadAll || propName == "hops"))
   {
      parent = createIndex(BrewTargetTreeItem::HOP,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & MISCMASK) &&
      (unloadAll || propName == "miscs"))
   {
      parent = createIndex(BrewTargetTreeItem::MISC,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & YEASTMASK) &&
      (unloadAll || propName == "yeasts"))
   {
      parent = createIndex(BrewTargetTreeItem::YEAST,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & STYLEMASK) &&
      (unloadAll || propName == "styles"))
   {
      parent = createIndex(BrewTargetTreeItem::STYLE,0,rootItem->child(0));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
}

Recipe* BrewTargetTreeModel::getRecipe(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getRecipe();
}

Equipment* BrewTargetTreeModel::getEquipment(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getEquipment();
}

Fermentable* BrewTargetTreeModel::getFermentable(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getFermentable();
}

Hop* BrewTargetTreeModel::getHop(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getHop();
}

Misc* BrewTargetTreeModel::getMisc(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getMisc();
}

Yeast* BrewTargetTreeModel::getYeast(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getYeast();
}

Style* BrewTargetTreeModel::getStyle(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getStyle();
}

BrewNote* BrewTargetTreeModel::getBrewNote(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getBrewNote();
}

BeerXMLElement* BrewTargetTreeModel::getThing(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getThing();
}

bool BrewTargetTreeModel::isRecipe(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::RECIPE;
}

bool BrewTargetTreeModel::isEquipment(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::EQUIPMENT;
}

bool BrewTargetTreeModel::isFermentable(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::FERMENTABLE;
}

bool BrewTargetTreeModel::isHop(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::HOP;
}

bool BrewTargetTreeModel::isMisc(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::MISC;
}

bool BrewTargetTreeModel::isYeast(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::YEAST;
}

bool BrewTargetTreeModel::isStyle(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::STYLE;
}

bool BrewTargetTreeModel::isBrewNote(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::BREWNOTE;
}

int BrewTargetTreeModel::getType(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType();
}

int BrewTargetTreeModel::getMask()
{
   return treeMask;
}

void BrewTargetTreeModel::equipmentChanged()
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

void BrewTargetTreeModel::fermentableChanged()
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

void BrewTargetTreeModel::hopChanged()
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

void BrewTargetTreeModel::miscChanged()
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

void BrewTargetTreeModel::recipeChanged()
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

void BrewTargetTreeModel::yeastChanged()
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

void BrewTargetTreeModel::styleChanged()
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

void BrewTargetTreeModel::brewNoteChanged()
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

void BrewTargetTreeModel::equipmentAdded(Equipment* victim)
{
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   // Not quite sure what my logic here was. The index now will be 1,0 in
   // Item list. Shouldn't this be 0,0?
   // QModelIndex parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,local);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   // This goes with the previous commented out statement
   // insertRow(breadth, createIndex(0,0,local),victim);
   insertRow(breadth,parent,victim);
   observeEquipment(victim);
}

void BrewTargetTreeModel::equipmentRemoved(Equipment* victim)
{
   QModelIndex index = findEquipment(victim);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(0));
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   if ( index.isValid() )
   {
      // This logic is still escaping me
      // removeRows(index.row(), 1, createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(0)));
      removeRows(index.row(), 1, parent );
      disconnect( victim, 0, this, 0 );
   }
}

void BrewTargetTreeModel::fermentableAdded(Fermentable* victim)
{
   // This is an import edge case. Things are being added to the db that are
   // marked not display. Don't do anything if they are not display
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,local);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   // insertRow(breadth, createIndex(0,0,local),victim);
   insertRow(breadth,parent,victim);
   observeFermentable(victim);
}

void BrewTargetTreeModel::fermentableRemoved(Fermentable* victim)
{
   QModelIndex index = findFermentable(victim);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,rootItem->child(0));
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

void BrewTargetTreeModel::hopAdded(Hop* victim)
{
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::HOP,0,local);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   // insertRow(breadth, createIndex(0,0,local),victim);
   insertRow(breadth,parent,victim);
   observeHop(victim);
}

void BrewTargetTreeModel::hopRemoved(Hop* victim)
{
   QModelIndex index = findHop(victim);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::HOP,0,rootItem->child(0));
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

void BrewTargetTreeModel::miscAdded(Misc* victim)
{
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::MISC,0,local);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   // insertRow(breadth, createIndex(0,0,local),victim);
   insertRow(breadth,parent,victim);
   observeMisc(victim);
}

void BrewTargetTreeModel::miscRemoved(Misc* victim)
{
   QModelIndex index = findMisc(victim);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::MISC,0,rootItem->child(0));
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

void BrewTargetTreeModel::recipeAdded(Recipe* victim)
{
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::RECIPE,0,local);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   // insertRow(breadth, createIndex(0,0,local),victim);
   insertRow(breadth,parent,victim);
   observeRecipe(victim);
}

void BrewTargetTreeModel::recipeRemoved(Recipe* victim)
{
   QModelIndex index = findRecipe(victim);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::RECIPE,0,rootItem->child(0));
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

void BrewTargetTreeModel::yeastAdded(Yeast* victim)
{
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::YEAST,0,local);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   // insertRow(breadth, createIndex(0,0,local),victim);
   insertRow(breadth,parent,victim);
   observeYeast(victim);
}

void BrewTargetTreeModel::yeastRemoved(Yeast* victim)
{
   QModelIndex index = findYeast(victim);
   // QModelIndex parent = createIndex(BrewTargetTreeItem::YEAST,0,rootItem->child(0));
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

void BrewTargetTreeModel::styleAdded(Style* victim)
{
   if ( ! victim->display() ) 
      return;

   BrewTargetTreeItem* local = rootItem->child(0);
   QModelIndex parent = createIndex(0,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

   insertRow(breadth,parent,victim);
   observeStyle(victim);
}

void BrewTargetTreeModel::styleRemoved(Style* victim)
{
   QModelIndex index = findStyle(victim);
   QModelIndex parent = createIndex(0,0,rootItem->child(0));

   removeRows(index.row(),1,parent);
   disconnect( victim, 0, this, 0 );
}

// BrewNotes get no respect, but they get signals. They also get mighty
// confusing
void BrewTargetTreeModel::brewNoteAdded(BrewNote* victim)
{
   // Get the brewnote's parent
   Recipe *parent = Database::instance().getParentRecipe(victim);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   BrewTargetTreeItem* pItem = getItem(pInd);

   int breadth = pItem->childCount();
   insertRow(breadth,pInd,victim,BrewTargetTreeItem::BREWNOTE);
   observeBrewNote(victim);
}

// deleting them is worse. Unfortunately, the blasted brewnote is deleted by
// the time we get this signal. So we have to rebuild the entire list.
void BrewTargetTreeModel::brewNoteRemoved(BrewNote* victim)
{
   // Get the brewnote's parent recipe
   Recipe *parent = Database::instance().getParentRecipe(victim);
   // Find that recipe's index in the tree
   QModelIndex parentInd = findRecipe(parent);
   // and get the treeItem
   BrewTargetTreeItem* parentItem = getItem(parentInd);

   disconnect( victim, 0, this, 0 );
   
   // If the tree item has children -- brewnotes -- remove them all
   if ( parentItem->childCount() )
      removeRows(0,parentItem->childCount(),parentInd);
   
   QList<BrewNote*> brewNotes = parent->brewNotes();
   for (int j=0; j < brewNotes.size(); ++j)
      insertRow(j,parentInd,brewNotes[j],BrewTargetTreeItem::BREWNOTE);
}

void BrewTargetTreeModel::observeEquipment(Equipment* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(equipmentChanged()) );
}

void BrewTargetTreeModel::observeFermentable(Fermentable* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(fermentableChanged()) );
}

void BrewTargetTreeModel::observeHop(Hop* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(hopChanged()) );
}

void BrewTargetTreeModel::observeMisc(Misc* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(miscChanged()) );
}

void BrewTargetTreeModel::observeRecipe(Recipe* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(recipeChanged()) );
}

void BrewTargetTreeModel::observeYeast(Yeast* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(yeastChanged()) );
}

void BrewTargetTreeModel::observeStyle(Style* d)
{
   connect( d, SIGNAL(changedName(QString)), this, SLOT(styleChanged()) );
}

void BrewTargetTreeModel::observeBrewNote(BrewNote* d)
{
   connect( d, SIGNAL(brewDateChanged(QDateTime)), this, SLOT(brewNoteChanged()) );
}  
