/*======================================================================================================================
 * trees/TreeNodeTraits.h is part of Brewtarget, and is copyright the following authors 2024-2026:
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

#include "config.h"
#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Folder.h"
#include "model/Hop.h"
#include "model/StockPurchaseFermentable.h"
#include "model/StockPurchaseMisc.h"
#include "model/StockPurchaseHop.h"
#include "model/StockPurchaseSalt.h"
#include "model/StockPurchaseYeast.h"
#include "model/StockUseIngredient.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

/**
 * Phantom type for distinguishing root node from regular ones.
 *
 * (In older versions of the code, we just treated root node as a folder, but now that folders are first-class entities,
 * that's not really true -- because an item at the top level of the tree is one with no folder.  Although we could have
 * decided to insist that Folders are non-optional for all NamedEntity objects that support them, it wouldn't have got
 * us out of having to write special code to ensure that the root folder always exists and can't be renamed or
 * dragged-and-dropped.  I think it's better to keep the Folder code simpler and constraint the root node complexity to
 * the tree classes.)
 *
 * @tparam NE
 */
template<class NE> class RootMarkerFor {
public:
   [[nodiscard]] static QString className() {
      return NE::staticMetaObject.className();
   }

   [[nodiscard]] static QString localisedName() {
      return NE::localisedName();
   }
};

/**
 * \brief See comment in qtModels/tableModels/TableModelBase.h for why we use a traits class to allow the following
 *        attributes from each \c Derived class to be accessible in \c TreeNodeBase:
 *           - \c ColumnIndex        = class enum for the columns of this node type
 *           - \c NumberOfColumns    = number of entries in the above.  (Yes, it is a bit frustrating that we cannot
 *                                     easily deduce the number of values of a class enum.  Hopefully this will change
 *                                     in future versions of C++.)
 *           - \c NodeClassifier     = \c TreeNodeClassifier for this node type
 *           - \c ParentPtrTypes     = std::variant of raw pointers to valid parent types
 *           - \c ChildPtrTypes      = std::variant of shared_ptrs to valid child types (or
 *                                     std::variant<std::monostate> if no children are allowed).
 *           - \c getDragNDropMimeType  = used with drag-and-drop to determine which things can be dropped where.  See
 *                                        \c mimeAccepted property in \c BtTabWidget referenced in
 *                                        \c MainWindow::setupDrops.  Note that this type determines where a dragged
 *                                        item can be dropped.  Broadly:
 *                                          - Recipes, equipment and styles get dropped on the recipe pane
 *                                          - Folders will be handled by themselves
 *                                          - Most other things get dropped on the ingredients pane
 *                                          - TBD what to do about Water
 *                                          - BrewLogs can't be dropped anywhere
 *
 *        We use smart pointers for children and raw pointers for parents because parents own their children (and not
 *        vice versa).  We use std::variant even in trees where all nodes have a single parent type because it
 *        simplifies the generic code.
 *
 * \param NE
 * \param TreeType When NE is a secondary item (eg BrewLog) this will be the primary item (eg Recipe)
 */
template<class NE, class TreeType>
struct TreeNodeTraits;

//
// NOTE that the ColumnIndex enums below need to correspond with the COLUMN_INFOS definitions in TreeNode.cpp
//

template<class NE> class TreeFolderNode;
template<class NE> class TreeItemNode;
template<class NE> class TreeRootNode;

