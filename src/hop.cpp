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

/************* Columns *************/
const QString kUse("use");
const QString kForm("form");
const QString kNotes("notes");
const QString kType("htype");
const QString kOrigin("origin");
const QString kSubstitutes("substitutes");
const QString kAlpha("alpha");
const QString kAmount("amount");
const QString kTime("time");
const QString kBeta("beta");
const QString kHSI("hsi");
const QString kHumulene("humulene");
const QString kCaryophyllene("caryophyllene");
const QString kCohumulone("cohumulone");
const QString kMyrcene("myrcene");

/************** Props **************/
const QString kNameProp("name");
const QString kAlphaProp("alpha_pct");
const QString kAmountProp("amount_kg");
const QString kInventoryProp("inventory");
const QString kUseProp("use");
const QString kTimeProp("time_min");
const QString kNotesProp("notes");
const QString kTypeProp("type");
const QString kFormProp("form");
const QString kBetaProp("beta_pct");
const QString kHSIProp("hsi_pct");
const QString kOriginProp("origin");
const QString kSubstitutesProp("substitutes");
const QString kHumuleneProp("humulene_pct");
const QString kCaryophylleneProp("caryophyllene_pct");
const QString kCohumuloneProp("cohumulone_pct");
const QString kMyrceneProp("myrcene_pct");

QStringList Hop::types = QStringList() << "Bittering" << "Aroma" << "Both";
QStringList Hop::forms = QStringList() << "Leaf" << "Pellet" << "Plug";
QStringList Hop::uses = QStringList() << "Mash" << "First Wort" << "Boil" << "Aroma" << "Dry Hop";
QHash<QString,QString> Hop::tagToProp = Hop::tagToPropHash();

QHash<QString,QString> Hop::tagToPropHash()
{
   QHash<QString,QString> propHash;
   
   propHash["NAME"] = kNameProp;
   propHash["ALPHA"] = kAlphaProp;
   propHash["AMOUNT"] = kAmountProp;
   propHash["INVENTORY"] = kInventoryProp;
   //propHash["USE"] = kUseProp;
   propHash["TIME"] = kTimeProp;
   propHash["NOTES"] = kNotesProp;
   //propHash["TYPE"] = kTypeProp;
   //propHash["FORM"] = kFormProp;
   propHash["BETA"] = kBetaProp;
   propHash["HSI"] = kHSIProp;
   propHash["ORIGIN"] = kOriginProp;
   propHash["SUBSTITUTES"] = kSubstitutesProp;
   propHash["HUMULENE"] = kHumuleneProp;
   propHash["CARYOPHYLLENE"] = kCaryophylleneProp;
   propHash["COHUMULONE"] = kCohumuloneProp;
   propHash["MYRCENE"] = kMyrceneProp;
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

QString Hop::classNameStr()
{
   static const QString name("Hop");
   return name;
}

Hop::Hop(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
}

Hop::Hop( Hop const& other )
   : BeerXMLElement(other)
{
}

//============================="SET" METHODS====================================
void Hop::setAlpha_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < alpha < 100: %1").arg(num) );
      return;
   }
   else
   {
      set(kAlphaProp, kAlpha, num);
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
      set(kAmountProp, kAmount, num);
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
      setInventory(kInventoryProp, kAmount, num);
   }
}

void Hop::setUse(Use u)
{
   if ( u >= 0 )
      set(kUseProp, kUse, uses.at(u));
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
      set(kTimeProp, kTime, num);
   }
}
      
void Hop::setNotes( const QString& str )
{
   set(kNotesProp, kNotes, str);
}

void Hop::setType(Type t)
{
  if ( t >= 0 )
     set(kTypeProp, kType, types.at(t));
}

void Hop::setForm( Form f )
{
   if ( f >= 0 )
     set(kFormProp, kForm, forms.at(f));
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
      set(kBetaProp, kBeta, num);
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
      set(kHSIProp, kHSI, num);
   }
}

void Hop::setOrigin( const QString& str )
{
   set(kOriginProp, kOrigin, str);
}

void Hop::setSubstitutes( const QString& str )
{
   set(kSubstitutesProp, kSubstitutes, str);
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
      set(kHumuleneProp, kHumulene, num);
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
      set(kCaryophylleneProp, kCaryophyllene, num);
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
      set(kCohumuloneProp, kCohumulone, num);
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
      set(kMyrceneProp, kMyrcene, num);
   }
}

//============================="GET" METHODS====================================

Hop::Use Hop::use() const
{
   return static_cast<Hop::Use>(uses.indexOf(useString()));
}

const QString Hop::useString() const
{
   return get(kUse).toString();
}

const QString Hop::notes() const
{
   return get(kNotes).toString();
}

Hop::Type Hop::type() const
{
   return static_cast<Hop::Type>(types.indexOf(typeString()));
}

const QString Hop::typeString() const
{
   return get(kType).toString();
}

Hop::Form Hop::form() const
{
   return static_cast<Hop::Form>(forms.indexOf(formString()));
}

const QString Hop::formString() const
{
   return get(kForm).toString();
}

const QString Hop::origin() const
{
   return get(kOrigin).toString();
}

const QString Hop::substitutes() const
{
   return get(kSubstitutes).toString();
}

double Hop::alpha_pct() const
{
   return get(kAlpha).toDouble();
}

double Hop::amount_kg() const
{
   return get(kAmount).toDouble();
}

double Hop::time_min() const
{
   return get(kTime).toDouble();
}

double Hop::beta_pct() const
{
   return get(kBeta).toDouble();
}

double Hop::hsi_pct() const
{
   return get(kHSI).toDouble();
}

double Hop::humulene_pct() const
{
   return get(kHumulene).toDouble();
}

double Hop::caryophyllene_pct() const
{
   return get(kCaryophyllene).toDouble();
}

double Hop::cohumulone_pct() const
{
   return get(kCohumulone).toDouble();
}

double Hop::myrcene_pct() const
{
   return get(kMyrcene).toDouble();
}

// inventory still must be handled separately, and I'm still annoyed.
double Hop::inventory() const
{
   return getInventory(kAmount).toDouble();
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

