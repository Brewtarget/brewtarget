/*
 * model/Salt.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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
#include "model/Salt.h"

#include <QDebug>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

bool Salt::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Salt const & rhs = static_cast<Salt const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_whenToAdd == rhs.m_whenToAdd &&
      this->m_type   == rhs.m_type
   );
}

ObjectStore & Salt::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Salt>::getInstance();
}

TypeLookup const Salt::typeLookup {
   "Salt",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::amount        , Salt::m_amount        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::amountIsWeight, Salt::m_amount_is_weight),  //<<
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::isAcid        , Salt::m_is_acid        ), //<<
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::percentAcid   , Salt::m_percent_acid   ), //<<
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::type          , Salt::m_type          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::whenToAdd     , Salt::m_whenToAdd     ),
   },
   // Parent class lookup
   &NamedEntity::typeLookup
};

Salt::Salt(QString name) :
   NamedEntity       {name, true},
   m_amount          {0.0},
   m_whenToAdd       {Salt::WhenToAdd::NEVER},
   m_type            {Salt::Types::NONE},
   m_amount_is_weight{true},
   m_percent_acid    {0.0},
   m_is_acid         {false} {
   return;
}

Salt::Salt(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity       {namedParameterBundle},
   m_amount          {namedParameterBundle.val<double         >(PropertyNames::Salt::amount        )},
   m_whenToAdd       {namedParameterBundle.val<Salt::WhenToAdd>(PropertyNames::Salt::whenToAdd     )},
   m_type            {namedParameterBundle.val<Salt::Types    >(PropertyNames::Salt::type          )},
   m_amount_is_weight{namedParameterBundle.val<bool           >(PropertyNames::Salt::amountIsWeight)},
   m_percent_acid    {namedParameterBundle.val<double         >(PropertyNames::Salt::percentAcid   )},
   m_is_acid         {namedParameterBundle.val<bool           >(PropertyNames::Salt::isAcid        )} {
   return;
}

Salt::Salt(Salt const & other) :
   NamedEntity       {other                   },
   m_amount          {other.m_amount          },
   m_whenToAdd       {other.m_whenToAdd       },
   m_type            {other.m_type            },
   m_amount_is_weight{other.m_amount_is_weight},
   m_percent_acid    {other.m_percent_acid    },
   m_is_acid         {other.m_is_acid         } {
   return;
}

Salt::~Salt() = default;

//================================"SET" METHODS=================================
void Salt::setAmount(double val) {
   this->setAndNotify(PropertyNames::Salt::amount, this->m_amount, val);
}

void Salt::setWhenToAdd(Salt::WhenToAdd val) {
   this->setAndNotify(PropertyNames::Salt::whenToAdd, this->m_whenToAdd, val);
}

// This may come to haunt me, but I am setting the isAcid flag and the
// amount_is_weight flags here.
void Salt::setType(Salt::Types type) {
   this->setAndNotify(PropertyNames::Salt::type,           this->m_type, type);
   this->setAndNotify(PropertyNames::Salt::isAcid,         this->m_is_acid, (type > Salt::Types::NAHCO3));
   this->setAndNotify(PropertyNames::Salt::amountIsWeight, this->m_amount_is_weight, !(type == Salt::Types::LACTIC || type == Salt::Types::H3PO4));
}

void Salt::setAmountIsWeight(bool val) {
   this->setAndNotify(PropertyNames::Salt::amountIsWeight, this->m_amount_is_weight, val);
}

void Salt::setIsAcid(bool val) {
   this->setAndNotify(PropertyNames::Salt::isAcid, this->m_is_acid, val);
}

void Salt::setPercentAcid(double val) {
   this->setAndNotify(PropertyNames::Salt::percentAcid, this->m_percent_acid, val);
}

//=========================="GET" METHODS=======================================
double          Salt::amount        () const { return this->m_amount          ; }
Salt::WhenToAdd Salt::whenToAdd     () const { return this->m_whenToAdd       ; }
Salt::Types     Salt::type          () const { return this->m_type            ; }
bool            Salt::isAcid        () const { return this->m_is_acid         ; }
bool            Salt::amountIsWeight() const { return this->m_amount_is_weight; }
double          Salt::percentAcid   () const { return this->m_percent_acid    ; }

//====== maths ===========
// All of these the per gram, per liter
// these values are taken from Bru'n Water's execellent water knowledge page
// https://sites.google.com/site/brunwater/water-knowledge
// the numbers are derived by dividing the molecular weight of the ion by the
// molecular weight of the molecule in grams and then multiplying by 1000 to
// mg
// eg:
//    NaHCO3 84 g/mol
//       Na provides    23 g/mol
//       HCO3 provides  61 g/mol (ish)
//     So 1 g of NaHCO3 in 1L of water provides 1000*(61/84) = 726 ppm HCO3
//
// the magic 1000 is here because masses are stored as kg. We need it in grams
// for this part
double Salt::Ca() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }

   switch (m_type) {
      case Salt::Types::CACL2: return 272.0 * m_amount * 1000.0;
      case Salt::Types::CACO3: return 200.0 * m_amount * 1000.0;
      case Salt::Types::CASO4: return 232.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::Cl() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }

   switch (m_type) {
      case Salt::Types::CACL2: return 483 * m_amount * 1000.0;
      case Salt::Types::NACL: return 607 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::CO3() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }

   return m_type == Salt::Types::CACO3 ? 610.0  * m_amount * 1000.0: 0.0;
}

double Salt::HCO3() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }

   return m_type == Salt::Types::NAHCO3 ? 726.0 * m_amount * 1000.0: 0.0;
}

double Salt::Mg() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }
   return m_type == Salt::Types::MGSO4 ? 99.0 * m_amount * 1000.0: 0.0;
}

double Salt::Na() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }
   switch (m_type) {
      case Salt::Types::NACL: return 393.0 * m_amount * 1000.0;
      case Salt::Types::NAHCO3: return 274.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::SO4() const {
   if ( m_whenToAdd == Salt::WhenToAdd::NEVER ) {
      return 0.0;
   }
   switch (m_type) {
      case Salt::Types::CASO4: return 558.0 * m_amount * 1000.0;
      case Salt::Types::MGSO4: return 389.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

Recipe * Salt::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
