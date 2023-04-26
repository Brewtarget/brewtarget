/*
 * model/Hop.cpp is part of Brewtarget, and is Copyright the following
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
#include "model/Hop.h"

#include <QDebug>
#include <QObject>

#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

EnumStringMapping const Hop::typeStringMapping {
   {"Aroma",                  Hop::Type::Aroma},
   {"Bittering",              Hop::Type::Bittering},
   {"Both",                   Hop::Type::Both},
};

EnumStringMapping const Hop::formStringMapping {
   {"Leaf",       Hop::Form::Leaf},
   {"Pellet",     Hop::Form::Pellet},
   {"Plug",       Hop::Form::Plug}
};

EnumStringMapping const Hop::useStringMapping {
   {"Mash",       Hop::Use::Mash},
   {"First Wort", Hop::Use::First_Wort},
   {"Boil",       Hop::Use::Boil},
   {"Aroma",      Hop::Use::Aroma},
   {"Dry Hop",    Hop::Use::Dry_Hop},
};

QMap<Hop::Type, QString> const Hop::typeDisplayNames {
   {Hop::Type::Aroma,                   tr("Aroma"                    )},
   {Hop::Type::Bittering,               tr("Bittering"                )},
   {Hop::Type::Both             ,       tr("Aroma & Bittering"        )},
};

QMap<Hop::Form, QString> const Hop::formDisplayNames {
   {Hop::Form::Leaf,    tr("Leaf"   )},
   {Hop::Form::Pellet,  tr("Pellet" )},
   {Hop::Form::Plug,    tr("Plug"   )},
};

QMap<Hop::Use, QString> const Hop::useDisplayNames {
   {Hop::Use::Mash,       tr("Mash"      )},
   {Hop::Use::First_Wort, tr("First Wort")},
   {Hop::Use::Boil,       tr("Boil"      )},
   {Hop::Use::Aroma,      tr("Post-Boil" )},
   {Hop::Use::Dry_Hop,    tr("Dry Hop"   )},
};

bool Hop::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Hop const & rhs = static_cast<Hop const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_use                   == rhs.m_use                   &&
      this->m_type                  == rhs.m_type                  &&
      this->m_form                  == rhs.m_form                  &&
      this->m_alpha_pct             == rhs.m_alpha_pct             &&
      this->m_beta_pct              == rhs.m_beta_pct              &&
      this->m_hsi_pct               == rhs.m_hsi_pct               &&
      this->m_origin                == rhs.m_origin                &&
      this->m_humulene_pct          == rhs.m_humulene_pct          &&
      this->m_caryophyllene_pct     == rhs.m_caryophyllene_pct     &&
      this->m_cohumulone_pct        == rhs.m_cohumulone_pct        &&
      this->m_myrcene_pct       == rhs.m_myrcene_pct
   );
}

ObjectStore & Hop::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Hop>::getInstance();
}

TypeLookup const Hop::typeLookup {
   "Hop",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::use                  , Hop::m_use                  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::type                 , Hop::m_type                 ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::form                 , Hop::m_form                 ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::alpha_pct            , Hop::m_alpha_pct            ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::amount_kg            , Hop::m_amount_kg            ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::time_min             , Hop::m_time_min             ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::notes                , Hop::m_notes                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::beta_pct             , Hop::m_beta_pct             ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::hsi_pct              , Hop::m_hsi_pct              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::origin               , Hop::m_origin               ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::substitutes          , Hop::m_substitutes          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::humulene_pct         , Hop::m_humulene_pct         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::caryophyllene_pct    , Hop::m_caryophyllene_pct    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::cohumulone_pct       , Hop::m_cohumulone_pct       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::myrcene_pct          , Hop::m_myrcene_pct          ),
   },
   // Parent class lookup.  NB: NamedEntityWithInventory not NamedEntity!
   &NamedEntityWithInventory::typeLookup
};
static_assert(std::is_base_of<NamedEntityWithInventory, Hop>::value);

Hop::Hop(QString name) :
   NamedEntityWithInventory{name, true},
   m_use                  {Hop::Use::Mash},
   m_type                 {Hop::Type::Bittering},
   m_form                 {Hop::Form::Leaf},
   m_alpha_pct            {0.0},
   m_amount_kg            {0.0},
   m_time_min             {0.0},
   m_notes                {"" },
   m_beta_pct             {0.0},
   m_hsi_pct              {0.0},
   m_origin               {"" },
   m_substitutes          {"" },
   m_humulene_pct         {0.0},
   m_caryophyllene_pct    {0.0},
   m_cohumulone_pct       {0.0},
   m_myrcene_pct      {0.0} {
   return;
}

Hop::Hop(NamedParameterBundle const & namedParameterBundle) :
   NamedEntityWithInventory{namedParameterBundle},
   m_use                  {namedParameterBundle.val<Hop::Use             >(PropertyNames::Hop::use                  )},
   m_type                 {namedParameterBundle.val<Hop::Type            >(PropertyNames::Hop::type                 )},
   m_form                 {namedParameterBundle.val<Hop::Form            >(PropertyNames::Hop::form                 )},
   m_alpha_pct            {namedParameterBundle.val<double               >(PropertyNames::Hop::alpha_pct            )},
   m_amount_kg            {namedParameterBundle.val<double               >(PropertyNames::Hop::amount_kg            )},
   m_time_min             {namedParameterBundle.val<double               >(PropertyNames::Hop::time_min             )},
   m_notes                {namedParameterBundle.val<QString              >(PropertyNames::Hop::notes                )},
   m_beta_pct             {namedParameterBundle.val<double               >(PropertyNames::Hop::beta_pct             )},
   m_hsi_pct              {namedParameterBundle.val<double               >(PropertyNames::Hop::hsi_pct              )},
   m_origin               {namedParameterBundle.val<QString              >(PropertyNames::Hop::origin               )},
   m_substitutes          {namedParameterBundle.val<QString              >(PropertyNames::Hop::substitutes          )},
   m_humulene_pct         {namedParameterBundle.val<double               >(PropertyNames::Hop::humulene_pct         )},
   m_caryophyllene_pct    {namedParameterBundle.val<double               >(PropertyNames::Hop::caryophyllene_pct    )},
   m_cohumulone_pct       {namedParameterBundle.val<double               >(PropertyNames::Hop::cohumulone_pct       )},
   m_myrcene_pct          {namedParameterBundle.val<double   >(PropertyNames::Hop::myrcene_pct          )} {
   return;
}

Hop::Hop(Hop const & other) :
   NamedEntityWithInventory{other                        },
   m_use                   {other.m_use                  },
   m_type                  {other.m_type                 },
   m_form                  {other.m_form                 },
   m_alpha_pct             {other.m_alpha_pct            },
   m_amount_kg             {other.m_amount_kg            },
   m_time_min              {other.m_time_min             },
   m_notes                 {other.m_notes                },
   m_beta_pct              {other.m_beta_pct             },
   m_hsi_pct               {other.m_hsi_pct              },
   m_origin                {other.m_origin               },
   m_substitutes           {other.m_substitutes          },
   m_humulene_pct          {other.m_humulene_pct         },
   m_caryophyllene_pct     {other.m_caryophyllene_pct    },
   m_cohumulone_pct        {other.m_cohumulone_pct       },
   m_myrcene_pct           {other.m_myrcene_pct      } {
   return;
}

Hop::~Hop() = default;

//============================="GET" METHODS====================================
Hop::Use              Hop::use()                   const { return this->m_use;                   }
QString               Hop::notes()                 const { return this->m_notes;                 }
Hop::Type             Hop::type()                  const { return this->m_type;                  }
Hop::Form             Hop::form()                  const { return this->m_form;                  }
QString               Hop::origin()                const { return this->m_origin;                }
QString               Hop::substitutes()           const { return this->m_substitutes;           }
double                Hop::alpha_pct()             const { return this->m_alpha_pct;             }
double                Hop::amount_kg()             const { return this->m_amount_kg;             }
double                Hop::time_min()              const { return this->m_time_min;              }
double                Hop::beta_pct()              const { return this->m_beta_pct;              }
double                Hop::hsi_pct()               const { return this->m_hsi_pct;               }
double                Hop::humulene_pct()          const { return this->m_humulene_pct;          }
double                Hop::caryophyllene_pct()     const { return this->m_caryophyllene_pct;     }
double                Hop::cohumulone_pct()        const { return this->m_cohumulone_pct;        }
double                Hop::myrcene_pct()           const { return this->m_myrcene_pct;           }

double Hop::inventory() const {
   return InventoryUtils::getAmount(*this);
}

//============================="SET" METHODS====================================
void Hop::setAlpha_pct            (double                const   val) { this->setAndNotify(PropertyNames::Hop::alpha_pct,             this->m_alpha_pct,             this->enforceMinAndMax(val, "alpha",                 0.0, 100.0)); }
void Hop::setAmount_kg            (double                const   val) { this->setAndNotify(PropertyNames::Hop::amount_kg,             this->m_amount_kg,             this->enforceMin      (val, "amount")                           ); }
void Hop::setUse                  (Hop::Use              const   val) { this->setAndNotify(PropertyNames::Hop::use,                   this->m_use,                   val                                                             ); }
void Hop::setTime_min             (double                const   val) { this->setAndNotify(PropertyNames::Hop::time_min,              this->m_time_min,              this->enforceMin      (val, "time")                             ); }
void Hop::setNotes                (QString               const & val) { this->setAndNotify(PropertyNames::Hop::notes,                 this->m_notes,                 val                                                             ); }
void Hop::setType                 (Hop::Type             const   val) { this->setAndNotify(PropertyNames::Hop::type,                  this->m_type,                  val                                                             ); }
void Hop::setForm                 (Hop::Form             const   val) { this->setAndNotify(PropertyNames::Hop::form,                  this->m_form,                  val                                                             ); }
void Hop::setBeta_pct             (double                const   val) { this->setAndNotify(PropertyNames::Hop::beta_pct,              this->m_beta_pct,              this->enforceMinAndMax(val, "beta",                  0.0, 100.0)); }
void Hop::setHsi_pct              (double                const   val) { this->setAndNotify(PropertyNames::Hop::hsi_pct,               this->m_hsi_pct,               this->enforceMinAndMax(val, "hsi",                   0.0, 100.0)); }
void Hop::setOrigin               (QString               const & val) { this->setAndNotify(PropertyNames::Hop::origin,                this->m_origin,                val                                                             ); }
void Hop::setSubstitutes          (QString               const & val) { this->setAndNotify(PropertyNames::Hop::substitutes,           this->m_substitutes,           val                                                             ); }
void Hop::setHumulene_pct         (double                const   val) { this->setAndNotify(PropertyNames::Hop::humulene_pct,          this->m_humulene_pct,          this->enforceMinAndMax(val, "humulene",              0.0, 100.0)); }
void Hop::setCaryophyllene_pct    (double                const   val) { this->setAndNotify(PropertyNames::Hop::caryophyllene_pct,     this->m_caryophyllene_pct,     this->enforceMinAndMax(val, "caryophyllene",         0.0, 100.0)); }
void Hop::setCohumulone_pct       (double                const   val) { this->setAndNotify(PropertyNames::Hop::cohumulone_pct,        this->m_cohumulone_pct,        this->enforceMinAndMax(val, "cohumulone",            0.0, 100.0)); }
void Hop::setMyrcene_pct          (double                const   val) { this->setAndNotify(PropertyNames::Hop::myrcene_pct,           this->m_myrcene_pct,           this->enforceMinAndMax(val, "myrcene",               0.0, 100.0)); }

void Hop::setInventoryAmount(double num) { InventoryUtils::setAmount(*this, num); }


Recipe * Hop::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}

bool hopLessThanByTime(Hop const * lhs, Hop const * rhs) {
   if (lhs->use() == rhs->use())    {
      if (lhs->time_min() == rhs->time_min()) {
         return lhs->name() < rhs->name();
      }
      return lhs->time_min() > rhs->time_min();
   }
   return lhs->use() < rhs->use();
}
