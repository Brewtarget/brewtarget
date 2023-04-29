/*
 * model/Misc.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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

#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

namespace {
   QStringList uses = QStringList() << "Boil" << "Mash" << "Primary" << "Secondary" << "Bottling";
   QStringList types = QStringList() << "Spice" << "Fining" << "Water Agent" << "Herb" << "Flavor" << "Other";
   QStringList amountTypes = QStringList() << "Weight" << "Volume";
   QStringList typesTr =
      QStringList() << Misc::tr("Spice") << Misc::tr("Fining") << Misc::tr("Water Agent") << Misc::tr("Herb") <<
      Misc::tr("Flavor") << Misc::tr("Other");
   QStringList usesTr =
      QStringList() << Misc::tr("Boil") << Misc::tr("Mash") << Misc::tr("Primary") << Misc::tr("Secondary") <<
      Misc::tr("Bottling");
   QStringList amountTypesTr = QStringList() << Misc::tr("Weight") << Misc::tr("Volume");
}

bool Misc::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Misc const & rhs = static_cast<Misc const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type == rhs.m_type
   );
}

ObjectStore & Misc::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Misc>::getInstance();
}

TypeLookup const Misc::typeLookup {
   "Misc",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::amount        , Misc::m_amount        , Measurement::PqEitherMassOrVolume),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::amountIsWeight, Misc::m_amountIsWeight),
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::amountType    , Misc::m_amountType    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::notes         , Misc::m_notes         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::time          , Misc::m_time          , Measurement::PhysicalQuantity::Time),
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::typeString    , Misc::m_typeString    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::type          , Misc::m_type          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::useFor        , Misc::m_useFor        ),
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::useString     , Misc::m_useString     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::use           , Misc::m_use           ),
   },
   // Parent class lookup.  NB: NamedEntityWithInventory not NamedEntity!
   &NamedEntityWithInventory::typeLookup
};
static_assert(std::is_base_of<NamedEntityWithInventory, Misc>::value);

//============================CONSTRUCTORS======================================

Misc::Misc(QString name) :
   NamedEntityWithInventory{name, true},
   m_type          {Misc::Type::Spice},
   m_use           {Misc::Use::Boil  },
   m_time          {0.0              },
   m_amount        {0.0              },
   m_amountIsWeight{false            },
   m_useFor        {""               },
   m_notes         {""               } {
   return;
}

Misc::Misc(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_type                  {namedParameterBundle.val<Misc::Type>(PropertyNames::Misc::type          )},
   m_use                   {namedParameterBundle.val<Misc::Use >(PropertyNames::Misc::use           )},
   m_time                  {namedParameterBundle.val<double    >(PropertyNames::Misc::time          )},
   m_amount                {namedParameterBundle.val<double    >(PropertyNames::Misc::amount        )},
   m_amountIsWeight        {namedParameterBundle.val<bool      >(PropertyNames::Misc::amountIsWeight)},
   m_useFor                {namedParameterBundle.val<QString   >(PropertyNames::Misc::useFor        )},
   m_notes                 {namedParameterBundle.val<QString   >(PropertyNames::Misc::notes         )} {
   return;
}

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

Misc::~Misc() = default;

//============================"GET" METHODS=====================================
Misc::Type Misc::type() const { return m_type; }

const QString Misc::typeString() const { return types.at(static_cast<int>(m_type)); }

Misc::Use Misc::use() const { return m_use; }

const QString Misc::useString() const { return uses.at(static_cast<int>(m_use)); }

double Misc::amount() const { return m_amount; }

double Misc::time() const { return m_time; }

bool Misc::amountIsWeight() const { return m_amountIsWeight; }

QString Misc::useFor() const { return m_useFor; }

QString Misc::notes() const { return m_notes; }

double Misc::inventory() const {
   return InventoryUtils::getAmount(*this);
}

Misc::AmountType Misc::amountType() const { return m_amountIsWeight ? Misc::AmountType::Weight : Misc::AmountType::Volume; }

const QString Misc::amountTypeString() const { return amountTypes.at(static_cast<int>(this->amountType())); }

const QString Misc::typeStringTr() const {
   int myType = static_cast<int>(this->m_type);
   if (myType >= 0 && myType < typesTr.size()  ) {
      return typesTr.at(myType);
   }
   // It's most likely a coding error if we get here
   return "???";
}

const QString Misc::useStringTr() const {
   int myUse = static_cast<int>(this->m_use);
   if (myUse >= 0 && myUse < usesTr.size() ) {
      return usesTr.at(myUse);
   }
   // It's most likely a coding error if we get here
   return "???";
}

const QString Misc::amountTypeStringTr() const {
   return amountTypesTr.at(static_cast<int>(this->amountType()));
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

void Misc::setAmountType(Misc::AmountType t) {
   this->setAmountIsWeight(t == Misc::AmountType::Weight);
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
      if (var == uses[i] )
         return true;

   return false;
}

bool Misc::isValidType( const QString& var )
{
   static const QString types[] = {"Spice", "Fining", "Water Agent", "Herb", "Flavor", "Other"};
   static const unsigned int size = 6;
   unsigned int i;

   for( i = 0; i < size; ++i )
      if (var == types[i] )
         return true;

   return false;
}

Recipe * Misc::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
