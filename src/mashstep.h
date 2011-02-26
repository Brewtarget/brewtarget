/*
 * mashstep.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MASHSTEP_H
#define _MASHSTEP_H
#include <string>
#include <exception>
#include "observable.h"
#include <QDomNode>
#include "BeerXMLElement.h"
#include <QStringList>
#include <QString>

class MashStep;

class MashStep : public Observable, public BeerXMLElement
{
public:

   enum Type {TYPEINFUSION, TYPETEMPERATURE, TYPEDECOCTION};

   MashStep();
   MashStep( const QDomNode& mashStepNode );

   friend bool operator<(MashStep &m1, MashStep &m2);
   friend bool operator==(MashStep &m1, MashStep &m2);

   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   //QString toXml();
   
   void setName( const QString &var );
   void setType( Type t );
   void setInfuseAmount_l( double var );
   void setStepTemp_c( double var );
   void setStepTime_min( double var );
   void setRampTime_min( double var );
   void setEndTemp_c( double var );

   QString getName() const;
   Type getType() const;
   const QString& getTypeString() const;
   //! Returns a translated type string.
   const QString& getTypeStringTr() const;
   double getInfuseAmount_l() const;
   double getStepTemp_c() const;
   double getStepTime_min() const;
   double getRampTime_min() const;
   double getEndTemp_c() const;

   // My extensions
   void setInfuseTemp_c( double var );
   double getInfuseTemp_c() const;
   void setDecoctionAmount_l( double var );
   double getDecoctionAmount_l() const;
   // ===

private:

   QString name;
   static const int version = 1;
   Type type;
   double infuseAmount_l;
   double stepTemp_c;
   double stepTime_min;
   double rampTime_min;
   double endTemp_c;

   // My extensions
   double infuseTemp_c;
   double decoctionAmount_l;
   // ===

   bool isValidType( const QString &str ) const;
   void setDefaults();

   static QStringList types;
   static QStringList typesTr;
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
