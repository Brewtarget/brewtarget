/*
 * model/Mash.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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
#ifndef MODEL_MASH_H
#define MODEL_MASH_H
#pragma once

#include <memory> // For PImpl

#include <QList>
#include <QMetaProperty>
#include <QSqlRecord>
#include <QString>
#include <QVariant>
#include <QVector>

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Mash { BtStringConst const property{#property}; }
AddPropertyName(equipAdjust          )
AddPropertyName(grainTemp_c          )
AddPropertyName(mashSteps            )
AddPropertyName(notes                )
AddPropertyName(ph                   )
AddPropertyName(spargeTemp_c         )
AddPropertyName(totalMashWater_l     )
AddPropertyName(totalTime            )
AddPropertyName(tunSpecificHeat_calGC)
AddPropertyName(tunTemp_c            )
AddPropertyName(tunWeight_kg         )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


// Forward declarations.
class MashStep;

/*!
 * \class Mash
 *
 * \brief Model class for a mash record in the database.
 *
 *        .:TBD:. Mashes have a freestanding existence and can, in principle, be shared between Recipes but the UI does
 *        not currently enforce them having non-empty names.
 */
class Mash : public NamedEntity {
   Q_OBJECT
   Q_CLASSINFO("signal", "mashs")

public:
   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Mash(QString name = "");
   Mash(NamedParameterBundle const & namedParameterBundle);
   Mash(Mash const & other);

   virtual ~Mash();

   //! \brief The initial grain temp in Celsius.
   Q_PROPERTY( double grainTemp_c READ grainTemp_c WRITE setGrainTemp_c /*NOTIFY changed*/ /*changedGrainTemp_c*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The initial tun temp in Celsius.
   Q_PROPERTY( double tunTemp_c READ tunTemp_c WRITE setTunTemp_c /*NOTIFY changed*/ /*changedTunTemp_c*/ )
   //! \brief The sparge temp in C.
   Q_PROPERTY( double spargeTemp_c READ spargeTemp_c WRITE setSpargeTemp_c /*NOTIFY changed*/ /*changedSpargeTemp_c*/ )
   //! \brief The pH.
   Q_PROPERTY( double ph READ ph WRITE setPh /*NOTIFY changed*/ /*changedPh*/ )
   //! \brief The mass of the tun in kg.
   Q_PROPERTY( double tunWeight_kg READ tunWeight_kg WRITE setTunWeight_kg /*NOTIFY changed*/ /*changedTunWeight_kg*/ )
   //! \brief The tun's specific heat in kcal/(g*C).
   Q_PROPERTY( double tunSpecificHeat_calGC READ tunSpecificHeat_calGC WRITE setTunSpecificHeat_calGC /*NOTIFY changed*/ /*changedTunSpecificHeat_calGC*/ )
   //! \brief Whether to adjust strike temperatures to account for the tun.
   Q_PROPERTY( bool equipAdjust READ equipAdjust WRITE setEquipAdjust /*NOTIFY changed*/ /*changedEquipAdjust*/ )
   //! \brief The total water that went into the mash in liters. Calculated.
   Q_PROPERTY( double totalMashWater_l READ totalMashWater_l /*WRITE*/ /*NOTIFY changed*/ /*changedTotalMashWater_l*/ STORED false )
   //! \brief The total mash time in minutes. Calculated.
   Q_PROPERTY( double totalTime READ totalTime /*NOTIFY changed*/ /*changedTotalTime*/ STORED false )
   //! \brief The individual mash steps.
   Q_PROPERTY( QList< std::shared_ptr<MashStep> > mashSteps  READ mashSteps /*WRITE*/ /*NOTIFY changed*/ /*changedTotalTime*/ STORED false )

   /**
    * \brief Connect MashStep changed signals to their parent Mashes.
    *
    *        Needs to be called \b after all the calls to ObjectStoreTyped<FooBar>::getInstance().loadAll()
    */
   static void connectSignals();

   virtual void setKey(int key);

   // Setters
   void setGrainTemp_c( double var );
   void setNotes( const QString &var );
   void setTunTemp_c( double var );
   void setSpargeTemp_c( double var );
   void setPh( double var );
   void setTunWeight_kg( double var );
   void setTunSpecificHeat_calGC( double var );
   void setEquipAdjust( bool var );

   // Getters
   double grainTemp_c() const;
   unsigned int numMashSteps() const;
   QString notes() const;
   double tunTemp_c() const;
   double spargeTemp_c() const;
   double ph() const;
   double tunWeight_kg() const;
   double tunSpecificHeat_calGC() const;
   bool equipAdjust() const;

   // Calculated getters
   //! \brief all the mash water, sparge and strike
   double totalMashWater_l();
   //! \brief all the infusion water, excluding sparge
   double totalInfusionAmount_l() const;
   //! \brief all the sparge water
   double totalSpargeAmount_l() const;
   double totalTime();

   bool hasSparge() const;

   // Relational getters
   QList< std::shared_ptr<MashStep> > mashSteps() const;

   /*!
    * \brief Swap MashSteps \c ms1 and \c ms2
    */
   void swapMashSteps(MashStep & ms1, MashStep & ms2);

   void removeAllMashSteps();

   virtual Recipe * getOwningRecipe();

   /**
    * \brief A Mash owns its MashSteps so needs to delete those if it itself is being deleted
    */
   virtual void hardDeleteOwnedEntities();

   std::shared_ptr<MashStep> addMashStep(std::shared_ptr<MashStep> mashStep);
   std::shared_ptr<MashStep> removeMashStep(std::shared_ptr<MashStep> mashStep);

public slots:
   void acceptMashStepChange(QMetaProperty, QVariant);

signals:
   // Emitted when the number of steps change, or when you should call mashSteps() again.
   void mashStepsChanged();

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   double m_grainTemp_c;
   QString m_notes;
   double m_tunTemp_c;
   double m_spargeTemp_c;
   double m_ph;
   double m_tunWeight_kg;
   double m_tunSpecificHeat_calGC;
   bool m_equipAdjust;

};

Q_DECLARE_METATYPE( Mash* )

#endif
