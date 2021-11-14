/*
 * model/Salt.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#ifndef MODEL_SALT_H
#define MODEL_SALT_H
#pragma once

#include <QString>
#include <QSqlRecord>
#include <QSqlRecord>

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Salt { BtStringConst const property{#property}; }
AddPropertyName(amount)
AddPropertyName(amountIsWeight)
AddPropertyName(type)
AddPropertyName(isAcid)
AddPropertyName(percentAcid)
AddPropertyName(addTo)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Salt
 *
 * \brief Model for salt records in the database.
 */
class Salt : public NamedEntity {
   Q_OBJECT
   Q_CLASSINFO("signal", "salts")


   friend class WaterDialog;
   friend class SaltTableModel;
public:

   enum WhenToAdd {
      NEVER,
      MASH,
      SPARGE,
      RATIO,
      EQUAL
   };

   enum Types {
      NONE,
      CACL2,
      CACO3,
      CASO4,
      MGSO4,
      NACL,
      NAHCO3,
      LACTIC,
      H3PO4,
      ACIDMLT,
      numTypes
   };

   Q_ENUMS(WhenToAdd Types)

   Salt(QString name = "", bool cache = true);
   Salt(NamedParameterBundle const & namedParameterBundle);
   Salt(Salt const & other);

   virtual ~Salt() = default;

   // On a base or target profile, bicarbonate and alkalinity cannot both be used. I'm gonna have fun figuring that out
   //! \brief The amount of salt to be added (always a weight)
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount_l*/ )
   //! \brief When to add the salt (mash or sparge)
   Q_PROPERTY( Salt::WhenToAdd addTo READ addTo WRITE setAddTo /*NOTIFY changed*/ /*changedCalcium_ppm*/ )
   //! \brief What kind of salt this is
   Q_PROPERTY( Salt::Types type READ type WRITE setType /*NOTIFY changed*/ /*changedBicarbonate_ppm*/ )
   //! \brief Is this a weight (like CaCO3) or a volume (like H3PO3)
   Q_PROPERTY( bool amountIsWeight READ amountIsWeight WRITE setAmountIsWeight /*NOTIFY changed*/ /*changedAmountIsWeight*/ )
   //! \brief What percent is acid (used for lactic acid, H3PO4 and acid malts)
   Q_PROPERTY( double percentAcid READ percentAcid WRITE setPercentAcid /*NOTIFY changed*/ /*changedPercentAcid*/ )
   //! \brief Is this an acid or salt?
   Q_PROPERTY( bool isAcid READ isAcid WRITE setIsAcid /*NOTIFY changed*/ /*changedIsAcid*/ )

   double amount() const;
   Salt::WhenToAdd addTo() const;
   Salt::Types type() const;
   bool amountIsWeight() const;
   double percentAcid() const;
   bool isAcid() const;
   int miscId() const;

   void setAmount( double var );
   void setAddTo( Salt::WhenToAdd var );
   void setType( Salt::Types var );
   void setAmountIsWeight( bool var );
   void setPercentAcid(double var);
   void setIsAcid( bool var );

   double Ca() const;
   double Cl() const;
   double CO3() const;
   double HCO3() const;
   double Mg() const;
   double Na() const;
   double SO4() const;

   virtual Recipe * getOwningRecipe();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   double m_amount;
   Salt::WhenToAdd m_add_to;
   Salt::Types m_type;
   bool m_amount_is_weight;
   double m_percent_acid;
   bool m_is_acid;
};

Q_DECLARE_METATYPE( QList<Salt*> )

#endif
