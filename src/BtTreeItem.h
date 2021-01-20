/*
 * BtTreeItem.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
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

#ifndef BTTTREEITEM_H_
#define BTTTREEITEM_H_

class BtTreeItem;

#include <QSharedPointer>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QWidget>
#include <QVector>
#include <QObject>

#include "ingredient.h"

// Forward declarations.
class BrewNote;
class Equipment;
class Fermentable;
class Hop;
class Recipe;
class Misc;
class Yeast;
class Style;
class BtFolder;
class Water;

/*!
 * \class BtTreeItem
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
class BtTreeItem
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

   enum FOLDERITEM {
      //! Name
      FOLDERNAMECOL,
      //! Path
      FOLDERPATHCOL,
      //! Full path
      FOLDERFULLCOL,
      //! and the standard for the number of columns
      FOLDERNUMCOLS
   };

   enum WATERITEM {
      //! Name
      WATERNAMECOL,
      //! Ca
      WATERCACOL,
      //! HCO3
      WATERHCO3COL,
      //! SO4
      WATERSO4COL,
      //! Cl
      WATERCLCOL,
      //! NA
      WATERNACOL,
      //! MG
      WATERMGCOL,
      //! pH
      WATERpHCOL,
      //! and the standard for the number of columns
      WATERNUMCOLS
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
      FOLDER,
      WATER,
      NUMTYPES
   };

   friend bool operator==(BtTreeItem &lhs, BtTreeItem &rhs);

   //! \brief A constructor that sets the \c type of the BtTreeItem and
   // the \c parent
   BtTreeItem(int _type = NUMTYPES, BtTreeItem *parent=nullptr );
   virtual ~BtTreeItem();

   //! \brief returns the child at \c number
   BtTreeItem *child(int number);
   //! \brief returns item's parent
   BtTreeItem *parent();

   //! \brief returns item's type
   int type();
   //! \brief returns the number of the item's children
   int childCount() const;
   //! \brief returns number of columns associated with the item's \c type
   int columnCount(int _type) const;
   //! \brief returns the data of the item of \c type at \c column
   QVariant data(int _type, int column);
   //! \brief returns the index of the item in it's parents list
   int childNumber() const;

   //! \brief provides a wrapper to data() so that the caller doesn't need to
   // know the type of the item
   QVariant data(int column);

   //! \brief sets the \c t type of the object and the \c d data
   void setData(int t, QObject *d);

   //! \brief returns the data as a Recipe
   Recipe*      recipe();
   //! \brief returns the data as an Equipment
   Equipment*   equipment();
   //! \brief returns the data as a fermentable
   Fermentable* fermentable();
   //! \brief returns the data as a hop
   Hop*         hop();
   //! \brief returns the data as a misc
   Misc*        misc();
   //! \brief returns the data as a yeast
   Yeast*       yeast();
   //! \brief returns the data as a brewnote
   BrewNote*    brewNote();
   //! \brief returns the data as a style
   Style*       style();
   //! \brief returns data as a folder
   BtFolder*   folder();
   //! \brief returns data as a water
   Water*   water();
   //! \brief returns the data as a Ingredient
   Ingredient* thing();

   //! \brief inserts \c count new items of \c type, starting at \c position
   bool insertChildren(int position, int count, int _type = RECIPE);
   //! \brief removes \c count items starting at \c position
   bool removeChildren(int position, int count);

   //! \brief returns the name.
   QString name();

private:
   /*!  Keep a pointer to the parent tree item. */
   BtTreeItem* parentItem;
   /*!  The list of children associated with this item */
   QList<BtTreeItem*> childItems;

   /*! the type of this item */
   int _type;
   /*! the data associated with this item */
   QObject* _thing;

   /*! helper functions to get the information from the item */
   QVariant dataRecipe(int column);
   QVariant dataEquipment(int column);
   QVariant dataFermentable(int column);
   QVariant dataHop(int column);
   QVariant dataMisc(int column);
   QVariant dataYeast(int column);
   QVariant dataBrewNote(int column);
   QVariant dataStyle(int column);
   QVariant dataFolder(int column);
   QVariant dataWater(int column);

   void setType(int t);
};

#endif /* BREWTARGETTREEITEM_H_ */
