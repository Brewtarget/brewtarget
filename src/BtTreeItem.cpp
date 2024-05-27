/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtTreeItem.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "BtTreeItem.h"

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <Qt>
#include <QVariant>
#include <QVector>

#include "BtFolder.h"
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
#include "utils/EnumStringMapping.h"

namespace {
   EnumStringMapping const itemTypeToName {
      {BtTreeItem::Type::Recipe     , "Recipe"     },
      {BtTreeItem::Type::Equipment  , "Equipment"  },
      {BtTreeItem::Type::Fermentable, "Fermentable"},
      {BtTreeItem::Type::Hop        , "Hop"        },
      {BtTreeItem::Type::Misc       , "Misc"       },
      {BtTreeItem::Type::Yeast      , "Yeast"      },
      {BtTreeItem::Type::BrewNote   , "BrewNote"   },
      {BtTreeItem::Type::Style      , "Style"      },
      {BtTreeItem::Type::Folder     , "Folder"     },
      {BtTreeItem::Type::Water      , "Water"      },
   };
}

template<> BtTreeItem::Type BtTreeItem::typeOf<Recipe>()      { return BtTreeItem::Type::Recipe;      }
template<> BtTreeItem::Type BtTreeItem::typeOf<Equipment>()   { return BtTreeItem::Type::Equipment;   }
template<> BtTreeItem::Type BtTreeItem::typeOf<Fermentable>() { return BtTreeItem::Type::Fermentable; }
template<> BtTreeItem::Type BtTreeItem::typeOf<Hop>()         { return BtTreeItem::Type::Hop;         }
template<> BtTreeItem::Type BtTreeItem::typeOf<Misc>()        { return BtTreeItem::Type::Misc;        }
template<> BtTreeItem::Type BtTreeItem::typeOf<Yeast>()       { return BtTreeItem::Type::Yeast;       }
template<> BtTreeItem::Type BtTreeItem::typeOf<BrewNote>()    { return BtTreeItem::Type::BrewNote;    }
template<> BtTreeItem::Type BtTreeItem::typeOf<Style>()       { return BtTreeItem::Type::Style;       }
template<> BtTreeItem::Type BtTreeItem::typeOf<BtFolder>()    { return BtTreeItem::Type::Folder;      }
template<> BtTreeItem::Type BtTreeItem::typeOf<Water>()       { return BtTreeItem::Type::Water;       }


bool operator==(BtTreeItem & lhs, BtTreeItem & rhs) {
   // Things of different types are not equal
   if (lhs.itemType != rhs.itemType) {
      return false;
   }

   return lhs.data(0) == rhs.data(0);
}

BtTreeItem::BtTreeItem(BtTreeItem::Type itemType, BtTreeItem * parent) :
   parentItem{parent},
   itemType{itemType},
   m_thing{nullptr},
   m_showMe{false} {
   return;
}

BtTreeItem::~BtTreeItem() {
   qDeleteAll(this->childItems);
}

BtTreeItem * BtTreeItem::child(int number) {
   if (number < this->childItems.count()) {
      return this->childItems.value(number);
   }

   return nullptr;
}

BtTreeItem * BtTreeItem::parent() {
   return parentItem;
}

BtTreeItem::Type BtTreeItem::type() const {
   return this->itemType;
}

int BtTreeItem::childCount() const {
   return this->childItems.count();
}

