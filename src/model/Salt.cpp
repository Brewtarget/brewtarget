/*
 * model/Salt.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
      this->m_add_to == rhs.m_add_to &&
      this->m_type   == rhs.m_type
   );
}

ObjectStore & Salt::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Salt>::getInstance();
}

Salt::Salt(QString name) :
   NamedEntity       {name, true},
   m_amount          {0.0},
   m_add_to          {NEVER},
   m_type            {NONE},
   m_amount_is_weight{true},
   m_percent_acid    {0.0},
   m_is_acid         {false} {
   return;
}

Salt::Salt(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity       {namedParameterBundle},
   m_amount          {namedParameterBundle(PropertyNames::Salt::amount         ).toDouble()},
   m_add_to          {static_cast<Salt::WhenToAdd>(namedParameterBundle(PropertyNames::Salt::addTo).toInt())},
   m_type            {static_cast<Salt::Types>(namedParameterBundle(PropertyNames::Salt::type).toInt())},
   m_amount_is_weight{namedParameterBundle(PropertyNames::Salt::amountIsWeight).toBool()},
   m_percent_acid    {namedParameterBundle(PropertyNames::Salt::percentAcid   ).toDouble()},
   m_is_acid         {namedParameterBundle(PropertyNames::Salt::isAcid          ).toBool()} {
   return;
}

Salt::Salt(Salt const & other) :
   NamedEntity       {other                   },
   m_amount          {other.m_amount          },
   m_add_to          {other.m_add_to          },
   m_type            {other.m_type            },
   m_amount_is_weight{other.m_amount_is_weight},
   m_percent_acid    {other.m_percent_acid    },
   m_is_acid         {other.m_is_acid         } {
   return;
}

//================================"SET" METHODS=================================
void Salt::setAmount(double var) {
   this->setAndNotify(PropertyNames::Salt::amount, this->m_amount, var);
}

void Salt::setAddTo(Salt::WhenToAdd var) {
   this->setAndNotify(PropertyNames::Salt::addTo, this->m_add_to, var);
}

// This may come to haunt me, but I am setting the isAcid flag and the
// amount_is_weight flags here.
void Salt::setType(Salt::Types type) {
   this->setAndNotify(PropertyNames::Salt::type,           this->m_type, type);
   this->setAndNotify(PropertyNames::Salt::isAcid,         this->m_is_acid, (type > NAHCO3));
   this->setAndNotify(PropertyNames::Salt::amountIsWeight, this->m_amount_is_weight, !(type == LACTIC || type == H3PO4));
}

void Salt::setAmountIsWeight(bool var) {
   this->setAndNotify(PropertyNames::Salt::amountIsWeight, this->m_amount_is_weight, var);
}

void Salt::setIsAcid(bool var) {
   this->setAndNotify(PropertyNames::Salt::isAcid, this->m_is_acid, var);
}

void Salt::setPercentAcid(double var) {
   this->setAndNotify(PropertyNames::Salt::percentAcid, this->m_percent_acid, var);
}

//=========================="GET" METHODS=======================================
double Salt::amount() const { return m_amount; }
Salt::WhenToAdd Salt::addTo() const { return m_add_to; }
Salt::Types Salt::type() const { return m_type; }
bool Salt::isAcid() const { return m_is_acid; }
bool Salt::amountIsWeight() const { return m_amount_is_weight; }
double Salt::percentAcid() const { return m_percent_acid; }

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
double Salt::Ca() const
{
   if ( m_add_to == Salt::NEVER ) {
      return 0.0;
   }

   switch (m_type) {
      case Salt::CACL2: return 272.0 * m_amount * 1000.0;
      case Salt::CACO3: return 200.0 * m_amount * 1000.0;
      case Salt::CASO4: return 232.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::Cl() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   switch (m_type) {
      case Salt::CACL2: return 483 * m_amount * 1000.0;
      case Salt::NACL: return 607 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::CO3() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   return m_type == Salt::CACO3 ? 610.0  * m_amount * 1000.0: 0.0;
}

double Salt::HCO3() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   return m_type == Salt::NAHCO3 ? 726.0 * m_amount * 1000.0: 0.0;
}

double Salt::Mg() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   return m_type == Salt::MGSO4 ? 99.0 * m_amount * 1000.0: 0.0;
}

double Salt::Na() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   switch (m_type) {
      case Salt::NACL: return 393.0 * m_amount * 1000.0;
      case Salt::NAHCO3: return 274.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::SO4() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   switch (m_type) {
      case Salt::CASO4: return 558.0 * m_amount * 1000.0;
      case Salt::MGSO4: return 389.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

Recipe * Salt::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
