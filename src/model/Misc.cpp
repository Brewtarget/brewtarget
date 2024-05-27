/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Misc.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "model/Misc.h"

#include <iostream>
#include <string>

#include <QDebug>
#include <QVector>

#include "database/ObjectStoreWrapper.h"
#include "model/InventoryMisc.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

QString Misc::localisedName() { return tr("Miscellaneous"); }

EnumStringMapping const Misc::typeStringMapping {
   {Misc::Type::Spice      , "spice"      },
   {Misc::Type::Fining     , "fining"     },
   {Misc::Type::Water_Agent, "water agent"},
   {Misc::Type::Herb       , "herb"       },
   {Misc::Type::Flavor     , "flavor"     },
   {Misc::Type::Other      , "other"      },
   {Misc::Type::Wood       , "wood"       },
};

EnumStringMapping const Misc::typeDisplayNames {
   {Misc::Type::Spice      , tr("Spice"      )},
   {Misc::Type::Fining     , tr("Fining"     )},
   {Misc::Type::Water_Agent, tr("Water Agent")},
   {Misc::Type::Herb       , tr("Herb"       )},
   {Misc::Type::Flavor     , tr("Flavor"     )},
   {Misc::Type::Other      , tr("Other"      )},
   {Misc::Type::Wood       , tr("Wood"       )},
};

bool Misc::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Misc const & rhs = static_cast<Misc const &>(other);
   // Base class will already have ensured names are equal
   bool const outlinesAreEqual{
      // "Outline" fields: In BeerJSON, all these fields are in the FermentableBase type
      this->m_producer  == rhs.m_producer  &&
      this->m_productId == rhs.m_productId &&
      this->m_type      == rhs.m_type
   };

   // If either object is an outline (see comment in model/OutlineableNamedEntity.h) then there is no point comparing
   // any more fields.  Note that an object will only be an outline whilst it is being read in from a BeerJSON file.
   if (this->m_outline || rhs.m_outline) {
      return outlinesAreEqual;
   }

   return (
      outlinesAreEqual &&

      // Remaining BeerJSON fields -- excluding inventories
      this->m_useFor == rhs.m_useFor &&
      this->m_notes  == rhs.m_notes
   );
}

ObjectStore & Misc::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Misc>::getInstance();
}

TypeLookup const Misc::typeLookup {
   "Misc",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::notes    , Misc::m_notes    , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::type     , Misc::m_type     , NonPhysicalQuantity::Enum  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::useFor   , Misc::m_useFor   , NonPhysicalQuantity::String),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::producer , Misc::m_producer , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Misc::productId, Misc::m_productId, NonPhysicalQuantity::String),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Misc>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Misc>::value);

//============================CONSTRUCTORS======================================

Misc::Misc(QString name) :
   Ingredient{name},
   m_type          {Misc::Type::Spice},
   m_useFor        {""               },
   m_notes         {""               },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_producer      {""               },
   m_productId     {""               } {
   return;
}

Misc::Misc(NamedParameterBundle const & namedParameterBundle) :
   Ingredient{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_type     , namedParameterBundle, PropertyNames::Misc::type     ),
   SET_REGULAR_FROM_NPB (m_useFor   , namedParameterBundle, PropertyNames::Misc::useFor   ),
   SET_REGULAR_FROM_NPB (m_notes    , namedParameterBundle, PropertyNames::Misc::notes    ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_producer , namedParameterBundle, PropertyNames::Misc::producer ),
   SET_REGULAR_FROM_NPB (m_productId, namedParameterBundle, PropertyNames::Misc::productId) {

   return;
}

Misc::Misc(Misc const & other) :
   Ingredient{other             },
   m_type     {other.m_type     },
   m_useFor   {other.m_useFor   },
   m_notes    {other.m_notes    },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_producer {other.m_producer },
   m_productId{other.m_productId} {
   return;
}

Misc::~Misc() = default;

//============================"GET" METHODS=====================================
Misc::Type Misc::type          () const { return m_type     ; }
QString    Misc::useFor        () const { return m_useFor   ; }
QString    Misc::notes         () const { return m_notes    ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
QString    Misc::producer      () const { return m_producer ; }
QString    Misc::productId     () const { return m_productId; }

//============================"SET" METHODS=====================================
void Misc::setType     (Type    const   val) { SET_AND_NOTIFY( PropertyNames::Misc::type    , this->m_type     , val); }
void Misc::setUseFor   (QString const & val) { SET_AND_NOTIFY( PropertyNames::Misc::useFor  , this->m_useFor   , val); }
void Misc::setNotes    (QString const & val) { SET_AND_NOTIFY( PropertyNames::Misc::notes   , this->m_notes    , val); }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Misc::setProducer (QString const & val) { SET_AND_NOTIFY(PropertyNames::Misc::producer , this->m_producer , val); }
void Misc::setProductId(QString const & val) { SET_AND_NOTIFY(PropertyNames::Misc::productId, this->m_productId, val); }

//========================OTHER METHODS=========================================

// Insert the boiler-plate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Misc)
