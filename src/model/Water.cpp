/*
 * model/Water.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "model/Water.h"

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

bool Water::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Water const & rhs = static_cast<Water const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_calcium_ppm      == rhs.m_calcium_ppm      &&
      this->m_bicarbonate_ppm  == rhs.m_bicarbonate_ppm  &&
      this->m_sulfate_ppm      == rhs.m_sulfate_ppm      &&
      this->m_chloride_ppm     == rhs.m_chloride_ppm     &&
      this->m_sodium_ppm       == rhs.m_sodium_ppm       &&
      this->m_magnesium_ppm    == rhs.m_magnesium_ppm    &&
      this->m_ph               == rhs.m_ph
   );
}

ObjectStore & Water::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Water>::getInstance();
}

Water::Water(QString name) :
   NamedEntity         {name, true},
   m_amount            {0.0               },
   m_calcium_ppm       {0.0               },
   m_bicarbonate_ppm   {0.0               },
   m_sulfate_ppm       {0.0               },
   m_chloride_ppm      {0.0               },
   m_sodium_ppm        {0.0               },
   m_magnesium_ppm     {0.0               },
   m_ph                {0.0               },
   m_alkalinity        {0.0               },
   m_notes             {""                },
   m_type              {Water::Types::NONE},
   m_mash_ro           {0.0               },
   m_sparge_ro         {0.0               },
   m_alkalinity_as_hco3{true              } {
   return;
}

Water::Water(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity         {namedParameterBundle},
   m_amount            {namedParameterBundle.val<double      >(PropertyNames::Water::amount          )},
   m_calcium_ppm       {namedParameterBundle.val<double      >(PropertyNames::Water::calcium_ppm     )},
   m_bicarbonate_ppm   {namedParameterBundle.val<double      >(PropertyNames::Water::bicarbonate_ppm )},
   m_sulfate_ppm       {namedParameterBundle.val<double      >(PropertyNames::Water::sulfate_ppm     )},
   m_chloride_ppm      {namedParameterBundle.val<double      >(PropertyNames::Water::chloride_ppm    )},
   m_sodium_ppm        {namedParameterBundle.val<double      >(PropertyNames::Water::sodium_ppm      )},
   m_magnesium_ppm     {namedParameterBundle.val<double      >(PropertyNames::Water::magnesium_ppm   )},
   m_ph                {namedParameterBundle.val<double      >(PropertyNames::Water::ph              )},
   m_alkalinity        {namedParameterBundle.val<double      >(PropertyNames::Water::alkalinity      )},
   m_notes             {namedParameterBundle.val<QString     >(PropertyNames::Water::notes           )},
   m_type              {namedParameterBundle.val<Water::Types>(PropertyNames::Water::type            )},
   m_mash_ro           {namedParameterBundle.val<double      >(PropertyNames::Water::mashRO          )},
   m_sparge_ro         {namedParameterBundle.val<double      >(PropertyNames::Water::spargeRO        )},
   m_alkalinity_as_hco3{namedParameterBundle.val<bool        >(PropertyNames::Water::alkalinityAsHCO3)} {
   return;
}

Water::Water(Water const& other) :
   NamedEntity         {other                     },
   m_amount            {other.m_amount            },
   m_calcium_ppm       {other.m_calcium_ppm       },
   m_bicarbonate_ppm   {other.m_bicarbonate_ppm   },
   m_sulfate_ppm       {other.m_sulfate_ppm       },
   m_chloride_ppm      {other.m_chloride_ppm      },
   m_sodium_ppm        {other.m_sodium_ppm        },
   m_magnesium_ppm     {other.m_magnesium_ppm     },
   m_ph                {other.m_ph                },
   m_alkalinity        {other.m_alkalinity        },
   m_notes             {other.m_notes             },
   m_type              {other.m_type              },
   m_mash_ro           {other.m_mash_ro           },
   m_sparge_ro         {other.m_sparge_ro         },
   m_alkalinity_as_hco3{other.m_alkalinity_as_hco3} {
   return;
}

void Water::swap(Water & other) noexcept {
   this->NamedEntity::swap(other);
   std::swap(this->m_amount            , other.m_amount            );
   std::swap(this->m_calcium_ppm       , other.m_calcium_ppm       );
   std::swap(this->m_bicarbonate_ppm   , other.m_bicarbonate_ppm   );
   std::swap(this->m_sulfate_ppm       , other.m_sulfate_ppm       );
   std::swap(this->m_chloride_ppm      , other.m_chloride_ppm      );
   std::swap(this->m_sodium_ppm        , other.m_sodium_ppm        );
   std::swap(this->m_magnesium_ppm     , other.m_magnesium_ppm     );
   std::swap(this->m_ph                , other.m_ph                );
   std::swap(this->m_alkalinity        , other.m_alkalinity        );
   std::swap(this->m_notes             , other.m_notes             );
   std::swap(this->m_type              , other.m_type              );
   std::swap(this->m_mash_ro           , other.m_mash_ro           );
   std::swap(this->m_sparge_ro         , other.m_sparge_ro         );
   std::swap(this->m_alkalinity_as_hco3, other.m_alkalinity_as_hco3);
   return;
}

Water::~Water() = default;
/*Water::~Water() {
   qDebug() <<
      Q_FUNC_INFO << "Deleting Water #" << this->key() << ":" << this->name() << "@" << static_cast<void *>(this);
   ObjectStoreWrapper::logDiagnostics<Water>();
   return;
}*/

