/*
 * model/Hop.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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

#include "model/NamedEntityWithInventory.h"
#include "TableSchema.h"

namespace PropertyNames::Hop { static char const * const alpha_pct = "alpha_pct"; /* previously kpropAlpha */ }
namespace PropertyNames::Hop { static char const * const amount_kg = "amount_kg"; /* previously kpropAmountKg */ }
namespace PropertyNames::Hop { static char const * const beta_pct = "beta_pct"; /* previously kpropBeta */ }
namespace PropertyNames::Hop { static char const * const caryophyllene_pct = "caryophyllene_pct"; /* previously kpropCaryophyllene */ }
namespace PropertyNames::Hop { static char const * const cohumulone_pct = "cohumulone_pct"; /* previously kpropCohumulone */ }
namespace PropertyNames::Hop { static char const * const form = "form"; /* previously kpropForm */ }
namespace PropertyNames::Hop { static char const * const formString = "formString"; /* previously kpropFormString */ }
namespace PropertyNames::Hop { static char const * const hsi_pct = "hsi_pct"; /* previously kpropHSI */ }
namespace PropertyNames::Hop { static char const * const humulene_pct = "humulene_pct"; /* previously kpropHumulene */ }
namespace PropertyNames::Hop { static char const * const myrcene_pct = "myrcene_pct"; /* previously kpropMyrcene */ }
namespace PropertyNames::Hop { static char const * const notes = "notes"; /* previously kpropNotes */ }
namespace PropertyNames::Hop { static char const * const origin = "origin"; /* previously kpropOrigin */ }
namespace PropertyNames::Hop { static char const * const substitutes = "substitutes"; /* previously kpropSubstitutes */ }
namespace PropertyNames::Hop { static char const * const time_min = "time_min"; /* previously kpropTime */ }
namespace PropertyNames::Hop { static char const * const typeString = "typeString"; /* previously kpropTypeString */ }
namespace PropertyNames::Hop { static char const * const type = "type"; /* previously kpropType */ }
namespace PropertyNames::Hop { static char const * const useString = "useString"; /* previously kpropUseString */ }
namespace PropertyNames::Hop { static char const * const use = "use"; /* previously kpropUse */ }

/*!
 * \class Hop
 *
 * \brief Model class for a hop record in the database.
 */
class Hop : public NamedEntityWithInventory {
   Q_OBJECT
   Q_CLASSINFO("signal", "hops")

   friend class Database;
   friend class BeerXML;
   friend class HopDialog;
public:

   //! \brief The type of hop, meaning for what properties it is used.
   enum Type {Bittering, Aroma, Both};
   //! \brief The form of the hop.
   enum Form {Leaf, Pellet, Plug};

   //! \brief The way the hop is used.
   // .:TBD:. (MY 2021-01-01) Shall we perhaps change "UseAroma" to "PostBoil", since this is what BeerXML means by
   // Aroma in this context?
   //         (MF 2021-04-09) Nope. These fields MUST remain as they are until we are certain that we have converted
   //                         all existing bt v1.0 XML databases. You know.  Forever.
   enum Use {Mash, First_Wort, Boil, UseAroma, Dry_Hop }; // NOTE: way bad. We have a duplicate enum (Aroma)
   Q_ENUMS( Type Form Use )

   Hop(QString name, bool cache = true);
   virtual ~Hop() {}

