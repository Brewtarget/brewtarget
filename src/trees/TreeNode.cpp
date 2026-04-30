/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeNode.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "trees/TreeNode.h"

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <Qt>
#include <QVariant>
#include <QVector>
#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

#include "Html.h"
#include "Localization.h"
#include "PersistentSettings.h"
#include "measurement/ColorMethods.h"
#include "measurement/IbuMethods.h"
#include "measurement/Measurement.h"
#include "model/BrewLog.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Folder.h"
#include "model/Hop.h"
#include "model/StockPurchaseFermentable.h"
#include "model/StockUseIngredient.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "trees/RecipeTreeModel.h"
#include "trees/TreeModel.h"
#include "utils/ColumnInfo.h"
#include "utils/ColumnOwnerTraits.h"

namespace {
   /**
    * \brief Create a suitable display string for an optional double representing minutes
    *
    * \param units Whether to show the result in minutes or days
    */
   QString showOptionalMins(std::optional<double> minutes, Measurement::Unit const & units) {
      if (minutes) {
         return QString{"%1"}.arg(Measurement::displayAmount(Measurement::Amount{units.fromCanonical(*minutes), units}));
      }
      return "-";
   }

   QString getHeader() {
      //
      // TODO: One day we should fix this so the string is constructed at compile-time rather than run-time.  Meanwhile,
      // the Meyers singleton here means we at least only construct it once at runtime.
      //
      static QString const header =
         QString("<html><head><style type=\"text/css\">%1</style></head>").arg(Html::getCss(":/css/tooltip.css"));
      return header;
   }
}

TreeNode::TreeNode(TreeModel & model) :
   m_model{model} {
   return;
}

TreeNode::~TreeNode() = default;

int TreeNode::nodeCount(TreeNodeClassifier const typeToMatch) const {
   // Include ourselves if we're the right type
   int total = (this->classifier() == typeToMatch) ? 1 : 0;
   // Then recursively add the counts of our children
   for (int childNum = 0; childNum < this->childCount(); ++childNum) {
      total += this->rawChild(childNum)->nodeCount(typeToMatch);
   }
   return total;
}

QString TreeNode::subTreeToString(QString const indent, QString const prefix) const {
   QString output{};
   QTextStream outputStream{&output};
   this->subTreeToStream(outputStream, indent, prefix);


   return output;
}

void TreeNode::subTreeToStream(QTextStream & outputStream, QString const & indent, QString const & prefix) const {
   // Print the current node.  Note that everything starts two spaces in.
   outputStream << "  " << indent << prefix;
   switch (this->classifier()) {
      // Apart from folder, these symbols are a bit arbitrary, but they have the merit of brevity
      case TreeNodeClassifier::Root         : // Use same symbol as folder, so fall-through
      case TreeNodeClassifier::Folder       : outputStream << "📁"; break;
      case TreeNodeClassifier::PrimaryItem  : outputStream << "🗎"; break;
      case TreeNodeClassifier::SecondaryItem: outputStream << "§"; break;
   }
   outputStream << " " << *this << "\n";

   // If we have children, recursively output them
   int const numChildren = this->childCount();
   if (numChildren > 0) {
      for (int childNum = 0; childNum < numChildren; ++childNum) {
         TreeNode * child = this->rawChild(childNum);
         //
         // As can be seen from the following example, to work out the indent for our children, we need to look at our
         // own prefix.  If our prefix is "├──", then we add "│  " to the indent; if it is "└──" then we add "   ";
         // otherwise we add nothing (as we are root node).
         //
         // NOTE this is the motivation for passing index as QString rather than char *.
         //
         // 📁 Top Folder
         // └──📁 Subfolder
         //    ├──📁 Sub-subfolder 1
         //    │  └──🗎 Primary Item
         //    │     ├──§ Secondary Item a
         //    │     └──§ Secondary Item b
         //    └──🗎 Primary Item 2
         //       ├──§ Secondary Item c
         //       └──§ Secondary Item d
         //
         QString childIndent = indent;
         if (prefix == "├──") {
            childIndent += "│  ";
         } else if (prefix == "└──") {
            childIndent += "   ";
         }

         QString childOutput{};
         // Different prefixes for the last child
         child->subTreeToStream(outputStream,
                                childIndent,
                                QString{childNum == numChildren - 1 ? "└──" : "├──"});
      }
   }
   return;
}


