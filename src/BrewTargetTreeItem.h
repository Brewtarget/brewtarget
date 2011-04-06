/*
 * BrewTargetTreeItem.h
 *
 *  Created on: Apr 3, 2011
 *      Author: mik
 *
 *      This is intended to wrap the Recipe objects and make them suitable for a tree view.
 */

#ifndef BREWTARGETTREEITEM_H_
#define BREWTARGETTREEITEM_H_

class BrewTargetTreeItem;

#include <QSharedPointer>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QWidget>
#include <Qt>
#include <QVector>

#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "recipe.h"


class BrewTargetTreeItem
{

public:

	enum{RECIPENAMECOL, RECIPEBREWDATECOL, RECIPESTYLECOL, RECIPENUMCOLS /*This one MUST be last*/};
    enum{EQUIPMENTNAMECOL, EQUIPMENTBOILTIMECOL, EQUIPMENTNUMCOLS};
    enum{FERMENTABLENAMECOL, FERMENTABLETYPECOL, FERMENTABLECOLORCOL, FERMENTABLENUMCOLS};
    enum{HOPNAMECOL, HOPFORMCOL, HOPUSECOL, HOPNUMCOLS};
    
    enum{RECIPE,EQUIPMENT,FERMENTABLE,HOP};


	BrewTargetTreeItem(BrewTargetTreeItem *parent=0 );
	virtual ~BrewTargetTreeItem();

	BrewTargetTreeItem *child(int number);       // Gets the child object
	BrewTargetTreeItem *parent();

    int getType();
	int childCount() const;
	int columnCount(int type) const;
	QVariant data(int type, int column);		  // gets the information at column X
	int childNumber() const;

	// The nature of the data means we only add entire rows/recipes.
	// Therefore, we don't need to implement anything for changing the
	// column data
    QVariant getData(int column);

	void setData(Recipe *data);
	void setData(Equipment *data);
	void setData(Fermentable *data);
	void setData(Hop *data);

	Recipe* getRecipe();
	Equipment* getEquipment();
	Fermentable* getFermentable();
	Hop* getHop();

	bool insertChildren(int position, int count);
	bool removeChildren(int position, int count);

    // Don't use this
    void setType(int t);
private:
	QList<BrewTargetTreeItem*> childItems;

	Recipe      *recipe;
	Equipment   *kit;
	Fermentable *ferm;
	Hop         *hop;

	int type;
	BrewTargetTreeItem* parentItem;

    // With great abstraction comes great ... sorry, I was distracted.
    QVariant dataRecipe(int column);
    QVariant dataEquipment(int column);
    QVariant dataFermentable(int column);
    QVariant dataHop(int column);

};

#endif /* BREWTARGETTREEITEM_H_ */
