/*
 * hop.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _HOP_H
#define _HOP_H

#include <exception>
#include "observable.h"
#include <QDomNode>
#include "BeerXMLElement.h"
#include <QString>
#include <QStringList>

class Hop;
class HopException;

class Hop : public Observable, public BeerXMLElement
{
   public:

   enum Type { TYPEBITTERING, TYPEAROMA, TYPEBOTH, NUMTYPES };
   enum Form { FORMLEAF, FORMPELLET, FORMPLUG, NUMFORMS };
   enum Use { USEBOIL, USEDRY_HOP, USEMASH, USEFIRST_WORT, USEAROMA, NUMUSES };

      Hop();
      Hop( Hop& other );
      Hop(const QDomNode& hopNode);

      friend bool operator<( Hop &h1, Hop &h2 );
      friend bool operator==( Hop &h1, Hop &h2 );

      virtual void fromNode(const QDomNode& node); // From BeerXMLElement
      virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
      //QString toXml();
      
      const QString& getName() const;
      int getVersion() const;
      double getAlpha_pct() const;
      double getAmount_kg() const;
      Use getUse() const;
      const QString& getUseString() const;
      double getTime_min() const;
      
      const QString& getNotes() const;
      Type getType() const;
      const QString& getTypeString() const;
      Form getForm() const;
      const QString& getFormString() const;
      double getBeta_pct() const;
      double getHsi_pct() const;
      const QString& getOrigin() const;
      const QString& getSubstitutes() const;
      double getHumulene_pct() const;
      double getCaryophyllene_pct() const;
      double getCohumulone_pct() const;
      double getMyrcene_pct() const;
      
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
      
   private:
      // Mandatory members.
      QString name;
      const static int version = 1;
      double alpha_pct;
      double amount_kg;
      Use use;
      double time_min;
      
      // Optional members.
      QString notes;
      Type type;
      Form form;
      double beta_pct;
      double hsi_pct;
      QString origin;
      QString substitutes;
      double humulene_pct;
      double caryophyllene_pct;
      double cohumulone_pct;
      double myrcene_pct;
      
      // Sets every member to zero or blank or whatever.
      void setDefaults();
      // Misc functions.
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
