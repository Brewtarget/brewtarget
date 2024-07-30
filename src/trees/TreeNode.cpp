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

#include "model/Folder.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"
#include "trees/TreeModel.h"


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

template<> EnumStringMapping const TreeItemNode<Misc>::columnDisplayNames {
   {TreeItemNode<Misc>::ColumnIndex::Name, Misc::tr("Name")},
   {TreeItemNode<Misc>::ColumnIndex::Type, Misc::tr("Type")},
};

template<> EnumStringMapping const TreeItemNode<Yeast>::columnDisplayNames {
   {TreeItemNode<Yeast>::ColumnIndex::Laboratory, Yeast::tr("Laboratory")},
   {TreeItemNode<Yeast>::ColumnIndex::Name      , Yeast::tr("Name"      )},
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

template<> bool TreeItemNode<Recipe>::isLessThan(TreeModel const & model,
                                                 QModelIndex const & left,
                                                 QModelIndex const & right,
                                                 TreeItemTraits<TreeItemNode<Recipe>>::ColumnIndex section,
                                                 Recipe const & lhs,
                                                 Recipe const & rhs) {
   if (model.showChild(left) && model.showChild(right)) {
      return lhs.key() > rhs.key();
   }

   switch (section) {
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

   // Default will be to just do a name sort. This doesn't likely make sense, but it will prevent a lot of warnings.
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Equipment>::isLessThan([[maybe_unused]] TreeModel const & model,
                                                    [[maybe_unused]] QModelIndex const & left,
                                                    [[maybe_unused]] QModelIndex const & right,
                                                    TreeItemTraits<TreeItemNode<Equipment>>::ColumnIndex section,
                                                    Equipment const & lhs,
                                                    Equipment const & rhs) {
   switch (section) {
      case TreeItemNode<Equipment>::ColumnIndex::Name:
         return lhs.name() < rhs.name();

      case TreeItemNode<Equipment>::ColumnIndex::BoilTime:
         return lhs.boilTime_min().value_or(Equipment::default_boilTime_mins) < rhs.boilTime_min().value_or(Equipment::default_boilTime_mins);
   }

   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Fermentable>::isLessThan([[maybe_unused]] TreeModel const & model,
                                                      [[maybe_unused]] QModelIndex const & left,
                                                      [[maybe_unused]] QModelIndex const & right,
                                                      TreeItemTraits<TreeItemNode<Fermentable>>::ColumnIndex section,
                                                      Fermentable const & lhs,
                                                      Fermentable const & rhs) {
   switch (section) {
      case TreeItemNode<Fermentable>::ColumnIndex::Name : return lhs.name()      < rhs.name();
      case TreeItemNode<Fermentable>::ColumnIndex::Type : return lhs.type()      < rhs.type();
      case TreeItemNode<Fermentable>::ColumnIndex::Color: return lhs.color_srm() < rhs.color_srm();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Hop>::isLessThan([[maybe_unused]] TreeModel const & model,
                                              [[maybe_unused]] QModelIndex const & left,
                                              [[maybe_unused]] QModelIndex const & right,
                                              TreeItemTraits<TreeItemNode<Hop>>::ColumnIndex section,
                                              Hop const & lhs,
                                              Hop const & rhs) {
   switch (section) {
      case TreeItemNode<Hop>::ColumnIndex::Name    : return lhs.name()      < rhs.name();
      case TreeItemNode<Hop>::ColumnIndex::Form    : return lhs.form()      < rhs.form();
      case TreeItemNode<Hop>::ColumnIndex::AlphaPct: return lhs.alpha_pct() < rhs.alpha_pct();
      case TreeItemNode<Hop>::ColumnIndex::Origin  : return lhs.origin()    < rhs.origin();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Misc>::isLessThan([[maybe_unused]] TreeModel const & model,
                                               [[maybe_unused]] QModelIndex const & left,
                                               [[maybe_unused]] QModelIndex const & right,
                                               TreeItemTraits<TreeItemNode<Misc>>::ColumnIndex section,
                                               Misc const & lhs,
                                               Misc const & rhs) {
   switch (section) {
      case TreeItemNode<Misc>::ColumnIndex::Name: return lhs.name() < rhs.name();
      case TreeItemNode<Misc>::ColumnIndex::Type: return lhs.type() < rhs.type();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Style>::isLessThan([[maybe_unused]] TreeModel const & model,
                                                [[maybe_unused]] QModelIndex const & left,
                                                [[maybe_unused]] QModelIndex const & right,
                                                TreeItemTraits<TreeItemNode<Style>>::ColumnIndex section,
                                                Style const & lhs,
                                                Style const & rhs) {
   switch (section) {
      case TreeItemNode<Style>::ColumnIndex::Name          : return lhs.name()           < rhs.name();
      case TreeItemNode<Style>::ColumnIndex::Category      : return lhs.category()       < rhs.category();
      case TreeItemNode<Style>::ColumnIndex::CategoryNumber: return lhs.categoryNumber() < rhs.categoryNumber();
      case TreeItemNode<Style>::ColumnIndex::CategoryLetter: return lhs.styleLetter()    < rhs.styleLetter();
      case TreeItemNode<Style>::ColumnIndex::StyleGuide    : return lhs.styleGuide()     < rhs.styleGuide();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Yeast>::isLessThan([[maybe_unused]] TreeModel const & model,
                                                [[maybe_unused]] QModelIndex const & left,
                                                [[maybe_unused]] QModelIndex const & right,
                                                TreeItemTraits<TreeItemNode<Yeast>>::ColumnIndex section,
                                                Yeast const & lhs,
                                                Yeast const & rhs) {
   switch (section) {
      case TreeItemNode<Yeast>::ColumnIndex::Laboratory: return lhs.laboratory() < rhs.laboratory();
      case TreeItemNode<Yeast>::ColumnIndex::Name      : return lhs.name()       < rhs.name();
      case TreeItemNode<Yeast>::ColumnIndex::Type      : return lhs.type()       < rhs.type();
      case TreeItemNode<Yeast>::ColumnIndex::Form      : return lhs.form()       < rhs.form();
   }
   return lhs.name() < rhs.name();
}

template<> bool TreeItemNode<Water>::isLessThan([[maybe_unused]] TreeModel const & model,
                                                [[maybe_unused]] QModelIndex const & left,
                                                [[maybe_unused]] QModelIndex const & right,
                                                TreeItemTraits<TreeItemNode<Water>>::ColumnIndex section,
                                                Water const & lhs,
                                                Water const & rhs) {
   switch (section) {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
   EnumStringMapping const itemTypeToName {
      {TreeNode::Type::Recipe     , "Recipe"     },
      {TreeNode::Type::Equipment  , "Equipment"  },
      {TreeNode::Type::Fermentable, "Fermentable"},
      {TreeNode::Type::Hop        , "Hop"        },
      {TreeNode::Type::Misc       , "Misc"       },
      {TreeNode::Type::Yeast      , "Yeast"      },
      {TreeNode::Type::BrewNote   , "BrewNote"   },
      {TreeNode::Type::Style      , "Style"      },
      {TreeNode::Type::Folder     , "Folder"     },
      {TreeNode::Type::Water      , "Water"      },
   };
}

template<> TreeNode::Type TreeNode::typeOf<Recipe>()      { return TreeNode::Type::Recipe;      }
template<> TreeNode::Type TreeNode::typeOf<Equipment>()   { return TreeNode::Type::Equipment;   }
template<> TreeNode::Type TreeNode::typeOf<Fermentable>() { return TreeNode::Type::Fermentable; }
template<> TreeNode::Type TreeNode::typeOf<Hop>()         { return TreeNode::Type::Hop;         }
template<> TreeNode::Type TreeNode::typeOf<Misc>()        { return TreeNode::Type::Misc;        }
template<> TreeNode::Type TreeNode::typeOf<Yeast>()       { return TreeNode::Type::Yeast;       }
template<> TreeNode::Type TreeNode::typeOf<BrewNote>()    { return TreeNode::Type::BrewNote;    }
template<> TreeNode::Type TreeNode::typeOf<Style>()       { return TreeNode::Type::Style;       }
template<> TreeNode::Type TreeNode::typeOf<Folder>()    { return TreeNode::Type::Folder;      }
template<> TreeNode::Type TreeNode::typeOf<Water>()       { return TreeNode::Type::Water;       }


bool operator==(TreeNode & lhs, TreeNode & rhs) {
   // Things of different types are not equal
   if (lhs.nodeType != rhs.nodeType) {
      return false;
   }

   return lhs.data(0) == rhs.data(0);
}

TreeNode::TreeNode(TreeNode::Type nodeType, TreeNode * parent) :
   parentItem{parent},
   nodeType{nodeType},
   m_thing{nullptr},
   m_showMe{false} {
   return;
}

TreeNode::~TreeNode() {
   qDeleteAll(this->childItems);
}

TreeNode * TreeNode::child(int number) {
   if (number < this->childItems.count()) {
      return this->childItems.value(number);
   }

   return nullptr;
}

TreeNode * TreeNode::parent() {
   return parentItem;
}

TreeNode::Type TreeNode::type() const {
   return this->nodeType;
}

int TreeNode::childCount() const {
   return this->childItems.count();
}

int TreeNode::columnCount(TreeNode::Type nodeType) const {
   switch (nodeType) {
      case TreeNode::Type::Recipe:      return static_cast<int>(TreeItemNode<     Recipe>::Info::NumberOfColumns);
      case TreeNode::Type::Equipment:   return static_cast<int>(TreeItemNode<  Equipment>::Info::NumberOfColumns);
      case TreeNode::Type::Fermentable: return static_cast<int>(TreeItemNode<Fermentable>::Info::NumberOfColumns);
      case TreeNode::Type::Hop:         return static_cast<int>(TreeItemNode<        Hop>::Info::NumberOfColumns);
      case TreeNode::Type::Misc:        return static_cast<int>(TreeItemNode<       Misc>::Info::NumberOfColumns);
      case TreeNode::Type::Yeast:       return static_cast<int>(TreeItemNode<      Yeast>::Info::NumberOfColumns);
      case TreeNode::Type::Style:       return static_cast<int>(TreeItemNode<      Style>::Info::NumberOfColumns);
      case TreeNode::Type::BrewNote:    return static_cast<int>(TreeItemNode<   BrewNote>::Info::NumberOfColumns);
      // All folders have the same columns, so it's a bit arbitrary which one we use here
      case TreeNode::Type::Folder:      return static_cast<int>(TreeFolderNode<      Hop>::Info::NumberOfColumns);
      case TreeNode::Type::Water:       return static_cast<int>(TreeItemNode<      Water>::Info::NumberOfColumns);
      default:
         qWarning() << Q_FUNC_INFO << "Bad column:" << static_cast<int>(nodeType);
         return 0;
   }
}

QVariant TreeNode::data(/*TreeNode::Type nodeType, */int column) {

   switch (this->nodeType) {
      case TreeNode::Type::Recipe:      return dataRecipe     (column);
      case TreeNode::Type::Equipment:   return dataEquipment  (column);
      case TreeNode::Type::Fermentable: return dataFermentable(column);
      case TreeNode::Type::Hop:         return dataHop        (column);
      case TreeNode::Type::Misc:        return dataMisc       (column);
      case TreeNode::Type::Yeast:       return dataYeast      (column);
      case TreeNode::Type::Style:       return dataStyle      (column);
      case TreeNode::Type::BrewNote:    return dataBrewNote   (column);
      case TreeNode::Type::Folder:      return dataFolder     (column);
      case TreeNode::Type::Water:       return dataWater      (column);
      default:
         qWarning() << Q_FUNC_INFO << "Bad column:" << static_cast<int>(nodeType);
         return QVariant();
   }
}

int TreeNode::childNumber() const {
   if (this->parentItem) {
      return parentItem->childItems.indexOf(const_cast<TreeNode *>(this));
   }
   return 0;
}

void TreeNode::setData(TreeNode::Type t, QObject * d) {
   this->m_thing = d;
   this->nodeType = t;
}

bool TreeNode::insertChildren(int position, int count, TreeNode::Type nodeType) {
//   qDebug() <<
//      Q_FUNC_INFO << "Inserting" << count << "children of type" << nodeType << "(" <<
//      this->itemTypeToString(static_cast<TreeNode::Type>(nodeType)) << ") at position" << position;
   if (position < 0  || position > this->childItems.size()) {
      qWarning() << Q_FUNC_INFO << "Position" << position << "outside range (0, " << this->childItems.size() << ")";
      return false;
   }

   for (int row = 0; row < count; ++row) {
      TreeNode * newItem = new TreeNode(nodeType, this);
      this->childItems.insert(position + row, newItem);
   }

   return true;
}

bool TreeNode::removeChildren(int position, int count) {
   if (position < 0 || position + count > this->childItems.count()) {
      return false;
   }

   for (int row = 0; row < count; ++row) {
      delete this->childItems.takeAt(position);
   }
   // FIXME: memory leak here. With delete, it's a concurrency/memory
   // access error, due to the fact that these pointers are floating around.
   //childItems.takeAt(position);

   return true;
}

QVariant TreeNode::dataRecipe(int column) {
   Recipe * recipe = qobject_cast<Recipe *>(this->m_thing);
   switch (static_cast<TreeItemNode<Recipe>::ColumnIndex>(column)) {
      case TreeItemNode<Recipe>::ColumnIndex::Name:
         if (!this->m_thing) {
            return QVariant(QObject::tr("Recipes"));
         } else {
            return QVariant(recipe->name());
         }
      case TreeItemNode<Recipe>::ColumnIndex::NumberOfAncestors:
         if (recipe) {
            return QVariant(recipe->ancestors().size());
         }
         break;
      case TreeItemNode<Recipe>::ColumnIndex::BrewDate:
         if (recipe && recipe->date()) {
            return Localization::displayDateUserFormated(*recipe->date());
         }
         break;
      case TreeItemNode<Recipe>::ColumnIndex::Style:
         if (recipe && recipe->style()) {
            return QVariant(recipe->style()->name());
         }
         break;
      default :
         qWarning() << QString("TreeNode::dataRecipe Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant TreeNode::dataEquipment(int column) {
   Equipment * kit = qobject_cast<Equipment *>(this->m_thing);
   switch (static_cast<TreeItemNode<Equipment>::ColumnIndex>(column)) {
      case TreeItemNode<Equipment>::ColumnIndex::Name:
         if (! kit) {
            return QVariant(QObject::tr("Equipment"));
         } else {
            return QVariant(kit->name());
         }
      case TreeItemNode<Equipment>::ColumnIndex::BoilTime:
         if (kit) {
            return QVariant::fromValue(kit->boilTime_min());
         }
         break;
      default :
         qWarning() << QString("TreeNode::dataEquipment Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant TreeNode::dataFermentable(int column) {
   Fermentable * ferm = qobject_cast<Fermentable *>(this->m_thing);

   switch (static_cast<TreeItemNode<Fermentable>::ColumnIndex>(column)) {
      case TreeItemNode<Fermentable>::ColumnIndex::Name:
         if (ferm) {
            return QVariant(ferm->name());
         } else {
            return QVariant(QObject::tr("Fermentables"));
         }
      case TreeItemNode<Fermentable>::ColumnIndex::Type:
         if (ferm) {
            return QVariant(Fermentable::typeDisplayNames[ferm->type()]);
         }
         break;
      case TreeItemNode<Fermentable>::ColumnIndex::Color:
         if (ferm) {
            return QVariant(Measurement::displayAmount(Measurement::Amount{ferm->color_srm(),
                                                                           Measurement::Units::srm}, 0));
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column:" << column;
         break;
   }
   return QVariant();
}

QVariant TreeNode::dataHop(int column) {
   Hop * hop = qobject_cast<Hop *>(this->m_thing);
   switch (static_cast<TreeItemNode<Hop>::ColumnIndex>(column)) {
      case TreeItemNode<Hop>::ColumnIndex::Name:
         if (! hop) {
            return QVariant(QObject::tr("Hops"));
         } else {
            return QVariant(hop->name());
         }
      case TreeItemNode<Hop>::ColumnIndex::Form:
         if (hop) {
            return QVariant(Hop::formDisplayNames[hop->form()]);
         }
         break;
      case TreeItemNode<Hop>::ColumnIndex::AlphaPct:
         if (hop) {
            return QVariant(hop->alpha_pct());
         }
         break;
      case TreeItemNode<Hop>::ColumnIndex::Origin:
         if (hop) {
            return QVariant(hop->origin());
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column:" << column;
   }
   return QVariant();
}

QVariant TreeNode::dataMisc(int column) {
   Misc * misc = qobject_cast<Misc *>(this->m_thing);
   switch (static_cast<TreeItemNode<Misc>::ColumnIndex>(column)) {
      case TreeItemNode<Misc>::ColumnIndex::Name:
         if (! misc) {
            return QVariant(QObject::tr("Miscellaneous"));
         } else {
            return QVariant(misc->name());
         }
      case TreeItemNode<Misc>::ColumnIndex::Type:
         if (misc) {
            return QVariant(Misc::typeDisplayNames[misc->type()]);
         }
         break;
///      case MISCUSECOL:
///         if (misc) {
///            // Note that EnumStringMapping::operator[] already handles returning blank string for unset optional enums
///            return QVariant(Misc::useDisplayNames[misc->use()]);
///         }
///         break;
      default :
         qWarning() << QString("TreeNode::dataMisc Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant TreeNode::dataYeast(int column) {
   Yeast * yeast = qobject_cast<Yeast *>(this->m_thing);
   switch (static_cast<TreeItemNode<Yeast>::ColumnIndex>(column)) {
      case TreeItemNode<Yeast>::ColumnIndex::Laboratory:
         if (yeast) {
            return QVariant(yeast->laboratory());
         }
         break;
      case TreeItemNode<Yeast>::ColumnIndex::Name:
         if (! yeast) {
            return QVariant(QObject::tr("Yeast"));
         } else {
            return QVariant(yeast->name());
         }
      case TreeItemNode<Yeast>::ColumnIndex::Type:
         if (yeast) {
            return QVariant(Yeast::typeDisplayNames[yeast->type()]);
         }
         break;
      case TreeItemNode<Yeast>::ColumnIndex::Form:
         if (yeast) {
            return QVariant(Yeast::formDisplayNames[yeast->form()]);
         }
         break;
      default :
         qWarning() << QString("TreeNode::dataYeast Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant TreeNode::dataBrewNote([[maybe_unused]] int column) {
   if (!this->m_thing) {
      return QVariant();
   }

   BrewNote * bNote = qobject_cast<BrewNote *>(this->m_thing);

   return bNote->brewDate_short();
}

QVariant TreeNode::dataStyle(int column) {
   Style * style = qobject_cast<Style *>(this->m_thing);

   if (! style && static_cast<TreeItemNode<Style>::ColumnIndex>(column) == TreeItemNode<Style>::ColumnIndex::Name) {
      return QVariant(QObject::tr("Style"));
   }
   if (style) {
      switch (static_cast<TreeItemNode<Style>::ColumnIndex>(column)) {
         case TreeItemNode<Style>::ColumnIndex::Name:
            return QVariant(style->name());
         case TreeItemNode<Style>::ColumnIndex::Category:
            return QVariant(style->category());
         case TreeItemNode<Style>::ColumnIndex::CategoryNumber:
            return QVariant(style->categoryNumber());
         case TreeItemNode<Style>::ColumnIndex::CategoryLetter:
            return QVariant(style->styleLetter());
         case TreeItemNode<Style>::ColumnIndex::StyleGuide:
            return QVariant(style->styleGuide());
         default :
            qWarning() << QString("TreeNode::dataStyle Bad column: %1").arg(column);
      }
   }
   return QVariant();
}

QVariant TreeNode::dataFolder(int column) {
   Folder * folder = qobject_cast<Folder *>(this->m_thing);

   // All folders have the same columns, so it's a bit arbitrary which one we use here
   if (! folder && static_cast<TreeFolderNode<Hop>::ColumnIndex>(column) == TreeFolderNode<Hop>::ColumnIndex::Name) {
      return QVariant(QObject::tr("Folder"));
   }

   if (! folder) {
      return QVariant(QObject::tr("Folder"));
   }
   if (static_cast<TreeFolderNode<Hop>::ColumnIndex>(column) == TreeFolderNode<Hop>::ColumnIndex::Name) {
      return QVariant(folder->name());
   }

   return QVariant();
}

QVariant TreeNode::dataWater(int column) {
   Water * water = qobject_cast<Water *>(this->m_thing);

   if (water == nullptr && static_cast<TreeItemNode<Water>::ColumnIndex>(column) == TreeItemNode<Water>::ColumnIndex::Name) {
      return QVariant(QObject::tr("Water"));
   }
   if (water) {
      switch (static_cast<TreeItemNode<Water>::ColumnIndex>(column)) {
         case TreeItemNode<Water>::ColumnIndex::Name:
            return QVariant(water->name());
         case TreeItemNode<Water>::ColumnIndex::Calcium:
            return QVariant(water->calcium_ppm());
         case TreeItemNode<Water>::ColumnIndex::Bicarbonate:
            return QVariant(water->bicarbonate_ppm());
         case TreeItemNode<Water>::ColumnIndex::Sulfate:
            return QVariant(water->sulfate_ppm());
         case TreeItemNode<Water>::ColumnIndex::Chloride:
            return QVariant(water->chloride_ppm());
         case TreeItemNode<Water>::ColumnIndex::Sodium:
            return QVariant(water->sodium_ppm());
         case TreeItemNode<Water>::ColumnIndex::Magnesium:
            return QVariant(water->magnesium_ppm());
         case TreeItemNode<Water>::ColumnIndex::pH:
            return water->ph() ? QVariant(*water->ph()) : QVariant();
         default :
            qWarning() << QString("TreeNode::dataWater Bad column: %1").arg(column);
      }
   }

   return QVariant();
}


template<class T>
T * TreeNode::getData() {
   if (this->nodeType == TreeNode::typeOf<T>() && this->m_thing) {
      return qobject_cast<T *>(this->m_thing);
   }

   return nullptr;
}
//
// Instantiate the above template function for the types that are going to use it
//
template Recipe      * TreeNode::getData<Recipe     >();
template Equipment   * TreeNode::getData<Equipment  >();
template Fermentable * TreeNode::getData<Fermentable>();
template Hop         * TreeNode::getData<Hop        >();
template Misc        * TreeNode::getData<Misc       >();
template Yeast       * TreeNode::getData<Yeast      >();
template BrewNote    * TreeNode::getData<BrewNote   >();
template Style       * TreeNode::getData<Style      >();
template Folder    * TreeNode::getData<Folder   >();
template Water       * TreeNode::getData<Water      >();

NamedEntity * TreeNode::thing() {
   if (m_thing) {
      return qobject_cast<NamedEntity *>(this->m_thing);
   }

   return nullptr;
}

QString TreeNode::name() {
   NamedEntity * tmp;
   if (! m_thing) {
      return QString();
   }
   tmp = qobject_cast<NamedEntity *>(this->m_thing);
   return tmp->name();
}

bool TreeNode::showMe() const {
   return m_showMe;
}
void TreeNode::setShowMe(bool val) {
   m_showMe = val;
}

template<class S>
S & operator<<(S & stream, TreeNode::Type const treeItemType) {
   std::optional<QString> itemTypeAsString = itemTypeToName.enumToString(treeItemType);
   if (itemTypeAsString) {
      stream << *itemTypeAsString;
   } else {
      // This is a coding error
      stream << "Unrecognised tree item type: " << static_cast<int>(treeItemType);
   }
   return stream;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template QDebug & operator<<(QDebug & stream, TreeNode::Type const treeItemType);
template QTextStream & operator<<(QTextStream & stream, TreeNode::Type const treeItemType);