Water & Water::operator=(Water other) {
   // Per https://en.wikibooks.org/wiki/More_C++_Idioms/Copy-and-swap and other places, the safest way to do assignment
   // is via the copy-and-swap idiom

   // I think it's a coding error if we're trying to assign to ourselves
   Q_ASSERT(this != &other);

   this->swap(other);

   // Using swap means we have bypassed all the magic of setAndNotify.  So we need to do a couple of things here:
   //   - if we are already stored in the DB then we need to update the data there
   //   - we need to issue the notifications for properties that changed as a result of the assignment
   if (this->key() > 0) {
      // We have to be careful not to create a new shared pointer for the object, but instead to get a copy of the one
      // held by the object store.
      qDebug() <<
         Q_FUNC_INFO << "After assignment, updating Water #" << this->key() << "(" << this->name() << ") @" <<
         static_cast<void *>(this) << "in DB";
      ObjectStoreWrapper::update(*this);
   }
   if (this->m_amount             != other.m_amount            ) { this->propagatePropertyChange(PropertyNames::Water::amount          ); }
   if (this->m_calcium_ppm        != other.m_calcium_ppm       ) { this->propagatePropertyChange(PropertyNames::Water::calcium_ppm     ); }
   if (this->m_bicarbonate_ppm    != other.m_bicarbonate_ppm   ) { this->propagatePropertyChange(PropertyNames::Water::bicarbonate_ppm ); }
   if (this->m_sulfate_ppm        != other.m_sulfate_ppm       ) { this->propagatePropertyChange(PropertyNames::Water::chloride_ppm    ); }
   if (this->m_chloride_ppm       != other.m_chloride_ppm      ) { this->propagatePropertyChange(PropertyNames::Water::sodium_ppm      ); }
   if (this->m_sodium_ppm         != other.m_sodium_ppm        ) { this->propagatePropertyChange(PropertyNames::Water::magnesium_ppm   ); }
   if (this->m_magnesium_ppm      != other.m_magnesium_ppm     ) { this->propagatePropertyChange(PropertyNames::Water::ph              ); }
   if (this->m_ph                 != other.m_ph                ) { this->propagatePropertyChange(PropertyNames::Water::alkalinity      ); }
   if (this->m_alkalinity         != other.m_alkalinity        ) { this->propagatePropertyChange(PropertyNames::Water::sulfate_ppm     ); }
   if (this->m_notes              != other.m_notes             ) { this->propagatePropertyChange(PropertyNames::Water::notes           ); }
   if (this->m_type               != other.m_type              ) { this->propagatePropertyChange(PropertyNames::Water::type            ); }
   if (this->m_mash_ro            != other.m_mash_ro           ) { this->propagatePropertyChange(PropertyNames::Water::mashRO          ); }
   if (this->m_sparge_ro          != other.m_sparge_ro         ) { this->propagatePropertyChange(PropertyNames::Water::spargeRO        ); }
   if (this->m_alkalinity_as_hco3 != other.m_alkalinity_as_hco3) { this->propagatePropertyChange(PropertyNames::Water::alkalinityAsHCO3); }

   return *this;
}

