/*
 * fermentable.cpp is part of Brewtarget, and is Copyright the following
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
#include <QVariant>
#include <QObject>
#include <QDebug>
#include "fermentable.h"
#include "brewtarget.h"

QStringList Fermentable::types = QStringList() << "Grain" << "Sugar" << "Extract" << "Dry Extract" << "Adjunct";
QHash<QString,QString> Fermentable::tagToProp = Fermentable::tagToPropHash();

QHash<QString,QString> Fermentable::tagToPropHash()
{
   QHash<QString,QString> propHash;
   
   propHash["NAME"] = "name";
   // NOTE: since type is actually stored as a string (not integer), have to handle separately.
   //propHash["TYPE"] = "type";
   propHash["AMOUNT"] = "amount_kg";
   propHash["INVENTORY"] = "inventory";
   propHash["YIELD"] = "yield_pct";
   propHash["COLOR"] = "color_srm";
   propHash["ADD_AFTER_BOIL"] = "addAfterBoil";
   propHash["ORIGIN"] = "origin";
   propHash["SUPPLIER"] = "supplier";
   propHash["NOTES"] = "notes";
   propHash["COARSE_FINE_DIFF"] = "coarseFineDiff_pct";
   propHash["MOISTURE"] = "moisture_pct";
   propHash["DIASTATIC_POWER"] = "diastaticPower_lintner";
   propHash["PROTEIN"] = "protein_pct";
   propHash["MAX_IN_BATCH"] = "maxInBatch_pct";
   propHash["RECOMMEND_MASH"] = "recommendMash";
   propHash["IS_MASHED"] = "isMashed";
   propHash["IBU_GAL_PER_LB"] = "ibuGalPerLb";
   return propHash;
}

bool operator<(Fermentable &f1, Fermentable &f2)
{
   return f1.name() < f2.name();
}

bool operator==(Fermentable &f1, Fermentable &f2)
{
   return f1.name() == f2.name();
}

Fermentable::Fermentable()
   : BeerXMLElement()
{
}

Fermentable::Fermentable( Fermentable const& other )
        : BeerXMLElement( other )
{
}

/*
void Fermentable::fromNode(const QDomNode& fermentableNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = fermentableNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;
      
      property = node.nodeName();
      textNode = child.toText();
      value = textNode.nodeValue();
      
      if( property == "NAME" )
      {
         name = value;
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("FERMENTABLE says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid type for FERMENTABLE. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            type = static_cast<Fermentable::Type>( ndx );
      }
      else if( property == "AMOUNT" )
      {
         setAmount_kg(getDouble(textNode));
      }
      else if( property == "YIELD" )
      {
         setYield_pct(getDouble(textNode));
      }
      else if( property == "COLOR" )
      {
         setColor_srm(getDouble(textNode));
      }
      else if( property == "ADD_AFTER_BOIL" )
      {
         setAddAfterBoil(getBool(textNode));
      }
      else if( property == "ORIGIN" )
      {
         setOrigin(value);
      }
      else if( property == "SUPPLIER" )
      {
         setSupplier(value);
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "COARSE_FINE_DIFF" )
      {
         setCoarseFineDiff_pct(getDouble(textNode));
      }
      else if( property == "MOISTURE" )
      {
         setMoisture_pct(getDouble(textNode));
      }
      else if( property == "DIASTATIC_POWER" )
      {
         setDiastaticPower_lintner(getDouble(textNode));
      }
      else if( property == "PROTEIN" )
      {
         setProtein_pct(getDouble(textNode));
      }
      else if( property == "MAX_IN_BATCH" )
      {
         setMaxInBatch_pct(getDouble(textNode));
      }
      else if( property == "RECOMMEND_MASH" )
      {
         setRecommendMash(getBool(textNode));
      }
      else if( property == "IS_MASHED" )
      {
         setIsMashed(getBool(textNode));
      }
      else if( property == "IBU_GAL_PER_LB" )
      {
         setIbuGalPerLb(getDouble(textNode));
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported FERMENTABLE property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}
*/

/*
void Fermentable::setDefaults()
{
   name = "";
   type = TYPEGRAIN;
   amount_kg = 0.0;
   yield_pct = 0.0;
   color_srm = 0.0;

   addAfterBoil = false;
   origin = "";
   supplier = "";
   notes = "";
   coarseFineDiff_pct = 0.0;
   moisture_pct = 0.0;
   diastaticPower_lintner = 0.0;
   protein_pct = 0.0;
   maxInBatch_pct = 0.0;
   recommendMash = false;
   isMashed = false;
   ibuGalPerLb = 0.0;
}
*/

// Get
const QString Fermentable::name() const { return get("name").toString(); }
const Fermentable::Type Fermentable::type() const { return static_cast<Fermentable::Type>(types.indexOf(get("ftype").toString())); }
const Fermentable::AdditionMethod Fermentable::additionMethod() const
{
   Fermentable::AdditionMethod additionMethod;
   if(isMashed())
      additionMethod = Fermentable::Mashed;
   else
   {
      if(type() == Fermentable::Grain)
         additionMethod = Fermentable::Steeped;
      else
         additionMethod = Fermentable::Not_Mashed;
   }
   return additionMethod;
}
const Fermentable::AdditionTime Fermentable::additionTime() const
{
   Fermentable::AdditionTime additionTime;
   if(addAfterBoil())
      additionTime = Fermentable::Late;
   else
      additionTime = Fermentable::Normal;

   return additionTime;
}
const QString Fermentable::typeString() const
{
   return types.at(type());
}
const QString Fermentable::typeStringTr() const
{
   static QStringList typesTr = QStringList () << QObject::tr("Grain") << QObject::tr("Sugar") << QObject::tr("Extract") << QObject::tr("Dry Extract") << QObject::tr("Adjunct");
   return typesTr.at(type());
}

