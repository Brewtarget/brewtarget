/*
 * model/MashStep.cpp is part of Brewtarget, and is Copyright the following
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
#include "model/MashStep.h"

#include <QDebug>
#include <QVector>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "PhysicalConstants.h"

QStringList const MashStep::types{"Infusion", "Temperature", "Decoction", "Fly Sparge", "Batch Sparge"};

namespace {
   QStringList typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction") << QObject::tr("Fly Sparge") << QObject::tr("Batch Sparge");
}

bool MashStep::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   MashStep const & rhs = static_cast<MashStep const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type              == rhs.m_type              &&
      this->m_infuseAmount_l    == rhs.m_infuseAmount_l    &&
      this->m_stepTemp_c        == rhs.m_stepTemp_c        &&
      this->m_stepTime_min      == rhs.m_stepTime_min      &&
      this->m_rampTime_min      == rhs.m_rampTime_min      &&
      this->m_endTemp_c         == rhs.m_endTemp_c         &&
      this->m_infuseTemp_c      == rhs.m_infuseTemp_c      &&
      this->m_decoctionAmount_l == rhs.m_decoctionAmount_l &&
      this->m_stepNumber        == rhs.m_stepNumber
   );
}

ObjectStore & MashStep::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<MashStep>::getInstance();
}

TypeLookup const MashStep::typeLookup {
   "MashStep",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::decoctionAmount_l, MashStep::m_decoctionAmount_l),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::endTemp_c        , MashStep::m_endTemp_c        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::infuseAmount_l   , MashStep::m_infuseAmount_l   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::infuseTemp_c     , MashStep::m_infuseTemp_c     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::mashId           , MashStep::m_mashId           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::rampTime_min     , MashStep::m_rampTime_min     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::stepNumber       , MashStep::m_stepNumber       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::stepTemp_c       , MashStep::m_stepTemp_c       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::stepTime_min     , MashStep::m_stepTime_min     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::type             , MashStep::m_type             ),
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::MashStep::typeString       , MashStep::m_typeString       ),
   },
   // Parent class lookup
   &NamedEntity::typeLookup
};

//==============================CONSTRUCTORS====================================

MashStep::MashStep(QString name) :
   NamedEntity        {name, true},
   m_type             {MashStep::Type::Infusion},
   m_infuseAmount_l   {0.0                     },
   m_stepTemp_c       {0.0                     },
   m_stepTime_min     {0.0                     },
   m_rampTime_min     {0.0                     },
   m_endTemp_c        {0.0                     },
   m_infuseTemp_c     {0.0                     },
   m_decoctionAmount_l{0.0                     },
   m_stepNumber       {0                       },
   m_mashId           {-1                      } {
   return;
}

MashStep::MashStep(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity        (namedParameterBundle                                                                ),
   m_type             (namedParameterBundle.val<MashStep::Type>(PropertyNames::MashStep::type             )),
   m_infuseAmount_l   (namedParameterBundle.val<double        >(PropertyNames::MashStep::infuseAmount_l   )),
   m_stepTemp_c       (namedParameterBundle.val<double        >(PropertyNames::MashStep::stepTemp_c       )),
   m_stepTime_min     (namedParameterBundle.val<double        >(PropertyNames::MashStep::stepTime_min     )),
   m_rampTime_min     (namedParameterBundle.val<double        >(PropertyNames::MashStep::rampTime_min     )),
   m_endTemp_c        (namedParameterBundle.val<double        >(PropertyNames::MashStep::endTemp_c        )),
   m_infuseTemp_c     (namedParameterBundle.val<double        >(PropertyNames::MashStep::infuseTemp_c     )),
   m_decoctionAmount_l(namedParameterBundle.val<double        >(PropertyNames::MashStep::decoctionAmount_l)),
   m_stepNumber       (namedParameterBundle.val<int           >(PropertyNames::MashStep::stepNumber       )),
   m_mashId           (namedParameterBundle.val<int           >(PropertyNames::MashStep::mashId           )) {
   return;
}

MashStep::MashStep(MashStep const & other) :
   NamedEntity        {other},
   m_type             {other.m_type             },
   m_infuseAmount_l   {other.m_infuseAmount_l   },
   m_stepTemp_c       {other.m_stepTemp_c       },
   m_stepTime_min     {other.m_stepTime_min     },
   m_rampTime_min     {other.m_rampTime_min     },
   m_endTemp_c        {other.m_endTemp_c        },
   m_infuseTemp_c     {other.m_infuseTemp_c     },
   m_decoctionAmount_l{other.m_decoctionAmount_l},
   m_stepNumber       {other.m_stepNumber       },
   m_mashId           {other.m_mashId           } {
   return;
}

MashStep::~MashStep() = default;


//================================"SET" METHODS=================================
void MashStep::setInfuseTemp_c     (double val) { this->setAndNotify(PropertyNames::MashStep::infuseTemp_c     , this->m_infuseTemp_c     , val                                                                ); }
void MashStep::setType             (Type   val) { this->setAndNotify(PropertyNames::MashStep::type             , this->m_type             , val                                                                ); }
void MashStep::setInfuseAmount_l   (double val) { this->setAndNotify(PropertyNames::MashStep::infuseAmount_l   , this->m_infuseAmount_l   , this->enforceMin(val, "infuse amount"                             )); }
void MashStep::setStepTemp_c       (double val) { this->setAndNotify(PropertyNames::MashStep::stepTemp_c       , this->m_stepTemp_c       , this->enforceMin(val, "step temp", PhysicalConstants::absoluteZero)); }
void MashStep::setStepTime_min     (double val) { this->setAndNotify(PropertyNames::MashStep::stepTime_min     , this->m_stepTime_min     , this->enforceMin(val, "step time"                                 )); }
void MashStep::setRampTime_min     (double val) { this->setAndNotify(PropertyNames::MashStep::rampTime_min     , this->m_rampTime_min     , this->enforceMin(val, "ramp time"                                 )); }
void MashStep::setEndTemp_c        (double val) { this->setAndNotify(PropertyNames::MashStep::endTemp_c        , this->m_endTemp_c        , this->enforceMin(val, "end temp" , PhysicalConstants::absoluteZero)); }
void MashStep::setDecoctionAmount_l(double val) { this->setAndNotify(PropertyNames::MashStep::decoctionAmount_l, this->m_decoctionAmount_l, val                                                                ); }
void MashStep::setStepNumber       (int    val) { this->setAndNotify(PropertyNames::MashStep::stepNumber       , this->m_stepNumber       , val                                                                ); }

void MashStep::setMashId           (int    val) { this->m_mashId = val;    this->propagatePropertyChange(PropertyNames::MashStep::mashId, false);    return; }

//============================="GET" METHODS====================================
MashStep::Type MashStep::type() const { return m_type; }
const QString MashStep::typeString() const { return types.at(static_cast<int>(this->m_type)); }
const QString MashStep::typeStringTr() const {
   int myType = static_cast<int>(this->m_type);
   Q_ASSERT(myType >= 0);
   Q_ASSERT(myType < typesTr.length());
   return typesTr.at(myType);
}
double MashStep::infuseTemp_c     () const { return this->m_infuseTemp_c     ; }
double MashStep::infuseAmount_l   () const { return this->m_infuseAmount_l   ; }
double MashStep::stepTemp_c       () const { return this->m_stepTemp_c       ; }
double MashStep::stepTime_min     () const { return this->m_stepTime_min     ; }
double MashStep::rampTime_min     () const { return this->m_rampTime_min     ; }
double MashStep::endTemp_c        () const { return this->m_endTemp_c        ; }
double MashStep::decoctionAmount_l() const { return this->m_decoctionAmount_l; }
int    MashStep::stepNumber       () const { return this->m_stepNumber       ; }
int    MashStep::getMashId        () const { return this->m_mashId           ; }

bool MashStep::isInfusion() const {
   return ( m_type == MashStep::Type::Infusion    ||
            m_type == MashStep::Type::batchSparge ||
            m_type == MashStep::Type::flySparge );
}

bool MashStep::isSparge() const {
   return ( m_type == MashStep::Type::batchSparge ||
            m_type == MashStep::Type::flySparge   ||
            name() == "Final Batch Sparge" );
}

bool MashStep::isTemperature() const {
   return ( m_type == MashStep::Type::Temperature );
}

bool MashStep::isDecoction() const {
   return ( m_type == MashStep::Type::Decoction );
}

Recipe * MashStep::getOwningRecipe() {
   Mash * mash = ObjectStoreWrapper::getByIdRaw<Mash>(this->m_mashId);
   if (!mash) {
      return nullptr;
   }
   return mash->getOwningRecipe();
}
