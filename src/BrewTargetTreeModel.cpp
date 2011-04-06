/*
 * BrewTargetTreeModel.cpp is part of Brewtarget and was written by Mik
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

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <Qt>
#include <QObject>

#include "brewtarget.h"
#include "recipe.h"
#include "BrewTargetTreeItem.h"
#include "BrewTargetTreeModel.h"
#include "BrewTargetTreeView.h"

BrewTargetTreeModel::BrewTargetTreeModel(BrewTargetTreeView *parent)
	: QAbstractItemModel(parent)
{
	// Initialize the tree structure
    rootItem = new BrewTargetTreeItem();
    rootItem->insertChildren(0,1,BrewTargetTreeItem::RECIPE);
    rootItem->insertChildren(1,1,BrewTargetTreeItem::EQUIPMENT);
    rootItem->insertChildren(2,1,BrewTargetTreeItem::FERMENTABLE);
    rootItem->insertChildren(3,1,BrewTargetTreeItem::HOP);

	parentTree = parent;
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
	BrewTargetTreeItem *pItem = getItem(parent);

	return pItem->childCount();
}

int BrewTargetTreeModel::columnCount( const QModelIndex &parent) const
{
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
	pItem = cItem->parent();

	if (pItem == rootItem)
		return QModelIndex();

	return createIndex(pItem->childNumber(),0,pItem);
}

QVariant BrewTargetTreeModel::data(const QModelIndex &index, int role) const
{
	if ( !index.isValid())
		return QVariant();

	if ( role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	BrewTargetTreeItem *item = getItem(index);
	return item->getData(index.column());
}

// This needs a metric crap ton of help.  How in hell do I figure out what the
// type is to do the right switches? Right now, it is wrong
QVariant BrewTargetTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
		return QVariant();

	switch(section)
	{
	case BrewTargetTreeItem::RECIPENAMECOL:
		return QVariant(tr("Name"));
	case BrewTargetTreeItem::RECIPEBREWDATECOL:
		return QVariant(tr("Brew Date"));
	case BrewTargetTreeItem::RECIPESTYLECOL:
		return QVariant(tr("Style"));
	default:
		Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(section));
		return QVariant();
	}
}

bool BrewTargetTreeModel::insertRow(int position, int type, void *data, const QModelIndex &parent)
{
	BrewTargetTreeItem *pItem = getItem(parent);
	bool success = true;

	beginInsertRows(parent, position, position);
	success = pItem->insertChildren(position, 1);
    if ( success ) 
    {
        BrewTargetTreeItem* newItem = pItem->child(position);
        newItem->setData(type,data);
    }
	endInsertRows();

	return success;
}

bool BrewTargetTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	BrewTargetTreeItem *pItem = getItem(parent);
	bool success = true;
	beginRemoveRows(parent, position, position + rows -1 );
	success = pItem->removeChildren(position,rows);
	endRemoveRows();

	return success;
}

QModelIndex BrewTargetTreeModel::findRecipe(Recipe* rec)
{
	BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::RECIPE);
	int i;

	if (! rec )
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
	BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::EQUIPMENT);
	int i;

	if (! kit )
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
	BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::FERMENTABLE);
	int i;

	if (! ferm )
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
	BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::FERMENTABLE);
	int i;

	if (! hop )
		return createIndex(0,0,pItem);

	for(i=0; i < pItem->childCount(); ++i)
	{
		if ( pItem->child(i)->getHop() == hop )
			return createIndex(i,0,pItem->child(i));
	}
	return QModelIndex();
}

void BrewTargetTreeModel::startObservingDB()
{
   dbObs = Database::getDatabase();
   setObserved(dbObs);
   loadTreeModel(DBALL);
}

void BrewTargetTreeModel::loadTreeModel(int reload)
{
   int i;

   if( ! Database::isInitialized() )
	   return;

   if ( reload == DBALL || reload == DBRECIPE)
   {
	   BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::RECIPE);
	   QList<Recipe*>::iterator it, end;
	   end = dbObs->getRecipeEnd();
	   for( it = dbObs->getRecipeBegin(), i = 0; it != end; ++it,++i )
		  insertRow(i,BrewTargetTreeItem::RECIPE,*it,createIndex(i,0,local));
   }

   if ( reload == DBALL || reload == DBEQUIP)
   {
	   BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::EQUIPMENT);
	   QList<Equipment*>::iterator it, end;
	   end = dbObs->getEquipmentEnd();
	   for( it = dbObs->getEquipmentBegin(), i = 0; it != end; ++it,++i )
		  insertRow(i,BrewTargetTreeItem::EQUIPMENT,*it,createIndex(i,0,local));
   }

   if ( reload == DBALL || reload == DBFERM)
   {
	   BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::FERMENTABLE);
	   QList<Fermentable*>::iterator it, end;
	   end = dbObs->getFermentableEnd();
	   for( it = dbObs->getFermentableBegin(), i = 0; it != end; ++it,++i )
		  insertRow(i,BrewTargetTreeItem::FERMENTABLE,*it,createIndex(i,0,local));
   }

   if ( reload == DBALL || reload == DBHOP)
   {
	   BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::HOP);
	   QList<Hop*>::iterator it, end;
	   end = dbObs->getHopEnd();
	   for( it = dbObs->getHopBegin(), i = 0; it != end; ++it,++i )
		  insertRow(i,BrewTargetTreeItem::HOP,*it,createIndex(i,0,local));
   }

}

void BrewTargetTreeModel::unloadTreeModel(int unload)
{
	int breadth;
	QModelIndex parent;

	if ( unload == DBALL || unload == DBRECIPE)
	{
		parent = createIndex(BrewTargetTreeItem::RECIPE,0,rootItem->child(BrewTargetTreeItem::RECIPE));
		breadth = rowCount(parent);
		removeRows(0,breadth,parent);
	}

	if (unload == DBALL || unload == DBEQUIP)
	{
		parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(BrewTargetTreeItem::EQUIPMENT));
		breadth = rowCount(parent);
		removeRows(0,breadth,parent);
	}

	if (unload == DBALL || unload == DBFERM)
	{
		parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,rootItem->child(BrewTargetTreeItem::FERMENTABLE));
		breadth = rowCount(parent);
		removeRows(0,breadth,parent);
	}

	if (unload == DBALL || unload == DBHOP)
	{
		parent = createIndex(BrewTargetTreeItem::HOP,0,rootItem->child(BrewTargetTreeItem::HOP));
		breadth = rowCount(parent);
		removeRows(0,breadth,parent);
	}
}

void BrewTargetTreeModel::notify(Observable* notifier, QVariant info)
{
//  unsigned int i, size;

   // Notifier could be the database. Only pay attention if the number of
   // recipes has changed.

   if( notifier == dbObs )
   {
	  unloadTreeModel(info.toInt());
	  loadTreeModel(info.toInt());
   }
   // This is the really hard part.  I believe I need to find the changed recipe in the
   // model and ... do something to it.  Which has two problems.  First, I don't have a
   // method for finding a specific recipe in the list and I am uncertain what will be
   // done with it.
/*
   else // Otherwise, we know that one of the recipes changed.
   {
	  size = recipeObs.size();
	  for( i = 0; i < size; ++i )
		 if( notifier == recipeObs[i] )
		 {
			// Notice we assume 'i' is an index into both 'recipeObs' and also
			// to the text list in this combo box...
			setItemText(i, recipeObs[i]->getName() );
		 }
   }
*/
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

int BrewTargetTreeModel::getType(const QModelIndex &index)
{
	BrewTargetTreeItem* item = getItem(index);
	return item->getType();
}
