/*
 * hop.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Kregg K <gigatropolis@yahoo.com>
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

#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "hop.h"
#include "brewtarget.h"

QStringList Hop::types = QStringList() << "Bittering" << "Aroma" << "Both";
QStringList Hop::forms = QStringList() << "Leaf" << "Pellet" << "Plug";
QStringList Hop::uses = QStringList() << "Mash" << "First Wort" << "Boil" << "Aroma" << "Dry Hop";
QHash<QString,QString> Hop::tagToProp = Hop::tagToPropHash();

QHash<QString,QString> Hop::tagToPropHash()
{
   QHash<QString,QString> propHash;
   
   propHash["NAME"] = "name";
   propHash["ALPHA"] = "alpha_pct";
   propHash["AMOUNT"] = "amount_kg";
   propHash["INVENTORY"] = "inventory";
   //propHash["USE"] = "use";
   propHash["TIME"] = "time_min";
   propHash["NOTES"] = "notes";
   //propHash["TYPE"] = "type";
   //propHash["FORM"] = "form";
   propHash["BETA"] = "beta_pct";
   propHash["HSI"] = "hsi_pct";
   propHash["ORIGIN"] = "origin";
   propHash["SUBSTITUTES"] = "substitutes";
   propHash["HUMULENE"] = "humulene_pct";
   propHash["CARYOPHYLLENE"] = "caryophyllene_pct";
   propHash["COHUMULONE"] = "cohumulone_pct";
   propHash["MYRCENE"] = "myrcene_pct";
   return propHash;
}

bool operator<( Hop &h1, Hop &h2 )
{
   return h1.name() < h2.name();
}

bool operator==( Hop &h1, Hop &h2 )
{
   return h1.name() == h2.name();
}

bool Hop::isValidUse(const QString& str)
{
   return (uses.indexOf(str) >= 0);
}

bool Hop::isValidType(const QString& str)
{
   return (types.indexOf(str) >= 0);
}

bool Hop::isValidForm(const QString& str)
{
   return (forms.indexOf(str) >= 0);
}

/*
void Hop::setDefaults()
{
   name = "";
   use = USEBOIL;
   notes = "";
   type = TYPEBOTH;
   form = FORMPELLET;
   origin = "";
   substitutes = "";
   
   alpha_pct = 0.0;
   amount_kg = 0.0;
   time_min = 0.0;
   beta_pct = 0.0;
   hsi_pct = 0.0;
   humulene_pct = 0.0;
   caryophyllene_pct = 0.0;
   cohumulone_pct = 0.0;
   myrcene_pct = 0.0;
}
*/

Hop::Hop()
   : BeerXMLElement()
{
}

Hop::Hop( Hop const& other )
   : BeerXMLElement(other)
{
}

//============================="SET" METHODS====================================
void Hop::setName( const QString& str )
{
   set("name","name",str);
   emit changedName(str);
}

void Hop::setAlpha_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < alpha < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("alpha_pct", "alpha", num);
   }
}

void Hop::setAmount_kg( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: amount < 0: %1").arg(num) );
      return;
   }
   else
   {
      set("amount_kg", "amount", num);
   }
}

void Hop::setInventoryAmount( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: inventory < 0: %1").arg(num) );
      return;
   }
   else
   {
      setInventory("inventory", "amount", num);
   }
}

void Hop::setUse(Use u)
{
   if ( u >= 0 )
      set("use", "use", uses.at(u));
}

void Hop::setTime_min( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: time < 0: %1").arg(num) );
      return;
   }
   else
   {
      set("time_min", "time", num);
   }
}
      
void Hop::setNotes( const QString& str )
{
   set("notes", "notes", str);
}

void Hop::setType(Type t)
{
  if ( t >= 0 )
     set("type", "htype", types.at(t));
}

void Hop::setForm( Form f )
{
   if ( f >= 0 )
     set("form", "form", forms.at(f));
}

void Hop::setBeta_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < beta < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("beta_pct", "beta", num);
   }
}

void Hop::setHsi_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < hsi < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("hsi_pct", "hsi", num);
   }
}

void Hop::setOrigin( const QString& str )
{
   set("origin", "origin", str);
}

void Hop::setSubstitutes( const QString& str )
{
   set("substitutes", "substitutes", str);
}

void Hop::setHumulene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < humulene < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("humulene_pct", "humulene", num);
   }
}

void Hop::setCaryophyllene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < cary < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("caryophyllene_pct", "caryophyllene", num);
   }
}

void Hop::setCohumulone_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < cohumulone < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("cohumulone_pct", "cohumulone", num);
   }
}

void Hop::setMyrcene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < myrcene < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("myrcene_pct", "myrcene", num);
   }
}

//============================="GET" METHODS====================================

const QString Hop::name() const { return get("name").toString(); }
Hop::Use Hop::use() const { return static_cast<Hop::Use>(uses.indexOf(get("use").toString())); }
const QString Hop::useString() const { return get("use").toString(); }
Hop::Form Hop::form() const { return static_cast<Hop::Form>(forms.indexOf(get("form").toString())); }
const QString Hop::notes() const { return get("notes").toString(); }
Hop::Type Hop::type() const { return static_cast<Hop::Type>(types.indexOf(get("htype").toString())); }
const QString Hop::typeString() const { return get("htype").toString(); }
const QString Hop::formString() const { return get("form").toString(); }
const QString Hop::origin() const { return get("origin").toString(); }
const QString Hop::substitutes() const { return get("substitutes").toString(); }

double Hop::alpha_pct()          const { return get("alpha").toDouble(); }
double Hop::amount_kg()          const { return get("amount").toDouble(); }
double Hop::time_min()           const { return get("time").toDouble(); }
double Hop::beta_pct()           const { return get("beta").toDouble(); }
double Hop::hsi_pct()            const { return get("hsi").toDouble(); }
double Hop::humulene_pct()       const { return get("humulene").toDouble(); }
double Hop::caryophyllene_pct()  const { return get("caryophyllene").toDouble(); }
double Hop::cohumulone_pct()     const { return get("cohumulone").toDouble(); }
double Hop::myrcene_pct()        const { return get("myrcene").toDouble(); }

// inventory still must be handled separately, and I'm still annoyed.
double Hop::inventory() const
{
   return getInventory("amount").toDouble();
}
 

const QString Hop::useStringTr() const
{
   static QStringList usesTr = QStringList() << tr("Mash") << tr("First Wort") << tr("Boil") << tr("Aroma") << tr("Dry Hop") ;
   return usesTr.at(use());
}

const QString Hop::typeStringTr() const
{
   static QStringList typesTr = QStringList() << tr("Bittering") << tr("Aroma") << tr("Both");
   return typesTr.at(type());
}

const QString Hop::formStringTr() const
{
   static QStringList formsTr = QStringList() << tr("Leaf") << tr("Pellet") << tr("Plug");
   return formsTr.at(form());
}

