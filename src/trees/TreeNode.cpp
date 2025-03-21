/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeNode.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include <QObject>
#include <QString>
#include <Qt>
#include <QVariant>
#include <QVector>

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
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "trees/RecipeTreeModel.h"
#include "trees/TreeModel.h"

void TreeNode::setShowMe(bool val) {
   this->m_showMe = val;
   return;
}

bool TreeNode::showMe() const {
   return this->m_showMe;
}

// NOTE: Each TreeItemNode<XYZ>::columnDisplayNames definition below should correspond with the columns defined in
//       TreeNodeTraits<XYZ, PQR>::ColumnIndex in trees/TreeNodeTraits.h.

template<> EnumStringMapping const TreeItemNode<Recipe>::columnDisplayNames {
   {TreeItemNode<Recipe>::ColumnIndex::Name             , Recipe::tr("Name"     )},
   {TreeItemNode<Recipe>::ColumnIndex::NumberOfAncestors, Recipe::tr("Snapshots")},
   {TreeItemNode<Recipe>::ColumnIndex::BrewDate         , Recipe::tr("Brew Date")},
   {TreeItemNode<Recipe>::ColumnIndex::Style            , Recipe::tr("Style"    )},
};

template<> EnumStringMapping const TreeItemNode<Equipment>::columnDisplayNames {
   {TreeItemNode<Equipment>::ColumnIndex::Name    , Equipment::tr("Name"     )},
   {TreeItemNode<Equipment>::ColumnIndex::BoilTime, Equipment::tr("Boil Time")},
};

template<> EnumStringMapping const TreeItemNode<Fermentable>::columnDisplayNames {
   {TreeItemNode<Fermentable>::ColumnIndex::Name , Fermentable::tr("Name" )},
   {TreeItemNode<Fermentable>::ColumnIndex::Type , Fermentable::tr("Type" )},
   {TreeItemNode<Fermentable>::ColumnIndex::Color, Fermentable::tr("Color")},
};

template<> EnumStringMapping const TreeItemNode<Hop>::columnDisplayNames {
   {TreeItemNode<Hop>::ColumnIndex::Name    , Hop::tr("Name"   )},
   {TreeItemNode<Hop>::ColumnIndex::Form    , Hop::tr("Type"   )},
   {TreeItemNode<Hop>::ColumnIndex::AlphaPct, Hop::tr("% Alpha")},
   {TreeItemNode<Hop>::ColumnIndex::Origin  , Hop::tr("Origin" )},
};

template<> EnumStringMapping const TreeItemNode<Mash>::columnDisplayNames {
   {TreeItemNode<Mash>::ColumnIndex::Name      , Mash::tr("Name"       )},
   {TreeItemNode<Mash>::ColumnIndex::TotalWater, Mash::tr("Total Water")},
   {TreeItemNode<Mash>::ColumnIndex::TotalTime , Mash::tr("Total Time" )},
};

template<> EnumStringMapping const TreeItemNode<Misc>::columnDisplayNames {
   {TreeItemNode<Misc>::ColumnIndex::Name, Misc::tr("Name")},
   {TreeItemNode<Misc>::ColumnIndex::Type, Misc::tr("Type")},
};

template<> EnumStringMapping const TreeItemNode<Salt>::columnDisplayNames {
   {TreeItemNode<Salt>::ColumnIndex::Name       , Salt::tr("Name"       )},
   {TreeItemNode<Salt>::ColumnIndex::Type       , Salt::tr("Type"       )},
   {TreeItemNode<Salt>::ColumnIndex::IsAcid     , Salt::tr("IsAcid"     )},
   {TreeItemNode<Salt>::ColumnIndex::PercentAcid, Salt::tr("PercentAcid")},
};