template <class NE> struct TreeNodeTraits<Folder<NE>, NE> {
   enum class ColumnIndex {
      // TBD: Not sure we need all these columns!
      Name    ,
      Path    ,
      FullPath,
   };
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::Folder;

   using ParentPtrTypes = std::variant<TreeRootNode<NE> *, TreeFolderNode<NE> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeFolderNode<NE>>, std::shared_ptr<TreeItemNode<NE>>>;
   static consteval char const * getDragNDropMimeType();
};
template<> consteval char const * TreeNodeTraits<Folder<Boil        >, Boil        >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Boil"        ; }
template<> consteval char const * TreeNodeTraits<Folder<Equipment   >, Equipment   >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Equipment"   ; }
template<> consteval char const * TreeNodeTraits<Folder<Fermentable >, Fermentable >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Fermentable" ; }
template<> consteval char const * TreeNodeTraits<Folder<Fermentation>, Fermentation>::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Fermentation"; }
template<> consteval char const * TreeNodeTraits<Folder<Hop         >, Hop         >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Hop"         ; }
template<> consteval char const * TreeNodeTraits<Folder<Mash        >, Mash        >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Mash"        ; }
template<> consteval char const * TreeNodeTraits<Folder<Misc        >, Misc        >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Misc"        ; }
template<> consteval char const * TreeNodeTraits<Folder<Recipe      >, Recipe      >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Recipe"      ; }
template<> consteval char const * TreeNodeTraits<Folder<Salt        >, Salt        >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Salt"        ; }
template<> consteval char const * TreeNodeTraits<Folder<Style       >, Style       >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Style"       ; }
template<> consteval char const * TreeNodeTraits<Folder<Water       >, Water       >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Water"       ; }
template<> consteval char const * TreeNodeTraits<Folder<Yeast       >, Yeast       >::getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Folder-Yeast"       ; }

template <class NE> struct TreeNodeTraits<RootMarkerFor<NE>, NE> {
   enum class ColumnIndex {
      TreeName,
   };
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::Root;

   using ParentPtrTypes = std::variant<TreeRootNode<NE> *>;
   using ChildPtrTypes = std::conditional_t<
      HasFolder<NE>,
      std::variant<std::shared_ptr<TreeFolderNode<NE>>, std::shared_ptr<TreeItemNode<NE>>>,
      std::variant<std::shared_ptr<TreeItemNode<NE>>>
   >;

   // It shouldn't be possible to drag and drop the root node, so we don't need it to have a class-specific MIME type.
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Root"; }
};

