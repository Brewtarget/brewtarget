/*
 * model/Misc.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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
#include "model/Misc.h"

#include <iostream>
#include <string>

#include <QDebug>
#include <QVector>

#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

namespace {
   QStringList uses = QStringList() << "Boil" << "Mash" << "Primary" << "Secondary" << "Bottling";
   QStringList types = QStringList() << "Spice" << "Fining" << "Water Agent" << "Herb" << "Flavor" << "Other";
   QStringList amountTypes = QStringList() << "Weight" << "Volume";
}

bool Misc::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Misc const & rhs = static_cast<Misc const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type == rhs.m_type &&
      this->m_use  == rhs.m_use
   );
}

ObjectStore & Misc::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Misc>::getInstance();
}

//============================CONSTRUCTORS======================================
Misc::Misc(Misc const & other) :
   NamedEntityWithInventory{other                 },
   m_type                  {other.m_type          },
   m_use                   {other.m_use           },
   m_time                  {other.m_time          },
   m_amount                {other.m_amount        },
   m_amountIsWeight        {other.m_amountIsWeight},
   m_useFor                {other.m_useFor        },
   m_notes                 {other.m_notes         } {
   return;
}

Misc::Misc(QString name, bool cache) :
   NamedEntityWithInventory{-1, cache, name, true},
   m_type                  {Misc::Spice          },
   m_use                   {Misc::Boil           },
   m_time                  {0.0                  },
   m_amount                {0.0                  },
   m_amountIsWeight        {false                },
   m_useFor                {""                   },
   m_notes                 {""                   } {
   return;
}

Misc::Misc(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_type                  {static_cast<Misc::Type>(namedParameterBundle(PropertyNames::Misc::type).toInt())},
   m_use                   {static_cast<Misc::Use>(namedParameterBundle(PropertyNames::Misc::use).toInt())},
   m_time                  {namedParameterBundle(PropertyNames::Misc::time          ).toDouble()},
   m_amount                {namedParameterBundle(PropertyNames::Misc::amount        ).toDouble()},
   m_amountIsWeight        {namedParameterBundle(PropertyNames::Misc::amountIsWeight).toBool()},
   m_useFor                {namedParameterBundle(PropertyNames::Misc::useFor        ).toString()},
   m_notes                 {namedParameterBundle(PropertyNames::Misc::notes         ).toString()} {
   return;
}


//============================"GET" METHODS=====================================
Misc::Type Misc::type() const { return m_type; }

const QString Misc::typeString() const { return types.at(m_type); }

Misc::Use Misc::use() const { return m_use; }

const QString Misc::useString() const { return uses.at(m_use); }

double Misc::amount() const { return m_amount; }

double Misc::time() const { return m_time; }

bool Misc::amountIsWeight() const { return m_amountIsWeight; }

QString Misc::useFor() const { return m_useFor; }

QString Misc::notes() const { return m_notes; }

double Misc::inventory() const {
   return InventoryUtils::getAmount(*this);
}

Misc::AmountType Misc::amountType() const { return m_amountIsWeight ? AmountType_Weight : AmountType_Volume; }

const QString Misc::amountTypeString() const { return amountTypes.at(amountType()); }

const QString Misc::typeStringTr() const
{
   QStringList typesTr = QStringList() << tr("Spice") << tr("Fining") << tr("Water Agent") << tr("Herb") << tr("Flavor") << tr("Other");
   if ( m_type >=  Spice && m_type < typesTr.size()  ) {
      return typesTr.at(m_type);
   }
   else {
      return QString("Spice");
   }
}

const QString Misc::useStringTr() const
{
   QStringList usesTr = QStringList() << tr("Boil") << tr("Mash") << tr("Primary") << tr("Secondary") << tr("Bottling");
   if ( m_use >= Boil && m_use < usesTr.size() ) {
      return usesTr.at(use());
   }
   else {
      return QString("Boil");
   }
}

const QString Misc::amountTypeStringTr() const
{
   QStringList amountTypesTr = QStringList() << tr("Weight") << tr("Volume");
   if ( amountType() ) {
      return amountTypesTr.at(amountType());
   }
   else {
      return QString("Weight");
   }
}

//============================"SET" METHODS=====================================
void Misc::setType(Type t) {
   this->setAndNotify( PropertyNames::Misc::type, this->m_type, t);
}

void Misc::setUse(Use u) {
   this->setAndNotify( PropertyNames::Misc::use, this->m_use, u);
}

void Misc::setUseFor(QString const & var) {
   this->setAndNotify( PropertyNames::Misc::useFor, this->m_useFor, var );
}

void Misc::setNotes(QString const & var) {
   this->setAndNotify( PropertyNames::Misc::notes, this->m_notes, var );
}

void Misc::setAmountType(AmountType t) {
   this->setAmountIsWeight(t == AmountType_Weight);
}

void Misc::setAmountIsWeight(bool var) {
   this->setAndNotify( PropertyNames::Misc::amountIsWeight, this->m_amountIsWeight, var);
}

void Misc::setAmount(double var) {
   this->setAndNotify( PropertyNames::Misc::amount, this->m_amount, this->enforceMin(var, "amount"));
}

void Misc::setInventoryAmount(double var) {
   InventoryUtils::setAmount(*this, var);
   return;
}

void Misc::setTime(double var) {
   this->setAndNotify( PropertyNames::Misc::time, this->m_time, this->enforceMin(var, "time"));
}

//========================OTHER METHODS=========================================

bool Misc::isValidUse( const QString& var )
{
   static const QString uses[] = {"Boil", "Mash", "Primary", "Secondary", "Bottling"};
   static const unsigned int size = 5;
   unsigned int i;

   for( i = 0; i < size; ++i )
      if( var == uses[i] )
         return true;

   return false;
}

bool Misc::isValidType( const QString& var )
{
   static const QString types[] = {"Spice", "Fining", "Water Agent", "Herb", "Flavor", "Other"};
   static const unsigned int size = 6;
   unsigned int i;

   for( i = 0; i < size; ++i )
      if( var == types[i] )
         return true;

   return false;
}

Recipe * Misc::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
