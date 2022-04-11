/*
 * BtTreeItem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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
      {QT_TR_NOOP("RECIPE"     )          , BtTreeItem::Type::RECIPE      },
      {QT_TR_NOOP("EQUIPMENT"  )          , BtTreeItem::Type::EQUIPMENT   },
      {QT_TR_NOOP("FERMENTABLE")          , BtTreeItem::Type::FERMENTABLE },
      {QT_TR_NOOP("HOP"        )          , BtTreeItem::Type::HOP         },
      {QT_TR_NOOP("MISC"       )          , BtTreeItem::Type::MISC        },
      {QT_TR_NOOP("YEAST"      )          , BtTreeItem::Type::YEAST       },
      {QT_TR_NOOP("BREWNOTE"   )          , BtTreeItem::Type::BREWNOTE    },
      {QT_TR_NOOP("STYLE"      )          , BtTreeItem::Type::STYLE       },
      {QT_TR_NOOP("FOLDER"     )          , BtTreeItem::Type::FOLDER      },
      {QT_TR_NOOP("WATER"      )          , BtTreeItem::Type::WATER       }
   };
}

template<> BtTreeItem::Type BtTreeItem::typeOf<Recipe>()      { return BtTreeItem::Type::RECIPE;      }
template<> BtTreeItem::Type BtTreeItem::typeOf<Equipment>()   { return BtTreeItem::Type::EQUIPMENT;   }
template<> BtTreeItem::Type BtTreeItem::typeOf<Fermentable>() { return BtTreeItem::Type::FERMENTABLE; }
template<> BtTreeItem::Type BtTreeItem::typeOf<Hop>()         { return BtTreeItem::Type::HOP;         }
template<> BtTreeItem::Type BtTreeItem::typeOf<Misc>()        { return BtTreeItem::Type::MISC;        }
template<> BtTreeItem::Type BtTreeItem::typeOf<Yeast>()       { return BtTreeItem::Type::YEAST;       }
template<> BtTreeItem::Type BtTreeItem::typeOf<BrewNote>()    { return BtTreeItem::Type::BREWNOTE;    }
template<> BtTreeItem::Type BtTreeItem::typeOf<Style>()       { return BtTreeItem::Type::STYLE;       }
template<> BtTreeItem::Type BtTreeItem::typeOf<BtFolder>()    { return BtTreeItem::Type::FOLDER;      }
template<> BtTreeItem::Type BtTreeItem::typeOf<Water>()       { return BtTreeItem::Type::WATER;       }


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
   _thing{nullptr},
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
      case BtTreeItem::Type::RECIPE:
         return RECIPENUMCOLS;
      case BtTreeItem::Type::EQUIPMENT:
         return EQUIPMENTNUMCOLS;
      case BtTreeItem::Type::FERMENTABLE:
         return FERMENTABLENUMCOLS;
      case BtTreeItem::Type::HOP:
         return HOPNUMCOLS;
      case BtTreeItem::Type::MISC:
         return MISCNUMCOLS;
      case BtTreeItem::Type::YEAST:
         return YEASTNUMCOLS;
      case BtTreeItem::Type::STYLE:
         return STYLENUMCOLS;
      case BtTreeItem::Type::BREWNOTE:
         return BREWNUMCOLS;
      case BtTreeItem::Type::FOLDER:
         return FOLDERNUMCOLS;
      case BtTreeItem::Type::WATER:
         return WATERNUMCOLS;
      default:
         qWarning() << Q_FUNC_INFO << "Bad column:" << static_cast<int>(itemType);
         return 0;
   }

}

QVariant BtTreeItem::data(/*BtTreeItem::Type itemType, */int column) {

   switch (this->itemType) {
      case BtTreeItem::Type::RECIPE:
         return dataRecipe(column);
      case BtTreeItem::Type::EQUIPMENT:
         return dataEquipment(column);
      case BtTreeItem::Type::FERMENTABLE:
         return dataFermentable(column);
      case BtTreeItem::Type::HOP:
         return dataHop(column);
      case BtTreeItem::Type::MISC:
         return dataMisc(column);
      case BtTreeItem::Type::YEAST:
         return dataYeast(column);
      case BtTreeItem::Type::STYLE:
         return dataStyle(column);
      case BtTreeItem::Type::BREWNOTE:
         return dataBrewNote(column);
      case BtTreeItem::Type::FOLDER:
         return dataFolder(column);
      case BtTreeItem::Type::WATER:
         return dataWater(column);
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
   this->_thing = d;
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
   Recipe * recipe = qobject_cast<Recipe *>(_thing);
   switch (column) {
      case RECIPENAMECOL:
         if (! _thing) {
            return QVariant(QObject::tr("Recipes"));
         } else {
            return QVariant(recipe->name());
         }
      case RECIPEANCCOUNT:
         if (recipe) {
            return QVariant(recipe->ancestors().size());
         }
         break;
      case RECIPEBREWDATECOL:
         if (recipe) {
            return Localization::displayDateUserFormated(recipe->date());
         }
         break;
      case RECIPESTYLECOL:
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
   Equipment * kit = qobject_cast<Equipment *>(_thing);
   switch (column) {
      case EQUIPMENTNAMECOL:
         if (! kit) {
            return QVariant(QObject::tr("Equipment"));
         } else {
            return QVariant(kit->name());
         }
      case EQUIPMENTBOILTIMECOL:
         if (kit) {
            return QVariant(kit->boilTime_min());
         }
         break;
      default :
         qWarning() << QString("BtTreeItem::dataEquipment Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataFermentable(int column) {
   Fermentable * ferm = qobject_cast<Fermentable *>(this->_thing);

   switch (column) {
      case FERMENTABLENAMECOL:
         if (ferm) {
            return QVariant(ferm->name());
         } else {
            return QVariant(QObject::tr("Fermentables"));
         }
      case FERMENTABLETYPECOL:
         if (ferm) {
            return QVariant(ferm->typeStringTr());
         }
         break;
      case FERMENTABLECOLORCOL:
         if (ferm) {
            return QVariant(Measurement::displayAmount(Measurement::Amount{ferm->color_srm(), Measurement::Units::srm},
                                                       BtString::EMPTY_STR,
                                                       PropertyNames::Fermentable::color_srm,
                                                       0));
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column:" << column;
         break;
   }
   return QVariant();
}

QVariant BtTreeItem::dataHop(int column) {
   Hop * hop = qobject_cast<Hop *>(_thing);
   switch (column) {
      case HOPNAMECOL:
         if (! hop) {
            return QVariant(QObject::tr("Hops"));
         } else {
            return QVariant(hop->name());
         }
      case HOPFORMCOL:
         if (hop) {
            return QVariant(hop->formStringTr());
         }
         break;
      case HOPUSECOL:
         if (hop) {
            return QVariant(hop->useStringTr());
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column:" << column;
   }
   return QVariant();
}

QVariant BtTreeItem::dataMisc(int column) {
   Misc * misc = qobject_cast<Misc *>(_thing);
   switch (column) {
      case MISCNAMECOL:
         if (! misc) {
            return QVariant(QObject::tr("Miscellaneous"));
         } else {
            return QVariant(misc->name());
         }
      case MISCTYPECOL:
         if (misc) {
            return QVariant(misc->typeStringTr());
         }
         break;
      case MISCUSECOL:
         if (misc) {
            return QVariant(misc->useStringTr());
         }
         break;
      default :
         qWarning() << QString("BtTreeItem::dataMisc Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataYeast(int column) {
   Yeast * yeast = qobject_cast<Yeast *>(_thing);
   switch (column) {
      case YEASTNAMECOL:
         if (! yeast) {
            return QVariant(QObject::tr("Yeast"));
         } else {
            return QVariant(yeast->name());
         }
      case YEASTTYPECOL:
         if (yeast) {
            return QVariant(yeast->typeStringTr());
         }
         break;
      case YEASTFORMCOL:
         if (yeast) {
            return QVariant(yeast->formStringTr());
         }
         break;
      default :
         qWarning() << QString("BtTreeItem::dataYeast Bad column: %1").arg(column);
   }
   return QVariant();
}

QVariant BtTreeItem::dataBrewNote(int column) {
   if (! _thing) {
      return QVariant();
   }

   BrewNote * bNote = qobject_cast<BrewNote *>(_thing);

   return bNote->brewDate_short();
}

QVariant BtTreeItem::dataStyle(int column) {
   Style * style = qobject_cast<Style *>(_thing);

   if (! style && column == STYLENAMECOL) {
      return QVariant(QObject::tr("Style"));
   } else if (style) {
      switch (column) {
         case STYLENAMECOL:
            return QVariant(style->name());
         case STYLECATEGORYCOL:
            return QVariant(style->category());
         case STYLENUMBERCOL:
            return QVariant(style->categoryNumber());
         case STYLELETTERCOL:
            return QVariant(style->styleLetter());
         case STYLEGUIDECOL:
            return QVariant(style->styleGuide());
         default :
            qWarning() << QString("BtTreeItem::dataStyle Bad column: %1").arg(column);
      }
   }
   return QVariant();
}

QVariant BtTreeItem::dataFolder(int column) {
   BtFolder * folder = qobject_cast<BtFolder *>(_thing);


   if (! folder && column == FOLDERNAMECOL) {
      return QVariant(QObject::tr("Folder"));
   }

   if (! folder) {
      return QVariant(QObject::tr("Folder"));
   } else if (column == FOLDERNAMECOL) {
      return QVariant(folder->name());
   }

   return QVariant();
}

QVariant BtTreeItem::dataWater(int column) {
   Water * water = qobject_cast<Water *>(_thing);

   if (water == nullptr && column == WATERNAMECOL) {
      return QVariant(QObject::tr("Water"));
   } else if (water) {
      switch (column) {
         case WATERNAMECOL:
            return QVariant(water->name());
         case WATERCACOL:
            return QVariant(water->calcium_ppm());
         case WATERHCO3COL:
            return QVariant(water->bicarbonate_ppm());
         case WATERSO4COL:
            return QVariant(water->sulfate_ppm());
         case WATERCLCOL:
            return QVariant(water->chloride_ppm());
         case WATERNACOL:
            return QVariant(water->sodium_ppm());
         case WATERMGCOL:
            return QVariant(water->magnesium_ppm());
         case WATERpHCOL:
            return QVariant(water->ph());
         default :
            qWarning() << QString("BtTreeItem::dataWater Bad column: %1").arg(column);
      }
   }

   return QVariant();
}


template<class T>
T * BtTreeItem::getData() {
   if (this->itemType == BtTreeItem::typeOf<T>() && this->_thing) {
      return qobject_cast<T *>(this->_thing);
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
   if (_thing) {
      return qobject_cast<NamedEntity *>(_thing);
   }

   return nullptr;
}

QString BtTreeItem::name() {
   NamedEntity * tmp;
   if (! _thing) {
      return QString();
   }
   tmp = qobject_cast<NamedEntity *>(_thing);
   return tmp->name();
}

bool BtTreeItem::showMe() const {
   return m_showMe;
}
void BtTreeItem::setShowMe(bool val) {
   m_showMe = val;
}