void TreeNode::setShowMe(bool val) {
   this->m_showMe = val;
   return;
}

bool TreeNode::showMe() const {
   return this->m_showMe;
}

COLUMN_INFOS(
   TreeItemNode<Recipe>,
   COLINFO_TREE_ITEM_NODE(Recipe, Name             , PropertyNames::NamedEntity::name        ), // "Name"
   COLINFO_TREE_ITEM_NODE(Recipe, NumberOfAncestors, PropertyNames::Recipe     ::numAncestors), // "Snapshots"
   COLINFO_TREE_ITEM_NODE(Recipe, DateCreated      , PropertyNames::Recipe     ::date        ), // "Date Created"
   COLINFO_TREE_ITEM_NODE(Recipe, Style            , PropertyPath{{PropertyNames::Recipe::style,             // "Style"
                                                                   PropertyNames::NamedEntity::name}, 0} ),
)

COLUMN_INFOS(
   TreeItemNode<BrewLog>,
   COLINFO_TREE_ITEM_NODE(BrewLog, BrewDate   , PropertyNames::BrewLog::brewDate), // "Brew Date"
   COLINFO_TREE_ITEM_NODE(BrewLog, BatchNumber, PropertyNames::NamedEntity::name), // "Batch Number"
)

COLUMN_INFOS(
   TreeItemNode<Equipment>,
   COLINFO_TREE_ITEM_NODE(Equipment, Name     , PropertyNames::NamedEntity::name              ), // "Name"
   COLINFO_TREE_ITEM_NODE(Equipment, BoilSize , PropertyNames::Equipment::kettleBoilSize_l    ), // "Boil Size"
   COLINFO_TREE_ITEM_NODE(Equipment, BatchSize, PropertyNames::Equipment::fermenterBatchSize_l), // "Batch Size"
)

COLUMN_INFOS(
   TreeItemNode<Mash>,
   COLINFO_TREE_ITEM_NODE(Mash, Name      , PropertyNames::NamedEntity::name     ), // "Name"
   COLINFO_TREE_ITEM_NODE(Mash, TotalWater, PropertyNames::Mash::totalMashWater_l), // "Total Water"
   COLINFO_TREE_ITEM_NODE(Mash, TotalTime , PropertyNames::Mash::totalTime_mins  ), // "Total Time"
)

COLUMN_INFOS(
   TreeItemNode<MashStep>,
   COLINFO_TREE_ITEM_NODE(MashStep, Name    , PropertyNames::NamedEntity::name      ), // "Name"
   COLINFO_TREE_ITEM_NODE(MashStep, Volume  , PropertyNames::MashStep::amount_l     ), // "Volume"
   COLINFO_TREE_ITEM_NODE(MashStep, StepTime, PropertyNames::StepBase::stepTime_mins), // "Step Time"
)

COLUMN_INFOS(
   TreeItemNode<Boil>,
   COLINFO_TREE_ITEM_NODE(Boil, Name              , PropertyNames::NamedEntity::name  ), // "Name"
   COLINFO_TREE_ITEM_NODE(Boil, PreBoilSize       , PropertyNames::Boil::preBoilSize_l), // "Pre-Boil Size"
   COLINFO_TREE_ITEM_NODE(Boil, LengthOfBoilProper, PropertyNames::Boil::boilTime_mins), // "Time At Boiling"
)

COLUMN_INFOS(
   TreeItemNode<BoilStep>,
   COLINFO_TREE_ITEM_NODE(BoilStep, Name    , PropertyNames::NamedEntity::name      ), // "Name"
   COLINFO_TREE_ITEM_NODE(BoilStep, StepTime, PropertyNames::StepBase::stepTime_mins), // "Step Time"
)

COLUMN_INFOS(
   TreeItemNode<Fermentation>,
   COLINFO_TREE_ITEM_NODE(Fermentation, Name       , PropertyNames::NamedEntity::name        ),
   COLINFO_TREE_ITEM_NODE(Fermentation, Description, PropertyNames::Fermentation::description),
)

