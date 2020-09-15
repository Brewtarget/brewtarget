/*
 * BtTreeModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
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

#include <QModelIndex>
#include <QMessageBox>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <Qt>
#include <QObject>
#include <QStringBuilder>
#include <QMimeData>

#include "brewtarget.h"
#include "BtTreeItem.h"
#include "BtTreeModel.h"
#include "BtTreeView.h"
#include "RecipeFormatter.h"
#include "database.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "recipe.h"
#include "yeast.h"
#include "brewnote.h"
#include "style.h"
#include "water.h"

// =========================================================================
// ============================ CLASS STUFF ================================
// =========================================================================

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
         connect( &(Database::instance()), SIGNAL(deletedSignal(Recipe*)),this, SLOT(elementRemoved(Recipe*)));
         // Brewnotes need love too!
         connect( &(Database::instance()), SIGNAL(newBrewNoteSignal(BrewNote*)),this, SLOT(elementAdded(BrewNote*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(BrewNote*)),this, SLOT(elementRemoved(BrewNote*)));
         _type = BtTreeItem::RECIPE;
         _mimeType = "application/x-brewtarget-recipe";
         break;
      case EQUIPMASK:
         rootItem->insertChildren(items,1,BtTreeItem::EQUIPMENT);
         connect( &(Database::instance()), SIGNAL(newEquipmentSignal(Equipment*)),this, SLOT(elementAdded(Equipment*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Equipment*)),this, SLOT(elementRemoved(Equipment*)));
         _type = BtTreeItem::EQUIPMENT;
         _mimeType = "application/x-brewtarget-recipe";
         break;
      case FERMENTMASK:
         rootItem->insertChildren(items,1,BtTreeItem::FERMENTABLE);
         connect( &(Database::instance()), SIGNAL(newFermentableSignal(Fermentable*)),this, SLOT(elementAdded(Fermentable*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Fermentable*)),this, SLOT(elementRemoved(Fermentable*)));
         _type = BtTreeItem::FERMENTABLE;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case HOPMASK:
         rootItem->insertChildren(items,1,BtTreeItem::HOP);
         connect( &(Database::instance()), SIGNAL(newHopSignal(Hop*)),this, SLOT(elementAdded(Hop*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Hop*)),this, SLOT(elementRemoved(Hop*)));
         _type = BtTreeItem::HOP;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case MISCMASK:
         rootItem->insertChildren(items,1,BtTreeItem::MISC);
         connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)),this, SLOT(elementAdded(Misc*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Misc*)),this, SLOT(elementRemoved(Misc*)));
         _type = BtTreeItem::MISC;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case STYLEMASK:
         rootItem->insertChildren(items,1,BtTreeItem::STYLE);
         connect( &(Database::instance()), SIGNAL(newStyleSignal(Style*)),this, SLOT(elementAdded(Style*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Style*)),this, SLOT(elementRemoved(Style*)));
         _type = BtTreeItem::STYLE;
         _mimeType = "application/x-brewtarget-recipe";
         break;
      case YEASTMASK:
         rootItem->insertChildren(items,1,BtTreeItem::YEAST);
         connect( &(Database::instance()), SIGNAL(newYeastSignal(Yeast*)),this, SLOT(elementAdded(Yeast*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Yeast*)),this, SLOT(elementRemoved(Yeast*)));
         _type = BtTreeItem::YEAST;
         _mimeType = "application/x-brewtarget-ingredient";
         break;
      case WATERMASK:
         rootItem->insertChildren(items,1,BtTreeItem::WATER);
         connect( &(Database::instance()), SIGNAL(newWaterSignal(Water*)),this, SLOT(elementAdded(Water*)));
         connect( &(Database::instance()), SIGNAL(deletedSignal(Water*)),this, SLOT(elementRemoved(Water*)));
         _type = BtTreeItem::WATER;
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
   rootItem = nullptr;
}

// =========================================================================
// =================== ABSTRACTITEMMODEL STUFF =============================
// =========================================================================

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

   return item(parent)->childCount();
}

int BtTreeModel::columnCount( const QModelIndex &parent) const
{
   Q_UNUSED(parent)
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
   case WATERMASK:
      return BtTreeItem::WATERNUMCOLS;
   default:
      return 0;
   }
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

   if ( parent.isValid() && parent.column() >= columnCount(parent) )
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

   if ( cItem == nullptr )
      return QModelIndex();

   pItem = cItem->parent();

   if (pItem == rootItem || pItem == nullptr )
      return QModelIndex();

   return createIndex(pItem->childNumber(),0,pItem);
}

QModelIndex BtTreeModel::first()
{
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
   case WATERMASK:
      maxColumns = BtTreeItem::WATERNUMCOLS;
      break;
   default:
      // Backwards compatibility. This MUST be fixed prior to releasing the code
      // I hate this comment! Why? Why MUST this be fixed prior to release?
      // It wasn't, so what is "this" and what are the consequences. I am such
      // an ass.
      maxColumns = BtTreeItem::RECIPENUMCOLS;
   }

   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns)
      return QVariant();

   if ( role == Qt::ToolTipRole )
      return toolTipData(index);

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

QVariant BtTreeModel::toolTipData(const QModelIndex &index) const
{
   RecipeFormatter* whiskey = new RecipeFormatter();

   switch(treeMask)
   {
      case RECIPEMASK:
         return whiskey->getToolTip(qobject_cast<Recipe*>(thing(index)));
      case STYLEMASK:
         return whiskey->getToolTip( qobject_cast<Style*>(thing(index)));
      case EQUIPMASK:
         return whiskey->getToolTip( qobject_cast<Equipment*>(thing(index)));
      case FERMENTMASK:
         return whiskey->getToolTip( qobject_cast<Fermentable*>(thing(index)));
      case HOPMASK:
         return whiskey->getToolTip( qobject_cast<Hop*>(thing(index)));
      case MISCMASK:
         return whiskey->getToolTip( qobject_cast<Misc*>(thing(index)));
      case YEASTMASK:
         return whiskey->getToolTip( qobject_cast<Yeast*>(thing(index)));
      case WATERMASK:
         // return thing(index)->name();
         // this must wait until I implement the call. SEE? That's a proper
         // comment. Not this weaksauce "must be fixed" shit.
         return whiskey->getToolTip( qobject_cast<Water*>(thing(index)));
      default:
         return item(index)->name();
   }
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
   case WATERMASK:
      return waterHeader(section);
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

   Brewtarget::logW( QString("BtTreeModel::getRecipeHeader Bad column: %1").arg(section));
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

   Brewtarget::logW( QString("BtTreeModel::getEquipmentHeader Bad column: %1").arg(section));
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

   Brewtarget::logW( QString("BtTreeModel::getFermentableHeader Bad column: %1").arg(section));
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

   Brewtarget::logW( QString("BtTreeModel::getHopHeader Bad column: %1").arg(section));
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

   Brewtarget::logW( QString("BtTreeModel::getMiscHeader Bad column: %1").arg(section));
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

QVariant BtTreeModel::waterHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::WATERNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::WATERCACOL:
      return QVariant(tr("Ca"));
   case BtTreeItem::WATERHCO3COL:
      return QVariant(tr("HCO3"));
   case BtTreeItem::WATERSO4COL:
      return QVariant(tr("SO4"));
   case BtTreeItem::WATERCLCOL:
      return QVariant(tr("Cl"));
   case BtTreeItem::WATERNACOL:
      return QVariant(tr("Na"));
   case BtTreeItem::WATERMGCOL:
      return QVariant(tr("Mg"));
   case BtTreeItem::WATERpHCOL:
      return QVariant(tr("pH"));
   }
   Brewtarget::logW( QString("BtTreeModel::waterHeader Bad column: %1").arg(section) );
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

// =========================================================================
// ====================== BREWTARGET STUFF =================================
// =========================================================================

// One find method for all things. This .. is nice
QModelIndex BtTreeModel::findElement(Ingredient* thing, BtTreeItem* parent)
{
   BtTreeItem* pItem;
   QList<BtTreeItem*> folders;

   int i;

   if ( parent == nullptr )
      pItem = rootItem->child(0);
   else
      pItem = parent;

   if (! thing )
      return createIndex(0,0,pItem);

   folders.append(pItem);

   // Recursion. Wonderful.
   while ( ! folders.isEmpty() )
   {
      BtTreeItem* target = folders.takeFirst();
      for(i=0; i < target->childCount(); ++i)
      {
         // If we've found what we are looking for, return
         if ( target->child(i)->thing() == thing )
            return createIndex(i,0,target->child(i));

         // If we have a folder, or we are looking for a brewnote and have a
         // recipe in hand, push the child onto the stack
         if ( target->child(i)->type() == BtTreeItem::FOLDER ||
              (qobject_cast<BrewNote*>(thing) && target->child(i)->type() == BtTreeItem::RECIPE ) )
            folders.append(target->child(i));
      }
   }
   return QModelIndex();
}

QList<Ingredient*> BtTreeModel::elements()
{
   QList<Ingredient*> elements;
   switch(treeMask)
   {
   case RECIPEMASK:
      foreach( Ingredient* elem, Database::instance().recipes() )
         elements.append(elem);
      break;
   case EQUIPMASK:
      foreach( Ingredient* elem, Database::instance().equipments() )
         elements.append(elem);
      break;
   case FERMENTMASK:
      foreach( Ingredient* elem, Database::instance().fermentables() )
         elements.append(elem);
      break;
   case HOPMASK:
      foreach( Ingredient* elem, Database::instance().hops() )
         elements.append(elem);
      break;
   case MISCMASK:
      foreach( Ingredient* elem, Database::instance().miscs() )
         elements.append(elem);
      break;
   case YEASTMASK:
      foreach( Ingredient* elem, Database::instance().yeasts() )
         elements.append(elem);
      break;
   case STYLEMASK:
      foreach( Ingredient* elem, Database::instance().styles() )
         elements.append(elem);
      break;
   case WATERMASK:
      foreach( Ingredient* elem, Database::instance().waters() )
         elements.append(elem);
      break;
   default:
      Brewtarget::logW(QString("Invalid treemask: %1").arg(treeMask));
   }
   return elements;
}

void BtTreeModel::loadTreeModel()
{
   int i;

   QModelIndex ndxLocal;
   BtTreeItem* local = nullptr;
   QList<Ingredient*> elems = elements();

   foreach( Ingredient* elem, elems ) {

      if (! elem->folder().isEmpty() ) {
         ndxLocal = findFolder( elem->folder(), rootItem->child(0), true );
         // I cannot imagine this failing, but what the hell
         if ( ! ndxLocal.isValid() ) {
            Brewtarget::logW("Invalid return from findFolder in loadTreeModel()");
            continue;
         }
         local = item(ndxLocal);
         i = local->childCount();
      }
      else {
         local = rootItem->child(0);
         i = local->childCount();
         ndxLocal = createIndex(i,0,local);
      }

      if ( ! insertRow(i,ndxLocal,elem,_type) ) {
         Brewtarget::logW("Insert failed in loadTreeModel()");
         continue;
      }

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
      // In previous insert loops, we ignore the error and soldier on. So we
      // will do that here too
      if ( ! insertRow(j, createIndex(i,0,temp), note, BtTreeItem::BREWNOTE) )
      {
         Brewtarget::logW("Brewnote insert failed in loadTreeModel()");
         continue;
      }
      observeElement(note);
      ++j;
   }
}

Recipe* BtTreeModel::recipe(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->recipe() : nullptr;
}

Equipment* BtTreeModel::equipment(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->equipment() : nullptr;
}

Fermentable* BtTreeModel::fermentable(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->fermentable() : nullptr;
}

Hop* BtTreeModel::hop(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->hop() : nullptr;
}

Misc* BtTreeModel::misc(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->misc() : nullptr;
}

Yeast* BtTreeModel::yeast(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->yeast() : nullptr;
}

Style* BtTreeModel::style(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->style() : nullptr;
}

BrewNote* BtTreeModel::brewNote(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->brewNote() : nullptr;
}

BtFolder* BtTreeModel::folder(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->folder() : nullptr;
}

Water* BtTreeModel::water(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->water() : nullptr;
}

Ingredient* BtTreeModel::thing(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->thing() : nullptr;
}

bool BtTreeModel::isRecipe(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::RECIPE;
}

bool BtTreeModel::isEquipment(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::EQUIPMENT;
}

bool BtTreeModel::isFermentable(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::FERMENTABLE;
}

bool BtTreeModel::isHop(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::HOP;
}

bool BtTreeModel::isMisc(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::MISC;
}

bool BtTreeModel::isYeast(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::YEAST;
}

bool BtTreeModel::isStyle(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::STYLE;
}

bool BtTreeModel::isBrewNote(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::BREWNOTE;
}

bool BtTreeModel::isWater(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::WATER;
}

bool BtTreeModel::isFolder(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::FOLDER;
}

int BtTreeModel::type(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->type() : -1;
}

QString BtTreeModel::name(const QModelIndex &idx)
{
   return idx.isValid() ? item(idx)->name() : QString();
}

int BtTreeModel::mask()
{
   return treeMask;
}

void BtTreeModel::copySelected(QList< QPair<QModelIndex, QString> > toBeCopied)
{
   bool failed = false;
   while ( ! toBeCopied.isEmpty() )
   {
      QPair<QModelIndex,QString> thisPair = toBeCopied.takeFirst();
      QModelIndex ndx = thisPair.first;
      QString name = thisPair.second;

      switch ( type(ndx) )
      {
         case BtTreeItem::EQUIPMENT:
            Equipment *copyKit,  *oldKit;
            oldKit = equipment(ndx);
            copyKit = Database::instance().newEquipment(oldKit); // Create a deep copy.
            if ( copyKit)
               copyKit->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::FERMENTABLE:
            Fermentable *copyFerm, *oldFerm;
            oldFerm = fermentable(ndx);
            copyFerm = Database::instance().newFermentable(oldFerm); // Create a deep copy.
            if ( copyFerm )
               copyFerm->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::HOP:
            Hop *copyHop,  *oldHop;
            oldHop = hop(ndx);
            copyHop = Database::instance().newHop(oldHop); // Create a deep copy.
            if ( copyHop )
               copyHop->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::MISC:
            Misc *copyMisc, *oldMisc;
            oldMisc = misc(ndx);
            copyMisc = Database::instance().newMisc(oldMisc); // Create a deep copy.
            if ( copyMisc )
               copyMisc->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::RECIPE:
            Recipe *copyRec,  *oldRec;
            oldRec = recipe(ndx);
            copyRec = Database::instance().newRecipe(oldRec); // Create a deep copy.
            if ( copyRec )
               copyRec->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::STYLE:
            Style *copyStyle, *oldStyle;
            oldStyle = style(ndx);
            copyStyle = Database::instance().newStyle(oldStyle); // Create a deep copy.
            if ( copyStyle )
               copyStyle->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::YEAST:
            Yeast *copyYeast, *oldYeast;
            oldYeast = yeast(ndx);
            copyYeast = Database::instance().newYeast(oldYeast); // Create a deep copy.
            if ( copyYeast )
               copyYeast->setName(name);
            else
               failed = true;
            break;
         case BtTreeItem::WATER:
            Water *copyWater, *oldWater;
            oldWater = water(ndx);
            copyWater = Database::instance().newWater(oldWater); // Create a deep copy.
            if ( copyWater )
               copyWater->setName(name);
            else
               failed = true;
            break;
         default:
            Brewtarget::logW(QString("copySelected:: unknown type %1").arg(type(ndx)));
      }
      if ( failed ) {
         QMessageBox::warning(nullptr,
                              tr("Could not copy"),
                              tr("There was an unexpected error creating %1").arg(name));
         return;
      }
   }
}

void BtTreeModel::deleteSelected(QModelIndexList victims)
{
   QModelIndexList toBeDeleted = victims; // trust me

   while ( ! toBeDeleted.isEmpty() )
   {
      QModelIndex ndx = toBeDeleted.takeFirst();
      switch ( type(ndx) )
      {
         case BtTreeItem::EQUIPMENT:
            Database::instance().remove( equipment(ndx) );
            break;
         case BtTreeItem::FERMENTABLE:
            Database::instance().remove( fermentable(ndx) );
            break;
         case BtTreeItem::HOP:
            Database::instance().remove( hop(ndx) );
            break;
         case BtTreeItem::MISC:
            Database::instance().remove( misc(ndx) );
            break;
         case BtTreeItem::RECIPE:
            Database::instance().remove( recipe(ndx) );
            break;
         case BtTreeItem::STYLE:
            Database::instance().remove( style(ndx) );
            break;
         case BtTreeItem::YEAST:
            Database::instance().remove( yeast(ndx) );
            break;
         case BtTreeItem::BREWNOTE:
            Database::instance().remove( brewNote(ndx) );
            break;
         case BtTreeItem::WATER:
            Database::instance().remove( water(ndx) );
            break;
         case BtTreeItem::FOLDER:
            // This one is weird.
            toBeDeleted += allChildren(ndx);
            removeFolder(ndx);
            break;
         default:
            Brewtarget::logW(QString("deleteSelected:: unknown type %1").arg(type(ndx)));
      }
   }
}

// =========================================================================
// ============================ FOLDER STUFF ===============================
// =========================================================================

// The actual magic shouldn't be hard. Once we trap the signal, find the
// recipe, remove it from the parent and add it to the target folder.
// It is not easy. Indexes are ephemeral things. We MUST calculate the insert
// index after we have removed the recipe. BAD THINGS happen otherwise.
//
void BtTreeModel::folderChanged(QString name)
{
   Ingredient* test = qobject_cast<Ingredient*>(sender());
   QModelIndex ndx, pIndex;
   bool expand = true;

   Q_UNUSED(name)
   if ( ! test )
      return;

   // Find it.
   ndx = findElement(test);
   if ( ! ndx.isValid() )
   {
      Brewtarget::logW("folderChanged:: could not find element");
      return;
   }

   pIndex = parent(ndx); // Get the parent
   // If the parent isn't valid, its the root
   if ( ! pIndex.isValid() )
      pIndex = createIndex(0,0,rootItem->child(0));

   int i = item(ndx)->childNumber();

   // Remove it
   if ( ! removeRows(i, 1, pIndex) )
   {
      Brewtarget::logW("folderChanged:: could not remove row");
      return;
   }

   // Find the new parent
   // That's awkward, but dropping a folder prolly does need a the folder
   // created.
   QModelIndex newNdx = findFolder(test->folder(), rootItem->child(0), true);
   if ( ! newNdx.isValid() )
   {
      newNdx = createIndex(0,0,rootItem->child(0));
      expand = false;
   }

   BtTreeItem* local = item(newNdx);
   int j = local->childCount();

   if ( !  insertRow(j,newNdx,test,_type) )
   {
      Brewtarget::logW("folderChanged:: could not insert row");
      return;
   }
   // If we have brewnotes, set them up here.
   if ( treeMask & RECIPEMASK )
      addBrewNoteSubTree(qobject_cast<Recipe*>(test),j,local);

   if ( expand )
      emit expandFolder(treeMask,newNdx);
   return;
}

bool BtTreeModel::addFolder(QString name)
{
   return findFolder(name, rootItem->child(0), true).isValid();
}

bool BtTreeModel::removeFolder(QModelIndex ndx)
{
   if ( ! ndx.isValid() )
      return false;

   int i = -1;
   QModelIndex pInd = parent(ndx);

   if ( ! pInd.isValid() )
      return false;

   BtTreeItem* start = item(ndx);

   // Remove the victim.
   i = start->childNumber();
   return removeRows(i, 1, pInd);
}

QModelIndexList BtTreeModel::allChildren(QModelIndex ndx)
{
   QModelIndexList leafNodes;
   QList<BtTreeItem*> folders;
   int i;

   // Don't send an invalid index or something that isn't a folder
   if ( ! ndx.isValid() || type(ndx) != BtTreeItem::FOLDER )
      return leafNodes;

   BtTreeItem* start = item(ndx);
   folders.append(start);

   while ( ! folders.isEmpty() )
   {
      BtTreeItem* target = folders.takeFirst();

      for (i=0; i < target->childCount(); ++i)
      {
         BtTreeItem* next = target->child(i);
         // If a folder, push it onto the folders stack for later processing
         if ( next->type() == BtTreeItem::FOLDER )
            folders.append(next);
         else // Leafnode
            leafNodes.append(createIndex(i,0,next));
      }
   }
   return leafNodes;
}

bool BtTreeModel::renameFolder(BtFolder* victim, QString newName)
{
   QModelIndex ndx = findFolder(victim->fullPath(), nullptr, false);
   QModelIndex pInd;
   QString targetPath = newName % "/" % victim->name();
   QPair<QString,BtTreeItem*> f;
   QList<QPair<QString, BtTreeItem*> > folders;
   // This space is important       ^
   int i, kids,src;

   if ( ! ndx.isValid() )
      return false;

   pInd = parent(ndx);
   if ( ! pInd.isValid() )
      return false;

   BtTreeItem* start = item(ndx);
   f.first  = targetPath;
   f.second = start;

   folders.append(f);

   while ( ! folders.isEmpty() )
   {
      // This looks weird, but it is needed for later
      f = folders.takeFirst();
      targetPath = f.first;
      BtTreeItem* target = f.second;

      // As we move things, childCount changes. This makes sure we loop
      // through all of the kids
      kids = target->childCount();
      src = 0;
      // Ok. We have a start and an index.
      for (i=0; i < kids; ++i)
      {
         // This looks weird and it is. As we move children out, the 0 items
         // changes to the next child. In the case of a folder, though, we
         // don't move it, so we need to get the item beyond that.
         BtTreeItem* next = target->child(src);
         // If a folder, push it onto the folders stack for latter processing
         if ( next->type() == BtTreeItem::FOLDER )
         {
            QPair<QString,BtTreeItem*> newTarget;
            newTarget.first = targetPath % "/" % next->name();
            newTarget.second = next;
            folders.append(newTarget);
            src++;
         }
         else // Leafnode
            next->thing()->setFolder(targetPath);
      }
   }
   // Last thing is to remove the victim.
   i = start->childNumber();
   return removeRows(i, 1, pInd);
}

QModelIndex BtTreeModel::createFolderTree( QStringList dirs, BtTreeItem* parent, QString pPath)
{
   BtTreeItem* pItem = parent;

   // Start the loop. We are going to return ndx at the end,
   // so we need to declare and initialize outside of the loop
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

      pItem->insertChildren(i, 1, BtTreeItem::FOLDER);
      pItem->child(i)->setData(BtTreeItem::FOLDER, temp);

      // Insert the item into the tree. If it fails, bug out
      //if ( ! insertRow(i, ndx, temp, BtTreeItem::FOLDER) )
      //{
      //   emit layoutChanged();
      //   return QModelIndex();
      //}

      // Set the parent item to point to the newly created tree
      pItem = pItem->child(i);

      // And this for the return
      ndx = createIndex(pItem->childCount(), 0, pItem);
   }
   emit layoutChanged();

   // May K&R have mercy on my soul
   return ndx;
}

QModelIndex BtTreeModel::findFolder( QString name, BtTreeItem* parent, bool create )
{
   BtTreeItem* pItem;
   QStringList dirs;
   QString current, fullPath, targetPath;
   int i;

   pItem = parent ? parent : rootItem->child(0);

   // Upstream interfaces should handle this for me, but I like belt and
   // suspenders
   name = name.simplified();
   // I am assuming asking me to find an empty name means find the root of the
   // tree.
   if ( name.isEmpty() )
      return createIndex(0,0,pItem);

   // Prepare all the variables for the first loop

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   dirs = name.split("/", QString::SkipEmptyParts);
#else
   dirs = name.split("/", Qt::SkipEmptyParts);
#endif

   if ( dirs.isEmpty() )
      return QModelIndex();

   current = dirs.takeFirst();
   fullPath = "/";
   targetPath = fullPath % current;

   i = 0;

   // Time to get funky with no recursion!
   while( i < pItem->childCount() ) {
      BtTreeItem* kid = pItem->child(i);
      // The kid is a folder
      if ( kid->type() == BtTreeItem::FOLDER ) {
         // The folder name matches the part we are looking at
         if ( kid->folder()->isFolder(targetPath) ) {
            // If there are no more subtrees to look for, we found it
            if ( dirs.isEmpty() )
               return createIndex(i,0,kid);
            // Otherwise, we found a parent folder in our path
            else {
               // get the next folder in the path
               current = dirs.takeFirst();
               // append that to the fullPath we are looking for
               fullPath = targetPath;
               targetPath = fullPath % "/" % current;

               // Set the parent to the folder
               pItem = kid;
               // Reset the counter
               i = 0;
               // And do the time warp again!
               continue;
            }
         }
      }
      // If we got this far, it wasn't a folder or it wasn't a match.
      i++;
   }
   // If we get here, we found no match.

   // If we are supposed to create something, then lets get busy
   if ( create ) {
      // push the current dir back on the stack
      dirs.prepend(current);
      // And start with the madness
      return createFolderTree( dirs, pItem, fullPath);
   }

   // If we weren't supposed to create, we drop to here and return an empty
   // index.
   return QModelIndex();
}

// =========================================================================
// ============================ SLOT STUFF ===============================
// =========================================================================

void BtTreeModel::elementChanged()
{
   Ingredient* d = qobject_cast<Ingredient*>(sender());
   if( !d )
      return;

   QModelIndex ndxLeft = findElement(d);
   if( ! ndxLeft.isValid() )
      return;

   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

/* I don't like this part, but Qt's signal/slot mechanism are pretty
 * simplistic and do a string compare on signatures. Each one of these one
 * liners is required to give the right signature and to be able to call
 * addElement() properly
 */
