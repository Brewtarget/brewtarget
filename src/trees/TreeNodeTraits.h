/*======================================================================================================================
 * trees/TreeNodeTraits.h is part of Brewtarget, and is copyright the following authors 2024-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#ifndef TREES_TREENODETRAITS_H
#define TREES_TREENODETRAITS_H
#pragma once

#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Folder.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

/**
 * \brief See comment in qtModels/tableModels/TableModelBase.h for why we use a traits class to allow the following attributes
 *        from each \c Derived class to be accessible in \c TreeNodeBase:
 *           - \c ColumnIndex        = class enum for the columns of this node type
 *           - \c NumberOfColumns    = number of entries in the above.  (Yes, it is a bit frustrating that we cannot
 *                                     easily deduce the number of values of a class enum.  Hopefully this will change
 *                                     in future versions of C++.)
 *           - \c NodeClassifier     = \c TreeNodeClassifier for this node type
 *           - \c ParentPtrTypes     = std::variant of raw pointers to valid parent types
 *           - \c ChildPtrTypes      = std::variant of shared_ptrs to valid child types (or
 *                                     std::variant<std::monostate> if no children are allowed).
 *           - \c DragNDropMimeType  = used with drag-and-drop to determine which things can be dropped where.  See
 *                                     \c mimeAccepted properties in \c ui/mainWindow.ui.  Note that this type
 *                                     determines where a dragged item can be dropped.  Broadly:
 *                                       - Recipes, equipment and styles get dropped on the recipe pane
 *                                       - Folders will be handled by themselves
 *                                       - Most other things get dropped on the ingredients pane
 *                                       - TBD what to do about Water
 *                                       - BrewNotes can't be dropped anywhere
 *        We use smart pointers for children and raw pointers for parents because parents own their children (and not
 *        vice versa).  We use std::variant even in trees where all nodes have a single parent type because it
 *        simplifies the generic code.
 */
template<class NE, class TreeType = NE>
struct TreeNodeTraits;


template<class NE> class TreeFolderNode;
template<class NE> class TreeItemNode;

