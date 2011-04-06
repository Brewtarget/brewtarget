/*
 * BrewTargetTreeItem.cpp
 *
 *  Created on: Apr 3, 2011
 *      Author: mik
 */

#include <QString>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QString>
#include <QObject>
#include <QVector>

#include "brewtarget.h"
#include "recipe.h"
#include "BrewTargetTreeItem.h"

BrewTargetTreeItem::BrewTargetTreeItem(BrewTargetTreeItem *parent)
{
	parentItem = parent;
	// 0 everything out.
	recipe = 0;
	kit    = 0;
	ferm   = 0;
	hop    = 0;
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

void BrewTargetTreeItem::setData(Recipe* data)
{
	recipe = data;
}

void BrewTargetTreeItem::setData(Fermentable* data)
{
	ferm = data;
}

void BrewTargetTreeItem::setData(Equipment* data)
{
	kit = data;
}

void BrewTargetTreeItem::setData(Hop* data)
{
	hop = data;
}

QVariant BrewTargetTreeItem::getData(int column)
{
    return data(type,column);
}

bool BrewTargetTreeItem::insertChildren(int position, int count)
{
    int i;
	if ( position < 0  || position > childItems.size())
		return false;

    for(i=0; i < count; ++i)
    {
        BrewTargetTreeItem *newItem = new BrewTargetTreeItem(this);
        newItem->setType(this->getType());
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
	switch(column)
	{
        case RECIPENAMECOL:
			if ( ! recipe )
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

void BrewTargetTreeItem::setType(int t)
{
    type = t;
}

Recipe* BrewTargetTreeItem::getRecipe()
{
	return recipe;
}

Equipment* BrewTargetTreeItem::getEquipment()
{
	return kit;
}

Fermentable* BrewTargetTreeItem::getFermentable()
{
	return ferm;
}

Hop* BrewTargetTreeItem::getHop()
{
	return hop;
}
