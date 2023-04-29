/*
 * model/Fermentable.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Kregg K <gigatropolis@yahoo.com>
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
#include "model/Fermentable.h"

#include <QDebug>
#include <QObject>
#include <QVariant>

#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

std::array<Fermentable::Type, 5> const Fermentable::allTypes {
   Fermentable::Type::Dry_Extract,
   Fermentable::Type::Extract    ,
   Fermentable::Type::Grain      ,
   Fermentable::Type::Sugar      ,
   Fermentable::Type::Adjunct    ,
};

EnumStringMapping const Fermentable::typeStringMapping {
   {"dry extract", Fermentable::Type::Dry_Extract},
   {"extract",     Fermentable::Type::Extract},
   {"grain",       Fermentable::Type::Grain},
   {"sugar",       Fermentable::Type::Sugar},
   {"adjunct",     Fermentable::Type::Adjunct}   // .:TODO:. Needs to change to "other" when we finish BeerJSON work
};

QMap<Fermentable::Type, QString> const Fermentable::typeDisplayNames {
   {Fermentable::Type::Dry_Extract,   tr("Dry Extract"  )},
   {Fermentable::Type::Extract,       tr("Extract"      )},
   {Fermentable::Type::Grain,         tr("Grain"        )},
   {Fermentable::Type::Sugar,         tr("Sugar"        )},
   {Fermentable::Type::Adjunct,       tr("Adjunct"      )},
};

bool Fermentable::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentable const & rhs = static_cast<Fermentable const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type           == rhs.m_type           &&
      this->m_yield_pct       == rhs.m_yield_pct       &&
      this->m_color_srm       == rhs.m_color_srm       &&
      this->m_origin         == rhs.m_origin         &&
      this->m_supplier       == rhs.m_supplier       &&
      this->m_coarseFineDiff_pct == rhs.m_coarseFineDiff_pct &&
      this->m_moisture_pct    == rhs.m_moisture_pct    &&
      this->m_diastaticPower_lintner == rhs.m_diastaticPower_lintner &&
      this->m_protein_pct     == rhs.m_protein_pct     &&
      this->m_maxInBatch_pct  == rhs.m_maxInBatch_pct
   );
}

ObjectStore & Fermentable::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Fermentable>::getInstance();
}

TypeLookup const Fermentable::typeLookup {
   "Fermentable",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::type                  , Fermentable::m_type                  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::amount_kg             , Fermentable::m_amount_kg             , Measurement::PhysicalQuantity::Mass          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::yield_pct             , Fermentable::m_yield_pct             ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::color_srm             , Fermentable::m_color_srm             , Measurement::PhysicalQuantity::Color         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::addAfterBoil          , Fermentable::m_addAfterBoil          ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::origin                , Fermentable::m_origin                ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::supplier              , Fermentable::m_supplier              ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::notes                 , Fermentable::m_notes                 ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::coarseFineDiff_pct    , Fermentable::m_coarseFineDiff_pct    ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::moisture_pct          , Fermentable::m_moisture_pct          ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::diastaticPower_lintner, Fermentable::m_diastaticPower_lintner, Measurement::PhysicalQuantity::DiastaticPower),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::protein_pct           , Fermentable::m_protein_pct           ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::maxInBatch_pct        , Fermentable::m_maxInBatch_pct        ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::recommendMash         , Fermentable::m_recommendMash         ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::ibuGalPerLb           , Fermentable::m_ibuGalPerLb           ,           NonPhysicalQuantity::Dimensionless ), // Not really dimensionless...
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::isMashed              , Fermentable::m_isMashed              ,           NonPhysicalQuantity::Bool          ),
   },
   // Parent class lookup.  NB: NamedEntityWithInventory not NamedEntity!
   &NamedEntityWithInventory::typeLookup
};
static_assert(std::is_base_of<NamedEntityWithInventory, Fermentable>::value);

Fermentable::Fermentable(QString name) :
   NamedEntityWithInventory{name, true},
   m_typeStr               {QString()         },
   m_type                  {Fermentable::Type::Grain},
   m_amount_kg             {0.0               },
   m_yield_pct             {0.0               },
   m_color_srm             {0.0               },
   m_addAfterBoil           {false             },
   m_origin                {QString()         },
   m_supplier              {QString()         },
   m_notes                 {QString()         },
   m_coarseFineDiff_pct    {0.0               },
   m_moisture_pct          {0.0               },
   m_diastaticPower_lintner{0.0               },
   m_protein_pct           {0.0               },
   m_maxInBatch_pct        {100.0             },
   m_recommendMash         {false             },
   m_ibuGalPerLb           {0.0               },
   m_isMashed              {false             } {
   return;
}

Fermentable::Fermentable(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_typeStr               {QString()},
   m_type                  {namedParameterBundle.val<Fermentable::Type             >(PropertyNames::Fermentable::type                             )},
   m_amount_kg              {namedParameterBundle.val<double           >(PropertyNames::Fermentable::amount_kg                        )},
   m_yield_pct              {namedParameterBundle.val<double           >(PropertyNames::Fermentable::yield_pct                        )},
   m_color_srm              {namedParameterBundle.val<double           >(PropertyNames::Fermentable::color_srm                        )},
   m_addAfterBoil           {namedParameterBundle.val<bool             >(PropertyNames::Fermentable::addAfterBoil                     )},
   m_origin                {namedParameterBundle.val<QString          >(PropertyNames::Fermentable::origin                , QString())},
   m_supplier              {namedParameterBundle.val<QString          >(PropertyNames::Fermentable::supplier              , QString())},
   m_notes                 {namedParameterBundle.val<QString          >(PropertyNames::Fermentable::notes                 , QString())},
   m_coarseFineDiff_pct        {namedParameterBundle.val<double           >(PropertyNames::Fermentable::coarseFineDiff_pct               )},
   m_moisture_pct           {namedParameterBundle.val<double           >(PropertyNames::Fermentable::moisture_pct                     )},
   m_diastaticPower_lintner        {namedParameterBundle.val<double           >(PropertyNames::Fermentable::diastaticPower_lintner           )},
   m_protein_pct            {namedParameterBundle.val<double           >(PropertyNames::Fermentable::protein_pct                      )},
   m_maxInBatch_pct         {namedParameterBundle.val<double           >(PropertyNames::Fermentable::maxInBatch_pct                   )},
   m_recommendMash         {namedParameterBundle.val<bool             >(PropertyNames::Fermentable::recommendMash                    )},
   m_ibuGalPerLb           {namedParameterBundle.val<double           >(PropertyNames::Fermentable::ibuGalPerLb                      )},
   m_isMashed              {namedParameterBundle.val<bool             >(PropertyNames::Fermentable::isMashed              , false    )} {
   return;
}

Fermentable::Fermentable(Fermentable const & other) :
   NamedEntityWithInventory{other                         },
   m_typeStr       {other.m_typeStr       },
   m_type                  {other.m_type                  },
   m_amount_kg      {other.m_amount_kg      },
   m_yield_pct      {other.m_yield_pct      },
   m_color_srm      {other.m_color_srm      },
   m_addAfterBoil   {other.m_addAfterBoil   },
   m_origin                {other.m_origin                },
   m_supplier              {other.m_supplier              },
   m_notes                 {other.m_notes                 },
   m_coarseFineDiff_pct{other.m_coarseFineDiff_pct},
   m_moisture_pct   {other.m_moisture_pct   },
   m_diastaticPower_lintner{other.m_diastaticPower_lintner},
   m_protein_pct    {other.m_protein_pct    },
   m_maxInBatch_pct {other.m_maxInBatch_pct },
   m_recommendMash         {other.m_recommendMash         },
   m_ibuGalPerLb           {other.m_ibuGalPerLb           },
   m_isMashed      {other.m_isMashed      } {
   return;
}

Fermentable::~Fermentable() = default;

// Gets

Fermentable::Type Fermentable::type() const { return m_type; }
double Fermentable::amount_kg() const { return m_amount_kg; }
double Fermentable::yield_pct() const { return m_yield_pct; }
double Fermentable::color_srm() const { return m_color_srm; }
bool Fermentable::addAfterBoil() const { return m_addAfterBoil; }
const QString Fermentable::origin() const { return m_origin; }
const QString Fermentable::supplier() const { return m_supplier; }
const QString Fermentable::notes() const { return m_notes; }
double Fermentable::coarseFineDiff_pct() const { return m_coarseFineDiff_pct; }
double Fermentable::moisture_pct() const { return m_moisture_pct; }
double Fermentable::diastaticPower_lintner() const { return m_diastaticPower_lintner; }
double Fermentable::protein_pct() const { return m_protein_pct; }
double Fermentable::maxInBatch_pct() const { return m_maxInBatch_pct; }
bool Fermentable::recommendMash() const { return m_recommendMash; }
double Fermentable::ibuGalPerLb() const { return m_ibuGalPerLb; }
bool Fermentable::isMashed() const { return m_isMashed; }

Fermentable::AdditionMethod Fermentable::additionMethod() const {
   if (this->isMashed()) {
      return Fermentable::AdditionMethod::Mashed;
   }

   if (this->type() == Fermentable::Type::Grain) {
      return Fermentable::AdditionMethod::Steeped;
   }

   return Fermentable::AdditionMethod::Not_Mashed;
}

Fermentable::AdditionTime Fermentable::additionTime() const {
   if (this->addAfterBoil()) {
      return Fermentable::AdditionTime::Late;
   }

   return Fermentable::AdditionTime::Normal;
}

const QString Fermentable::additionMethodStringTr() const {
   if (this->isMashed()) {
      return tr("Mashed");
   }

   if (this->type() == Fermentable::Type::Grain) {
      return tr("Steeped");
   }

   return tr("Not mashed");
}

const QString Fermentable::additionTimeStringTr() const {
   if (this->addAfterBoil()) {
      return tr("Late");
   }
   return tr("Normal");
}

bool Fermentable::isExtract() const {
   return ((type() == Fermentable::Type::Extract) || (type() == Fermentable::Type::Dry_Extract));
}

bool Fermentable::isSugar() const {
   return (type() == Fermentable::Type::Sugar);
}

// Sets
void Fermentable::setType( Type t ) {
   this->setAndNotify(PropertyNames::Fermentable::type, this->m_type, t);
}

void Fermentable::setAdditionMethod( Fermentable::AdditionMethod m ) {
   this->setIsMashed(m == Fermentable::AdditionMethod::Mashed);
}

void Fermentable::setAdditionTime( Fermentable::AdditionTime t ) {
   this->setAddAfterBoil(t == Fermentable::AdditionTime::Late);
}

void Fermentable::setAddAfterBoil( bool b ) {
   this->setAndNotify(PropertyNames::Fermentable::addAfterBoil, this->m_addAfterBoil, b);
}

void Fermentable::setOrigin( const QString& str ) {
   this->setAndNotify( PropertyNames::Fermentable::origin, this->m_origin, str);
}

void Fermentable::setSupplier( const QString& str) {
   this->setAndNotify( PropertyNames::Fermentable::supplier, this->m_supplier, str);
   }

void Fermentable::setNotes( const QString& str ) {
   this->setAndNotify( PropertyNames::Fermentable::notes, this->m_notes, str);
}

void Fermentable::setRecommendMash( bool b ) {
   this->setAndNotify( PropertyNames::Fermentable::recommendMash, this->m_recommendMash, b);
}

void Fermentable::setIsMashed(bool var) {
   this->setAndNotify( PropertyNames::Fermentable::isMashed, this->m_isMashed, var);
}

void Fermentable::setIbuGalPerLb( double num ) {
   this->setAndNotify( PropertyNames::Fermentable::ibuGalPerLb, this->m_ibuGalPerLb, num);
}

double Fermentable::equivSucrose_kg() const {
   double ret = amount_kg() * yield_pct() * (1.0-moisture_pct()/100.0) / 100.0;

   // If this is a steeped grain...
   if (type() == Fermentable::Type::Grain && !isMashed() )
      return 0.60 * ret; // Reduce the yield by 60%.
   else
   return ret;
}

void Fermentable::setAmount_kg( double var ) {
   this->setAndNotify( PropertyNames::Fermentable::amount_kg, this->m_amount_kg, this->enforceMin(var, "amount"));
   return;
}

void Fermentable::setInventoryAmount(double num) {
   InventoryUtils::setAmount(*this, num);
   return;
}

double Fermentable::inventory() const {
   return InventoryUtils::getAmount(*this);
}

void Fermentable::setYield_pct(double var) {
   this->setAndNotify(PropertyNames::Fermentable::yield_pct, this->m_yield_pct, this->enforceMinAndMax(var, "amount", 0.0, 100.0));
}

void Fermentable::setColor_srm(double var) {
   this->setAndNotify(PropertyNames::Fermentable::color_srm, this->m_color_srm, this->enforceMin(var, "color"));
   }

void Fermentable::setCoarseFineDiff_pct(double var) {
   this->setAndNotify(PropertyNames::Fermentable::coarseFineDiff_pct, this->m_coarseFineDiff_pct, this->enforceMinAndMax(var, "coarseFineDiff", 0.0, 100.0));
}

void Fermentable::setMoisture_pct(double var) {
   this->setAndNotify(PropertyNames::Fermentable::moisture_pct, this->m_moisture_pct, this->enforceMinAndMax(var, "moisture", 0.0, 100.0));
}

void Fermentable::setDiastaticPower_lintner(double var) {
   this->setAndNotify( PropertyNames::Fermentable::diastaticPower_lintner, this->m_diastaticPower_lintner, this->enforceMin(var, "diastatic power"));
}

void Fermentable::setProtein_pct(double var) {
   this->setAndNotify( PropertyNames::Fermentable::protein_pct, this->m_protein_pct, this->enforceMinAndMax(var, "protein", 0.0, 100.0));
}

void Fermentable::setMaxInBatch_pct(double var) {
   this->setAndNotify( PropertyNames::Fermentable::maxInBatch_pct, this->m_maxInBatch_pct, this->enforceMinAndMax(var, "max in batch", 0.0, 100.0));
}

Recipe * Fermentable::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
