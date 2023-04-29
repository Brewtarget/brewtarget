/*
 * model/Hop.h is part of Brewtarget, and is Copyright the following
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
#ifndef MODEL_HOP_H
#define MODEL_HOP_H
#pragma once

#include <QString>
#include <QStringList>
#include <QSqlRecord>

#include "model/NamedEntityWithInventory.h"
#include "utils/EnumStringMapping.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Hop { BtStringConst const property{#property}; }
AddPropertyName(alpha_pct            )
AddPropertyName(amount_kg            )
AddPropertyName(beta_pct             )
AddPropertyName(caryophyllene_pct    )
AddPropertyName(cohumulone_pct       )
AddPropertyName(form                 )
AddPropertyName(hsi_pct              )
AddPropertyName(humulene_pct         )
AddPropertyName(myrcene_pct          )
AddPropertyName(notes                )
AddPropertyName(origin               )
AddPropertyName(substitutes          )
AddPropertyName(time_min             )
AddPropertyName(type                 )
AddPropertyName(use                  )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Hop
 *
 * \brief Model class for a hop record in the database.
 */
class Hop : public NamedEntityWithInventory {
   Q_OBJECT
   Q_CLASSINFO("signal", "hops")

public:
   //! \brief The type of hop, meaning for what properties it is used.
   enum class Type {Bittering, Aroma, Both};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Hop::Type and string values suitable for serialisation in DB, BeerJSON, etc (but \b not
    *        BeerXML)
    */
   static EnumStringMapping const typeStringMapping;

