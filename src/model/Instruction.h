/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Instruction.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_INSTRUCTION_H
#define MODEL_INSTRUCTION_H
#pragma once

#include <QDomNode>
#include <QString>
#include <QVector>

#include "model/NamedEntity.h"
#include "model/EnumeratedBase.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Instruction { inline BtStringConst const property{#property}; }
AddPropertyName(completed    )
AddPropertyName(directions   )
AddPropertyName(hasTimer     )
AddPropertyName(interval_mins)
AddPropertyName(timerValue   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

// Forward declarations;
class Recipe;

/*!
 * \class Instruction
 *
 * \brief Model class for an instruction record in the database.
 *
 *        NOTE that \c Instruction is not part of the official BeerXML or BeerJSON standards.  We add it in to our
 *             BeerXML files, because we can, but TBD whether this is possible with BeerJSON.
 *
 *        NB: We do not inherit from \c OwnedByRecipe, because doing so would duplicate part of what we get from
 *            \c EnumeratedBase.
 */
class Instruction : public NamedEntity,
                    public EnumeratedBase<Instruction, Recipe> {
   Q_OBJECT

   ENUMERATED_COMMON_DECL(Instruction, Recipe)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId      READ ownerId      WRITE setOwnerId   )
   Q_PROPERTY(int stepNumber   READ stepNumber   WRITE setStepNumber)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_completed    ();
   static QString localisedName_directions   ();
   static QString localisedName_hasTimer     ();
   static QString localisedName_interval_mins();
   static QString localisedName_timerValue   ();

   using OwnerClass = Recipe;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Instruction(QString name = "");
   Instruction(NamedParameterBundle const & namedParameterBundle);
   Instruction(Instruction const & other);

   virtual ~Instruction();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(QString        directions        READ directions         WRITE setDirections   )
   Q_PROPERTY(bool           hasTimer          READ hasTimer           WRITE setHasTimer     )
   Q_PROPERTY(QString        timerValue        READ timerValue         WRITE setTimerValue   )
   Q_PROPERTY(bool           completed         READ completed          WRITE setCompleted    )
   Q_PROPERTY(double         interval_mins     READ interval_mins      WRITE setInterval_mins)
   Q_PROPERTY(QList<QString> reagents          READ reagents           STORED false          )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QString directions   () const;
   bool    hasTimer     () const;
   QString timerValue   () const;
   bool    completed    () const;
   double  interval_mins() const;
   //! This is a non-stored temporary in-memory set.
   QList<QString> reagents() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setDirections   (QString const & val);
   void setHasTimer     (bool    const   val);
   void setTimerValue   (QString const & val);
   void setCompleted    (bool    const   val);
   void setInterval_mins(double  const   val);
   void addReagent      (QString const & val);

signals:

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

private:
   QString m_directions;
   bool    m_hasTimer;
   QString m_timerValue;
   bool    m_completed;
   double  m_interval_mins;

   QList<QString> m_reagents;
};

/**
 * \brief Because \c Instruction inherits from multiple bases, more than one of which has a match for \c operator<<, we
 *        need to provide an overload of \c operator<< that combines the output of those for all the base classes.
 */
template<class S>
S & operator<<(S & stream, Instruction const & instruction) {
   stream <<
      static_cast<NamedEntity const &>(instruction) << " " <<
      static_cast<EnumeratedBase<Instruction, Recipe> const &>(instruction);
   return stream;
}

BT_DECLARE_METATYPES(Instruction)

//! \brief Compares Instruction pointers by Instruction::instructionNumber().
bool operator<(Instruction & lhs, Instruction & rhs);

#endif
