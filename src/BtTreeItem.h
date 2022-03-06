/*
 * BtTreeItem.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef BTTTREEITEM_H
#define BTTTREEITEM_H
#pragma once

#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "model/NamedEntity.h"

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
 *
 * \brief Model for an item in a tree.
 *
 * This provides a generic item from which the trees are built. Since most of
 * the actions required are the same regardless of the item being stored (e.g.
 * hop or equipment), this class considers them all the same.
 *
 * It does assume that everything being stored can be cast into a QObject.
 */
class BtTreeItem {

public:

   /*!
    * The columns being displayed for recipes
    */
   enum RECIPEITEM {
      //! Recipe name
      RECIPENAMECOL,
      //! Recipe ancestors
      RECIPEANCCOUNT,
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
   enum class Type {
      RECIPE,
      EQUIPMENT,
      FERMENTABLE,
      HOP,
      MISC,
      YEAST,
      BREWNOTE,
      STYLE,
      FOLDER,
      WATER
   };

   /**
    * \brief This templated function will convert a class to its \c BtTreeItem::Type. Eg \c typeOf<Hop>() returns
    *        \c BtTreeItem::Type::Hop
    */
   template<class T>
   static BtTreeItem::Type typeOf();

   friend bool operator==(BtTreeItem & lhs, BtTreeItem & rhs);

   //! \brief A constructor that sets the \c type of the BtTreeItem and
   // the \c parent
   BtTreeItem(BtTreeItem::Type itemType = BtTreeItem::Type::FOLDER, BtTreeItem * parent = nullptr);
   virtual ~BtTreeItem();

   //! \brief returns the child at \c number
   BtTreeItem * child(int number);
   //! \brief returns item's parent
   BtTreeItem * parent();

   //! \brief returns item's type
   BtTreeItem::Type type() const;
   //! \brief returns the number of the item's children
   int childCount() const;
   //! \brief returns number of columns associated with the item's \c type
   int columnCount(BtTreeItem::Type itemType) const;
   //! \brief returns the data of the item of \c type at \c column
   QVariant data(/*BtTreeItem::Type itemType, */int column);
   //! \brief returns the index of the item in it's parents list
   int childNumber() const;

   //! \brief sets the \c t type of the object and the \c d data
   void setData(BtTreeItem::Type t, QObject * d);

   //! \brief returns the data as a T
   template<class T> T * getData();

   //! \brief returns the data as a NamedEntity
   NamedEntity * thing();

   //! \brief inserts \c count new items of \c type, starting at \c position
   bool insertChildren(int position, int count, BtTreeItem::Type itemType = BtTreeItem::Type::RECIPE);
   //! \brief removes \c count items starting at \c position
   bool removeChildren(int position, int count);

   //! \brief returns the name.
   QString name();
   //! \brief flag this node to override display() or not
   void setShowMe(bool val);
   //! \brief does the node want to be shown regardless of display()
   bool showMe() const;

private:
   /*!  Keep a pointer to the parent tree item. */
   BtTreeItem * parentItem;
   /*!  The list of children associated with this item */
   QList<BtTreeItem *> childItems;

   /*! the type of this item */
   BtTreeItem::Type itemType;

   /*! the data associated with this item */
   QObject * _thing;
   //! \b overrides the display()
   bool m_showMe;

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
};

#endif