template<> EnumStringMapping const TreeItemNode<Yeast>::columnDisplayNames {
   {TreeItemNode<Yeast>::ColumnIndex::Name      , Yeast::tr("Name"      )},
   {TreeItemNode<Yeast>::ColumnIndex::Laboratory, Yeast::tr("Laboratory")},
   {TreeItemNode<Yeast>::ColumnIndex::ProductId , Yeast::tr("Product ID")},
   {TreeItemNode<Yeast>::ColumnIndex::Type      , Yeast::tr("Type"      )},
   {TreeItemNode<Yeast>::ColumnIndex::Form      , Yeast::tr("Form"      )},
};

template<> EnumStringMapping const TreeItemNode<Style>::columnDisplayNames {
   {TreeItemNode<Style>::ColumnIndex::Name          , Style::tr("Name"    )},
   {TreeItemNode<Style>::ColumnIndex::Category      , Style::tr("Category")},
   {TreeItemNode<Style>::ColumnIndex::CategoryNumber, Style::tr("Number"  )},
   {TreeItemNode<Style>::ColumnIndex::CategoryLetter, Style::tr("Letter"  )},
   {TreeItemNode<Style>::ColumnIndex::StyleGuide    , Style::tr("Guide"   )},
};

template<> EnumStringMapping const TreeItemNode<Water>::columnDisplayNames {
   {TreeItemNode<Water>::ColumnIndex::Name       , Water::tr("Name")},
   {TreeItemNode<Water>::ColumnIndex::Calcium    , Water::tr("Ca"  )},
   {TreeItemNode<Water>::ColumnIndex::Bicarbonate, Water::tr("HCO3")},
   {TreeItemNode<Water>::ColumnIndex::Sulfate    , Water::tr("SO4" )},
   {TreeItemNode<Water>::ColumnIndex::Chloride   , Water::tr("Cl"  )},
   {TreeItemNode<Water>::ColumnIndex::Sodium     , Water::tr("Na"  )},
   {TreeItemNode<Water>::ColumnIndex::Magnesium  , Water::tr("Mg"  )},
   {TreeItemNode<Water>::ColumnIndex::pH         , Water::tr("pH"  )},
};

// All folders have the same columns, so, for the moment, we just define the one with the shortest name.
// At some point, we might want to move this to the header file
template<> EnumStringMapping const TreeFolderNode<Hop>::columnDisplayNames {
   {TreeFolderNode<Hop>::ColumnIndex::Name    , Folder::tr("Name"    )},
   {TreeFolderNode<Hop>::ColumnIndex::Path    , Folder::tr("PATH"    )},
   {TreeFolderNode<Hop>::ColumnIndex::FullPath, Folder::tr("FULLPATH")},
};

template<> bool TreeItemNode<Recipe>::columnIsLessThan(TreeItemNode<Recipe> const & other,
                                                       TreeNodeTraits<Recipe>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;

   //
   // If two ancestor recipes share the same parent, we show them in reverse order of creation, regardless of what
   // column we are sorting by.
   //
   // We are safe to dereference rawParent() here because the root node is always a Folder and never a Recipe
   //
   if (this->rawParent() == other.rawParent() &&
       this->rawParent()->classifier() == TreeNodeClassifier::PrimaryItem) {
      return lhs.key() > rhs.key();
   }

   switch (column) {
      case TreeItemNode<Recipe>::ColumnIndex::Name:
         return lhs.name() < rhs.name();

      case TreeItemNode<Recipe>::ColumnIndex::BrewDate:
         return lhs.date() < rhs.date();

      case TreeItemNode<Recipe>::ColumnIndex::Style:
         if (!lhs.style()) {
            return true;
         }
         if (!rhs.style()) {
            return false;
         }
         return lhs.style()->name() < rhs.style()->name();

      case TreeItemNode<Recipe>::ColumnIndex::NumberOfAncestors:
         return lhs.ancestors().length() < rhs.ancestors().length();
   }

//   std::unreachable();
}

template<> bool TreeItemNode<BrewNote>::columnIsLessThan(TreeItemNode<BrewNote> const & other,
                                                         TreeNodeTraits<BrewNote, Recipe>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<BrewNote>::ColumnIndex::BrewDate:
         return lhs.brewDate() < rhs.brewDate();
   }