COLUMN_INFOS(
   TreeItemNode<FermentationStep>,
   COLINFO_TREE_ITEM_NODE(FermentationStep, Name    , PropertyNames::NamedEntity::name      ),
   COLINFO_TREE_ITEM_NODE(FermentationStep, StepTime, PropertyNames::StepBase::stepTime_days), // NB Days not Mins for fermentation steps
)

COLUMN_INFOS(
   TreeItemNode<Fermentable>,
   COLINFO_TREE_ITEM_NODE(Fermentable, Name , PropertyNames::NamedEntity::name     ), // "Name"
   COLINFO_TREE_ITEM_NODE(Fermentable, Type , PropertyNames::Fermentable::type     ), // "Type"
   COLINFO_TREE_ITEM_NODE(Fermentable, Color, PropertyNames::Fermentable::color_srm), // "Color"
)

COLUMN_INFOS(
   TreeItemNode<Hop>,
   COLINFO_TREE_ITEM_NODE(Hop, Name    , PropertyNames::NamedEntity::name), // "Name"
   COLINFO_TREE_ITEM_NODE(Hop, Form    , PropertyNames::Hop::form        ), // "Type"
   COLINFO_TREE_ITEM_NODE(Hop, AlphaPct, PropertyNames::Hop::alpha_pct   ), // "% Alpha"
   COLINFO_TREE_ITEM_NODE(Hop, Origin  , PropertyNames::Hop::origin      ), // "Origin"
)

COLUMN_INFOS(
   TreeItemNode<StockPurchaseFermentable>,
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, Name           , PropertyPath{{PropertyNames::StockPurchaseFermentable::fermentable,
                                                                                   PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, DateOrdered    , PropertyNames::StockPurchase::dateOrdered ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, Supplier       , PropertyNames::StockPurchase::supplier    ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, DateReceived   , PropertyNames::StockPurchase::dateReceived),
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, AmountReceived , PropertyNames::StockPurchaseBase::amountReceived ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, AmountRemaining, PropertyNames::StockPurchaseBase::amountRemaining),
   COLINFO_TREE_ITEM_NODE(StockPurchaseFermentable, Note           , PropertyNames::StockPurchase::note        ),
)

