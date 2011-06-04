/*
 * BrewTargetTreeItem.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
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
#include "misc.h"
#include "yeast.h"

class BrewTargetTreeItem
{

public:

   enum{RECIPENAMECOL, RECIPEBREWDATECOL, RECIPESTYLECOL, RECIPENUMCOLS /*This one MUST be last*/};
   enum{EQUIPMENTNAMECOL, EQUIPMENTBOILTIMECOL, EQUIPMENTNUMCOLS};
   enum{FERMENTABLENAMECOL, FERMENTABLETYPECOL, FERMENTABLECOLORCOL, FERMENTABLENUMCOLS};
   enum{HOPNAMECOL, HOPFORMCOL, HOPUSECOL, HOPNUMCOLS};
   enum{MISCNAMECOL, MISCTYPECOL, MISCUSECOL, MISCNUMCOLS};
   enum{YEASTNAMECOL, YEASTTYPECOL, YEASTFORMCOL, YEASTNUMCOLS};
   enum{BREWDATE,BREWNUMCOLS};
    
   enum{RECIPE,EQUIPMENT,FERMENTABLE,HOP,MISC,YEAST,BREWNOTE,NUMTYPES};

   friend bool operator==(BrewTargetTreeItem &lhs, BrewTargetTreeItem &rhs);

   BrewTargetTreeItem(int type = NUMTYPES, BrewTargetTreeItem *parent=0 );
   virtual ~BrewTargetTreeItem();

   BrewTargetTreeItem *child(int number);       // Gets the child object
   BrewTargetTreeItem *parent();

   int getType();
   int childCount() const;
   int columnCount(int type) const;
   QVariant data(int type, int column);        // gets the information at column X
   int childNumber() const;

   // The nature of the data means we only add entire rows/recipes.
   // Therefore, we don't need to implement anything for changing the
   // column data
    QVariant getData(int column);

   void setData(int t, void *d);

   Recipe*      getRecipe();
   Equipment*   getEquipment();
   Fermentable* getFermentable();
   Hop*         getHop();
   Misc*        getMisc();
   Yeast*       getYeast();
   BrewNote*    getBrewNote();

   bool insertChildren(int position, int count, int type = RECIPE);
   bool removeChildren(int position, int count);


private:
   BrewTargetTreeItem* parentItem;
   QList<BrewTargetTreeItem*> childItems;

   int type;
   void *thing;

   // With great abstraction comes great ... sorry, I was distracted.
   QVariant dataRecipe(int column);
   QVariant dataEquipment(int column);
   QVariant dataFermentable(int column);
   QVariant dataHop(int column);
   QVariant dataMisc(int column);
   QVariant dataYeast(int column);
   QVariant dataBrewNote(int column);

   void setType(int t);
};

#endif /* BREWTARGETTREEITEM_H_ */
