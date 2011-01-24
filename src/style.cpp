/*
 * style.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "brewtarget.h"
#include <iostream>
#include <string>
#include <vector>
#include "stringparsing.h"
#include "style.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>

bool operator<(Style &s1, Style &s2)
{
   return s1.name < s2.name;
}

bool operator==(Style &s1, Style &s2)
{
   return s1.name == s2.name;
}

void Style::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement styleNode;
   QDomElement tmpNode;
   QDomText tmpText;

   styleNode = doc.createElement("STYLE");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CATEGORY");
   tmpText = doc.createTextNode(category.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CATEGORY_NUMBER");
   tmpText = doc.createTextNode(categoryNumber.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STYLE_LETTER");
   tmpText = doc.createTextNode(styleLetter.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STYLE_GUIDE");
   tmpText = doc.createTextNode(styleGuide.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(type.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG_MIN");
   tmpText = doc.createTextNode(text(ogMin));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG_MAX");
   tmpText = doc.createTextNode(text(ogMax));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG_MIN");
   tmpText = doc.createTextNode(text(fgMin));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG_MAX");
   tmpText = doc.createTextNode(text(fgMax));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_MIN");
   tmpText = doc.createTextNode(text(ibuMin));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_MAX");
   tmpText = doc.createTextNode(text(ibuMax));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR_MIN");
   tmpText = doc.createTextNode(text(colorMin_srm));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR_MAX");
   tmpText = doc.createTextNode(text(colorMax_srm));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ABV_MIN");
   tmpText = doc.createTextNode(text(abvMin_pct));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ABV_MAX");
   tmpText = doc.createTextNode(text(abvMax_pct));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARB_MIN");
   tmpText = doc.createTextNode(text(carbMin_vol));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARB_MAX");
   tmpText = doc.createTextNode(text(carbMax_vol));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PROFILE");
   tmpText = doc.createTextNode(profile.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INGREDIENTS");
   tmpText = doc.createTextNode(ingredients.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("EXAMPLES");
   tmpText = doc.createTextNode(examples.c_str());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   parent.appendChild(styleNode);
}

//===========================CONSTRUCTORS=======================================

void Style::setDefaults()
{
   name = "";
   category = "";
   categoryNumber = "";
   styleLetter = "";
   styleGuide = "";
   type = "Ale";
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

Style::Style()
{
   setDefaults();
}

Style::Style(const QDomNode& styleNode)
{
   fromNode(styleNode);
}

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
         setName(value.toStdString());
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QString("WATER says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "CATEGORY" )
      {
         setCategory(value.toStdString());
      }
      else if( property == "CATEGORY_NUMBER" )
      {
         setCategoryNumber(value.toStdString());
      }
      else if( property == "STYLE_LETTER" )
      {
         setStyleLetter(value.toStdString());
      }
      else if( property == "STYLE_GUIDE" )
      {
         setStyleGuide(value.toStdString());
      }
      else if( property == "TYPE" )
      {
         setType(value.toStdString());
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
         setNotes(value.toStdString());
      }
      else if( property == "PROFILE" )
      {
         setProfile(value.toStdString());
      }
      else if( property == "INGREDIENTS" )
      {
         setIngredients(value.toStdString());
      }
      else if( property == "EXAMPLES" )
      {
         setExamples(value.toStdString());
      }
      else
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported STYLE property: %1. Line %2").arg(property).arg(node.lineNumber()) );
      }
   }
}

//==============================="SET" METHODS==================================
void Style::setName( const std::string& var )
{
   name = std::string(var);
   hasChanged();
}

void Style::setCategory( const std::string& var )
{
   category = std::string(var);
   hasChanged();
}

void Style::setCategoryNumber( const std::string& var )
{
   categoryNumber = std::string(var);
   hasChanged();
}

void Style::setStyleLetter( const std::string& var )
{
   styleLetter= std::string(var);
   hasChanged();
}

void Style::setStyleGuide( const std::string& var )
{
   styleGuide = std::string(var);
   hasChanged();
}

void Style::setType( const std::string& var )
{
   if( ! isValidType(var) )
      return;
   else
   {
      type = std::string(var);
      hasChanged();
   }
}

void Style::setOgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      ogMin = var;
      hasChanged();
   }
}

void Style::setOgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      ogMax = var;
      hasChanged();
   }
}

void Style::setFgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      fgMin = var;
      hasChanged();
   }
}

void Style::setFgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      fgMax = var;
      hasChanged();
   }
}

void Style::setIbuMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      ibuMin = var;
      hasChanged();
   }
}

void Style::setIbuMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      ibuMax = var;
      hasChanged();
   }
}

void Style::setColorMin_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      colorMin_srm = var;
      hasChanged();
   }
}

void Style::setColorMax_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      colorMax_srm = var;
      hasChanged();
   }
}

void Style::setCarbMin_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      carbMin_vol = var;
      hasChanged();
   }
}

void Style::setCarbMax_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      carbMax_vol = var;
      hasChanged();
   }
}

void Style::setAbvMin_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      abvMin_pct = var;
      hasChanged();
   }
}

void Style::setAbvMax_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      abvMax_pct = var;
      hasChanged();
   }
}

void Style::setNotes( const std::string& var )
{
   notes = std::string(var);
   hasChanged();
}

void Style::setProfile( const std::string& var )
{
   profile = std::string(var);
   hasChanged();
}

void Style::setIngredients( const std::string& var )
{
   ingredients = std::string(var);
   hasChanged();
}

void Style::setExamples( const std::string& var )
{
   examples = std::string(var);
   hasChanged();
}

//============================="GET" METHODS====================================
std::string Style::getName() const
{
   return name;
}

std::string Style::getCategory() const
{
   return category;
}

std::string Style::getCategoryNumber() const
{
   return categoryNumber;
}

std::string Style::getStyleLetter() const
{
   return styleLetter;
}

std::string Style::getStyleGuide() const
{
   return styleGuide;
}

std::string Style::getType() const
{
   return type;
}

double Style::getOgMin() const
{
   return ogMin;
}

double Style::getOgMax() const
{
   return ogMax;
}

double Style::getFgMin() const
{
   return fgMin;
}

double Style::getFgMax() const
{
   return fgMax;
}

double Style::getIbuMin() const
{
   return ibuMin;
}

double Style::getIbuMax() const
{
   return ibuMax;
}

double Style::getColorMin_srm() const
{
   return colorMin_srm;
}

double Style::getColorMax_srm() const
{
   return colorMax_srm;
}

double Style::getCarbMin_vol() const
{
   return carbMin_vol;
}

double Style::getCarbMax_vol() const
{
   return carbMax_vol;
}

double Style::getAbvMin_pct() const
{
   return abvMin_pct;
}

double Style::getAbvMax_pct() const
{
   return abvMax_pct;
}

std::string Style::getNotes() const
{
   return notes;
}

std::string Style::getProfile() const
{
   return profile;
}

std::string Style::getIngredients() const
{
   return ingredients;
}

std::string Style::getExamples() const
{
   return examples;
}

bool Style::isValidType( const std::string &str )
{
   static const std::string types[] = {"Lager", "Ale", "Mead", "Wheat", "Mixed", "Cider"};
   static const unsigned int size = 7;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}