COLUMN_INFOS(
   TreeItemNode<StockUseFermentable>,
   COLINFO_TREE_ITEM_NODE(StockUseFermentable, Reason         , PropertyNames::StockUse::reason),
   COLINFO_TREE_ITEM_NODE(StockUseFermentable, Date           , PropertyNames::StockUse::date  ),
   COLINFO_TREE_ITEM_NODE(StockUseFermentable, Comment        , PropertyNames::StockUse::comment),
   COLINFO_TREE_ITEM_NODE(StockUseFermentable, Recipe         , PropertyPath{{PropertyNames::StockUse::brewLog,
                                                                              PropertyNames::OwnedByRecipe::recipe,
                                                                              PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockUseFermentable, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ),
   COLINFO_TREE_ITEM_NODE(StockUseFermentable, AmountRemaining, PropertyNames::StockUseBase::amountRemaining),
)

COLUMN_INFOS(
   TreeItemNode<StockPurchaseHop>,
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, Name           , PropertyPath{{PropertyNames::StockPurchaseHop::hop,
                                                                           PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, DateOrdered    , PropertyNames::StockPurchase::dateOrdered ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, Supplier       , PropertyNames::StockPurchase::supplier    ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, DateReceived   , PropertyNames::StockPurchase::dateReceived),
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, AmountReceived , PropertyNames::StockPurchaseBase::amountReceived ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, AmountRemaining, PropertyNames::StockPurchaseBase::amountRemaining),
   COLINFO_TREE_ITEM_NODE(StockPurchaseHop, Note           , PropertyNames::StockPurchase::note        ),
)

COLUMN_INFOS(
   TreeItemNode<StockUseHop>,
   COLINFO_TREE_ITEM_NODE(StockUseHop, Reason         , PropertyNames::StockUse::reason),
   COLINFO_TREE_ITEM_NODE(StockUseHop, Date           , PropertyNames::StockUse::date  ),
   COLINFO_TREE_ITEM_NODE(StockUseHop, Comment        , PropertyNames::StockUse::comment),
   COLINFO_TREE_ITEM_NODE(StockUseHop, Recipe         , PropertyPath{{PropertyNames::StockUse::brewLog,
                                                                      PropertyNames::OwnedByRecipe::recipe,
                                                                      PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockUseHop, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ),
   COLINFO_TREE_ITEM_NODE(StockUseHop, AmountRemaining, PropertyNames::StockUseBase::amountRemaining),
)

COLUMN_INFOS(
   TreeItemNode<StockPurchaseMisc>,
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, Name           , PropertyPath{{PropertyNames::StockPurchaseMisc::misc,
                                                                            PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, DateOrdered    , PropertyNames::StockPurchase::dateOrdered ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, Supplier       , PropertyNames::StockPurchase::supplier    ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, DateReceived   , PropertyNames::StockPurchase::dateReceived),
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, AmountReceived , PropertyNames::StockPurchaseBase::amountReceived ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, AmountRemaining, PropertyNames::StockPurchaseBase::amountRemaining),
   COLINFO_TREE_ITEM_NODE(StockPurchaseMisc, Note           , PropertyNames::StockPurchase::note        ),
)

COLUMN_INFOS(
   TreeItemNode<StockUseMisc>,
   COLINFO_TREE_ITEM_NODE(StockUseMisc, Reason         , PropertyNames::StockUse::reason),
   COLINFO_TREE_ITEM_NODE(StockUseMisc, Date           , PropertyNames::StockUse::date  ),
   COLINFO_TREE_ITEM_NODE(StockUseMisc, Comment        , PropertyNames::StockUse::comment),
   COLINFO_TREE_ITEM_NODE(StockUseMisc, Recipe         , PropertyPath{{PropertyNames::StockUse::brewLog,
                                                                       PropertyNames::OwnedByRecipe::recipe,
                                                                       PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockUseMisc, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ),
   COLINFO_TREE_ITEM_NODE(StockUseMisc, AmountRemaining, PropertyNames::StockUseBase::amountRemaining),
)

COLUMN_INFOS(
   TreeItemNode<StockPurchaseSalt>,
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, Name           , PropertyPath{{PropertyNames::StockPurchaseSalt::salt,
                                                                            PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, DateOrdered    , PropertyNames::StockPurchase::dateOrdered ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, Supplier       , PropertyNames::StockPurchase::supplier    ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, DateReceived   , PropertyNames::StockPurchase::dateReceived),
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, AmountReceived , PropertyNames::StockPurchaseBase::amountReceived ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, AmountRemaining, PropertyNames::StockPurchaseBase::amountRemaining),
   COLINFO_TREE_ITEM_NODE(StockPurchaseSalt, Note           , PropertyNames::StockPurchase::note        ),
)

COLUMN_INFOS(
   TreeItemNode<StockUseSalt>,
   COLINFO_TREE_ITEM_NODE(StockUseSalt, Reason         , PropertyNames::StockUse::reason),
   COLINFO_TREE_ITEM_NODE(StockUseSalt, Date           , PropertyNames::StockUse::date  ),
   COLINFO_TREE_ITEM_NODE(StockUseSalt, Comment        , PropertyNames::StockUse::comment),
   COLINFO_TREE_ITEM_NODE(StockUseSalt, Recipe         , PropertyPath{{PropertyNames::StockUse::brewLog,
                                                                       PropertyNames::OwnedByRecipe::recipe,
                                                                       PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockUseSalt, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ),
   COLINFO_TREE_ITEM_NODE(StockUseSalt, AmountRemaining, PropertyNames::StockUseBase::amountRemaining),
)

COLUMN_INFOS(
   TreeItemNode<StockPurchaseYeast>,
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, Name           , PropertyPath{{PropertyNames::StockPurchaseYeast::yeast,
                                                                             PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, DateOrdered    , PropertyNames::StockPurchase::dateOrdered ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, Supplier       , PropertyNames::StockPurchase::supplier    ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, DateReceived   , PropertyNames::StockPurchase::dateReceived),
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, AmountReceived , PropertyNames::StockPurchaseBase::amountReceived ),
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, AmountRemaining, PropertyNames::StockPurchaseBase::amountRemaining),
   COLINFO_TREE_ITEM_NODE(StockPurchaseYeast, Note           , PropertyNames::StockPurchase::note        ),
)

COLUMN_INFOS(
   TreeItemNode<StockUseYeast>,
   COLINFO_TREE_ITEM_NODE(StockUseYeast, Reason         , PropertyNames::StockUse::reason),
   COLINFO_TREE_ITEM_NODE(StockUseYeast, Date           , PropertyNames::StockUse::date  ),
   COLINFO_TREE_ITEM_NODE(StockUseYeast, Comment        , PropertyNames::StockUse::comment),
   COLINFO_TREE_ITEM_NODE(StockUseYeast, Recipe         , PropertyPath{{PropertyNames::StockUse::brewLog,
                                                                        PropertyNames::OwnedByRecipe::recipe,
                                                                        PropertyNames::NamedEntity::name}, 1}),
   COLINFO_TREE_ITEM_NODE(StockUseYeast, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ),
   COLINFO_TREE_ITEM_NODE(StockUseYeast, AmountRemaining, PropertyNames::StockUseBase::amountRemaining),
)

COLUMN_INFOS(
   TreeItemNode<Misc>,
   COLINFO_TREE_ITEM_NODE(Misc, Name, PropertyNames::NamedEntity::name),
   COLINFO_TREE_ITEM_NODE(Misc, Type, PropertyNames::Misc::type       ),
)

COLUMN_INFOS(
   TreeItemNode<Salt>,
   COLINFO_TREE_ITEM_NODE(Salt, Name       , PropertyNames::NamedEntity::name),
   COLINFO_TREE_ITEM_NODE(Salt, Type       , PropertyNames::Salt::type       ),
   COLINFO_TREE_ITEM_NODE(Salt, IsAcid     , PropertyNames::Salt::isAcid     ),
   COLINFO_TREE_ITEM_NODE(Salt, PercentAcid, PropertyNames::Salt::percentAcid),
)

COLUMN_INFOS(
   TreeItemNode<Yeast>,
   COLINFO_TREE_ITEM_NODE(Yeast, Name      , PropertyNames::NamedEntity::name), // "Name"
   COLINFO_TREE_ITEM_NODE(Yeast, Laboratory, PropertyNames::Yeast::laboratory), // "Laboratory"
   COLINFO_TREE_ITEM_NODE(Yeast, ProductId , PropertyNames::Yeast::productId ), // "Product ID"
   COLINFO_TREE_ITEM_NODE(Yeast, Type      , PropertyNames::Yeast::type      ), // "Type"
   COLINFO_TREE_ITEM_NODE(Yeast, Form      , PropertyNames::Yeast::form      ), // "Form"
)

COLUMN_INFOS(
   TreeItemNode<Style>,
   COLINFO_TREE_ITEM_NODE(Style, Name          , PropertyNames::NamedEntity::name    ), // "Name"
   COLINFO_TREE_ITEM_NODE(Style, Category      , PropertyNames::Style::category      ), // "Category"
   COLINFO_TREE_ITEM_NODE(Style, CategoryNumber, PropertyNames::Style::categoryNumber), // "Number"
   COLINFO_TREE_ITEM_NODE(Style, StyleLetter   , PropertyNames::Style::styleLetter   ), // "Letter"
   COLINFO_TREE_ITEM_NODE(Style, StyleGuide    , PropertyNames::Style::styleGuide    ), // "Guide"
)

COLUMN_INFOS(
   TreeItemNode<Water>,
   COLINFO_TREE_ITEM_NODE(Water, Name       , PropertyNames::NamedEntity::name     ), // "Name"
   COLINFO_TREE_ITEM_NODE(Water, Calcium    , PropertyNames::Water::calcium_ppm    ), // "Ca"
   COLINFO_TREE_ITEM_NODE(Water, Bicarbonate, PropertyNames::Water::bicarbonate_ppm), // "HCO3"
   COLINFO_TREE_ITEM_NODE(Water, Sulfate    , PropertyNames::Water::sulfate_ppm    ), // "SO4"
   COLINFO_TREE_ITEM_NODE(Water, Chloride   , PropertyNames::Water::chloride_ppm   ), // "Cl"
   COLINFO_TREE_ITEM_NODE(Water, Sodium     , PropertyNames::Water::sodium_ppm     ), // "Na"
   COLINFO_TREE_ITEM_NODE(Water, Magnesium  , PropertyNames::Water::magnesium_ppm  ), // "Mg"
   COLINFO_TREE_ITEM_NODE(Water, pH         , PropertyNames::Water::ph             ), // "pH"
)


template<> QString TreeItemNode<Recipe>::getToolTip() const {
   auto const style = this->m_underlyingItem->style();

   QString const header = getHeader();

   QString body   = "<body>";
   //body += QString("<h1>%1</h1>").arg(this->m_underlyingItem->getName()());
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1 (%2%3)</caption>")
         .arg( style ? style->name() : Recipe::tr("unknown style"))
         .arg( style ? style->categoryNumber() : Recipe::tr("N/A") )
         .arg( style ? style->styleLetter() : "" );

   // First row: OG and FG
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Recipe::tr("OG"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->og(), Measurement::Units::specificGravity}, 3));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Recipe::tr("FG"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->fg(), Measurement::Units::specificGravity}, 3));

   // Second row: Color and Bitterness.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td>")
           .arg(Recipe::tr("Color"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->color_srm(), Measurement::Units::srm}, 1))
           .arg(ColorMethods::formulaName());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td></tr>")
           .arg(Recipe::tr("IBU"))
           .arg(Measurement::displayQuantity(this->m_underlyingItem->IBU(), 1))
           .arg(IbuMethods::formulaName() );
   // Third row: DB ID
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Recipe::tr("ID in DB"))
           .arg(this->m_underlyingItem->key());
   body += QString("<td class=\"left\"> </td><td class=\"value\"> </td></tr>");

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<BrewLog>::getToolTip() const {
   QString const header = getHeader();
   QString body = "<body>";
   body += BrewLog::tr("Brew Log #%1 for brew on %2").arg(
      this->m_underlyingItem->key()
   ).arg(
      Localization::displayDate(this->m_underlyingItem->brewDate())
   );
   body += "</body></html>";
   return header + body;
}

template<> QString TreeItemNode<Style>::getToolTip() const {
   QString const header = getHeader();

   QString body = "<body>";
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>").arg(this->m_underlyingItem->name());

   // First row -- category and number (letter)
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Style::tr("Category"))
           .arg(this->m_underlyingItem->category());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2%3</td></tr>")
           .arg(Style::tr("Code"))
           .arg(this->m_underlyingItem->categoryNumber())
           .arg(this->m_underlyingItem->styleLetter());

   // Second row: guide and type
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Style::tr("Guide"))
           .arg(this->m_underlyingItem->styleGuide());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Style::tr("Type"))
           .arg(Style::typeDisplayNames[this->m_underlyingItem->type()]);

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Equipment>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );

   // First row -- batchsize and boil time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Equipment::tr("Preboil"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->kettleBoilSize_l(), Measurement::Units::liters}) );
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Equipment::tr("BoilTime"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->boilTime_min().value_or(Equipment::default_boilTime_mins), Measurement::Units::minutes}) );

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Mash>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- total time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Mash::tr("Total time (mins)"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->totalTime_mins(),
                                                               Measurement::Units::minutes}, 0));
   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<MashStep>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- step time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(MashStep::tr("Step time (mins)"))
           .arg(showOptionalMins(this->m_underlyingItem->stepTime_mins(), Measurement::Units::minutes));

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Boil>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- total time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Boil::tr("Boil time (mins)"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->boilTime_mins(),
                                                               Measurement::Units::minutes}, 0));

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<BoilStep>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- step time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(BoilStep::tr("Step time (mins)"))
           .arg(showOptionalMins(this->m_underlyingItem->stepTime_mins(), Measurement::Units::minutes));

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Fermentation>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- description
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Fermentation::tr("Description"))
           .arg(this->m_underlyingItem->description());

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<FermentationStep>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- step time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(FermentationStep::tr("Step time (days)"))
           .arg(showOptionalMins(this->m_underlyingItem->stepTime_mins(), Measurement::Units::days));

   body += "</table></body></html>";

   return header + body;
}

