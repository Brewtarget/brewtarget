/*
 * model/Yeast.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
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
#ifndef MODEL_YEAST_H
#define MODEL_YEAST_H
#pragma once

#include <QSqlRecord>
#include <QString>
#include <QStringList>

#include "model/NamedEntityWithInventory.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Yeast { BtStringConst const property{#property}; }
AddPropertyName(addToSecondary    )
AddPropertyName(amount            )
AddPropertyName(amountIsWeight    )
AddPropertyName(attenuation_pct   )
AddPropertyName(bestFor           )
AddPropertyName(flocculation      )
AddPropertyName(flocculationString)
AddPropertyName(form              )
AddPropertyName(formString        )
AddPropertyName(laboratory        )
AddPropertyName(maxReuse          )
AddPropertyName(maxTemperature_c  )
AddPropertyName(minTemperature_c  )
AddPropertyName(notes             )
AddPropertyName(productID         )
AddPropertyName(timesCultured     )
AddPropertyName(typeString        )
AddPropertyName(type              )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Yeast
 *
 * \brief Model for yeast records in the database.
 */
class Yeast : public NamedEntityWithInventory {
   Q_OBJECT
   Q_CLASSINFO("signal", "yeasts")

public:
   //! \brief What beverage the yeast is for.
   enum class Type {Ale, Lager, Wheat, Wine, Champagne};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   //! \brief What form the yeast comes in.
   enum class Form {Liquid, Dry, Slant, Culture};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Form)

   //! \brief How flocculant the strain is.
   enum class Flocculation {Low, Medium, High, Very_High};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Flocculation)

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Yeast(QString name = "");
   Yeast(NamedParameterBundle const & namedParameterBundle);
   Yeast(Yeast const & other);

   virtual ~Yeast();

   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The \c Type string.
   Q_PROPERTY( QString typeString READ typeString )
   //! \brief The translated \c Type string.
   Q_PROPERTY( QString typeStringTr READ typeStringTr )
   //! \brief The \c Form.
   Q_PROPERTY( Form form READ form WRITE setForm /*NOTIFY changed*/ /*changedForm*/ )
   //! \brief The \c Form string.
   Q_PROPERTY( QString formString READ formString )
   //! \brief The translated \c Form string.
   Q_PROPERTY( QString formStringTr READ formStringTr )
   //! \brief The amount in either liters or kg depending on \c amountIsWeight().
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount*/ )
   //! \brief Whether the \c amount() is weight (kg) or volume (liters).
   Q_PROPERTY( bool amountIsWeight READ amountIsWeight WRITE setAmountIsWeight /*NOTIFY changed*/ /*changedAmountIsWeight*/ )
   //! \brief The lab from which it came.
   Q_PROPERTY( QString laboratory READ laboratory WRITE setLaboratory /*NOTIFY changed*/ /*changedLaboratory*/ )
   //! \brief The product ID.
   Q_PROPERTY( QString productID READ productID WRITE setProductID /*NOTIFY changed*/ /*changedProductID*/ )
   //! \brief The minimum fermenting temperature.
   Q_PROPERTY( double minTemperature_c READ minTemperature_c WRITE setMinTemperature_c /*NOTIFY changed*/ /*changedMinTemperature_c*/ )
   //! \brief The maximum fermenting temperature.
   Q_PROPERTY( double maxTemperature_c READ maxTemperature_c WRITE setMaxTemperature_c /*NOTIFY changed*/ /*changedMaxTemperature_c*/ )
   //! \brief The \c Flocculation.
   Q_PROPERTY( Flocculation flocculation READ flocculation WRITE setFlocculation /*NOTIFY changed*/ /*changedFlocculation*/ )
   //! \brief The \c Flocculation string.
   Q_PROPERTY( QString flocculationString READ flocculationString )
   //! \brief The translated \c Flocculation string.
   Q_PROPERTY( QString flocculationStringTr READ flocculationStringTr )
   //! \brief The apparent attenuation in percent.
   Q_PROPERTY( double attenuation_pct READ attenuation_pct WRITE setAttenuation_pct /*NOTIFY changed*/ /*changedAttenuation_pct*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief What styles the strain is best for.
   Q_PROPERTY( QString bestFor READ bestFor WRITE setBestFor /*NOTIFY changed*/ /*changedBestFor*/ )
   //! \brief The number of times recultured.
   Q_PROPERTY( int timesCultured READ timesCultured WRITE setTimesCultured /*NOTIFY changed*/ /*changedTimesCultured*/ )
   //! \brief The maximum recommended number of reculturings.
   Q_PROPERTY( int maxReuse READ maxReuse WRITE setMaxReuse /*NOTIFY changed*/ /*changedMaxReuse*/ )
   //! \brief Whether the yeast is added to secondary or primary.
   Q_PROPERTY( bool addToSecondary READ addToSecondary WRITE setAddToSecondary /*NOTIFY changed*/ /*changedAddToSecondary*/ )

   // Setters
   void setType( Type t);
   void setForm( Form f);
   void setAmount( double var);
   virtual void setInventoryAmount(double var);
   void setInventoryQuanta(int var);
   void setAmountIsWeight( bool var);
   void setLaboratory( const QString& var);
   void setProductID( const QString& var);
   void setMinTemperature_c( double var);
   void setMaxTemperature_c( double var);
   void setFlocculation( Flocculation f);
   void setAttenuation_pct( double var);
   void setNotes( const QString& var);
   void setBestFor( const QString& var);
   void setTimesCultured( int var);
   void setMaxReuse( int var);
   void setAddToSecondary( bool var);

   // Getters
   Type type() const;
   const QString typeString() const;
   const QString typeStringTr() const;
   Form form() const;
   const QString formString() const;
   const QString formStringTr() const;
   double amount() const;
   virtual double inventory() const;
   bool amountIsWeight() const;
   QString laboratory() const;
   QString productID() const;
   double minTemperature_c() const;
   double maxTemperature_c() const;
   Flocculation flocculation() const;
   const QString flocculationString() const;
   const QString flocculationStringTr() const;
   double attenuation_pct() const;
   QString notes() const;
   QString bestFor() const;
   int timesCultured() const;
   int maxReuse() const;
   bool addToSecondary() const;

   virtual Recipe * getOwningRecipe();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   Type m_type;
   Form m_form;
   Flocculation m_flocculation;
   double m_amount;
   bool m_amountIsWeight;
   QString m_laboratory;
   QString m_productID;
   double m_minTemperature_c;
   double m_maxTemperature_c;
   double m_attenuation_pct;
   QString m_notes;
   QString m_bestFor;
   int m_timesCultured;
   int m_maxReuse;
   bool m_addToSecondary;
   int m_inventory_id;
};

Q_DECLARE_METATYPE( QList<Yeast*> )

#endif
