/*
 * model/Yeast.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/Yeast.h"

#include <QDebug>

#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "PhysicalConstants.h"

namespace {
   QStringList const types{"Ale", "Lager", "Wheat", "Wine", "Champagne"};
   QStringList const forms{"Liquid", "Dry", "Slant", "Culture"};
   QStringList const flocculations{"Low", "Medium", "High", "Very High"};
}

bool Yeast::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Yeast const & rhs = static_cast<Yeast const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type         == rhs.m_type         &&
      this->m_form         == rhs.m_form         &&
      this->m_laboratory   == rhs.m_laboratory   &&
      this->m_productID    == rhs.m_productID    &&
      this->m_flocculation == rhs.m_flocculation
   );
}

ObjectStore & Yeast::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Yeast>::getInstance();
}

//============================CONSTRUCTORS======================================

Yeast::Yeast(QString name) :
   NamedEntityWithInventory{name, true},
   m_type                  {Yeast::Type::Ale},
   m_form                  {Yeast::Form::Liquid},
   m_flocculation          {Yeast::Flocculation::Low},
   m_amount                {0.0},
   m_amountIsWeight        {false},
   m_laboratory            {""},
   m_productID             {""},
   m_minTemperature_c      {0.0},
   m_maxTemperature_c      {0.0},
   m_attenuation_pct       {0.0},
   m_notes                 {""},
   m_bestFor               {""},
   m_timesCultured         {0},
   m_maxReuse              {0},
   m_addToSecondary        {false} {
   return;
}

Yeast::Yeast(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_type                  {namedParameterBundle.val<Yeast::Type        >(PropertyNames::Yeast::type            )},
   m_form                  {namedParameterBundle.val<Yeast::Form        >(PropertyNames::Yeast::form            )},
   m_flocculation          {namedParameterBundle.val<Yeast::Flocculation>(PropertyNames::Yeast::flocculation    )},
   m_amount                {namedParameterBundle.val<double             >(PropertyNames::Yeast::amount          )},
   m_amountIsWeight        {namedParameterBundle.val<bool               >(PropertyNames::Yeast::amountIsWeight  )},
   m_laboratory            {namedParameterBundle.val<QString            >(PropertyNames::Yeast::laboratory      )},
   m_productID             {namedParameterBundle.val<QString            >(PropertyNames::Yeast::productID       )},
   m_minTemperature_c      {namedParameterBundle.val<double             >(PropertyNames::Yeast::minTemperature_c)},
   m_maxTemperature_c      {namedParameterBundle.val<double             >(PropertyNames::Yeast::maxTemperature_c)},
   m_attenuation_pct       {namedParameterBundle.val<double             >(PropertyNames::Yeast::attenuation_pct )},
   m_notes                 {namedParameterBundle.val<QString            >(PropertyNames::Yeast::notes           )},
   m_bestFor               {namedParameterBundle.val<QString            >(PropertyNames::Yeast::bestFor         )},
   m_timesCultured         {namedParameterBundle.val<int                >(PropertyNames::Yeast::timesCultured   )},
   m_maxReuse              {namedParameterBundle.val<int                >(PropertyNames::Yeast::maxReuse        )},
   m_addToSecondary        {namedParameterBundle.val<bool               >(PropertyNames::Yeast::addToSecondary  )} {
   return;
}

Yeast::Yeast(Yeast const & other) :
   NamedEntityWithInventory{other                     },
   m_type                  {other.m_type              },
   m_form                  {other.m_form              },
   m_flocculation          {other.m_flocculation      },
   m_amount                {other.m_amount            },
   m_amountIsWeight        {other.m_amountIsWeight    },
   m_laboratory            {other.m_laboratory        },
   m_productID             {other.m_productID         },
   m_minTemperature_c      {other.m_minTemperature_c  },
   m_maxTemperature_c      {other.m_maxTemperature_c  },
   m_attenuation_pct       {other.m_attenuation_pct   },
   m_notes                 {other.m_notes             },
   m_bestFor               {other.m_bestFor           },
   m_timesCultured         {other.m_timesCultured     },
   m_maxReuse              {other.m_maxReuse          },
   m_addToSecondary        {other.m_addToSecondary    } {
   return;
}

Yeast::~Yeast() = default;

//============================="GET" METHODS====================================
QString Yeast::laboratory() const { return m_laboratory; }

QString Yeast::productID() const { return m_productID; }

QString Yeast::notes() const { return m_notes; }

QString Yeast::bestFor() const { return m_bestFor; }

const QString Yeast::typeString() const { return types.at(static_cast<int>(this->m_type)); }

const QString Yeast::formString() const { return forms.at(static_cast<int>(this->m_form)); }

const QString Yeast::flocculationString() const { return flocculations.at(static_cast<int>(this->m_flocculation)); }

double Yeast::amount() const { return m_amount; }

double Yeast::minTemperature_c() const { return m_minTemperature_c; }

double Yeast::maxTemperature_c() const { return m_maxTemperature_c; }

double Yeast::attenuation_pct() const { return m_attenuation_pct; }

double Yeast::inventory() const {
   return InventoryUtils::getAmount(*this);
}

int Yeast::timesCultured() const { return m_timesCultured; }

int Yeast::maxReuse() const { return m_maxReuse; }

bool Yeast::addToSecondary() const { return m_addToSecondary; }

bool Yeast::amountIsWeight() const { return m_amountIsWeight; }

Yeast::Form Yeast::form() const { return  m_form; }

Yeast::Flocculation Yeast::flocculation() const { return m_flocculation; }

Yeast::Type Yeast::type() const { return m_type; }

const QString Yeast::typeStringTr() const {
   static QStringList typesTr = QStringList() << QObject::tr("Ale")
                                       << QObject::tr("Lager")
                                       << QObject::tr("Wheat")
                                       << QObject::tr("Wine")
                                       << QObject::tr("Champagne");

   if ( m_type < typesTr.size() && m_type >= 0 ) {
      return typesTr.at(m_type);
   }
   return typesTr.at(0);
}

const QString Yeast::formStringTr() const {
   static QStringList formsTr = QStringList() << QObject::tr("Liquid")
                                       << QObject::tr("Dry")
                                       << QObject::tr("Slant")
                                       << QObject::tr("Culture");
   if ( m_form < formsTr.size() && m_form >= 0  ) {
      return formsTr.at(m_form);
   }
   return formsTr.at(0);
}

const QString Yeast::flocculationStringTr() const {
   static QStringList flocculationsTr = QStringList() << QObject::tr("Low")
                                               << QObject::tr("Medium")
                                               << QObject::tr("High")
                                               << QObject::tr("Very High");
   if ( m_flocculation < flocculationsTr.size() && m_flocculation >= 0 ) {
      return flocculationsTr.at(m_flocculation);
   }
   return flocculationsTr.at(0);
}

//============================="SET" METHODS====================================
void Yeast::setType(Yeast::Type t) {
   this->setAndNotify(PropertyNames::Yeast::type, this->m_type, t);
   return;
}

void Yeast::setForm(Yeast::Form f) {
   this->setAndNotify(PropertyNames::Yeast::form, this->m_form, f);
}

void Yeast::setAmount(double var) {
   this->setAndNotify(PropertyNames::Yeast::amount,
                      this->m_amount,
                      this->enforceMin(var, "amount", 0.0));
}

void Yeast::setInventoryAmount(double var) {
   InventoryUtils::setAmount(*this, var);
   return;
}

// .:TBD:. I'm not wild about using "quanta" here (presumably to mean number of packets or number of cultures)
//         Storing an int in a double is safe, so, for now, just leave this in place but as a wrapper around the more
//         generic setInventoryAmount().
void Yeast::setInventoryQuanta(int var) {
   this->setInventoryAmount(var);
   return;
}

void Yeast::setAmountIsWeight(bool var) {
   this->setAndNotify(PropertyNames::Yeast::amountIsWeight, this->m_amountIsWeight, var);
}

void Yeast::setLaboratory(QString const & var) {
   this->setAndNotify(PropertyNames::Yeast::laboratory, this->m_laboratory, var);
}

void Yeast::setProductID(QString const & var) {
   this->setAndNotify(PropertyNames::Yeast::productID, this->m_productID, var);
}

void Yeast::setMinTemperature_c(double var){
   // It seems a bit of overkill to enforce absolute zero as the lowest allowable temperature, but we do
   this->setAndNotify(PropertyNames::Yeast::minTemperature_c,
                      this->m_minTemperature_c,
                      this->enforceMin(var, "max temp", PhysicalConstants::absoluteZero, 0.0));
}

void Yeast::setMaxTemperature_c(double var) {
   // It seems a bit of overkill to enforce absolute zero as the lowest allowable temperature, but we do
   this->setAndNotify(PropertyNames::Yeast::maxTemperature_c,
                      this->m_maxTemperature_c,
                      this->enforceMin(var, "max temp", PhysicalConstants::absoluteZero, 0.0));
}

// Remember -- always make sure the value is in range before we set
// coredumps happen otherwise
void Yeast::setFlocculation(Yeast::Flocculation f) {
   this->setAndNotify(PropertyNames::Yeast::flocculation, this->m_flocculation, f);
}

void Yeast::setAttenuation_pct(double var) {
   this->setAndNotify(PropertyNames::Yeast::attenuation_pct,
                      this->m_attenuation_pct,
                      this->enforceMinAndMax(var, "pct attenuation", 0.0, 100.0, 0.0));
}

void Yeast::setNotes(QString const & var) {
   this->setAndNotify(PropertyNames::Yeast::notes, this->m_notes, var);
}

void Yeast::setBestFor(QString const & var) {
   this->setAndNotify(PropertyNames::Yeast::bestFor, this->m_bestFor, var);
}

void Yeast::setTimesCultured(int var) {
   this->setAndNotify(PropertyNames::Yeast::timesCultured,
                      this->m_timesCultured,
                      this->enforceMin(var, "times cultured"));
}

void Yeast::setMaxReuse(int var) {
   this->setAndNotify(PropertyNames::Yeast::maxReuse,
                      this->m_maxReuse,
                      this->enforceMin(var, "max reuse"));
}

void Yeast::setAddToSecondary( bool var ) {
   this->setAndNotify(PropertyNames::Yeast::addToSecondary, this->m_addToSecondary, var);
}

Recipe * Yeast::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
