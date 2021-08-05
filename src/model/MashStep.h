/*
 * model/MashStep.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
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
#ifndef MODEL_MASHSTEP_H
#define MODEL_MASHSTEP_H
#pragma once

#include <QString>
#include <QStringList>

#include "model/Mash.h"
#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::MashStep { BtStringConst const property{#property}; }
AddPropertyName(decoctionAmount_l)
AddPropertyName(endTemp_c)
AddPropertyName(infuseAmount_l)
AddPropertyName(infuseTemp_c)
AddPropertyName(mashId)
AddPropertyName(rampTime_min)
AddPropertyName(stepNumber)
AddPropertyName(stepTemp_c)
AddPropertyName(stepTime_min)
AddPropertyName(typeString)
AddPropertyName(type)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class MashStep
 *
 * \brief Model for a mash step record in the database.
 */
class MashStep : public NamedEntity {
   Q_OBJECT
   Q_CLASSINFO("signal", "mashsteps")

   // this seems to be a class with a lot of friends

   friend class MashStepItemDelegate;
   friend class MashWizard;
   friend class MashDesigner;
   friend class MainWindow;
public:

   //! \brief The type of step.
   enum Type { Infusion, Temperature, Decoction, flySparge, batchSparge };
   Q_ENUMS( Type )

   MashStep(QString name = "", bool cache = true);
   MashStep(NamedParameterBundle const & namedParameterBundle);
   MashStep( MashStep const& other );

   virtual ~MashStep() = default;

   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The translated \c Type string.
   Q_PROPERTY( QString typeStringTr READ typeStringTr )
   //! \brief The untranslated \c Type string that we use in the database
   Q_PROPERTY( QString typeString READ typeString )
   //! \brief The infusion amount in liters.
   Q_PROPERTY( double infuseAmount_l READ infuseAmount_l WRITE setInfuseAmount_l /*NOTIFY changed*/ /*changedInfuseAmount_l*/ )
   //! \brief The target temperature of this step in C.
   Q_PROPERTY( double stepTemp_c READ stepTemp_c WRITE setStepTemp_c /*NOTIFY changed*/ /*changedStepTemp_c*/ )
   //! \brief The time of the step in min.
   Q_PROPERTY( double stepTime_min READ stepTime_min WRITE setStepTime_min /*NOTIFY changed*/ /*changedStepTime_min*/ )
   //! \brief The time it takes to ramp the temp to the target temp in min.
   Q_PROPERTY( double rampTime_min READ rampTime_min WRITE setRampTime_min /*NOTIFY changed*/ /*changedRampTime_min*/ )
   //! \brief The target ending temp of the step in C.
   Q_PROPERTY( double endTemp_c READ endTemp_c WRITE setEndTemp_c /*NOTIFY changed*/ /*changedEndTemp_c*/ )
   //! \brief The infusion temp in C.
   Q_PROPERTY( double infuseTemp_c READ infuseTemp_c WRITE setInfuseTemp_c /*NOTIFY changed*/ /*changedInfuseTemp_c*/ )
   //! \brief The decoction amount in liters.
   Q_PROPERTY( double decoctionAmount_l READ decoctionAmount_l WRITE setDecoctionAmount_l /*NOTIFY changed*/ /*changedDecoctionAmount_l*/ )
   //! \brief The step number in a sequence of other steps.
   Q_PROPERTY( int stepNumber READ stepNumber WRITE setStepNumber /*NOTIFY changed*/ STORED false )
   //! \brief The Mash to which this MashStep belongs
   Q_PROPERTY( int mashId READ getMashId WRITE setMashId )

   void setType( Type t);
   void setInfuseAmount_l( double var);
   void setStepTemp_c( double var);
   void setStepTime_min( double var);
   void setRampTime_min( double var);
   void setEndTemp_c( double var);
   void setInfuseTemp_c( double var);
   void setDecoctionAmount_l( double var);
   void setStepNumber(int stepNumber);
   void setMashId(int mashId);
//   void setMash(Mash * mash);

   Type type() const;
   const QString typeString() const;
   const QString typeStringTr() const;
   double infuseAmount_l() const;
   double stepTemp_c() const;
   double stepTime_min() const;
   double rampTime_min() const;
   double endTemp_c() const;
   double infuseTemp_c() const;
   double decoctionAmount_l() const;
   int getMashId() const;
//   Mash * mash() const;

   //! What number this step is in the mash.
   int stepNumber() const;

   //! some convenience methods
   bool isInfusion() const;
   bool isSparge() const;
   bool isTemperature() const;
   bool isDecoction() const;

   virtual Recipe * getOwningRecipe();

   static QStringList const types;
signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   Type m_type;
   double m_infuseAmount_l;
   double m_stepTemp_c;
   double m_stepTime_min;
   double m_rampTime_min;
   double m_endTemp_c;
   double m_infuseTemp_c;
   double m_decoctionAmount_l;
   int m_stepNumber;
   int mashId;
};

#endif
