/*
 * mashstep.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _MASHSTEP_H
#define _MASHSTEP_H

#include "ingredient.h"
#include <QStringList>
#include <QString>

// Forward declarations.
class MashStep;
bool operator<(MashStep &m1, MashStep &m2);
bool operator==(MashStep &m1, MashStep &m2);

/*!
 * \class MashStep
 * \author Philip G. Lee
 *
 * \brief Model for a mash step record in the database.
 */
class MashStep : public Ingredient
{
   Q_OBJECT

   // this seems to be a class with a lot of friends
   friend class Database;
   friend class BeerXML;
   friend class MashStepItemDelegate;
   friend class MashWizard;
   friend class MashDesigner;
   friend class MainWindow;
public:

   //! \brief The type of step.
   enum Type { Infusion, Temperature, Decoction, flySparge, batchSparge };
   Q_ENUMS( Type )

   virtual ~MashStep() {}

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
   Q_PROPERTY( int stepNumber READ stepNumber /*WRITE*/ /*NOTIFY changed*/ STORED false )

   void setType( Type t);
   void setInfuseAmount_l( double var);
   void setStepTemp_c( double var);
   void setStepTime_min( double var);
   void setRampTime_min( double var);
   void setEndTemp_c( double var);
   void setInfuseTemp_c( double var);
   void setDecoctionAmount_l( double var);
   void setCacheOnly(bool cache);

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
   bool cacheOnly() const;

   //! What number this step is in the mash.
   int stepNumber() const;

   //! some convenience methods
   bool isInfusion() const;
   bool isSparge() const;
   bool isTemperature() const;
   bool isDecoction() const;

   static QString classNameStr();

signals:

private:
   MashStep(Brewtarget::DBTable table, int key);
   MashStep(Brewtarget::DBTable table, int key, QSqlRecord rec);
   MashStep( MashStep const& other );
   MashStep(bool cache);

   QString m_typeStr;
   Type m_type;
   double m_infuseAmount_l;
   double m_stepTemp_c;
   double m_stepTime_min;
   double m_rampTime_min;
   double m_endTemp_c;
   double m_infuseTemp_c;
   double m_decoctionAmount_l;
   int m_stepNumber;
   bool m_cacheOnly;

   bool isValidType( const QString &str ) const;

   static QStringList types;
   static QStringList typesTr;

   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
};

inline bool MashStepPtrLt( MashStep* lhs, MashStep* rhs)
{
   return *lhs < *rhs;
}

inline bool MashStepPtrEq( MashStep* lhs, MashStep* rhs)
{
   return *lhs == *rhs;
}

struct MashStep_ptr_cmp
{
   bool operator()( MashStep* lhs, MashStep* rhs)
   {
      return *lhs < *rhs;
   }
};

struct MashStep_ptr_equals
{
   bool operator()( MashStep* lhs, MashStep* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif //_MASHSTEP_H