// Once we do inventory, this needs to be fixed to show amount on hand
template<> QString TreeItemNode<Fermentable>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );

   // First row -- type and color
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Fermentable::tr("Type"))
           .arg(Fermentable::typeDisplayNames[this->m_underlyingItem->type()]);
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Fermentable::tr("Color"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->color_srm(), Measurement::Units::srm}, 1));
   // Second row -- isMashed and yield?
//   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
//           .arg(Fermentable::tr("Mashed"))
//           .arg( fermentable->stage() == RecipeAddition::Stage::Mash ? Fermentable::tr("Yes") : Fermentable::tr("No") );
   body += QString("<tr><td class=\"left\">.</td><td class=\"value\">.</td>");
   auto const yield = this->m_underlyingItem->fineGrindYield_pct();
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Fermentable::tr("Extract Yield Dry Basis Fine Grind (DBFG)"))
           .arg(yield ? Measurement::displayQuantity(*yield, 3) : "?");

   body += "</table></body></html>";

   return header + body;
}

//
// TBD: For the moment, the TreeItemNode::getToolTip code for StockPurchaseFermentable, StockPurchaseHop, etc is just
//      copy-paste identical.  In time, we'll either want to tweak individual ones or merge them all.
//
template<> QString TreeItemNode<StockPurchaseFermentable>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};

   output <<
      "<body>"
        "<div id=\"headerdiv\">"
          "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateOrdered() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateOrdered()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateReceived() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateReceived()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountReceived() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountReceived(), 1) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountRemaining() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountRemaining(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";

   return outputString;
}

