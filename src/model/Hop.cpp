/*
 * model/Hop.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "model/Hop.h"

#include <QDebug>
#include <QObject>

#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

namespace {
   QStringList types = QStringList() << "Bittering" << "Aroma" << "Both";
   QStringList forms = QStringList() << "Leaf" << "Pellet" << "Plug";
   QStringList uses = QStringList() << "Mash" << "First Wort" << "Boil" << "Aroma" << "Dry Hop";
}

bool Hop::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Hop const & rhs = static_cast<Hop const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_use               == rhs.m_use               &&
      this->m_type              == rhs.m_type              &&
      this->m_form              == rhs.m_form              &&
      this->m_alpha_pct         == rhs.m_alpha_pct         &&
      this->m_beta_pct          == rhs.m_beta_pct          &&
      this->m_hsi_pct           == rhs.m_hsi_pct           &&
      this->m_origin            == rhs.m_origin            &&
      this->m_humulene_pct      == rhs.m_humulene_pct      &&
      this->m_caryophyllene_pct == rhs.m_caryophyllene_pct &&
      this->m_cohumulone_pct    == rhs.m_cohumulone_pct    &&
      this->m_myrcene_pct       == rhs.m_myrcene_pct
   );
}

ObjectStore & Hop::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Hop>::getInstance();
}

Hop::Hop(QString name) :
   NamedEntityWithInventory{name, true},
   m_use              {Hop::Mash},
   m_type             {Hop::Bittering},
   m_form             {Hop::Leaf},
   m_alpha_pct        {0.0},
   m_amount_kg        {0.0},
   m_time_min         {0.0},
   m_notes            {"" },
   m_beta_pct         {0.0},
   m_hsi_pct          {0.0},
   m_origin           {"" },
   m_substitutes      {"" },
   m_humulene_pct     {0.0},
   m_caryophyllene_pct{0.0},
   m_cohumulone_pct   {0.0},
   m_myrcene_pct      {0.0} {
   return;
}

Hop::Hop(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_use                   {static_cast<Hop::Use>(namedParameterBundle(PropertyNames::Hop::use).toInt())},
   m_type                  {static_cast<Hop::Type>(namedParameterBundle(PropertyNames::Hop::type).toInt())},
   m_form                  {static_cast<Hop::Form>(namedParameterBundle(PropertyNames::Hop::form).toInt())},
   m_alpha_pct             {namedParameterBundle(PropertyNames::Hop::alpha_pct        ).toDouble()},
   m_amount_kg             {namedParameterBundle(PropertyNames::Hop::amount_kg        ).toDouble()},
   m_time_min              {namedParameterBundle(PropertyNames::Hop::time_min         ).toDouble()},
   m_notes                 {namedParameterBundle(PropertyNames::Hop::notes            ).toString()},
   m_beta_pct              {namedParameterBundle(PropertyNames::Hop::beta_pct         ).toDouble()},
   m_hsi_pct               {namedParameterBundle(PropertyNames::Hop::hsi_pct          ).toDouble()},
   m_origin                {namedParameterBundle(PropertyNames::Hop::origin           ).toString()},
   m_substitutes           {namedParameterBundle(PropertyNames::Hop::substitutes      ).toString()},
   m_humulene_pct          {namedParameterBundle(PropertyNames::Hop::humulene_pct     ).toDouble()},
   m_caryophyllene_pct     {namedParameterBundle(PropertyNames::Hop::caryophyllene_pct).toDouble()},
   m_cohumulone_pct        {namedParameterBundle(PropertyNames::Hop::cohumulone_pct   ).toDouble()},
   m_myrcene_pct           {namedParameterBundle(PropertyNames::Hop::myrcene_pct      ).toDouble()} {
   return;
}

Hop::Hop(Hop const & other) :
   NamedEntityWithInventory{other                    },
   m_use                   {other.m_use              },
   m_type                  {other.m_type             },
   m_form                  {other.m_form             },
   m_alpha_pct             {other.m_alpha_pct        },
   m_amount_kg             {other.m_amount_kg        },
   m_time_min              {other.m_time_min         },
   m_notes                 {other.m_notes            },
   m_beta_pct              {other.m_beta_pct         },
   m_hsi_pct               {other.m_hsi_pct          },
   m_origin                {other.m_origin           },
   m_substitutes           {other.m_substitutes      },
   m_humulene_pct          {other.m_humulene_pct     },
   m_caryophyllene_pct     {other.m_caryophyllene_pct},
   m_cohumulone_pct        {other.m_cohumulone_pct   },
   m_myrcene_pct           {other.m_myrcene_pct      } {
   return;
}

Hop::~Hop() {
//   qDebug() << Q_FUNC_INFO << "Destructor for Hop #" << this->key();
   return;
}

//============================="SET" METHODS====================================
void Hop::setAlpha_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::alpha_pct, this->m_alpha_pct, this->enforceMinAndMax(var, "alpha", 0.0, 100.0));
}

void Hop::setAmount_kg(double var) {
   this->setAndNotify(PropertyNames::Hop::amount_kg, this->m_amount_kg, this->enforceMin(var, "amount"));
}

void Hop::setInventoryAmount(double num) {
   InventoryUtils::setAmount(*this, num);
   return;
}

void Hop::setUse(Use u) {
   this->setAndNotify(PropertyNames::Hop::use, this->m_use, u);
}

void Hop::setTime_min(double var) {
   this->setAndNotify(PropertyNames::Hop::time_min, this->m_time_min, this->enforceMin(var, "time"));
}

void Hop::setNotes(QString const & str) {
   this->setAndNotify(PropertyNames::Hop::notes, this->m_notes, str);
}

void Hop::setType(Type t) {
   this->setAndNotify(PropertyNames::Hop::type, this->m_type, t);
}

void Hop::setForm(Form f) {
   this->setAndNotify(PropertyNames::Hop::form, this->m_form, f);
}

void Hop::setBeta_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::beta_pct, this->m_beta_pct, this->enforceMinAndMax(var, "beta", 0.0, 100.0));
}

void Hop::setHsi_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::hsi_pct, this->m_hsi_pct, this->enforceMinAndMax(var, "hsi", 0.0, 100.0));
}

void Hop::setOrigin(QString const & str) {
   this->setAndNotify(PropertyNames::Hop::origin, this->m_origin, str);
}

void Hop::setSubstitutes(QString const & str) {
   this->setAndNotify(PropertyNames::Hop::substitutes, this->m_substitutes, str);
}

void Hop::setHumulene_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::humulene_pct, this->m_humulene_pct, this->enforceMinAndMax(var, "humulene", 0.0, 100.0));
}

void Hop::setCaryophyllene_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::caryophyllene_pct, this->m_caryophyllene_pct, this->enforceMinAndMax(var, "caryophyllene", 0.0, 100.0));
}

void Hop::setCohumulone_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::cohumulone_pct, this->m_cohumulone_pct, this->enforceMinAndMax(var, "cohumulone", 0.0, 100.0));
}

void Hop::setMyrcene_pct(double var) {
   this->setAndNotify(PropertyNames::Hop::myrcene_pct, this->m_myrcene_pct, this->enforceMinAndMax(var, "myrcene", 0.0, 100.0));
}

//============================="GET" METHODS====================================

Hop::Use Hop::use() const { return m_use; }
const QString Hop::useString() const { return uses.at(m_use); }
const QString Hop::notes() const { return m_notes; }
Hop::Type Hop::type() const { return m_type; }
const QString Hop::typeString() const { return types.at(m_type); }
Hop::Form Hop::form() const { return m_form; }
const QString Hop::formString() const { return forms.at(m_form); }
const QString Hop::origin() const { return m_origin; }
const QString Hop::substitutes() const { return m_substitutes; }
double Hop::alpha_pct() const { return m_alpha_pct; }
double Hop::amount_kg() const { return m_amount_kg; }
double Hop::time_min() const { return m_time_min; }
double Hop::beta_pct() const { return m_beta_pct; }
double Hop::hsi_pct() const { return m_hsi_pct; }
double Hop::humulene_pct() const { return m_humulene_pct; }
double Hop::caryophyllene_pct() const { return m_caryophyllene_pct; }
double Hop::cohumulone_pct() const { return m_cohumulone_pct; }
double Hop::myrcene_pct() const { return m_myrcene_pct; }

double Hop::inventory() const {
   return InventoryUtils::getAmount(*this);
}

const QString Hop::useStringTr() const
{
   static QStringList usesTr = QStringList() << tr("Mash") << tr("First Wort") << tr("Boil") << tr("Aroma") << tr("Dry Hop") ;
   if ( m_use < usesTr.size() && m_use >= 0 ) {
      return usesTr.at(m_use);
   }
   else {
      return "";
   }
}

const QString Hop::typeStringTr() const
{
   static QStringList typesTr = QStringList() << tr("Bittering") << tr("Aroma") << tr("Both");
   if ( m_type < typesTr.size()  && m_type >= 0 ) {
      return typesTr.at(m_type);
   }
   else {
      return "";
   }
}

const QString Hop::formStringTr() const
{
   static QStringList formsTr = QStringList() << tr("Leaf") << tr("Pellet") << tr("Plug");
   if ( m_form < formsTr.size() && m_form >= 0 ) {
      return formsTr.at(m_form);
   }
   else {
      return "";
   }
}

Recipe * Hop::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