template <class NE> struct TreeNodeTraits<Folder, NE> {
   enum class ColumnIndex {
      // TBD: Not sure we need all these columns!
      Name    ,
      Path    ,
      FullPath,
   };
   static constexpr size_t NumberOfColumns = 3;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::Folder;
   using TreeType = NE;
   using ParentPtrTypes = std::variant<TreeFolderNode<NE> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeFolderNode<NE>>, std::shared_ptr<TreeItemNode<NE>>>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-folder";

   static QVariant data(Folder const & folder, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(folder.name());
         case ColumnIndex::Path:
            return QVariant(folder.path());
         case ColumnIndex::FullPath:
            return QVariant(folder.fullPath());
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<BrewNote, Recipe> {
   enum class ColumnIndex {
      BrewDate,
   };
   static constexpr size_t NumberOfColumns = 1;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::SecondaryItem;
   using TreeType = Recipe;
   using ParentPtrTypes = std::variant<TreeItemNode<Recipe> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // BrewNotes can't be dropped anywhere, so there isn't anywhere in the program that accepts drops with this MIME type
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-brewnote";

   static QVariant data(BrewNote const & brewNote, ColumnIndex const column) {
      // I know this is a bit overkill when we only have one column, but I prefer to keep the same code structure for
      // all node types - in case we decide to add more columns in future.
      switch (column) {
         case ColumnIndex::BrewDate:
            return QVariant(brewNote.brewDate_short());
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Recipe, Recipe> {
   enum class ColumnIndex {
      Name             ,
      NumberOfAncestors,
      BrewDate         ,
      Style            ,
   };
   static constexpr size_t NumberOfColumns = 4;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Recipe;
   using ParentPtrTypes = std::variant<TreeFolderNode<Recipe> *, TreeItemNode<Recipe> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<BrewNote>>, std::shared_ptr<TreeItemNode<Recipe>>>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-recipe";

   static QString getRootName() { return Recipe::tr("Recipes"); }

   static QVariant data(Recipe const & recipe, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(recipe.name());
         case ColumnIndex::NumberOfAncestors:
            return QVariant(recipe.ancestors().size());
         case ColumnIndex::BrewDate:
            return recipe.date() ? Localization::displayDateUserFormated(*recipe.date()) : QVariant();
         case ColumnIndex::Style:
            return recipe.style() ? QVariant(recipe.style()->name()) : QVariant();
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Equipment, Equipment> {
   enum class ColumnIndex {
      Name    ,
      BoilTime,
   };
   static constexpr size_t NumberOfColumns = 2;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Equipment;
   using ParentPtrTypes = std::variant<TreeFolderNode<Equipment> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   //
   // Although it seems odd for Equipment to have a drag-and-drop MIME type of recipe, it is intentional.  This means an
   // Equipment can be dropped on the recipe pane (MainWindow::tabWidget_recipeView).
   //
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-recipe";

   static QString getRootName() { return Equipment::tr("Equipments"); }

   static QVariant data(Equipment const & equipment, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(equipment.name());
         case ColumnIndex::BoilTime:
            return QVariant::fromValue(equipment.boilTime_min());
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Fermentable, Fermentable> {
   enum class ColumnIndex {
      Name ,
      Type ,
      Color,
   };
   static constexpr size_t NumberOfColumns = 3;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Fermentable;
   using ParentPtrTypes = std::variant<TreeFolderNode<Fermentable> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // Fermentables and other ingredients can be dropped on MainWindow::tabWidget_ingredients
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-ingredient";

   static QString getRootName() { return Fermentable::tr("Fermentables"); }

   static QVariant data(Fermentable const & fermentable, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(fermentable.name());
         case ColumnIndex::Type:
            return QVariant(Fermentable::typeDisplayNames[fermentable.type()]);
         case ColumnIndex::Color:
            return QVariant(Measurement::displayAmount(Measurement::Amount{fermentable.color_srm(),
                                                                           Measurement::Units::srm}, 0));
      }
//      std::unreachable();
   }

};

template<> struct TreeNodeTraits<Hop, Hop> {
   enum class ColumnIndex {
      Name    ,
      Form    ,
      AlphaPct, // % Alpha Acid
      Origin  , // Country of origin
   };
   static constexpr size_t NumberOfColumns = 4;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Hop;
   using ParentPtrTypes = std::variant<TreeFolderNode<Hop> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-ingredient";

   static QString getRootName() { return Hop::tr("Hops"); }

   static QVariant data(Hop const & hop, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(hop.name());
         case ColumnIndex::Form:
            return QVariant(Hop::formDisplayNames[hop.form()]);
         case ColumnIndex::AlphaPct:
            return QVariant(hop.alpha_pct());
         case ColumnIndex::Origin:
            return QVariant(hop.origin());
      }
//      std::unreachable();
   }
};

// TODO: Add MashSteps
template<> struct TreeNodeTraits<Mash, Mash> {
   enum class ColumnIndex {
      Name      ,
      TotalWater,
      TotalTime ,
   };
   static constexpr size_t NumberOfColumns = 2;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Mash;
   using ParentPtrTypes = std::variant<TreeFolderNode<Mash> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-mash";

   static QString getRootName() { return Mash::tr("Mashes"); }

   static QVariant data(Mash const & mash, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(mash.name());
         case ColumnIndex::TotalWater:
            return QVariant(Measurement::displayAmount(Measurement::Amount{mash.totalMashWater_l(),
                                                                           Measurement::Units::liters}, 0));
         case ColumnIndex::TotalTime:
            return QVariant::fromValue(mash.totalTime_mins());
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Misc, Misc> {
   enum class ColumnIndex {
      Name,
      Type,
   };
   static constexpr size_t NumberOfColumns = 2;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Misc;
   using ParentPtrTypes = std::variant<TreeFolderNode<Misc> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-ingredient";

   static QString getRootName() { return Misc::tr("Miscellaneous"); }

   static QVariant data(Misc const & misc, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(misc.name());
         case ColumnIndex::Type:
            return QVariant(Misc::typeDisplayNames[misc.type()]);
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Yeast, Yeast> {
   enum class ColumnIndex {
      // It's tempting to put Laboratory first, and have it at the first column, but it messes up the way the folders
      // work if the first column isn't Name
      Name,
      Laboratory,
      ProductId,
      Type,
      Form,
   };
   static constexpr size_t NumberOfColumns = 5;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Yeast;
   using ParentPtrTypes = std::variant<TreeFolderNode<Yeast> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-ingredient";

   static QString getRootName() { return Yeast::tr("Yeasts"); }

   static QVariant data(Yeast const & yeast, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(yeast.name());
         case ColumnIndex::Laboratory:
            return QVariant(yeast.laboratory());
         case ColumnIndex::ProductId:
            return QVariant(yeast.productId());
         case ColumnIndex::Type:
            return QVariant(Yeast::typeDisplayNames[yeast.type()]);
         case ColumnIndex::Form:
            return QVariant(Yeast::formDisplayNames[yeast.form()]);
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Style, Style> {
   enum class ColumnIndex {
      Name          ,
      Category      ,
      CategoryNumber,
      CategoryLetter,
      StyleGuide    ,
   };
   static constexpr size_t NumberOfColumns = 5;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Style;
   using ParentPtrTypes = std::variant<TreeFolderNode<Style> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-recipe";

   static QString getRootName() { return Style::tr("Styles"); }

   static QVariant data(Style const & style, ColumnIndex const column) {
      switch (column) {
         case ColumnIndex::Name:
            return QVariant(style.name());
         case ColumnIndex::Category:
            return QVariant(style.category());
         case ColumnIndex::CategoryNumber:
            return QVariant(style.categoryNumber());
         case ColumnIndex::CategoryLetter:
            return QVariant(style.styleLetter());
         case ColumnIndex::StyleGuide:
            return QVariant(style.styleGuide());
      }
//      std::unreachable();
   }
};

template<> struct TreeNodeTraits<Water, Water> {
   enum class ColumnIndex {
      Name       ,
      Calcium    ,
      Bicarbonate,
      Sulfate    ,
      Chloride   ,
      Sodium     ,
      Magnesium  ,
      pH         ,
   };
   static constexpr size_t NumberOfColumns = 8;
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::PrimaryItem;
   using TreeType = Water;
   using ParentPtrTypes = std::variant<TreeFolderNode<Water> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static constexpr char const * DragNDropMimeType = DEF_CONFIG_MIME_PREFIX "-ingredient";

   static QString getRootName() { return Water::tr("Waters"); }

   static QVariant data(Water const & water, ColumnIndex const column) {
      switch (static_cast<ColumnIndex>(column)) {
         case ColumnIndex::Name:
            return QVariant(water.name());
         case ColumnIndex::Calcium:
            return QVariant(water.calcium_ppm());
         case ColumnIndex::Bicarbonate:
            return QVariant(water.bicarbonate_ppm());
         case ColumnIndex::Sulfate:
            return QVariant(water.sulfate_ppm());
         case ColumnIndex::Chloride:
            return QVariant(water.chloride_ppm());
         case ColumnIndex::Sodium:
            return QVariant(water.sodium_ppm());
         case ColumnIndex::Magnesium:
            return QVariant(water.magnesium_ppm());
         case ColumnIndex::pH:
            return water.ph() ? QVariant(*water.ph()) : QVariant();
      }
//      std::unreachable();
   }
};


#endif