const QString Fermentable::additionMethodStringTr() const
{
    QString retString;

    if(isMashed())
       retString = tr("Mashed");
    else
    {
       if(type() == Fermentable::Grain)
          retString = tr("Steeped");
       else
          retString = tr("Not mashed");
    }
    return retString;
}

const QString Fermentable::additionTimeStringTr() const
{
    QString retString;

    if(addAfterBoil())
       retString = tr("Late");
    else
       retString = tr("Normal");

    return retString;
}

double Fermentable::amount_kg()              const { return get("amount").toDouble(); }
double Fermentable::yield_pct()              const { return get("yield").toDouble(); }
double Fermentable::color_srm()              const { return get("color").toDouble(); }
double Fermentable::coarseFineDiff_pct()     const { return get("coarse_fine_diff").toDouble(); }
double Fermentable::moisture_pct()           const { return get("moisture").toDouble(); }
double Fermentable::diastaticPower_lintner() const { return get("diastatic_power").toDouble(); }
double Fermentable::protein_pct()            const { return get("protein").toDouble(); }
double Fermentable::maxInBatch_pct()         const { return get("max_in_batch").toDouble(); }
double Fermentable::ibuGalPerLb()            const { return get("ibu_gal_per_lb").toDouble(); }

// inventory must be handled separately, to my great annoyance
double Fermentable::inventory() const 
{ 
   return getInventory("amount").toDouble();
}

bool Fermentable::addAfterBoil() const { return get("add_after_boil").toBool(); }
const QString Fermentable::origin() const { return get("origin").toString(); }
const QString Fermentable::supplier() const { return get("supplier").toString(); }
const QString Fermentable::notes() const { return get("notes").toString(); }
bool Fermentable::recommendMash() const { return get("recommend_mash").toBool(); }
bool Fermentable::isMashed() const { return get("is_mashed").toBool(); }
bool Fermentable::isExtract() { return ((type() == Extract) || (type() == Dry_Extract)); }
bool Fermentable::isSugar() { return (type() == Sugar); }


double Fermentable::equivSucrose_kg() const
{
   double ret = amount_kg() * yield_pct() * (1.0-moisture_pct()/100.0) / 100.0;
   
   // If this is a steeped grain...
   if( type() == Grain && !isMashed() )
      return 0.60 * ret; // Reduce the yield by 60%.
   else
      return ret;
}

// disabled per-cell work
/*
unitDisplay Fermentable::displayUnit() const  { return (unitDisplay)get("display_unit").toInt(); }
unitScale Fermentable::displayScale() const { return (unitScale)get("display_scale").toInt(); }
*/

// Set
void Fermentable::setName( const QString& str )
{
   set("name", "name", str);
   emit changedName(str);
}

void Fermentable::setType( Type t )
{
   set("type", "ftype", types.at(t));
}

void Fermentable::setAdditionMethod( Fermentable::AdditionMethod m )
{
   if( m == Fermentable::Mashed )
      setIsMashed(true);
   else
      setIsMashed(false);
}

void Fermentable::setAdditionTime( Fermentable::AdditionTime t )
{
   if( t == Fermentable::Late )
      setAddAfterBoil(true);
   else
      setAddAfterBoil(false);
}

void Fermentable::setAmount_kg( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative amount: %1").arg(num) );
      return;
   }
   else
   {
      set("amount_kg", "amount", num);
   }
}
void Fermentable::setInventoryAmount( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative inventory: %1").arg(num) );
      return;
   }
   else
   {
      setInventory("inventory", "amount", num);
   }
}
void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      set("yield_pct", "yield", num);
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < yield < 100: %1").arg(num) );
   }
}
void Fermentable::setColor_srm( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative color: %1").arg(num) );
      return;
   }
   else
   {
      set("color_srm", "color", num);
   }
}

void Fermentable::setAddAfterBoil( bool b )
{
   set("addAfterBoil", "add_after_boil", b);
}
void Fermentable::setOrigin( const QString& str ) { set("origin","origin",str);}
void Fermentable::setSupplier( const QString& str) { set("supplier","supplier",str);}
void Fermentable::setNotes( const QString& str ) { set("notes","notes",str);}
void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      set("coarseFineDiff_pct", "coarse_fine_diff", num);
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < coarsefinediff < 100: %1").arg(num) );
   }
}
void Fermentable::setMoisture_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      set("moisture_pct", "moisture", num);
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < moisture < 100: %1").arg(num) );
   }
}
void Fermentable::setDiastaticPower_lintner( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative DP: %1").arg(num) );
      return;
   }
   else
   {
      set("diastaticPower_lintner", "diastatic_power", num);
   }
}
void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      set("protein_pct", "protein", num);
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < protein < 100: %1").arg(num) );
   }
}
void Fermentable::setMaxInBatch_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      set("maxInBatch_pct", "max_in_batch", num);
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < maxinbatch < 100: %1").arg(num) );
   }
}
void Fermentable::setRecommendMash( bool b ) { set("recommendMash","recommend_mash",b);}
void Fermentable::setIsMashed(bool var) { set("isMashed","is_mashed",var); }
void Fermentable::setIbuGalPerLb( double num ) { set("ibuGalPerLb","ibu_gal_per_lb",num);}

bool Fermentable::isValidType( const QString& str )
{
   return (types.indexOf(str) >= 0);
}

// disabled per-cell work
/*
void Fermentable::setDisplayUnit( unitDisplay unit ) 
{ 
   set("displayUnit", "display_unit", unit); 
   set("displayScale", "display_scale", noScale);
}

void Fermentable::setDisplayScale( unitScale scale ) { set("displayScale", "display_scale", scale); }
*/