//
// TBD: For the moment, the TreeItemNode::getToolTip code for StockUseFermentable, StockUseHop, etc is just
//      copy-paste identical.  In time, we'll either want to tweak individual ones or merge them all.
//
template<> QString TreeItemNode<StockUseFermentable>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};
   output <<
      "<body>"
        "<div id=\"headerdiv\">"
        "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << Localization::displayDateUserFormated(this->m_underlyingItem->date()) << "</td>"
            "<td class=\"value\">" << StockUse::reasonDisplayNames[this->m_underlyingItem->reason()] << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountUsed(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";
   return outputString;
}

template<> QString TreeItemNode<StockPurchaseHop>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};

   output <<
      "<body>"
        "<div id=\"headerdiv\">"
          "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateOrdered() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateOrdered()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateReceived() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateReceived()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountReceived() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountReceived(), 1) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountRemaining() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountRemaining(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";

   return outputString;
}

template<> QString TreeItemNode<StockUseHop>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};
   output <<
      "<body>"
        "<div id=\"headerdiv\">"
        "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << Localization::displayDateUserFormated(this->m_underlyingItem->date()) << "</td>"
            "<td class=\"value\">" << StockUse::reasonDisplayNames[this->m_underlyingItem->reason()] << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountUsed(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";
   return outputString;
}