//   std::unreachable();
}

template<> bool TreeItemNode<Equipment>::columnIsLessThan(TreeItemNode<Equipment> const & other,
                                                          TreeNodeTraits<Equipment>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Equipment>::ColumnIndex::Name:
         return lhs.name() < rhs.name();

      case TreeItemNode<Equipment>::ColumnIndex::BoilTime:
         return lhs.boilTime_min().value_or(Equipment::default_boilTime_mins) <
                rhs.boilTime_min().value_or(Equipment::default_boilTime_mins);
   }

//   std::unreachable();
}

template<> bool TreeItemNode<Fermentable>::columnIsLessThan(TreeItemNode<Fermentable> const & other,
                                                            TreeNodeTraits<Fermentable>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Fermentable>::ColumnIndex::Name : return lhs.name()      < rhs.name();
      case TreeItemNode<Fermentable>::ColumnIndex::Type : return lhs.type()      < rhs.type();
      case TreeItemNode<Fermentable>::ColumnIndex::Color: return lhs.color_srm() < rhs.color_srm();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Hop>::columnIsLessThan(TreeItemNode<Hop> const & other,
                                                    TreeNodeTraits<Hop>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Hop>::ColumnIndex::Name    : return lhs.name()      < rhs.name();
      case TreeItemNode<Hop>::ColumnIndex::Form    : return lhs.form()      < rhs.form();
      case TreeItemNode<Hop>::ColumnIndex::AlphaPct: return lhs.alpha_pct() < rhs.alpha_pct();
      case TreeItemNode<Hop>::ColumnIndex::Origin  : return lhs.origin()    < rhs.origin();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Mash>::columnIsLessThan(TreeItemNode<Mash> const & other,
                                                     TreeNodeTraits<Mash>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Mash>::ColumnIndex::Name      : return lhs.name() < rhs.name();
      case TreeItemNode<Mash>::ColumnIndex::TotalWater: return lhs.totalMashWater_l() < rhs.totalMashWater_l();
      case TreeItemNode<Mash>::ColumnIndex::TotalTime : return lhs.totalTime_mins()   < rhs.totalTime_mins();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Misc>::columnIsLessThan(TreeItemNode<Misc> const & other,
                                                     TreeNodeTraits<Misc>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Misc>::ColumnIndex::Name: return lhs.name() < rhs.name();
      case TreeItemNode<Misc>::ColumnIndex::Type: return lhs.type() < rhs.type();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Salt>::columnIsLessThan(TreeItemNode<Salt> const & other,
                                                     TreeNodeTraits<Salt>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Salt>::ColumnIndex::Name       : return lhs.name()        < rhs.name();
      case TreeItemNode<Salt>::ColumnIndex::Type       : return lhs.type()        < rhs.type();
      case TreeItemNode<Salt>::ColumnIndex::IsAcid     : return lhs.isAcid()      < rhs.isAcid();
      case TreeItemNode<Salt>::ColumnIndex::PercentAcid: return lhs.percentAcid() < rhs.percentAcid();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Style>::columnIsLessThan(TreeItemNode<Style> const & other,
                                                      TreeNodeTraits<Style>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Style>::ColumnIndex::Name          : return lhs.name()           < rhs.name();
      case TreeItemNode<Style>::ColumnIndex::Category      : return lhs.category()       < rhs.category();
      case TreeItemNode<Style>::ColumnIndex::CategoryNumber: return lhs.categoryNumber() < rhs.categoryNumber();
      case TreeItemNode<Style>::ColumnIndex::CategoryLetter: return lhs.styleLetter()    < rhs.styleLetter();
      case TreeItemNode<Style>::ColumnIndex::StyleGuide    : return lhs.styleGuide()     < rhs.styleGuide();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Yeast>::columnIsLessThan(TreeItemNode<Yeast> const & other,
                                                      TreeNodeTraits<Yeast>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Yeast>::ColumnIndex::Name      : return lhs.name()       < rhs.name();
      case TreeItemNode<Yeast>::ColumnIndex::Laboratory: return lhs.laboratory() < rhs.laboratory();
      case TreeItemNode<Yeast>::ColumnIndex::ProductId : return lhs.productId()  < rhs.productId();
      case TreeItemNode<Yeast>::ColumnIndex::Type      : return lhs.type()       < rhs.type();
      case TreeItemNode<Yeast>::ColumnIndex::Form      : return lhs.form()       < rhs.form();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Water>::columnIsLessThan(TreeItemNode<Water> const & other,
                                                      TreeNodeTraits<Water>::ColumnIndex column) const {
   auto const & lhs = *this->m_underlyingItem;
   auto const & rhs = *other.m_underlyingItem;
   switch (column) {
      case TreeItemNode<Water>::ColumnIndex::Name       : return lhs.name()            < rhs.name();
      case TreeItemNode<Water>::ColumnIndex::pH         : return lhs.ph()              < rhs.ph();
      case TreeItemNode<Water>::ColumnIndex::Bicarbonate: return lhs.bicarbonate_ppm() < rhs.bicarbonate_ppm();
      case TreeItemNode<Water>::ColumnIndex::Sulfate    : return lhs.sulfate_ppm()     < rhs.sulfate_ppm();
      case TreeItemNode<Water>::ColumnIndex::Chloride   : return lhs.chloride_ppm()    < rhs.chloride_ppm();
      case TreeItemNode<Water>::ColumnIndex::Sodium     : return lhs.sodium_ppm()      < rhs.sodium_ppm();
      case TreeItemNode<Water>::ColumnIndex::Magnesium  : return lhs.magnesium_ppm()   < rhs.magnesium_ppm();
      case TreeItemNode<Water>::ColumnIndex::Calcium    : return lhs.calcium_ppm()     < rhs.calcium_ppm();
   }
   return lhs.name() < rhs.name();
}

template<> QString TreeItemNode<Recipe>::getToolTip() const {
   auto style = this->m_underlyingItem->style();

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";
   //body += QString("<h1>%1</h1>").arg(this->m_underlyingItem->getName()());
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1 (%2%3)</caption>")
         .arg( style ? style->name() : Recipe::tr("unknown style"))
         .arg( style ? style->categoryNumber() : Recipe::tr("N/A") )
         .arg( style ? style->styleLetter() : "" );

   // Third row: OG and FG
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Recipe::tr("OG"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->og(), Measurement::Units::specificGravity}, 3));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(Recipe::tr("FG"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->fg(), Measurement::Units::specificGravity}, 3));

   // Fourth row: Color and Bitterness.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td>")
           .arg(Recipe::tr("Color"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->color_srm(), Measurement::Units::srm}, 1))
           .arg(ColorMethods::colorFormulaName());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td></tr>")
           .arg(Recipe::tr("IBU"))
           .arg(Measurement::displayQuantity(this->m_underlyingItem->IBU(), 1))
           .arg(IbuMethods::ibuFormulaName() );

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<BrewNote>::getToolTip() const {
   return Localization::displayDate(this->m_underlyingItem->brewDate());
}

template<> QString TreeItemNode<Style>::getToolTip() const {
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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

// Once we do inventory, this needs to be fixed to show amount on hand
template<> QString TreeItemNode<Fermentable>::getToolTip() const {
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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

template<> QString TreeItemNode<Hop>::getToolTip() const {
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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

template<> QString TreeItemNode<Mash>::getToolTip() const {
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( this->m_underlyingItem->name() );
   // First row -- total time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(Mash::tr("Total time (mins)"))
           .arg(Measurement::displayAmount(Measurement::Amount{this->m_underlyingItem->totalTime_mins(), Measurement::Units::minutes}) );

   body += "</table></body></html>";

   return header + body;
}

template<> QString TreeItemNode<Misc>::getToolTip() const {
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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
   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

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
