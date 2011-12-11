/*
 * yeast.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _YEAST_H
#define   _YEAST_H

#include "BeerXMLElement.h"
#include <QDomNode>
#include <QString>
#include <QStringList>

// Forward declarations.
class Yeast;
bool operator<(Yeast &y1, Yeast &y2);
bool operator==(Yeast &y1, Yeast &y2);

class Yeast : public BeerXMLElement
{
   Q_OBJECT
   
   friend class Database;
public:
   //enum Type {TYPEALE, TYPELAGER, TYPEWHEAT, TYPEWINE, TYPECHAMPAGNE};
   enum Type {Ale, Lager, Wheat, Wine, Champagne};
   //enum Form {FORMLIQUID, FORMDRY, FORMSLANT, FORMCULTURE};
   enum Form {Liquid, Dry, Slant, Culture};
   //enum Flocculation {FLOCLOW, FLOCMEDIUM, FLOCHIGH, FLOCVERY_HIGH};
   enum Flocculation {Low, Medium, High, Very_High}; // NOTE: BeerXML expects a space in "Very High", but not possible with enum. What to do?
   Q_ENUMS( Type Form Flocculation )
   
   virtual ~Yeast() {}
   
   Q_PROPERTY( QString name READ name WRITE setName NOTIFY changed /*changedName*/ );
   Q_PROPERTY( Type type READ type WRITE setType NOTIFY changed /*changedType*/ );
   Q_PROPERTY( Form form READ form WRITE setForm NOTIFY changed /*changedForm*/ );
   Q_PROPERTY( double amount READ amount WRITE setAmount NOTIFY changed /*changedAmount*/ );
   Q_PROPERTY( bool amountIsWeight READ amountIsWeight WRITE setAmountIsWeight NOTIFY changed /*changedAmountIsWeight*/ );
   Q_PROPERTY( QString laboratory READ laboratory WRITE setLaboratory NOTIFY changed /*changedLaboratory*/ );
   Q_PROPERTY( QString productID READ productID WRITE setProductID NOTIFY changed /*changedProductID*/ );
   Q_PROPERTY( double minTemperature_c READ minTemperature_c WRITE setMinTemperature_c NOTIFY changed /*changedMinTemperature_c*/ );
   Q_PROPERTY( double maxTemperature_c READ maxTemperature_c WRITE setMaxTemperature_c NOTIFY changed /*changedMaxTemperature_c*/ );
   Q_PROPERTY( Flocculation flocculation READ flocculation WRITE setFlocculation NOTIFY changed /*changedFlocculation*/ );
   Q_PROPERTY( double attenuation_pct READ attenuation_pct WRITE setAttenuation_pct NOTIFY changed /*changedAttenuation_pct*/ );
   Q_PROPERTY( QString notes READ notes WRITE setNotes NOTIFY changed /*changedNotes*/ );
   Q_PROPERTY( QString bestFor READ bestFor WRITE setBestFor NOTIFY changed /*changedBestFor*/ );
   Q_PROPERTY( int timesCultured READ timesCultured WRITE setTimesCultured NOTIFY changed /*changedTimesCultured*/ );
   Q_PROPERTY( int maxReuse READ maxReuse WRITE setMaxReuse NOTIFY changed /*changedMaxReuse*/ );
   Q_PROPERTY( bool addToSecondary READ addToSecondary WRITE setAddToSecondary NOTIFY changed /*changedAddToSecondary*/ );
   
   // Setters
   void setName( const QString& var );
   void setType( Type t );
   void setForm( Form f );
   void setAmount( double var );
   void setAmountIsWeight( bool var );
   void setLaboratory( const QString& var );
   void setProductID( const QString& var );
   void setMinTemperature_c( double var );
   void setMaxTemperature_c( double var );
   void setFlocculation( Flocculation f );
   void setAttenuation_pct( double var );
   void setNotes( const QString& var );
   void setBestFor( const QString& var );
   void setTimesCultured( int var );
   void setMaxReuse( int var );
   void setAddToSecondary( bool var );
   
   // Getters
   QString name() const;
   Type type() const;
   const QString typeString() const;
   //! Returns a translated type string.
   const QString typeStringTr() const;
   Form form() const;
   const QString formString() const;
   //! Returns a translated form string.
   const QString formStringTr() const;
   double amount() const;
   bool amountIsWeight() const;
   QString laboratory() const;
   QString productID() const;
   double minTemperature_c() const;
   double maxTemperature_c() const;
   Flocculation flocculation() const;
   const QString flocculationString() const;
   //! Returns a flocculation string.
   const QString flocculationStringTr() const;
   double attenuation_pct() const;
   QString notes() const;
   QString bestFor() const;
   int timesCultured() const;
   int maxReuse() const;
   bool addToSecondary() const;
   
signals:
   /*
   void changedName(QString);
   void changedType(Type);
   void changedForm(Form);
   void changedAmount(double);
   void changedAmountIsWeight(bool);
   void changedLaboratory(QString);
   void changedProductID(QString);
   void changedMinTemperature_c(double);
   void changedMaxTemperature_c(double);
   void changedFlocculation(Flocculation);
   void changedAttenuation_pct(double);
   void changedNotes(QString);
   void changedBestFor(QString);
   void changedTimesCultured(int);
   void changedMaxReuse(int);
   void changedAddToSecondary(bool);
   */

private:
   Yeast();
   Yeast(Yeast const& other);
   
   static QStringList types;
   static QStringList forms;
   static QStringList flocculations;

   // Methods
   bool isValidType(const QString& str) const;
   bool isValidForm(const QString& str) const;
   bool isValidFlocculation(const QString& str) const;
   //void setDefaults();
};

inline bool YeastPtrLt( Yeast* lhs, Yeast* rhs)
{
   return *lhs < *rhs;
}

inline bool YeastPtrEq( Yeast* lhs, Yeast* rhs)
{
   return *lhs == *rhs;
}

struct Yeast_ptr_cmp
{
   bool operator()( Yeast* lhs, Yeast* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Yeast_ptr_equals
{
   bool operator()( Yeast* lhs, Yeast* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif   /* _YEAST_H */