   //! \brief The percent alpha.
   Q_PROPERTY( double alpha_pct READ alpha_pct WRITE setAlpha_pct /*NOTIFY changed*/ /*changedAlpha_pct*/ )
   //! \brief The amount in kg.
   Q_PROPERTY( double amount_kg READ amount_kg WRITE setAmount_kg /*NOTIFY changed*/ /*changedAmount_kg*/ )
   //! \brief The \c Use.
   Q_PROPERTY( Use use READ use WRITE setUse /*NOTIFY changed*/ /*changedUse*/ )
   //! \brief The untranslated \c Use string.
   Q_PROPERTY( QString useString READ useString /* WRITE setUse NOTIFY changed*/ /*changedUse*/ )
   //! \brief The time in minutes that the hop is used.
   Q_PROPERTY( double time_min READ time_min WRITE setTime_min /*NOTIFY changed*/ /*changedTime_min*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The untranslated string type
   Q_PROPERTY( QString typeString READ typeString /* WRITE setType NOTIFY changed*/ /*changedType*/ )
   //! \brief The \c Form.
   Q_PROPERTY( Form form READ form WRITE setForm /*NOTIFY changed*/ /*changedForm*/ )
   //! \brief The untranslated \c Form string.
   Q_PROPERTY( QString formString READ formString /* WRITE setForm NOTIFY changed*/ /*changedForm*/ )
   //! \brief The percent of beta acids.
   Q_PROPERTY( double beta_pct READ beta_pct WRITE setBeta_pct /*NOTIFY changed*/ /*changedBeta_pct*/ )
   //! \brief The hop stability index in percent.
   Q_PROPERTY( double hsi_pct READ hsi_pct WRITE setHsi_pct /*NOTIFY changed*/ /*changedHsi_pct*/ )
   //! \brief The origin.
   Q_PROPERTY( QString origin READ origin WRITE setOrigin /*NOTIFY changed*/ /*changedOrigin*/ )
   //! \brief The list of substitutes.
   Q_PROPERTY( QString substitutes READ substitutes WRITE setSubstitutes /*NOTIFY changed*/ /*changedSubstitutes*/ )
   //! \brief Humulene as a percentage of total hop oil.
   Q_PROPERTY( double humulene_pct READ humulene_pct WRITE setHumulene_pct /*NOTIFY changed*/ /*changedHumulene_pct*/ )
   //! \brief Caryophyllene as a percentage of total hop oil.
   Q_PROPERTY( double caryophyllene_pct READ caryophyllene_pct WRITE setCaryophyllene_pct /*NOTIFY changed*/ /*changedCaryophyllene_pct*/ )
   //! \brief Cohumulone as a percentage of total hop oil.
   Q_PROPERTY( double cohumulone_pct READ cohumulone_pct WRITE setCohumulone_pct /*NOTIFY changed*/ /*changedCohumulone_pct*/ )
   //! \brief Myrcene as a percentage of total hop oil.
   Q_PROPERTY( double myrcene_pct READ myrcene_pct WRITE setMyrcene_pct /*NOTIFY changed*/ /*changedMyrcene_pct*/ )

   double alpha_pct() const;
   double amount_kg() const;
   // Use in enumerated, untranslated and translated versions
   Use use() const;
   const QString useString() const;
   const QString useStringTr() const;

   double time_min() const;
   const QString notes() const;

   // Type in enumerated, untranslated and translated versions
   Type type() const;
   const QString typeString() const;
   const QString typeStringTr() const;

   // Form in enumerated, untranslated and translated versions
   Form form() const;
   const QString formString() const;
   const QString formStringTr() const;

   double beta_pct() const;
   double hsi_pct() const;
   const QString origin() const;
   const QString substitutes() const;
   double humulene_pct() const;
   double caryophyllene_pct() const;
   double cohumulone_pct() const;
   double myrcene_pct() const;

   //set
   void setAlpha_pct( double num);
   void setAmount_kg( double num);
   void setUse( Use u);
   void setTime_min( double num);

   void setNotes( const QString& str);
   void setType( Type t);
   void setForm( Form f);
   void setBeta_pct( double num);
   void setHsi_pct( double num);
   void setOrigin( const QString& str);
   void setSubstitutes( const QString& str);
   void setHumulene_pct( double num);
   void setCaryophyllene_pct( double num);
   void setCohumulone_pct( double num);
   void setMyrcene_pct( double num);

   static QString classNameStr();

   NamedEntity * getParent();
   virtual int insertInDatabase();
   virtual void removeFromDatabase();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
   // Hop(Brewtarget::DBTable table, int key);
   Hop(TableSchema* table, QSqlRecord rec, int t_key = -1);
   Hop( Hop & other );

   QString m_useStr;
   Use m_use;
   QString m_typeStr;
   Type m_type;
   QString m_formStr;
   Form m_form;
   double m_alpha_pct;
   double m_amount_kg;
   double m_time_min;
   QString m_notes;
   double m_beta_pct;
   double m_hsi_pct;
   QString m_origin;
   QString m_substitutes;
   double m_humulene_pct;
   double m_caryophyllene_pct;
   double m_cohumulone_pct;
   double m_myrcene_pct;

   void setDefaults();

   static bool isValidUse(const QString& str);
   static bool isValidType(const QString& str);
   static bool isValidForm(const QString& str);

   static QStringList uses;
   static QStringList types;
   static QStringList forms;
};

Q_DECLARE_METATYPE( QList<Hop*> )

inline bool hopLessThanByTime(const Hop* lhs, const Hop* rhs)
{
   if ( lhs->use() == rhs->use() )
   {
      if ( lhs->time_min() == rhs->time_min() )
         return lhs->name() < rhs->name();

      return lhs->time_min() > rhs->time_min();
   }
   return lhs->use() < rhs->use();
}

#endif
