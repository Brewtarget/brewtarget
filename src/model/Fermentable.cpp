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

QStringList Fermentable::types = QStringList() << "Grain" << "Sugar" << "Extract" << "Dry Extract" << "Adjunct";

bool Fermentable::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentable const & rhs = static_cast<Fermentable const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type           == rhs.m_type           &&
      this->m_yieldPct       == rhs.m_yieldPct       &&
      this->m_colorSrm       == rhs.m_colorSrm       &&
      this->m_origin         == rhs.m_origin         &&
      this->m_supplier       == rhs.m_supplier       &&
      this->m_coarseFineDiff == rhs.m_coarseFineDiff &&
      this->m_moisturePct    == rhs.m_moisturePct    &&
      this->m_diastaticPower == rhs.m_diastaticPower &&
      this->m_proteinPct     == rhs.m_proteinPct     &&
      this->m_maxInBatchPct  == rhs.m_maxInBatchPct
   );
}

ObjectStore & Fermentable::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Fermentable>::getInstance();
}

Fermentable::Fermentable(QString name) :
   NamedEntityWithInventory{name, true},
   m_typeStr       {QString()         },
   m_type          {Fermentable::Type::Grain},
   m_amountKg      {0.0               },
   m_yieldPct      {0.0               },
   m_colorSrm      {0.0               },
   m_isAfterBoil   {false             },
   m_origin        {QString()         },
   m_supplier      {QString()         },
   m_notes         {QString()         },
   m_coarseFineDiff{0.0               },
   m_moisturePct   {0.0               },
   m_diastaticPower{0.0               },
   m_proteinPct    {0.0               },
   m_maxInBatchPct {100.0             },
   m_recommendMash {false             },
   m_ibuGalPerLb   {0.0               },
   m_isMashed      {false             } {
   return;
}

