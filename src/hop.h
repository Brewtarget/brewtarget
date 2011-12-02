/*
 * hop.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#ifndef _HOP_H
#define _HOP_H

#include <QDomNode>
#include "BeerXMLElement.h"
#include <QString>
#include <QStringList>

class Hop;
class HopException;

class Hop : public BeerXMLElement
{
   Q_OBJECT
   
   friend class Database;
public:

   enum Type { TYPEBITTERING, TYPEAROMA, TYPEBOTH, NUMTYPES };
   enum Form { FORMLEAF, FORMPELLET, FORMPLUG, NUMFORMS };
   enum Use { USEBOIL, USEDRY_HOP, USEMASH, USEFIRST_WORT, USEAROMA, NUMUSES };

   virtual ~Hop() {}

   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   
   Q_PROPERTY( QString name READ name WRITE setName NOTIFY changed /*changedName*/ )
   Q_PROPERTY( double alpha_pct READ alpha_pct WRITE setAlpha_pct NOTIFY changed /*changedAlpha_pct*/ )
   Q_PROPERTY( double amount_kg READ amount_kg WRITE setAmount_kg NOTIFY changed /*changedAmount_kg*/ )
   Q_PROPERTY( Use use READ use WRITE setUse NOTIFY changed /*changedUse*/ )
   Q_PROPERTY( double time_min READ time_min WRITE setTime_min NOTIFY changed /*changedTime_min*/ )
   Q_PROPERTY( QString notes READ notes WRITE setNotes NOTIFY changed /*changedNotes*/ )
   Q_PROPERTY( Type type READ type WRITE setType NOTIFY changed /*changedType*/ )
   Q_PROPERTY( Form form READ form WRITE setForm NOTIFY changed /*changedForm*/ )
   Q_PROPERTY( double beta_pct READ beta_pct WRITE setBeta_pct NOTIFY changed /*changedBeta_pct*/ )
   Q_PROPERTY( double hsi_pct READ hsi_pct WRITE setHsi_pct NOTIFY changed /*changedHsi_pct*/ )
   Q_PROPERTY( QString origin READ origin WRITE setOrigin NOTIFY changed /*changedOrigin*/ )
   Q_PROPERTY( QString substitutes READ substitutes WRITE setSubstitutes NOTIFY changed /*changedSubstitutes*/ )
   Q_PROPERTY( double humulene_pct READ humulene_pct WRITE setHumulene_pct NOTIFY changed /*changedHumulene_pct*/ )
   Q_PROPERTY( double caryophyllene_pct READ caryophyllene_pct WRITE setCaryophyllene_pct NOTIFY changed /*changedCaryophyllene_pct*/ )
   Q_PROPERTY( double cohumulone_pct READ cohumulone_pct WRITE setCohumulone_pct NOTIFY changed /*changedCohumulone_pct*/ )
   Q_PROPERTY( double myrcene_pct READ myrcene_pct WRITE setMyrcene_pct NOTIFY changed /*changedMyrcene_pct*/ )
   
   const QString name() const;
   double alpha_pct() const;
   double amount_kg() const;
   Use use() const;
   const QString useString() const;
   //! Returns a translated use string.
   const QString useStringTr() const;
   double time_min() const;
   const QString notes() const;
   Type type() const;
   const QString typeString() const;
   //! Returns a translated type string.
   const QString typeStringTr() const;
   Form form() const;
   const QString formString() const;
   //! Returns a translated form string.
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
   void setName( const QString& str );
   void setAlpha_pct( double num );
   void setAmount_kg( double num );
   bool setUse( Use u );
   void setTime_min( double num );
   
   void setNotes( const QString& str );
   bool setType( Type t );
   bool setForm( Form f );
   void setBeta_pct( double num );
   void setHsi_pct( double num );
   void setOrigin( const QString& str );
   void setSubstitutes( const QString& str );
   void setHumulene_pct( double num );
   void setCaryophyllene_pct( double num );
   void setCohumulone_pct( double num );
   void setMyrcene_pct( double num );

signals:
   /*
   void changedName(QString);
   void changedAlpha_pct(double);
   void changedAmount_kg(double);
   void changedUse(Use);
   void changedTime_min(double);
   void changedNotes(QString);
   void changedType(Type);
   void changedForm(Form);
   void changedBeta_pct(double);
   void changedHsi_pct(double);
   void changedOrigin(QString);
   void changedSubstitutes(QString);
   void changedHumulene_pct(double);
   void changedCaryophyllene_pct(double);
   void changedCohumulone_pct(double);
   void changedMyrcene_pct(double);
   */
   
private:
   Hop();
   Hop( Hop const& other );
   
   void setDefaults();

   static bool isValidUse(const QString& str);
   static bool isValidType(const QString& str);
   static bool isValidForm(const QString& str);

   static QStringList uses;
   static QStringList types;
   static QStringList forms;
};

inline bool HopPtrLt( Hop* lhs, Hop* rhs)
{
   return *lhs < *rhs;
}

inline bool HopPtrEq( Hop* lhs, Hop* rhs)
{
   return *lhs == *rhs;
}

struct Hop_ptr_cmp
{
   bool operator()( Hop* lhs, Hop* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Hop_ptr_equals
{
   bool operator()( Hop* lhs, Hop* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif // _HOP_H
