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




/************* Columns *************/
const QString kName("name");
const QString kCategory("category");
const QString kCategoryNumber("category_number");
const QString kStyleLetter("style_letter");
const QString kStyleGuide("style_guide");
const QString kType("s_type");
const QString kOGMin("og_min");
const QString kOGMax("og_max");
const QString kFGMin("fg_min");
const QString kFGMax("fg_max");
const QString kIBUMin("ibu_min");
const QString kIBUMax("ibu_max");
const QString kColorMin("color_min");
const QString kColorMax("color_max");
const QString kCarbMin("carb_min");
const QString kCarbMax("carb_max");
const QString kABVMin("abv_min");
const QString kABVMax("abv_max");
const QString kNotes("notes");
const QString kProfile("profile");
const QString kIngredients("ingredients");
const QString kExamples("examples");


/************** Props **************/
const QString kNameProp("name");
const QString kCategoryProp("category");
const QString kCategoryNumberProp("categoryNumber");
const QString kStyleLetterProp("styleLetter");
const QString kStyleGuideProp("styleGuide");
const QString kTypeProp("type");
const QString kOGMinProp("ogMin");
const QString kOGMaxProp("ogMax");
const QString kFGMinProp("fgMin");
const QString kFGMaxProp("fgMax");
const QString kIBUMinProp("ibuMin");
const QString kIBUMaxProp("ibuMax");
const QString kColorMinProp("colorMin_srm");
const QString kColorMaxProp("colorMax_srm");
const QString kCarbMinProp("carbMin_vol");
const QString kCarbMaxProp("carbMax_vol");
const QString kABVMinProp("abvMin_pct");
const QString kABVMaxProp("abvMax_pct");
const QString kNotesProp("notes");
const QString kProfileProp("profile");
const QString kIngredientsProp("ingredients");
const QString kExamplesProp("examples");


QStringList Style::types = QStringList() << "Lager" << "Ale" << "Mead" << "Wheat" << "Mixed" << "Cider";
QHash<QString,QString> Style::tagToProp = Style::tagToPropHash();

QHash<QString,QString> Style::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = kNameProp;
   propHash["CATEGORY"] = kCategoryProp;
   propHash["CATEGORY_NUMBER"] = kCategoryNumberProp;
   propHash["STYLE_LETTER"] = kStyleLetterProp;
   propHash["STYLE_GUIDE"] = kStyleGuideProp;
   //propHash["TYPE"] = kTypeProp;
   propHash["OG_MIN"] = kOGMinProp;
   propHash["OG_MAX"] = kOGMaxProp;
   propHash["FG_MIN"] = kFGMinProp;
   propHash["FG_MAX"] = kFGMaxProp;
   propHash["IBU_MIN"] = kIBUMinProp;
   propHash["IBU_MAX"] = kIBUMaxProp;
   propHash["COLOR_MIN"] = kColorMinProp;
   propHash["COLOR_MAX"] = kColorMaxProp;
   propHash["CARB_MIN"] = kCarbMinProp;
   propHash["CARB_MAX"] = kCarbMaxProp;
   propHash["ABV_MIN"] = kABVMinProp;
   propHash["ABV_MAX"] = kABVMaxProp;
   propHash["NOTES"] = kNotesProp;
   propHash["PROFILE"] = kProfileProp;
   propHash["INGREDIENTS"] = kIngredientsProp;
   propHash["EXAMPLES"] = kExamplesProp;
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

QString Style::classNameStr()
{
   static const QString name("Style");
   return name;
}

Style::Style(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
}

//==============================="SET" METHODS==================================
void Style::setCategory( const QString& var )
{
   set( kCategoryProp, kCategory, var );
}

void Style::setCategoryNumber( const QString& var )
{
   set( kCategoryNumberProp, kCategoryNumber, var );
}

void Style::setStyleLetter( const QString& var )
{
   set( kStyleLetterProp, kStyleLetter, var );
}

void Style::setStyleGuide( const QString& var )
{
   set( kStyleGuideProp, kStyleGuide, var );
}

void Style::setType( Type t )
{
   set( kTypeProp, kType, types.at(t) );
}

void Style::setOgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kOGMinProp, kOGMin, var);
   }
}

void Style::setOgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kOGMaxProp, kOGMax, var);
   }
}

void Style::setFgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kFGMinProp, kFGMin, var);
   }
}

void Style::setFgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kFGMaxProp, kFGMax, var);
   }
}

void Style::setIbuMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kIBUMinProp, kIBUMin, var);
   }
}

void Style::setIbuMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kIBUMaxProp, kIBUMax, var);
   }
}

void Style::setColorMin_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kColorMinProp, kColorMin, var);
   }
}

void Style::setColorMax_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kColorMaxProp, kColorMax, var);
   }
}

void Style::setCarbMin_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kCarbMinProp, kCarbMin, var);
   }
}

void Style::setCarbMax_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      set(kCarbMaxProp, kCarbMax, var);
   }
}

void Style::setAbvMin_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      set(kABVMinProp, kABVMin, var);
   }
}

void Style::setAbvMax_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      set(kABVMaxProp, kABVMax, var);
   }
}

void Style::setNotes( const QString& var )
{
   set(kNotesProp, kNotes, var);
}

void Style::setProfile( const QString& var )
{
   set(kProfileProp, kProfile, var);
}

void Style::setIngredients( const QString& var )
{
   set(kIngredientsProp, kIngredients, var);
}

void Style::setExamples( const QString& var )
{
   set(kExamplesProp, kExamples, var);
}

//============================="GET" METHODS====================================
QString Style::category() const
{
   return get(kCategory).toString();
}

QString Style::categoryNumber() const
{
   return get(kCategoryNumber).toString();
}

QString Style::styleLetter() const
{
   return get(kStyleLetter).toString();
}

QString Style::styleGuide() const
{
   return get(kStyleGuide).toString();
}

QString Style::notes() const
{
   return get(kNotes).toString();
}

QString Style::profile() const
{
   return get(kProfile).toString();
}

QString Style::ingredients() const
{
   return get(kIngredients).toString();
}

QString Style::examples() const
{
   return get(kExamples).toString();
}

const Style::Type Style::type() const
{
   return static_cast<Style::Type>(types.indexOf(get(kType).toString()));
}

const QString Style::typeString() const
{
   return types.at(type());
}

double Style::ogMin() const
{
   return get(kOGMin).toDouble();
}

double Style::ogMax() const
{
   return get(kOGMax).toDouble();
}

double Style::fgMin() const
{
   return get(kFGMin).toDouble();
}

double Style::fgMax() const
{
   return get(kFGMax).toDouble();
}

double Style::ibuMin() const
{
   return get(kIBUMin).toDouble();
}

double Style::ibuMax() const
{
   return get(kIBUMax).toDouble();
}

double Style::colorMin_srm() const
{
   return get(kColorMin).toDouble();
}

double Style::colorMax_srm() const
{
   return get(kColorMax).toDouble();
}

double Style::carbMin_vol() const
{
   return get(kCarbMin).toDouble();
}

double Style::carbMax_vol() const
{
   return get(kCarbMax).toDouble();
}

double Style::abvMin_pct() const
{
   return get(kABVMin).toDouble();
}

double Style::abvMax_pct() const
{
   return get(kABVMax).toDouble();
}

bool Style::isValidType( const QString &str )
{
   return types.contains( str );
}