template<> QString TreeItemNode<StockPurchaseMisc>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};

   output <<
      "<body>"
        "<div id=\"headerdiv\">"
          "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateOrdered() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateOrdered()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateReceived() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateReceived()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountReceived() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountReceived(), 1) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountRemaining() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountRemaining(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";

   return outputString;
}

template<> QString TreeItemNode<StockUseMisc>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};
   output <<
      "<body>"
        "<div id=\"headerdiv\">"
        "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << Localization::displayDateUserFormated(this->m_underlyingItem->date()) << "</td>"
            "<td class=\"value\">" << StockUse::reasonDisplayNames[this->m_underlyingItem->reason()] << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountUsed(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";
   return outputString;
}

template<> QString TreeItemNode<StockPurchaseSalt>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};

   output <<
      "<body>"
        "<div id=\"headerdiv\">"
          "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateOrdered() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateOrdered()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateReceived() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateReceived()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountReceived() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountReceived(), 1) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountRemaining() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountRemaining(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";

   return outputString;
}

template<> QString TreeItemNode<StockUseSalt>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};
   output <<
      "<body>"
        "<div id=\"headerdiv\">"
        "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << Localization::displayDateUserFormated(this->m_underlyingItem->date()) << "</td>"
            "<td class=\"value\">" << StockUse::reasonDisplayNames[this->m_underlyingItem->reason()] << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountUsed(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";
   return outputString;
}

