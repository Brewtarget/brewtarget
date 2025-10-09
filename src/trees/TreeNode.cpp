/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeNode.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Folder.h"
#include "model/Hop.h"
#include "model/InventoryFermentable.h"
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
   TREE_NODE_HEADER(TreeItemNode, Recipe, Name             , tr("Name"     ), PropertyNames::NamedEntity::name        ),
   TREE_NODE_HEADER(TreeItemNode, Recipe, NumberOfAncestors, tr("Snapshots"), PropertyNames::Recipe     ::numAncestors),
   TREE_NODE_HEADER(TreeItemNode, Recipe, BrewDate         , tr("Date"     ), PropertyNames::Recipe     ::date        ),
   TREE_NODE_HEADER(TreeItemNode, Recipe, Style            , tr("Style"    ), {PropertyNames::Recipe::style,
                                                                               PropertyNames::NamedEntity::name}      ),
)

COLUMN_INFOS(
   TreeItemNode<BrewNote>,
   TREE_NODE_HEADER(TreeItemNode, BrewNote, BrewDate, tr("Date"), PropertyNames::BrewNote::brewDate),
)

COLUMN_INFOS(
   TreeItemNode<Equipment>,
   TREE_NODE_HEADER(TreeItemNode, Equipment, Name        , tr("Name"      ), PropertyNames::NamedEntity::name              ),
   TREE_NODE_HEADER(TreeItemNode, Equipment, BoilSize    , tr("Boil Size" ), PropertyNames::Equipment::kettleBoilSize_l    ),
   TREE_NODE_HEADER(TreeItemNode, Equipment, BatchSize   , tr("Batch Size"), PropertyNames::Equipment::fermenterBatchSize_l),
)

COLUMN_INFOS(
   TreeItemNode<Mash>,
   TREE_NODE_HEADER(TreeItemNode, Mash, Name      , tr("Name"       ), PropertyNames::NamedEntity::name     ),
   TREE_NODE_HEADER(TreeItemNode, Mash, TotalWater, tr("Total Water"), PropertyNames::Mash::totalMashWater_l),
   TREE_NODE_HEADER(TreeItemNode, Mash, TotalTime , tr("Total Time" ), PropertyNames::Mash::totalTime_mins  ),
)

COLUMN_INFOS(
   TreeItemNode<MashStep>,
   TREE_NODE_HEADER(TreeItemNode, MashStep, Name    , tr("Name"     ), PropertyNames::NamedEntity::name      ),
   TREE_NODE_HEADER(TreeItemNode, MashStep, StepTime, tr("Step Time"), PropertyNames::StepBase::stepTime_mins),
)

COLUMN_INFOS(
   TreeItemNode<Boil>,
   TREE_NODE_HEADER(TreeItemNode, Boil, Name              , tr("Name"           ), PropertyNames::NamedEntity::name  ),
   TREE_NODE_HEADER(TreeItemNode, Boil, PreBoilSize       , tr("Pre-Boil Size"  ), PropertyNames::Boil::preBoilSize_l),
   TREE_NODE_HEADER(TreeItemNode, Boil, LengthOfBoilProper, tr("Time At Boiling"), PropertyNames::Boil::boilTime_mins),
)

COLUMN_INFOS(
   TreeItemNode<BoilStep>,
   TREE_NODE_HEADER(TreeItemNode, BoilStep, Name    , tr("Name"     ), PropertyNames::NamedEntity::name      ),
   TREE_NODE_HEADER(TreeItemNode, BoilStep, StepTime, tr("Step Time"), PropertyNames::StepBase::stepTime_mins),
)

COLUMN_INFOS(
   TreeItemNode<Fermentation>,
   TREE_NODE_HEADER(TreeItemNode, Fermentation, Name       , tr("Name"       ), PropertyNames::NamedEntity::name        ),
   TREE_NODE_HEADER(TreeItemNode, Fermentation, Description, tr("Description"), PropertyNames::Fermentation::description),
)

COLUMN_INFOS(
   TreeItemNode<FermentationStep>,
   TREE_NODE_HEADER(TreeItemNode, FermentationStep, Name    , tr("Name"     ), PropertyNames::NamedEntity::name      ),
   TREE_NODE_HEADER(TreeItemNode, FermentationStep, StepTime, tr("Step Time"), PropertyNames::StepBase::stepTime_days), // NB Days not Mins for fermentation steps
)

COLUMN_INFOS(
   TreeItemNode<Fermentable>,
   TREE_NODE_HEADER(TreeItemNode, Fermentable, Name , tr("Name" ), PropertyNames::NamedEntity::name     ),
   TREE_NODE_HEADER(TreeItemNode, Fermentable, Type , tr("Type" ), PropertyNames::Fermentable::type     ),
   TREE_NODE_HEADER(TreeItemNode, Fermentable, Color, tr("Color"), PropertyNames::Fermentable::color_srm),
)

COLUMN_INFOS(
   TreeItemNode<Hop>,
   TREE_NODE_HEADER(TreeItemNode, Hop, Name    , tr("Name"   ), PropertyNames::NamedEntity::name),
   TREE_NODE_HEADER(TreeItemNode, Hop, Form    , tr("Type"   ), PropertyNames::Hop::form        ),
   TREE_NODE_HEADER(TreeItemNode, Hop, AlphaPct, tr("% Alpha"), PropertyNames::Hop::alpha_pct   ),
   TREE_NODE_HEADER(TreeItemNode, Hop, Origin  , tr("Origin" ), PropertyNames::Hop::origin      ),
)