int BtTreeItem::columnCount(BtTreeItem::Type itemType) const {
   switch (itemType) {
      case BtTreeItem::Type::Recipe:      return static_cast<int>(BtTreeItem::     RecipeColumn::NumberOfColumns);
      case BtTreeItem::Type::Equipment:   return static_cast<int>(BtTreeItem::  EquipmentColumn::NumberOfColumns);
      case BtTreeItem::Type::Fermentable: return static_cast<int>(BtTreeItem::FermentableColumn::NumberOfColumns);
      case BtTreeItem::Type::Hop:         return static_cast<int>(BtTreeItem::        HopColumn::NumberOfColumns);
      case BtTreeItem::Type::Misc:        return static_cast<int>(BtTreeItem::       MiscColumn::NumberOfColumns);
      case BtTreeItem::Type::Yeast:       return static_cast<int>(BtTreeItem::      YeastColumn::NumberOfColumns);
      case BtTreeItem::Type::Style:       return static_cast<int>(BtTreeItem::      StyleColumn::NumberOfColumns);
      case BtTreeItem::Type::BrewNote:    return static_cast<int>(BtTreeItem::   BrewNoteColumn::NumberOfColumns);
      case BtTreeItem::Type::Folder:      return static_cast<int>(BtTreeItem::     FolderColumn::NumberOfColumns);
      case BtTreeItem::Type::Water:       return static_cast<int>(BtTreeItem::      WaterColumn::NumberOfColumns);
      default:
         qWarning() << Q_FUNC_INFO << "Bad column:" << static_cast<int>(itemType);
         return 0;
   }
}

QVariant BtTreeItem::data(/*BtTreeItem::Type itemType, */int column) {

   switch (this->itemType) {
      case BtTreeItem::Type::Recipe:      return dataRecipe     (column);
      case BtTreeItem::Type::Equipment:   return dataEquipment  (column);
      case BtTreeItem::Type::Fermentable: return dataFermentable(column);
      case BtTreeItem::Type::Hop:         return dataHop        (column);
      case BtTreeItem::Type::Misc:        return dataMisc       (column);
      case BtTreeItem::Type::Yeast:       return dataYeast      (column);
      case BtTreeItem::Type::Style:       return dataStyle      (column);
      case BtTreeItem::Type::BrewNote:    return dataBrewNote   (column);
      case BtTreeItem::Type::Folder:      return dataFolder     (column);
      case BtTreeItem::Type::Water:       return dataWater      (column);
      default:
         qWarning() << Q_FUNC_INFO << "Bad column:" << static_cast<int>(itemType);
         return QVariant();
   }
}

int BtTreeItem::childNumber() const {
   if (this->parentItem) {
      return parentItem->childItems.indexOf(const_cast<BtTreeItem *>(this));
   }
   return 0;
}

void BtTreeItem::setData(BtTreeItem::Type t, QObject * d) {
   this->m_thing = d;
   this->itemType = t;
}

bool BtTreeItem::insertChildren(int position, int count, BtTreeItem::Type itemType) {
//   qDebug() <<
//      Q_FUNC_INFO << "Inserting" << count << "children of type" << itemType << "(" <<
//      this->itemTypeToString(static_cast<BtTreeItem::Type>(itemType)) << ") at position" << position;
   if (position < 0  || position > this->childItems.size()) {
      qWarning() << Q_FUNC_INFO << "Position" << position << "outside range (0, " << this->childItems.size() << ")";
      return false;
   }

   for (int row = 0; row < count; ++row) {
      BtTreeItem * newItem = new BtTreeItem(itemType, this);
      this->childItems.insert(position + row, newItem);
   }

   return true;
}

