/*
 * style.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "brewtarget.h"
#include "style.h"

QStringList Style::types = QStringList() << "Lager" << "Ale" << "Mead" << "Wheat" << "Mixed" << "Cider";

bool operator<(Style &s1, Style &s2)
{
   return s1.name() < s2.name();
}

bool operator==(Style &s1, Style &s2)
{
   return s1.name() == s2.name();
}

//===========================CONSTRUCTORS=======================================

/*
void Style::setDefaults()
{
   name = "";
   category = "";
   categoryNumber = "";
   styleLetter = "";
   styleGuide = "";
   type = TYPEALE;
   ogMin = 0.0;
   ogMax = 0.0;
   fgMin = 0.0;
   fgMax = 0.0;
   ibuMin = 0.0;
   ibuMax = 0.0;
   colorMin_srm = 0.0;
   colorMax_srm = 0.0;
   
   // Optional fields
   carbMin_vol = 0.0;
   carbMax_vol = 0.0;
   abvMin_pct = 0.0;
   abvMax_pct = 0.0;
   notes = "";
   profile = "";
   ingredients = "";
   examples = "";
}
*/

Style::Style()
   : BeerXMLElement()
{
}

/*
void Style::fromNode(const QDomNode& styleNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = styleNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line is not an element. Line %1").arg(textNode.lineNumber()) );
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
         setName(value);
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QString("WATER says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "CATEGORY" )
      {
         setCategory(value);
      }
      else if( property == "CATEGORY_NUMBER" )
      {
         setCategoryNumber(value);
      }
      else if( property == "STYLE_LETTER" )
      {
         setStyleLetter(value);
      }
      else if( property == "STYLE_GUIDE" )
      {
         setStyleGuide(value);
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QString("%1 is not a valid type for STYLE. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            type = static_cast<Style::Type>( ndx );
      }
      else if( property == "OG_MIN" )
      {
         setOgMin(getDouble(textNode));
      }
      else if( property == "OG_MAX" )
      {
         setOgMax(getDouble(textNode));
      }
      else if( property == "FG_MIN" )
      {
         setFgMin(getDouble(textNode));
      }
      else if( property == "FG_MAX" )
      {
         setFgMax(getDouble(textNode));
      }
      else if( property == "IBU_MIN" )
      {
         setIbuMin(getDouble(textNode));
      }
      else if( property == "IBU_MAX" )
      {
         setIbuMax(getDouble(textNode));
      }
      else if( property == "COLOR_MIN" )
      {
         setColorMin_srm(getDouble(textNode));
      }
      else if( property == "COLOR_MAX" )
      {
         setColorMax_srm(getDouble(textNode));
      }
      else if( property == "CARB_MIN" )
      {
         setCarbMin_vol(getDouble(textNode));
      }
      else if( property == "CARB_MAX" )
      {
         setCarbMax_vol(getDouble(textNode));
      }
      else if( property == "ABV_MIN" )
      {
         setAbvMin_pct(getDouble(textNode));
      }
      else if( property == "ABV_MAX" )
      {
         setAbvMax_pct(getDouble(textNode));
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "PROFILE" )
      {
         setProfile(value);
      }
      else if( property == "INGREDIENTS" )
      {
         setIngredients(value);
      }
      else if( property == "EXAMPLES" )
      {
         setExamples(value);
      }
      else
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported STYLE property: %1. Line %2").arg(property).arg(node.lineNumber()) );
      }
   }
}
*/

//==============================="SET" METHODS==================================
void Style::setName( const QString& var )
{
   set( "name", "name", var );
}

void Style::setCategory( const QString& var )
{
   set( "category", "category", var );
}

void Style::setCategoryNumber( const QString& var )
{
   set( "categoryNumber", "category_number", var );
}

void Style::setStyleLetter( const QString& var )
{
   set( "styleLetter", "style_letter", var );
}

void Style::setStyleGuide( const QString& var )
{
   set( "styleGuide", "style_guide", var );
}

void Style::setType( Type t )
{
   set( "type", "s_type", static_cast<int>(t) );
}

void Style::setOgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("ogMin", "og_min", var);
   }
}

void Style::setOgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("ogMax", "og_max", var);
   }
}

void Style::setFgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("fgMin", "fg_min", var);
   }
}

void Style::setFgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("fgMax", "fg_max", var);
   }
}

void Style::setIbuMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("ibuMin", "ibu_min", var);
   }
}

void Style::setIbuMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("ibuMax", "ibu_max", var);
   }
}

void Style::setColorMin_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("colorMin_srm", "color_min", var);
   }
}

void Style::setColorMax_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("colorMax_srm", "color_max", var);
   }
}

void Style::setCarbMin_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("carbMin_vol", "carb_min", var);
   }
}

void Style::setCarbMax_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set("carbMax_vol", "carb_max", var);
   }
}

void Style::setAbvMin_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      set("abvMin_pct", "abv_min", var);
   }
}

void Style::setAbvMax_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      set("abvMax_pct", "abv_max", var);
   }
}

void Style::setNotes( const QString& var )
{
   set("notes", "notes", var);
}

void Style::setProfile( const QString& var )
{
   set("profile", "profile", var);
}

void Style::setIngredients( const QString& var )
{
   set("ingredients", "ingredients", var);
}

void Style::setExamples( const QString& var )
{
   set("examples", "examples", var);
}

//============================="GET" METHODS====================================
QString Style::name() const
{
   return get("name").toString();
}

QString Style::category() const
{
   return get("category").toString();
}

QString Style::categoryNumber() const
{
   return get("category_number").toString();
}

QString Style::styleLetter() const
{
   return get("style_letter").toString();
}

QString Style::styleGuide() const
{
   return get("style_guide").toString();
}

const Style::Type Style::type() const
{
   return static_cast<Style::Type>(get("s_type").toInt());
}

const QString Style::typeString() const
{
   return types.at(type());
}

double Style::ogMin() const
{
   return get("og_min").toDouble();
}

double Style::ogMax() const
{
   return get("og_max").toDouble();
}

double Style::fgMin() const
{
   return get("fg_min").toDouble();
}

double Style::fgMax() const
{
   return get("fg_max").toDouble();
}

double Style::ibuMin() const
{
   return get("ibu_min").toDouble();
}

double Style::ibuMax() const
{
   return get("ibu_max").toDouble();
}

double Style::colorMin_srm() const
{
   return get("color_min").toDouble();
}

double Style::colorMax_srm() const
{
   return get("color_max").toDouble();
}

double Style::carbMin_vol() const
{
   return get("carb_min").toDouble();
}

double Style::carbMax_vol() const
{
   return get("carb_max").toDouble();
}

double Style::abvMin_pct() const
{
   return get("abv_min").toDouble();
}

double Style::abvMax_pct() const
{
   return get("abv_max").toDouble();
}

QString Style::notes() const
{
   return get("notes").toString();
}

QString Style::profile() const
{
   return get("profile").toString();
}

QString Style::ingredients() const
{
   return get("ingredients").toString();
}

QString Style::examples() const
{
   return get("examples").toString();
}

bool Style::isValidType( const QString &str )
{
   return types.contains( str );
}

