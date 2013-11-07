/*
 * btTreeItem.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
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

#ifndef BREWTARGETTREEITEM_H_
#define BREWTARGETTREEITEM_H_

class btTreeItem;

#include <QSharedPointer>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QWidget>
#include <QVector>
#include <QObject>

#include "BeerXMLElement.h"

// Forward declarations.
class BrewNote;
class Equipment;
class Fermentable;
class Hop;
class Recipe;
class Misc;
class Yeast;
class Style;

/*!
 * \class btTreeItem
 * \author Mik Firestone
 *
 * \brief Model for an item in a tree.
 *
 * This provides a generic item from which the trees are built. Since most of
 * the actions required are the same regardless of the item being stored (e.g.
 * hop or equipment), this class considers them all the same. 
 *
 * It does assume that everything being stored can be cast into a QObject.
 */
class btTreeItem
{

public:

   /*! 
    * The columns being displayed for recipes
    */
   enum RECIPEITEM {
      //! Recipe name
      RECIPENAMECOL, 
      //! Recipe brewdate
      RECIPEBREWDATECOL, 
      //! Recipe style
      RECIPESTYLECOL, 
      //! the number of columns available for recipes
      RECIPENUMCOLS 
   };
   /*! 
    * The columns being displayed for equipment
    */
   enum EQUIPITEM {
      //! Equipment name
      EQUIPMENTNAMECOL, 
      //! Equipment boil time
      EQUIPMENTBOILTIMECOL, 
      //! the number of columns available for equipment
      EQUIPMENTNUMCOLS
   };
   /*! 
    * The columns being displayed for fermentables
    */
   enum FERMITEM {
      //! Fermentable name
      FERMENTABLENAMECOL, 
      //! Fermentable type
      FERMENTABLETYPECOL, 
      //! Fermentable color
      FERMENTABLECOLORCOL, 
      //! the number of columns available for fermentables
      FERMENTABLENUMCOLS
   };
   /*! 
    * The columns being displayed for hops
    */
   enum HOPITEM {
      //! Hop name
      HOPNAMECOL, 
      //! Hop form
      HOPFORMCOL, 
      //! Hop use
      HOPUSECOL, 
      //! the number of columns available for hops
      HOPNUMCOLS
   };
   /*! 
    * The columns being displayed for misc
    */
   enum MISCITEM {
      //! Misc name
      MISCNAMECOL, 
      //! Misc type
      MISCTYPECOL, 
      //! Misc user
      MISCUSECOL, 
      //! the number of columns available for misc
      MISCNUMCOLS
   };
   /*! 
    * The columns being displayed for yeast
    */
   enum YEASTITEM {
      //! Yeast name
      YEASTNAMECOL, 
      //! Yeast type
      YEASTTYPECOL, 
      //! Yeast form
      YEASTFORMCOL, 
      //! the number of columns available for yeast
      YEASTNUMCOLS
   };
   /*! 
    * The columns being displayed for brewnotes
    */
   enum BREWNOTEITEM {
      //! Brew date
      BREWDATE,
      //! the number of columns available for brewnote
      BREWNUMCOLS
   };
   /*!
    * The columns to display for styles
    */
   enum STYLEITEM {
      //! Name
      STYLENAMECOL,
      //! Category
      STYLECATEGORYCOL,
      //! category number 
      STYLENUMBERCOL,
      //! category letter
      STYLELETTERCOL,
      //! which style guide definition comes from
      STYLEGUIDECOL,
      //! And the standard number of columns
      STYLENUMCOLS
   };

   
   /*! 
    * This enum lists the different things that we can store in an item
    */
   enum ITEMTYPE {
      RECIPE,
      EQUIPMENT,
      FERMENTABLE,
      HOP,
      MISC,
      YEAST,
      BREWNOTE,
      STYLE,
      NUMTYPES
   };

   friend bool operator==(btTreeItem &lhs, btTreeItem &rhs);

   //! \brief A constructor that sets the \c type of the btTreeItem and
   // the \c parent
   btTreeItem(int type = NUMTYPES, btTreeItem *parent=0 );
   virtual ~btTreeItem();

   //! \brief returns the child at \c number
   btTreeItem *child(int number);       
   //! \brief returns item's parent
   btTreeItem *parent();

   //! \brief returns item's type
   int getType();
   //! \brief returns the number of the item's children
   int childCount() const;
   //! \brief returns number of columns associated with the item's \c type
   int columnCount(int type) const;
   //! \brief returns the data of the item of \c type at \c column
   QVariant data(int type, int column);        
   //! \brief returns the index of the item in it's parents list
   int childNumber() const;

   //! \brief provides a wrapper to data() so that the caller doesn't need to
   // know the type of the item
    QVariant getData(int column);

   //! \brief sets the \c t type of the object and the \d data 
   void setData(int t, QObject *d);

   //! \brief returns the data as a Recipe
   Recipe*      getRecipe();
   //! \brief returns the data as an Equipment
   Equipment*   getEquipment();
   //! \brief returns the data as a fermentable
   Fermentable* getFermentable();
   //! \brief returns the data as a hop
   Hop*         getHop();
   //! \brief returns the data as a misc
   Misc*        getMisc();
   //! \brief returns the data as a yeast
   Yeast*       getYeast();
   //! \brief returns the data as a brewnote
   BrewNote*    getBrewNote();
   //! \brief returns the data as a style
   Style*       getStyle();
   //! \brief returns the data as a BeerXMLElement
   BeerXMLElement* getThing();

   //! \brief inserts \c count new items of \c type, starting at \c position
   bool insertChildren(int position, int count, int type = RECIPE);
   //! \brief removes \c count items starting at \c position
   bool removeChildren(int position, int count);


private:
   /*!  Keep a pointer to the parent tree item. */
   btTreeItem* parentItem;
   /*!  The list of children associated with this item */
   QList<btTreeItem*> childItems;

   /*! the type of this item */
   int type;
   /*! the data associated with this item */
   QObject* thing;

   /*! helper functions to get the information from the item */
   QVariant dataRecipe(int column);
   QVariant dataEquipment(int column);
   QVariant dataFermentable(int column);
   QVariant dataHop(int column);
   QVariant dataMisc(int column);
   QVariant dataYeast(int column);
   QVariant dataBrewNote(int column);
   QVariant dataStyle(int column);

   void setType(int t);
};

#endif /* BREWTARGETTREEITEM_H_ */
