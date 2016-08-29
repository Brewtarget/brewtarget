/*
 * style.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#include "brewtarget.h"
#include "style.h"
#include <QDebug>

QStringList Style::types = QStringList() << "Lager" << "Ale" << "Mead" << "Wheat" << "Mixed" << "Cider";
QHash<QString,QString> Style::tagToProp = Style::tagToPropHash();

QHash<QString,QString> Style::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   propHash["CATEGORY"] = "category";
   propHash["CATEGORY_NUMBER"] = "categoryNumber";
   propHash["STYLE_LETTER"] = "styleLetter";
   propHash["STYLE_GUIDE"] = "styleGuide";
   //propHash["TYPE"] = "type";
   propHash["OG_MIN"] = "ogMin";
   propHash["OG_MAX"] = "ogMax";
   propHash["FG_MIN"] = "fgMin";
   propHash["FG_MAX"] = "fgMax";
   propHash["IBU_MIN"] = "ibuMin";
   propHash["IBU_MAX"] = "ibuMax";
   propHash["COLOR_MIN"] = "colorMin_srm";
   propHash["COLOR_MAX"] = "colorMax_srm";
   propHash["CARB_MIN"] = "carbMin_vol";
   propHash["CARB_MAX"] = "carbMax_vol";
   propHash["ABV_MIN"] = "abvMin_pct";
   propHash["ABV_MAX"] = "abvMax_pct";
   propHash["NOTES"] = "notes";
   propHash["PROFILE"] = "profile";
   propHash["INGREDIENTS"] = "ingredients";
   propHash["EXAMPLES"] = "examples";
   return propHash;
}

bool operator<(Style &s1, Style &s2)
{
   return s1.name() < s2.name();
}

bool operator==(Style &s1, Style &s2)
{
   return s1.key() == s2.key();
}

Style::Style()
   : BeerXMLElement()
{
}

//==============================="SET" METHODS==================================
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
   set( "type", "s_type", types.at(t) );
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
QString Style::category() const { return get("category").toString(); }
QString Style::categoryNumber() const { return get("category_number").toString(); }
QString Style::styleLetter() const { return get("style_letter").toString(); }
QString Style::styleGuide() const { return get("style_guide").toString(); }
QString Style::notes() const { return get("notes").toString(); }
QString Style::profile() const { return get("profile").toString(); }
QString Style::ingredients() const { return get("ingredients").toString(); }
QString Style::examples() const { return get("examples").toString(); }

const Style::Type Style::type() const { return static_cast<Style::Type>(types.indexOf(get("s_type").toString())); }
const QString Style::typeString() const { return types.at(type()); }

double Style::ogMin()        const { return get("og_min").toDouble(); }
double Style::ogMax()        const { return get("og_max").toDouble(); }
double Style::fgMin()        const { return get("fg_min").toDouble(); }
double Style::fgMax()        const { return get("fg_max").toDouble(); }
double Style::ibuMin()       const { return get("ibu_min").toDouble(); }
double Style::ibuMax()       const { return get("ibu_max").toDouble(); }
double Style::colorMin_srm() const { return get("color_min").toDouble(); }
double Style::colorMax_srm() const { return get("color_max").toDouble(); }
double Style::carbMin_vol()  const { return get("carb_min").toDouble(); }
double Style::carbMax_vol()  const { return get("carb_max").toDouble(); }
double Style::abvMin_pct()   const { return get("abv_min").toDouble(); }
double Style::abvMax_pct()   const { return get("abv_max").toDouble(); }

bool Style::isValidType( const QString &str ) { return types.contains( str ); }

