/*
 * model/Water.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
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
#ifndef MODEL_WATER_H
#define MODEL_WATER_H
#pragma once

#include <QString>
#include <QSqlRecord>

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Water { BtStringConst const property{#property}; }
AddPropertyName(alkalinity      )
AddPropertyName(alkalinityAsHCO3)
AddPropertyName(amount          )
AddPropertyName(bicarbonate_ppm )
AddPropertyName(calcium_ppm     )
AddPropertyName(chloride_ppm    )
AddPropertyName(magnesium_ppm   )
AddPropertyName(mashRO          )
AddPropertyName(notes           )
AddPropertyName(ph              )
AddPropertyName(sodium_ppm      )
AddPropertyName(spargeRO        )
AddPropertyName(sulfate_ppm     )
AddPropertyName(type            )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Water
 *
 * \brief Model for water records in the database.
 */
class Water : public NamedEntity {
   Q_OBJECT
   Q_CLASSINFO("signal", "waters")

public:

   enum class Types {
      NONE,
      BASE,
      TARGET
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Types)

   enum class Ions {
      Ca,
      Cl,
      HCO3,
      Mg,
      Na,
      SO4,
      numIons      // .:TODO:. Get rid of this pseudo enum value
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Ions)

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Water(QString name = "");
   Water(NamedParameterBundle const & namedParameterBundle);
   Water(Water const & other);

   virtual ~Water();

   // It is useful to be able to assign one Water to another - see eg WaterEditor.cpp
   Water & operator=(Water other);

protected:
   /**
    * \brief Swap the contents of two Water objects - which provides an exception-safe way of implementing operator=
    */
   void swap(Water & other) noexcept;

public:
   // .:TODO:. On a base or target profile, bicarbonate and alkalinity cannot both be used. I'm gonna have fun figuring that out

   //! \brief The amount in liters.
   // .:TBD:. (MY 2020-01-03) In Hop we have amount_kg, so might be more consistent here to have amount_l or similar
   Q_PROPERTY(double amount READ amount WRITE setAmount)
   //! \brief The ppm of calcium.
   Q_PROPERTY(double calcium_ppm READ calcium_ppm WRITE setCalcium_ppm)
   //! \brief The ppm of bicarbonate.
   Q_PROPERTY(double bicarbonate_ppm READ bicarbonate_ppm WRITE setBicarbonate_ppm)
   //! \brief The ppm of sulfate.
   Q_PROPERTY(double sulfate_ppm READ sulfate_ppm WRITE setSulfate_ppm)
   //! \brief The ppm of chloride.
   Q_PROPERTY(double chloride_ppm READ chloride_ppm WRITE setChloride_ppm)
   //! \brief The ppm of sodium.
   Q_PROPERTY(double sodium_ppm READ sodium_ppm WRITE setSodium_ppm)
   //! \brief The ppm of magnesium.
   Q_PROPERTY(double magnesium_ppm READ magnesium_ppm WRITE setMagnesium_ppm)
   //! \brief The pH.
   Q_PROPERTY(double ph READ ph WRITE setPh)
   //! \brief The residual alkalinity.  Units are ppm  .:TBD:. Probably should change name to reflect units!
   Q_PROPERTY(double alkalinity READ alkalinity WRITE setAlkalinity)
   //! \brief The notes.
   Q_PROPERTY(QString notes READ notes WRITE setNotes)
   //! \brief What kind of water is this
   Q_PROPERTY(Water::Types type READ type WRITE setType)
   //! \brief percent of the mash water that is RO (reverse osmosis) .:TBD:. Probably should add _pct suffix
   Q_PROPERTY(double mashRO READ mashRO WRITE setMashRO)
   //! \brief percent of the sparge water that is RO (reverse osmosis) .:TBD:. Probably should add _pct suffix
   Q_PROPERTY(double spargeRO READ spargeRO WRITE setSpargeRO)
   //! \brief is the alkalinity measured as HCO3 (bicarbonate) or CO3 (carbonate)?
   Q_PROPERTY(bool alkalinityAsHCO3 READ alkalinityAsHCO3 WRITE setAlkalinityAsHCO3)

   // Getters
   double       amount()           const;
   double       calcium_ppm()      const;
   double       bicarbonate_ppm()  const;
   double       sulfate_ppm()      const;
   double       chloride_ppm()     const;
   double       sodium_ppm()       const;
   double       magnesium_ppm()    const;
   double       ph()               const;
   double       alkalinity()       const;
   QString      notes()            const;
   Water::Types type()             const;
   double       mashRO()           const;
   double       spargeRO()         const;
   bool         alkalinityAsHCO3() const;

   double       ppm(Water::Ions const ion) const;

   // Setters
   void setAmount( double var );
   void setCalcium_ppm( double var );
   void setSulfate_ppm( double var );
   void setBicarbonate_ppm( double var );
   void setChloride_ppm( double var );
   void setSodium_ppm( double var );
   void setMagnesium_ppm( double var );
   void setPh( double var );
   void setAlkalinity(double var);
   void setNotes( const QString &var );
   void setType(Types var);
   void setMashRO(double var);
   void setSpargeRO(double var);
   void setAlkalinityAsHCO3(bool var);

   virtual Recipe * getOwningRecipe();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   double       m_amount;
   double       m_calcium_ppm;
   double       m_bicarbonate_ppm;
   double       m_sulfate_ppm;
   double       m_chloride_ppm;
   double       m_sodium_ppm;
   double       m_magnesium_ppm;
   double       m_ph;
   double       m_alkalinity;
   QString      m_notes;
   Water::Types m_type;
   double       m_mash_ro;
   double       m_sparge_ro;
   bool         m_alkalinity_as_hco3;
};

#endif