Fermentable::Fermentable(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_typeStr               {QString()},
   m_type                  {namedParameterBundle.val<Fermentable::Type>(PropertyNames::Fermentable::type                             )},
   m_amountKg              {namedParameterBundle.val<double           >(PropertyNames::Fermentable::amount_kg                        )},
   m_yieldPct              {namedParameterBundle.val<double           >(PropertyNames::Fermentable::yield_pct                        )},
   m_colorSrm              {namedParameterBundle.val<double           >(PropertyNames::Fermentable::color_srm                        )},
   m_isAfterBoil           {namedParameterBundle.val<bool             >(PropertyNames::Fermentable::addAfterBoil                     )},
   m_origin                {namedParameterBundle.val<QString          >(PropertyNames::Fermentable::origin                , QString())},
   m_supplier              {namedParameterBundle.val<QString          >(PropertyNames::Fermentable::supplier              , QString())},
   m_notes                 {namedParameterBundle.val<QString          >(PropertyNames::Fermentable::notes                 , QString())},
   m_coarseFineDiff        {namedParameterBundle.val<double           >(PropertyNames::Fermentable::coarseFineDiff_pct               )},
   m_moisturePct           {namedParameterBundle.val<double           >(PropertyNames::Fermentable::moisture_pct                     )},
   m_diastaticPower        {namedParameterBundle.val<double           >(PropertyNames::Fermentable::diastaticPower_lintner           )},
   m_proteinPct            {namedParameterBundle.val<double           >(PropertyNames::Fermentable::protein_pct                      )},
   m_maxInBatchPct         {namedParameterBundle.val<double           >(PropertyNames::Fermentable::maxInBatch_pct                   )},
   m_recommendMash         {namedParameterBundle.val<bool             >(PropertyNames::Fermentable::recommendMash                    )},
   m_ibuGalPerLb           {namedParameterBundle.val<double           >(PropertyNames::Fermentable::ibuGalPerLb                      )},
   m_isMashed              {namedParameterBundle.val<bool             >(PropertyNames::Fermentable::isMashed              , false    )} {
   return;
}

Fermentable::Fermentable(Fermentable const & other) :
   NamedEntityWithInventory{other                 },
   m_typeStr       {other.m_typeStr       },
   m_type          {other.m_type          },
   m_amountKg      {other.m_amountKg      },
   m_yieldPct      {other.m_yieldPct      },
   m_colorSrm      {other.m_colorSrm      },
   m_isAfterBoil   {other.m_isAfterBoil   },
   m_origin        {other.m_origin        },
   m_supplier      {other.m_supplier      },
   m_notes         {other.m_notes         },
   m_coarseFineDiff{other.m_coarseFineDiff},
   m_moisturePct   {other.m_moisturePct   },
   m_diastaticPower{other.m_diastaticPower},
   m_proteinPct    {other.m_proteinPct    },
   m_maxInBatchPct {other.m_maxInBatchPct },
   m_recommendMash {other.m_recommendMash },
   m_ibuGalPerLb   {other.m_ibuGalPerLb   },
   m_isMashed      {other.m_isMashed      } {
   return;
}

Fermentable::~Fermentable() = default;

// Gets

Fermentable::Type Fermentable::type() const { return m_type; }
double Fermentable::amount_kg() const { return m_amountKg; }
double Fermentable::yield_pct() const { return m_yieldPct; }
double Fermentable::color_srm() const { return m_colorSrm; }
bool Fermentable::addAfterBoil() const { return m_isAfterBoil; }
const QString Fermentable::origin() const { return m_origin; }
const QString Fermentable::supplier() const { return m_supplier; }
const QString Fermentable::notes() const { return m_notes; }
double Fermentable::coarseFineDiff_pct() const { return m_coarseFineDiff; }
double Fermentable::moisture_pct() const { return m_moisturePct; }
double Fermentable::diastaticPower_lintner() const { return m_diastaticPower; }
double Fermentable::protein_pct() const { return m_proteinPct; }
double Fermentable::maxInBatch_pct() const { return m_maxInBatchPct; }
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

const QString Fermentable::typeString() const {
   int myType = static_cast<int>(this->type());
   if (myType > types.length()) {
      return "";
   }
   return types.at(myType);
}

const QString Fermentable::typeStringTr() const {
   int myType = static_cast<int>(this->type());
   static QStringList typesTr = QStringList () << QObject::tr("Grain") << QObject::tr("Sugar") << QObject::tr("Extract") << QObject::tr("Dry Extract") << QObject::tr("Adjunct");
   if (myType > typesTr.length() || myType < 0) {
      return "";
   }

   return typesTr.at(myType);
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

bool Fermentable::isValidType( const QString& str ) {
   return (types.indexOf(str) >= 0);
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
   this->setAndNotify(PropertyNames::Fermentable::addAfterBoil, this->m_isAfterBoil, b);
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
   this->setAndNotify( PropertyNames::Fermentable::amount_kg, this->m_amountKg, this->enforceMin(var, "amount"));
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
   this->setAndNotify(PropertyNames::Fermentable::yield_pct, this->m_yieldPct, this->enforceMinAndMax(var, "amount", 0.0, 100.0));
}

void Fermentable::setColor_srm(double var) {
   this->setAndNotify(PropertyNames::Fermentable::color_srm, this->m_colorSrm, this->enforceMin(var, "color"));
}

void Fermentable::setCoarseFineDiff_pct(double var) {
   this->setAndNotify(PropertyNames::Fermentable::coarseFineDiff_pct, this->m_coarseFineDiff, this->enforceMinAndMax(var, "coarseFineDiff", 0.0, 100.0));
}

void Fermentable::setMoisture_pct(double var) {
   this->setAndNotify(PropertyNames::Fermentable::moisture_pct, this->m_moisturePct, this->enforceMinAndMax(var, "moisture", 0.0, 100.0));
}

void Fermentable::setDiastaticPower_lintner(double var) {
   this->setAndNotify( PropertyNames::Fermentable::diastaticPower_lintner, this->m_diastaticPower, this->enforceMin(var, "diastatic power"));
}

void Fermentable::setProtein_pct(double var) {
   this->setAndNotify( PropertyNames::Fermentable::protein_pct, this->m_proteinPct, this->enforceMinAndMax(var, "protein", 0.0, 100.0));
}

void Fermentable::setMaxInBatch_pct(double var) {
   this->setAndNotify( PropertyNames::Fermentable::maxInBatch_pct, this->m_maxInBatchPct, this->enforceMinAndMax(var, "max in batch", 0.0, 100.0));
}

Recipe * Fermentable::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