template<> QString TreeItemNode<StockPurchaseYeast>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};

   output <<
      "<body>"
        "<div id=\"headerdiv\">"
          "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateOrdered() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateOrdered()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchase::localisedName_dateReceived() << "</td>"
            "<td class=\"value\">" << Localization::displayDateUserFormated(this->m_underlyingItem->dateReceived()) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountReceived() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountReceived(), 1) << "</td>"
          "</tr>"
          "<tr>"
            "<td class=\"left\">" << StockPurchaseFermentable::localisedName_amountRemaining() << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountRemaining(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";

   return outputString;
}

template<> QString TreeItemNode<StockUseYeast>::getToolTip() const {
   QString outputString = getHeader();
   QTextStream output{&outputString};
   output <<
      "<body>"
        "<div id=\"headerdiv\">"
        "<caption>" << this->m_underlyingItem->name() << "</caption>"
        "</div>"
        "<table id=\"tooltip\">"
          "<tr>"
            "<td class=\"left\">" << Localization::displayDateUserFormated(this->m_underlyingItem->date()) << "</td>"
            "<td class=\"value\">" << StockUse::reasonDisplayNames[this->m_underlyingItem->reason()] << "</td>"
            "<td class=\"value\">" << Measurement::displayAmount(this->m_underlyingItem->amountUsed(), 1) << "</td>"
          "</tr>"
        "</table>"
      "</body>"
      "</html>";
   return outputString;
}

template<> QString TreeItemNode<Hop>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>").arg(this->m_underlyingItem->name());

   // First row -- alpha and beta
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Hop::tr("Alpha"))
           .arg(Measurement::displayQuantity(this->m_underlyingItem->alpha_pct(), 3));
   if (this->m_underlyingItem->beta_pct()) {
      body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td>")
            .arg(Hop::tr("Beta"))
            .arg(Measurement::displayQuantity(*this->m_underlyingItem->beta_pct(), 3));
   }
   body += QString("</tr>");

   // Second row -- form and type
   body += QString("<tr>");
   if (this->m_underlyingItem->form()) {
      body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
            .arg(Hop::tr("Form"))
            .arg(Hop::formDisplayNames[*this->m_underlyingItem->form()]);
   }
   if (this->m_underlyingItem->type()) {
      body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
            .arg(Hop::tr("Type"))
            .arg(Hop::typeDisplayNames[*this->m_underlyingItem->type()]);
   }
   body += QString("</tr>");
   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Misc>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- type and use
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Misc::tr("Type"))
           .arg(Misc::typeDisplayNames[this->m_underlyingItem->type()]);

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Salt>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- type and use
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Salt::tr("Type"))
           .arg(Salt::typeDisplayNames[this->m_underlyingItem->type()]);

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Yeast>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );

   // First row -- type and form
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Yeast::tr("Type"))
           .arg(Yeast::typeDisplayNames[this->m_underlyingItem->type()]);
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Yeast::tr("Form"))
           .arg(Yeast::formDisplayNames[this->m_underlyingItem->form()]);
   // Second row -- lab and attenuation
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Yeast::tr("Lab"))
           .arg(this->m_underlyingItem->laboratory());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2 %</td></tr>")
           .arg(Yeast::tr("Attenuation"))
           .arg(Measurement::displayQuantity(this->m_underlyingItem->attenuationTypical_pct(), 0));

   // Third row -- prod id and flocculation
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Yeast::tr("Id"))
           .arg(this->m_underlyingItem->productId());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Yeast::tr("Flocculation"))
           .arg(Yeast::flocculationDisplayNames[this->m_underlyingItem->flocculation()]);

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Water>::getToolTip() const {
   QString const header = getHeader();

   QString body   = "<body>";
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>").arg(this->m_underlyingItem->name());

   // First row -- Ca and Mg
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Water::tr("Ca"))
           .arg(this->m_underlyingItem->calcium_ppm());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Water::tr("Mg"))
           .arg(this->m_underlyingItem->magnesium_ppm());
   // Second row -- SO4 and Na
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Water::tr("SO<sub>4</sub>"))
           .arg(this->m_underlyingItem->sulfate_ppm());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Water::tr("Na"))
           .arg(this->m_underlyingItem->sodium_ppm());
   // third row -- Cl and HCO3
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Water::tr("Cl"))
           .arg(this->m_underlyingItem->chloride_ppm());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Water::tr("HCO<sub>3</sub>"))
           .arg( this->m_underlyingItem->bicarbonate_ppm());

   body += "</table></body></html>";

   return header + body;
}