COLUMN_INFOS(
   TreeItemNode<InventoryFermentable>,
   TREE_NODE_HEADER(TreeItemNode, InventoryFermentable, Name    , tr("Name"   ), {PropertyNames::InventoryFermentable::fermentable,
                                                                                  PropertyNames::NamedEntity::name}       ),
   TREE_NODE_HEADER(TreeItemNode, InventoryFermentable, DateOrdered    , tr("Date Ordered"    ), PropertyNames::Inventory::dateOrdered),
   TREE_NODE_HEADER(TreeItemNode, InventoryFermentable, Type           , tr("Type"            ), {PropertyNames::InventoryFermentable::fermentable,
                                                                                                  PropertyNames::Fermentable::type}),
   TREE_NODE_HEADER(TreeItemNode, InventoryFermentable, AmountReceived , tr("Amount Received" ), PropertyNames::InventoryBase::amountReceived ),
   TREE_NODE_HEADER(TreeItemNode, InventoryFermentable, AmountRemaining, tr("Amount Remaining"), PropertyNames::InventoryBase::amountRemaining),
)

COLUMN_INFOS(
   TreeItemNode<Misc>,
   TREE_NODE_HEADER(TreeItemNode, Misc, Name, tr("Name"), PropertyNames::NamedEntity::name),
   TREE_NODE_HEADER(TreeItemNode, Misc, Type, tr("Type"), PropertyNames::Misc::type       ),
)

COLUMN_INFOS(
   TreeItemNode<Salt>,
   TREE_NODE_HEADER(TreeItemNode, Salt, Name       , tr("Name"       ), PropertyNames::NamedEntity::name),
   TREE_NODE_HEADER(TreeItemNode, Salt, Type       , tr("Type"       ), PropertyNames::Salt::type       ),
   TREE_NODE_HEADER(TreeItemNode, Salt, IsAcid     , tr("IsAcid"     ), PropertyNames::Salt::isAcid     ),
   TREE_NODE_HEADER(TreeItemNode, Salt, PercentAcid, tr("PercentAcid"), PropertyNames::Salt::percentAcid),
)

COLUMN_INFOS(
   TreeItemNode<Yeast>,
   TREE_NODE_HEADER(TreeItemNode, Yeast, Name      , tr("Name"      ), PropertyNames::NamedEntity::name),
   TREE_NODE_HEADER(TreeItemNode, Yeast, Laboratory, tr("Laboratory"), PropertyNames::Yeast::laboratory),
   TREE_NODE_HEADER(TreeItemNode, Yeast, ProductId , tr("Product ID"), PropertyNames::Yeast::productId ),
   TREE_NODE_HEADER(TreeItemNode, Yeast, Type      , tr("Type"      ), PropertyNames::Yeast::type      ),
   TREE_NODE_HEADER(TreeItemNode, Yeast, Form      , tr("Form"      ), PropertyNames::Yeast::form      ),
)

COLUMN_INFOS(
   TreeItemNode<Style>,
   TREE_NODE_HEADER(TreeItemNode, Style, Name          , tr("Name"    ), PropertyNames::NamedEntity::name    ),
   TREE_NODE_HEADER(TreeItemNode, Style, Category      , tr("Category"), PropertyNames::Style::category      ),
   TREE_NODE_HEADER(TreeItemNode, Style, CategoryNumber, tr("Number"  ), PropertyNames::Style::categoryNumber),
   TREE_NODE_HEADER(TreeItemNode, Style, StyleLetter   , tr("Letter"  ), PropertyNames::Style::styleLetter   ),
   TREE_NODE_HEADER(TreeItemNode, Style, StyleGuide    , tr("Guide"   ), PropertyNames::Style::styleGuide    ),
)

COLUMN_INFOS(
   TreeItemNode<Water>,
   TREE_NODE_HEADER(TreeItemNode, Water, Name       , tr("Name"), PropertyNames::NamedEntity::name     ),
   TREE_NODE_HEADER(TreeItemNode, Water, Calcium    , tr("Ca"  ), PropertyNames::Water::calcium_ppm    ),
   TREE_NODE_HEADER(TreeItemNode, Water, Bicarbonate, tr("HCO3"), PropertyNames::Water::bicarbonate_ppm),
   TREE_NODE_HEADER(TreeItemNode, Water, Sulfate    , tr("SO4" ), PropertyNames::Water::sulfate_ppm    ),
   TREE_NODE_HEADER(TreeItemNode, Water, Chloride   , tr("Cl"  ), PropertyNames::Water::chloride_ppm   ),
   TREE_NODE_HEADER(TreeItemNode, Water, Sodium     , tr("Na"  ), PropertyNames::Water::sodium_ppm     ),
   TREE_NODE_HEADER(TreeItemNode, Water, Magnesium  , tr("Mg"  ), PropertyNames::Water::magnesium_ppm  ),
   TREE_NODE_HEADER(TreeItemNode, Water, pH         , tr("pH"  ), PropertyNames::Water::ph             ),
)


template<> QString TreeItemNode<Recipe>::getToolTip() const {
   auto style = this->m_underlyingItem->style();

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

template<> QString TreeItemNode<BrewNote>::getToolTip() const {
   QString const header = getHeader();
   QString body = "<body>";
   body += BrewNote::tr("Brew Note #%1 for brew on %2").arg(
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

template<> QString TreeItemNode<InventoryFermentable>::getToolTip() const {
   // TODO: This is placeholder
   QString const header = getHeader();

   QString body   = "<body>";
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   body += "</table></body></html>";

   return header + body;
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