   //! \brief The form of the hop.
   enum class Form {Leaf, Pellet, Plug};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Form)

   /*!
    * \brief Mapping between \c Hop::Form and string values suitable for serialisation in DB, BeerJSON, etc (but \b not
    *        BeerXML)
    */
   static EnumStringMapping const formStringMapping;

   /*!
    * \brief The way the hop is used.
    *        NOTE that this is not stored in BeerJSON
    */
   enum class Use {Mash,
                   First_Wort,
                   Boil,
                   Aroma,
                   Dry_Hop};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Use)

   /*!
    * \brief Mapping between \c Hop::Form and string values suitable for serialisation in DB, BeerXML, etc (but \b not
    *        used in BeerJSON)
    */
   static EnumStringMapping const useStringMapping;

   /*!
    * \brief Localised names of \c Hop::Type values suitable for displaying to the end user
    */
   static QMap<Hop::Type, QString> const typeDisplayNames;

   /*!
    * \brief Localised names of \c Hop::Form values suitable for displaying to the end user
    */
   static QMap<Hop::Form, QString> const formDisplayNames;

   /*!
    * \brief Localised names of \c Hop::Use values suitable for displaying to the end user
    */
   static QMap<Hop::Use, QString> const useDisplayNames;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Hop(QString name = "");
   Hop(NamedParameterBundle const & namedParameterBundle);
   Hop(Hop const & other);

   virtual ~Hop();

   //! \brief The percent alpha acid
   Q_PROPERTY(double alpha_pct READ alpha_pct WRITE setAlpha_pct /*NOTIFY changed*/ /*changedAlpha_pct*/ )
   //! \brief The amount in kg.
   Q_PROPERTY(double amount_kg READ amount_kg WRITE setAmount_kg /*NOTIFY changed*/ /*changedAmount_kg*/ )
   //! \brief The \c Use.
   Q_PROPERTY(Use use READ use WRITE setUse /*NOTIFY changed*/ /*changedUse*/ )
   //! \brief The time in minutes that the hop is used.
   Q_PROPERTY(double time_min READ time_min WRITE setTime_min /*NOTIFY changed*/ /*changedTime_min*/ )
   //! \brief The notes.
   Q_PROPERTY(QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The \c Type.
   Q_PROPERTY(Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The \c Form.
   Q_PROPERTY(Form form READ form WRITE setForm /*NOTIFY changed*/ /*changedForm*/ )
   //! \brief The percent of beta acids.
   Q_PROPERTY(double beta_pct READ beta_pct WRITE setBeta_pct /*NOTIFY changed*/ /*changedBeta_pct*/ )
   //! \brief The hop stability index in percent.  The Hop Stability Index (HSI) is defined as the percentage of hop
   //         alpha lost in 6 months of storage.  It may be related to the Hop Storage Index...
   Q_PROPERTY(double hsi_pct READ hsi_pct WRITE setHsi_pct /*NOTIFY changed*/ /*changedHsi_pct*/ )
   //! \brief The origin.
   Q_PROPERTY(QString origin READ origin WRITE setOrigin /*NOTIFY changed*/ /*changedOrigin*/ )
   //! \brief The list of substitutes.
   Q_PROPERTY(QString substitutes READ substitutes WRITE setSubstitutes /*NOTIFY changed*/ /*changedSubstitutes*/ )
   //! \brief Humulene as a percentage of total hop oil.
   Q_PROPERTY(double humulene_pct READ humulene_pct WRITE setHumulene_pct /*NOTIFY changed*/ /*changedHumulene_pct*/ )
   //! \brief Caryophyllene as a percentage of total hop oil.
   Q_PROPERTY(double caryophyllene_pct READ caryophyllene_pct WRITE setCaryophyllene_pct /*NOTIFY changed*/ /*changedCaryophyllene_pct*/ )
   //! \brief Cohumulone as a percentage of total hop oil.
   Q_PROPERTY(double cohumulone_pct READ cohumulone_pct WRITE setCohumulone_pct /*NOTIFY changed*/ /*changedCohumulone_pct*/ )
   //! \brief Myrcene as a percentage of total hop oil.
   Q_PROPERTY(double myrcene_pct READ myrcene_pct WRITE setMyrcene_pct /*NOTIFY changed*/ /*changedMyrcene_pct*/ )

   //============================="GET" METHODS====================================
   double  alpha_pct            () const;
   double  amount_kg            () const;
   Use     use                  () const;
   double  time_min             () const;
   QString notes                () const;
   Type    type                 () const;
   Form    form                 () const;
   double  beta_pct             () const;
   double  hsi_pct              () const;
   QString origin               () const;
   QString substitutes          () const;
   double  humulene_pct         () const;
   double  caryophyllene_pct    () const;
   double  cohumulone_pct       () const;
   double  myrcene_pct          () const;

   virtual double inventory() const;

   //============================="SET" METHODS====================================
   void setAlpha_pct            (double  const   val);
   void setAmount_kg            (double  const   val);
   void setUse                  (Use     const   val);
   void setTime_min             (double  const   val);
   void setNotes                (QString const & val);
   void setType                 (Type    const   val);
   void setForm                 (Form    const   val);
   void setBeta_pct             (double  const   val);
   void setHsi_pct              (double  const   val);
   void setOrigin               (QString const & val);
   void setSubstitutes          (QString const & val);
   void setHumulene_pct         (double  const   val);
   void setCaryophyllene_pct    (double  const   val);
   void setCohumulone_pct       (double  const   val);
   void setMyrcene_pct          (double  const   val);

   virtual void setInventoryAmount(double const val);

   virtual Recipe * getOwningRecipe();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   Use     m_use;
   Type    m_type;
   Form    m_form;
   double  m_alpha_pct;
   double  m_amount_kg;
   double  m_time_min;
   QString m_notes;
   double  m_beta_pct;
   double  m_hsi_pct;
   QString m_origin;
   QString m_substitutes;
   double  m_humulene_pct;
   double  m_caryophyllene_pct;
   double  m_cohumulone_pct;
   double  m_myrcene_pct;

   void setDefaults();
};

Q_DECLARE_METATYPE( QList<Hop*> )

bool hopLessThanByTime(const Hop* lhs, const Hop* rhs);
#endif