//================================"SET" METHODS=================================
void Water::setAmount(double var)          { this->setAndNotify(PropertyNames::Water::amount,           this->m_amount,             var); }
void Water::setCalcium_ppm(double var)     { this->setAndNotify(PropertyNames::Water::calcium_ppm,      this->m_calcium_ppm,        var); }
void Water::setBicarbonate_ppm(double var) { this->setAndNotify(PropertyNames::Water::bicarbonate_ppm,  this->m_bicarbonate_ppm,    var); }
void Water::setChloride_ppm(double var)    { this->setAndNotify(PropertyNames::Water::chloride_ppm,     this->m_chloride_ppm,       var); }
void Water::setSodium_ppm(double var)      { this->setAndNotify(PropertyNames::Water::sodium_ppm,       this->m_sodium_ppm,         var); }
void Water::setMagnesium_ppm(double var)   { this->setAndNotify(PropertyNames::Water::magnesium_ppm,    this->m_magnesium_ppm,      var); }
void Water::setPh(double var)              { this->setAndNotify(PropertyNames::Water::ph,               this->m_ph,                 var); }
void Water::setAlkalinity(double var)      { this->setAndNotify(PropertyNames::Water::alkalinity,       this->m_alkalinity,         var); }
void Water::setSulfate_ppm(double var)     { this->setAndNotify(PropertyNames::Water::sulfate_ppm,      this->m_sulfate_ppm,        var); }
void Water::setNotes(QString const & var)  { this->setAndNotify(PropertyNames::Water::notes,            this->m_notes,              var); }
void Water::setType(Types var)             { this->setAndNotify(PropertyNames::Water::type,             this->m_type,               var); }
void Water::setMashRO(double var)          { this->setAndNotify(PropertyNames::Water::mashRO,           this->m_mash_ro,            var); }
void Water::setSpargeRO(double var)        { this->setAndNotify(PropertyNames::Water::spargeRO,         this->m_sparge_ro,          var); }
void Water::setAlkalinityAsHCO3(bool var)  { this->setAndNotify(PropertyNames::Water::alkalinityAsHCO3, this->m_alkalinity_as_hco3, var); }

//=========================="GET" METHODS=======================================
QString      Water::notes()            const { return this->m_notes;              }
double       Water::sulfate_ppm()      const { return this->m_sulfate_ppm;        }
double       Water::amount()           const { return this->m_amount;             }
double       Water::calcium_ppm()      const { return this->m_calcium_ppm;        }
double       Water::bicarbonate_ppm()  const { return this->m_bicarbonate_ppm;    }
double       Water::chloride_ppm()     const { return this->m_chloride_ppm;       }
double       Water::sodium_ppm()       const { return this->m_sodium_ppm;         }
double       Water::magnesium_ppm()    const { return this->m_magnesium_ppm;      }
double       Water::ph()               const { return this->m_ph;                 }
double       Water::alkalinity()       const { return this->m_alkalinity;         }
Water::Types Water::type()             const { return this->m_type;               }
double       Water::mashRO()           const { return this->m_mash_ro;            }
double       Water::spargeRO()         const { return this->m_sparge_ro;          }
bool         Water::alkalinityAsHCO3() const { return this->m_alkalinity_as_hco3; }

double Water::ppm(Water::Ions const ion) const {
   switch(ion) {
      case Water::Ions::Ca:   return this->m_calcium_ppm;
      case Water::Ions::Cl:   return this->m_chloride_ppm;
      case Water::Ions::HCO3: return this->m_bicarbonate_ppm;
      case Water::Ions::Mg:   return this->m_magnesium_ppm;
      case Water::Ions::Na:   return this->m_sodium_ppm;
      case Water::Ions::SO4:  return this->m_sulfate_ppm;
      default: return 0.0;
   }

   return 0.0;
}

Recipe * Water::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
