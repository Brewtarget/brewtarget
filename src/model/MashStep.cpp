/*
 * model/MashStep.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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

//==============================CONSTRUCTORS====================================

MashStep::MashStep(QString name) :
   NamedEntity        {name, true},
   m_type             {MashStep::Infusion},
   m_infuseAmount_l   {0.0},
   m_stepTemp_c       {0.0},
   m_stepTime_min     {0.0},
   m_rampTime_min     {0.0},
   m_endTemp_c        {0.0},
   m_infuseTemp_c     {0.0},
   m_decoctionAmount_l{0.0},
   m_stepNumber       {0},
   mashId             {-1} {
   return;
}

MashStep::MashStep(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity        {namedParameterBundle},
   m_type             {static_cast<MashStep::Type>(namedParameterBundle(PropertyNames::MashStep::type).toInt())},
   m_infuseAmount_l   {namedParameterBundle(PropertyNames::MashStep::infuseAmount_l   ).toDouble()},
   m_stepTemp_c       {namedParameterBundle(PropertyNames::MashStep::stepTemp_c       ).toDouble()},
   m_stepTime_min     {namedParameterBundle(PropertyNames::MashStep::stepTime_min     ).toDouble()},
   m_rampTime_min     {namedParameterBundle(PropertyNames::MashStep::rampTime_min     ).toDouble()},
   m_endTemp_c        {namedParameterBundle(PropertyNames::MashStep::endTemp_c        ).toDouble()},
   m_infuseTemp_c     {namedParameterBundle(PropertyNames::MashStep::infuseTemp_c     ).toDouble()},
   m_decoctionAmount_l{namedParameterBundle(PropertyNames::MashStep::decoctionAmount_l).toDouble()},
   m_stepNumber       {namedParameterBundle(PropertyNames::MashStep::stepNumber       ).toInt()},
   mashId             {namedParameterBundle(PropertyNames::MashStep::mashId           ).toInt()} {
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
   mashId             {other.mashId             } {
   return;
}


MashStep::~MashStep() = default;


//================================"SET" METHODS=================================
void MashStep::setInfuseTemp_c(double var) {
   this->setAndNotify(PropertyNames::MashStep::infuseTemp_c, this->m_infuseTemp_c, var);
}

void MashStep::setType(Type t) {
   this->setAndNotify(PropertyNames::MashStep::type, this->m_type, t);
}

void MashStep::setInfuseAmount_l(double var) {
   this->setAndNotify(PropertyNames::MashStep::infuseAmount_l, this->m_infuseAmount_l, this->enforceMin(var, "infuse amount"));
}

void MashStep::setStepTemp_c(double var) {
   this->setAndNotify(PropertyNames::MashStep::stepTemp_c, this->m_stepTemp_c, this->enforceMin(var, "step temp", PhysicalConstants::absoluteZero));
}

void MashStep::setStepTime_min(double var) {
   this->setAndNotify(PropertyNames::MashStep::stepTime_min, this->m_stepTime_min, this->enforceMin(var, "step time"));
}

void MashStep::setRampTime_min(double var) {
   this->setAndNotify(PropertyNames::MashStep::rampTime_min, this->m_rampTime_min, this->enforceMin(var, "ramp time"));
}

void MashStep::setEndTemp_c(double var) {
   this->setAndNotify(PropertyNames::MashStep::endTemp_c, this->m_endTemp_c, this->enforceMin(var, "end temp", PhysicalConstants::absoluteZero));
}

void MashStep::setDecoctionAmount_l(double var) {
   this->setAndNotify(PropertyNames::MashStep::decoctionAmount_l, this->m_decoctionAmount_l, var);
}


void MashStep::setStepNumber(int stepNumber) {
   this->setAndNotify(PropertyNames::MashStep::stepNumber, this->m_stepNumber, stepNumber);
}

void MashStep::setMashId(int mashId) {
   this->mashId = mashId;
   this->propagatePropertyChange(PropertyNames::MashStep::mashId, false);
   return;
}

//============================="GET" METHODS====================================
MashStep::Type MashStep::type() const { return m_type; }
const QString MashStep::typeString() const { return types.at(m_type); }
const QString MashStep::typeStringTr() const {
   if ( m_type < 0 || m_type > typesTr.length() ) {
      return "";
   }
   return typesTr.at(m_type);
}
double MashStep::infuseTemp_c() const { return m_infuseTemp_c; }
double MashStep::infuseAmount_l() const { return m_infuseAmount_l; }
double MashStep::stepTemp_c() const { return m_stepTemp_c; }
double MashStep::stepTime_min() const { return m_stepTime_min; }
double MashStep::rampTime_min() const { return m_rampTime_min; }
double MashStep::endTemp_c() const { return m_endTemp_c; }
double MashStep::decoctionAmount_l() const { return m_decoctionAmount_l; }
int MashStep::stepNumber() const { return m_stepNumber; }
//Mash * MashStep::mash( ) const { return m_mash; }
int MashStep::getMashId() const { return this->mashId; }

bool MashStep::isInfusion() const
{
   return ( m_type == MashStep::Infusion    ||
            m_type == MashStep::batchSparge ||
            m_type == MashStep::flySparge );
}

bool MashStep::isSparge() const
{
   return ( m_type == MashStep::batchSparge ||
            m_type == MashStep::flySparge   ||
            name() == "Final Batch Sparge" );
}

bool MashStep::isTemperature() const
{
   return ( m_type == MashStep::Temperature );
}

bool MashStep::isDecoction() const
{
   return ( m_type == MashStep::Decoction );
}

Recipe * MashStep::getOwningRecipe() {
   Mash * mash = ObjectStoreWrapper::getByIdRaw<Mash>(this->mashId);
   if (!mash) {
      return nullptr;
   }
   return mash->getOwningRecipe();
}