bool BtTreeItem::removeChildren(int position, int count) {
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

QVariant BtTreeItem::dataRecipe(int column) {
   Recipe * recipe = qobject_cast<Recipe *>(this->m_thing);
   switch (static_cast<BtTreeItem::RecipeColumn>(column)) {
      case BtTreeItem::RecipeColumn::Name:
         if (!this->m_thing) {
            return QVariant(QObject::tr("Recipes"));
         } else {
            return QVariant(recipe->name());
         }
      case BtTreeItem::RecipeColumn::NumberOfAncestors:
         if (recipe) {
            return QVariant(recipe->ancestors().size());
         }
         break;
      case BtTreeItem::RecipeColumn::BrewDate:
         if (recipe && recipe->date()) {
            return Localization::displayDateUserFormated(*recipe->date());
         }
         break;
      case BtTreeItem::RecipeColumn::Style:
         if (recipe && recipe->style()) {
            return QVariant(recipe->style()->name());
         }
         break;
      default :
         qWarning() << QString("BtTreeItem::dataRecipe Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataEquipment(int column) {
   Equipment * kit = qobject_cast<Equipment *>(this->m_thing);
   switch (static_cast<BtTreeItem::EquipmentColumn>(column)) {
      case BtTreeItem::EquipmentColumn::Name:
         if (! kit) {
            return QVariant(QObject::tr("Equipment"));
         } else {
            return QVariant(kit->name());
         }
      case BtTreeItem::EquipmentColumn::BoilTime:
         if (kit) {
            return QVariant::fromValue(kit->boilTime_min());
         }
         break;
      default :
         qWarning() << QString("BtTreeItem::dataEquipment Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataFermentable(int column) {
   Fermentable * ferm = qobject_cast<Fermentable *>(this->m_thing);

   switch (static_cast<BtTreeItem::FermentableColumn>(column)) {
      case BtTreeItem::FermentableColumn::Name:
         if (ferm) {
            return QVariant(ferm->name());
         } else {
            return QVariant(QObject::tr("Fermentables"));
         }
      case BtTreeItem::FermentableColumn::Type:
         if (ferm) {
            return QVariant(Fermentable::typeDisplayNames[ferm->type()]);
         }
         break;
      case BtTreeItem::FermentableColumn::Color:
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

QVariant BtTreeItem::dataHop(int column) {
   Hop * hop = qobject_cast<Hop *>(this->m_thing);
   switch (static_cast<BtTreeItem::HopColumn>(column)) {
      case BtTreeItem::HopColumn::Name:
         if (! hop) {
            return QVariant(QObject::tr("Hops"));
         } else {
            return QVariant(hop->name());
         }
      case BtTreeItem::HopColumn::Form:
         if (hop) {
            return QVariant(Hop::formDisplayNames[hop->form()]);
         }
         break;
      case BtTreeItem::HopColumn::AlphaPct:
         if (hop) {
            return QVariant(hop->alpha_pct());
         }
         break;
      case BtTreeItem::HopColumn::Origin:
         if (hop) {
            return QVariant(hop->origin());
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column:" << column;
   }
   return QVariant();
}

QVariant BtTreeItem::dataMisc(int column) {
   Misc * misc = qobject_cast<Misc *>(this->m_thing);
   switch (static_cast<BtTreeItem::MiscColumn>(column)) {
      case BtTreeItem::MiscColumn::Name:
         if (! misc) {
            return QVariant(QObject::tr("Miscellaneous"));
         } else {
            return QVariant(misc->name());
         }
      case BtTreeItem::MiscColumn::Type:
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
         qWarning() << QString("BtTreeItem::dataMisc Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataYeast(int column) {
   Yeast * yeast = qobject_cast<Yeast *>(this->m_thing);
   switch (static_cast<BtTreeItem::YeastColumn>(column)) {
      case BtTreeItem::YeastColumn::Name:
         if (! yeast) {
            return QVariant(QObject::tr("Yeast"));
         } else {
            return QVariant(yeast->name());
         }
      case BtTreeItem::YeastColumn::Type:
         if (yeast) {
            return QVariant(Yeast::typeDisplayNames[yeast->type()]);
         }
         break;
      case BtTreeItem::YeastColumn::Form:
         if (yeast) {
            return QVariant(Yeast::formDisplayNames[yeast->form()]);
         }
         break;
      default :
         qWarning() << QString("BtTreeItem::dataYeast Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataBrewNote([[maybe_unused]] int column) {
   if (!this->m_thing) {
      return QVariant();
   }

   BrewNote * bNote = qobject_cast<BrewNote *>(this->m_thing);

   return bNote->brewDate_short();
}

QVariant BtTreeItem::dataStyle(int column) {
   Style * style = qobject_cast<Style *>(this->m_thing);

   if (! style && static_cast<BtTreeItem::StyleColumn>(column) == BtTreeItem::StyleColumn::Name) {
      return QVariant(QObject::tr("Style"));
   }
   if (style) {
      switch (static_cast<BtTreeItem::StyleColumn>(column)) {
         case BtTreeItem::StyleColumn::Name:
            return QVariant(style->name());
         case BtTreeItem::StyleColumn::Category:
            return QVariant(style->category());
         case BtTreeItem::StyleColumn::CategoryNumber:
            return QVariant(style->categoryNumber());
         case BtTreeItem::StyleColumn::CategoryLetter:
            return QVariant(style->styleLetter());
         case BtTreeItem::StyleColumn::StyleGuide:
            return QVariant(style->styleGuide());
         default :
            qWarning() << QString("BtTreeItem::dataStyle Bad column: %1").arg(column);
      }
   }
   return QVariant();
}

QVariant BtTreeItem::dataFolder(int column) {
   BtFolder * folder = qobject_cast<BtFolder *>(this->m_thing);


   if (! folder && static_cast<BtTreeItem::FolderColumn>(column) == BtTreeItem::FolderColumn::Name) {
      return QVariant(QObject::tr("Folder"));
   }

   if (! folder) {
      return QVariant(QObject::tr("Folder"));
   }
   if (static_cast<BtTreeItem::FolderColumn>(column) == BtTreeItem::FolderColumn::Name) {
      return QVariant(folder->name());
   }

   return QVariant();
}

QVariant BtTreeItem::dataWater(int column) {
   Water * water = qobject_cast<Water *>(this->m_thing);

   if (water == nullptr && static_cast<BtTreeItem::WaterColumn>(column) == BtTreeItem::WaterColumn::Name) {
      return QVariant(QObject::tr("Water"));
   }
   if (water) {
      switch (static_cast<BtTreeItem::WaterColumn>(column)) {
         case BtTreeItem::WaterColumn::Name:
            return QVariant(water->name());
         case BtTreeItem::WaterColumn::Calcium:
            return QVariant(water->calcium_ppm());
         case BtTreeItem::WaterColumn::Bicarbonate:
            return QVariant(water->bicarbonate_ppm());
         case BtTreeItem::WaterColumn::Sulfate:
            return QVariant(water->sulfate_ppm());
         case BtTreeItem::WaterColumn::Chloride:
            return QVariant(water->chloride_ppm());
         case BtTreeItem::WaterColumn::Sodium:
            return QVariant(water->sodium_ppm());
         case BtTreeItem::WaterColumn::Magnesium:
            return QVariant(water->magnesium_ppm());
         case BtTreeItem::WaterColumn::pH:
            return water->ph() ? QVariant(*water->ph()) : QVariant();
         default :
            qWarning() << QString("BtTreeItem::dataWater Bad column: %1").arg(column);
      }
   }

   return QVariant();
}


template<class T>
T * BtTreeItem::getData() {
   if (this->itemType == BtTreeItem::typeOf<T>() && this->m_thing) {
      return qobject_cast<T *>(this->m_thing);
   }

   return nullptr;
}
//
// Instantiate the above template function for the types that are going to use it
//
template Recipe      * BtTreeItem::getData<Recipe     >();
template Equipment   * BtTreeItem::getData<Equipment  >();
template Fermentable * BtTreeItem::getData<Fermentable>();
template Hop         * BtTreeItem::getData<Hop        >();
template Misc        * BtTreeItem::getData<Misc       >();
template Yeast       * BtTreeItem::getData<Yeast      >();
template BrewNote    * BtTreeItem::getData<BrewNote   >();
template Style       * BtTreeItem::getData<Style      >();
template BtFolder    * BtTreeItem::getData<BtFolder   >();
template Water       * BtTreeItem::getData<Water      >();

NamedEntity * BtTreeItem::thing() {
   if (m_thing) {
      return qobject_cast<NamedEntity *>(this->m_thing);
   }

   return nullptr;
}

QString BtTreeItem::name() {
   NamedEntity * tmp;
   if (! m_thing) {
      return QString();
   }
   tmp = qobject_cast<NamedEntity *>(this->m_thing);
   return tmp->name();
}

bool BtTreeItem::showMe() const {
   return m_showMe;
}
void BtTreeItem::setShowMe(bool val) {
   m_showMe = val;
}

template<class S>
S & operator<<(S & stream, BtTreeItem::Type const treeItemType) {
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
template QDebug & operator<<(QDebug & stream, BtTreeItem::Type const treeItemType);
template QTextStream & operator<<(QTextStream & stream, BtTreeItem::Type const treeItemType);