template<> struct TreeNodeTraits<BrewLog, Recipe> {
   enum class ColumnIndex {
      BrewDate,
      BatchNumber
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<Recipe> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // BrewLogs can't be dropped anywhere, so there isn't anywhere in the program that accepts drops with this MIME type
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-BrewLog"; }
};

template<> struct TreeNodeTraits<Recipe, Recipe> {
   enum class ColumnIndex {
      Name             ,
      NumberOfAncestors,
      DateCreated      ,
      Style            ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Recipe> *, TreeFolderNode<Recipe> *, TreeItemNode<Recipe> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<BrewLog>>, std::shared_ptr<TreeItemNode<Recipe>>>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Recipe"; }

   static QString getRootName() { return Recipe::tr("Recipes"); }
};

template<> struct TreeNodeTraits<Equipment, Equipment> {
   enum class ColumnIndex {
      Name     ,
      BoilSize ,
      BatchSize,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Equipment> *, TreeFolderNode<Equipment> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   //
   // Although it seems odd for Equipment to have a drag-and-drop MIME type of recipe, it is intentional.  This means an
   // Equipment can be dropped on the recipe pane (MainWindow::tabWidget_recipeView).
   //
   // TBD: We should probably revisit this at some point, as it could otherwise one day be a source of bugs.
   //
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Recipe"; }

   static QString getRootName() { return Equipment::tr("Equipments"); }
};

template<> struct TreeNodeTraits<Fermentable, Fermentable> {
   enum class ColumnIndex {
      Name ,
      Type ,
      Color,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Fermentable> *, TreeFolderNode<Fermentable> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // Fermentables and other ingredients can be dropped on MainWindow::tabWidget_ingredients
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Ingredient"; }

   static QString getRootName() { return Fermentable::tr("Fermentables"); }
};

template<> struct TreeNodeTraits<Hop, Hop> {
   enum class ColumnIndex {
      Name    ,
      Form    ,
      AlphaPct, // % Alpha Acid
      Origin  , // Country of origin
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Hop> *, TreeFolderNode<Hop> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Ingredient"; }

   static QString getRootName() { return Hop::tr("Hops"); }
};

template<> struct TreeNodeTraits<StockUseFermentable, StockPurchaseFermentable> {
   enum class ColumnIndex {
      Reason         ,
      Date           ,
      Comment        ,
      Recipe         ,
      AmountUsed     ,
      AmountRemaining,
   };
   static constexpr TreeNodeClassifier NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<StockPurchaseFermentable> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // StockUseFermentables cannot be dropped anywhere
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockUse"; }
};

template<> struct TreeNodeTraits<StockPurchaseFermentable, StockPurchaseFermentable> {
   enum class ColumnIndex {
      Name           ,
      DateOrdered    ,
      Supplier       ,
      DateReceived   ,
      AmountReceived ,
      AmountRemaining,
      Note           ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   // We have to support folder node for the root node
   using ParentPtrTypes = std::variant<TreeRootNode<StockPurchaseFermentable> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<StockUseFermentable>>>;
   // StockPurchaseFermentables cannot be dropped anywhere except folders
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockPurchase"; }

   static QString getRootName() { return Fermentable::tr("Fermentable Purchases"); }
};


template<> struct TreeNodeTraits<StockUseHop, StockPurchaseHop> {
   enum class ColumnIndex {
      Reason         ,
      Date           ,
      Comment        ,
      Recipe         ,
      AmountUsed     ,
      AmountRemaining,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<StockPurchaseHop> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // StockUseHops cannot be dropped anywhere
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockUse"; }
};

template<> struct TreeNodeTraits<StockPurchaseHop, StockPurchaseHop> {
   enum class ColumnIndex {
      Name           ,
      DateOrdered    ,
      Supplier       ,
      DateReceived   ,
      AmountReceived ,
      AmountRemaining,
      Note           ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   // We have to support folder node for the root node
   using ParentPtrTypes = std::variant<TreeRootNode<StockPurchaseHop> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<StockUseHop>>>;
   // StockPurchaseHops cannot be dropped anywhere except folders
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockPurchase"; }

   static QString getRootName() { return Hop::tr("Hop Purchases"); }
};

template<> struct TreeNodeTraits<StockUseMisc, StockPurchaseMisc> {
   enum class ColumnIndex {
      Reason         ,
      Date           ,
      Comment        ,
      Recipe         ,
      AmountUsed     ,
      AmountRemaining,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<StockPurchaseMisc> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // StockUseMiscs cannot be dropped anywhere
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockUse"; }
};

template<> struct TreeNodeTraits<StockPurchaseMisc, StockPurchaseMisc> {
   enum class ColumnIndex {
      Name           ,
      DateOrdered    ,
      Supplier       ,
      DateReceived   ,
      AmountReceived ,
      AmountRemaining,
      Note           ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   // We have to support folder node for the root node
   using ParentPtrTypes = std::variant<TreeRootNode<StockPurchaseMisc> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<StockUseMisc>>>;
   // StockPurchaseMiscs cannot be dropped anywhere except folders
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockPurchase"; }

   static QString getRootName() { return Misc::tr("Misc Purchases"); }
};

template<> struct TreeNodeTraits<StockUseSalt, StockPurchaseSalt> {
   enum class ColumnIndex {
      Reason         ,
      Date           ,
      Comment        ,
      Recipe         ,
      AmountUsed     ,
      AmountRemaining,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<StockPurchaseSalt> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // StockUseSalts cannot be dropped anywhere
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockUse"; }
};

template<> struct TreeNodeTraits<StockPurchaseSalt, StockPurchaseSalt> {
   enum class ColumnIndex {
      Name           ,
      DateOrdered    ,
      Supplier       ,
      DateReceived   ,
      AmountReceived ,
      AmountRemaining,
      Note           ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   // We have to support folder node for the root node
   using ParentPtrTypes = std::variant<TreeRootNode<StockPurchaseSalt> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<StockUseSalt>>>;
   // StockPurchaseSalts cannot be dropped anywhere except folders
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockPurchase"; }

   static QString getRootName() { return Salt::tr("Salt Purchases"); }
};

template<> struct TreeNodeTraits<StockUseYeast, StockPurchaseYeast> {
   enum class ColumnIndex {
      Reason         ,
      Date           ,
      Comment        ,
      Recipe         ,
      AmountUsed     ,
      AmountRemaining,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<StockPurchaseYeast> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // StockUseYeasts cannot be dropped anywhere
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockUse"; }
};

template<> struct TreeNodeTraits<StockPurchaseYeast, StockPurchaseYeast> {
   enum class ColumnIndex {
      Name           ,
      DateOrdered    ,
      Supplier       ,
      DateReceived   ,
      AmountReceived ,
      AmountRemaining,
      Note           ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   // We have to support folder node for the root node
   using ParentPtrTypes = std::variant<TreeRootNode<StockPurchaseYeast> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<StockUseYeast>>>;
   // StockPurchaseYeasts cannot be dropped anywhere except folders
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-StockPurchase"; }

   static QString getRootName() { return Yeast::tr("Yeast Purchases"); }
};

template<> struct TreeNodeTraits<MashStep, Mash> {
   enum class ColumnIndex {
      Name    ,
      Volume  ,
      StepTime,
//      Type        ,
//      Amount      ,
//      InfusionTemp,
//      TargetTemp  ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<Mash> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // MashSteps can't be dropped anywhere, so there isn't anywhere in the program that accepts drops with this MIME type
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-MashStep"; }
};

template<> struct TreeNodeTraits<Mash, Mash> {
   enum class ColumnIndex {
      Name      ,
      TotalWater,
      TotalTime ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Mash> *, TreeFolderNode<Mash> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<MashStep>>>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Mash"; }

   static QString getRootName() { return Mash::tr("Mash Profiles"); }
};

template<> struct TreeNodeTraits<BoilStep, Boil> {
   enum class ColumnIndex {
      Name        ,
      StepTime    ,
//      StartTemp   ,
//      RampTime    ,
//      EndTemp     ,
//      StartAcidity,
//      EndAcidity  ,
//      StartGravity,
//      EndGravity  ,
//      ChillingType,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<Boil> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // BoilSteps can't be dropped anywhere, so there isn't anywhere in the program that accepts drops with this MIME type
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-BoilStep"; }
};

template<> struct TreeNodeTraits<Boil, Boil> {
   enum class ColumnIndex {
      Name              ,
      PreBoilSize       ,
      LengthOfBoilProper,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Boil> *, TreeFolderNode<Boil> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<BoilStep>>>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Boil"; }

   static QString getRootName() { return Boil::tr("Boil Profiles"); }
};

template<> struct TreeNodeTraits<FermentationStep, Fermentation> {
   enum class ColumnIndex {
      Name     ,
      StepTime     ,
//      StartTemp,
//      EndTemp  ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::SecondaryItem;

   using ParentPtrTypes = std::variant<TreeItemNode<Fermentation> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   // FermentationSteps can't be dropped anywhere, so there isn't anywhere in the program that accepts drops with this MIME type
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-FermentationStep"; }
};

template<> struct TreeNodeTraits<Fermentation, Fermentation> {
   enum class ColumnIndex {
      Name       ,
      Description,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Fermentation> *, TreeFolderNode<Fermentation> *>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<FermentationStep>>>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Fermentation"; }

   static QString getRootName() { return Fermentation::tr("Fermentation Profiles"); }
};

template<> struct TreeNodeTraits<Misc, Misc> {
   enum class ColumnIndex {
      Name,
      Type,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Misc> *, TreeFolderNode<Misc> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Ingredient"; }

   static QString getRootName() { return Misc::tr("Miscellaneous"); }
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
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Yeast> *, TreeFolderNode<Yeast> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Ingredient"; }

   static QString getRootName() { return Yeast::tr("Yeasts"); }
};

template<> struct TreeNodeTraits<Salt, Salt> {
   enum class ColumnIndex {
      Name       ,
      Type       ,
      IsAcid     ,
      PercentAcid,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Salt> *, TreeFolderNode<Salt> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Ingredient"; }

   static QString getRootName() { return Salt::tr("Salts"); }
};

template<> struct TreeNodeTraits<Style, Style> {
   enum class ColumnIndex {
      Name          ,
      Category      ,
      CategoryNumber,
      StyleLetter   ,
      StyleGuide    ,
   };
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Style> *, TreeFolderNode<Style> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Recipe"; }

   static QString getRootName() { return Style::tr("Styles"); }
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
   static constexpr auto NodeClassifier = TreeNodeClassifier::PrimaryItem;

   using ParentPtrTypes = std::variant<TreeRootNode<Water> *, TreeFolderNode<Water> *>;
   using ChildPtrTypes = std::variant<std::monostate>;
   static consteval char const * getDragNDropMimeType() { return DEF_CONFIG_MIME_PREFIX "-Ingredient"; }

   static QString getRootName() { return Water::tr("Waters"); }
};

#endif