void BtTreeModel::elementAdded(Recipe* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Equipment* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Fermentable* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Hop* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Misc* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Style* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Yeast* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(BrewNote* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementAdded(Water* victim) { elementAdded(qobject_cast<Ingredient*>(victim)); }

// I guess this isn't too bad. Better than this same function copied 7 times
void BtTreeModel::elementAdded(Ingredient* victim)
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

   if ( ! insertRow(breadth,pIdx,victim,lType) )
      return;

   // We need some special processing here to add brewnotes on a recipe import
   if ( qobject_cast<Recipe*>(victim) ) {
      Recipe* parent = qobject_cast<Recipe*>(victim);
      QList<BrewNote*> notes = parent->brewNotes();

      if ( notes.size() )
      {
         pIdx = findElement(parent);
         lType = BtTreeItem::BREWNOTE;
         int row = 0;
         foreach (BrewNote* note, notes)
            insertRow(row++,pIdx,note,lType);
      }
   }
   observeElement(victim);
}

void BtTreeModel::elementRemoved(Recipe* victim)      { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Equipment* victim)   { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Fermentable* victim) { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Hop* victim)         { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Misc* victim)        { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Style* victim)       { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Yeast* victim)       { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(BrewNote* victim)    { elementRemoved(qobject_cast<Ingredient*>(victim)); }
void BtTreeModel::elementRemoved(Water* victim)       { elementRemoved(qobject_cast<Ingredient*>(victim)); }

void BtTreeModel::elementRemoved(Ingredient* victim)
{
   QModelIndex index,pIndex;

   if ( ! victim )
      return;

   index = findElement(victim);
   if ( ! index.isValid() )
      return;

   pIndex = parent(index);
   if ( ! pIndex.isValid() )
      return;

   if ( removeRows(index.row(),1,pIndex) )
      return;

   disconnect( victim, nullptr, this, nullptr );
}

void BtTreeModel::observeElement(Ingredient* d)
{
   if ( ! d )
      return;

   if ( qobject_cast<BrewNote*>(d) )
      connect( d, SIGNAL(brewDateChanged(QDateTime)), this, SLOT(elementChanged()) );
   else
   {
      connect( d, SIGNAL(changedName(QString)), this, SLOT(elementChanged()) );
      connect( d, SIGNAL(changedFolder(QString)), this, SLOT(folderChanged(QString)));
   }
}


// =========================================================================
// ===================== DRAG AND DROP STUFF ===============================
// =========================================================================

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
      Ingredient* _thing = thing(parent);

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
      Ingredient* elem = nullptr;
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
         case BtTreeItem::WATER:
            elem = Database::instance().water(id);
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

         renameFolder(victim, target);
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
