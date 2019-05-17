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

Style::Style(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   _typeStr = rec.value(kType).toString();
   _type = static_cast<Style::Type>(types.indexOf(_typeStr));

   _category = rec.value(kCategory).toString();
   _categoryNumber = rec.value(kCategoryNumber).toString();
   _styleLetter = rec.value(kStyleLetter).toString();
   _styleGuide = rec.value(kStyleGuide).toString();

   _ogMin = rec.value(kOGMin).toDouble();
   _ogMax = rec.value(kOGMax).toDouble();
   _fgMin = rec.value(kFGMin).toDouble();
   _fgMax = rec.value(kFGMax).toDouble();
   _ibuMin = rec.value(kIBUMin).toDouble();
   _ibuMax = rec.value(kIBUMax).toDouble();
   _colorMin_srm = rec.value(kColorMin).toDouble();
   _colorMax_srm = rec.value(kColorMax).toDouble();
   _carbMin_vol = rec.value(kCarbMin).toDouble();
   _carbMax_vol = rec.value(kCarbMax).toDouble();
   _abvMin_pct = rec.value(kABVMin).toDouble();
   _abvMax_pct = rec.value(kABVMax).toDouble();
   _notes = rec.value(kNotes).toString();
   _profile = rec.value(kProfile).toString();
   _ingredients = rec.value(kIngredients).toString();
   _examples = rec.value(kExamples).toString();
}

//==============================="SET" METHODS==================================
void Style::setCategory( const QString& var )
{
   _category = var;
   set( kCategoryProp, kCategory, var );
}

void Style::setCategoryNumber( const QString& var )
{
   _categoryNumber = var;
   set( kCategoryNumberProp, kCategoryNumber, var );
}

void Style::setStyleLetter( const QString& var )
{
   _styleLetter = var;
   set( kStyleLetterProp, kStyleLetter, var );
}

void Style::setStyleGuide( const QString& var )
{
   _styleGuide = var;
   set( kStyleGuideProp, kStyleGuide, var );
}

void Style::setType( Type t )
{
   _type = t;
   _typeStr = types.at(t);
   set( kTypeProp, kType, _typeStr);
}

void Style::setOgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _ogMin = var;
      set(kOGMinProp, kOGMin, var);
   }
}

void Style::setOgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _ogMax = var;
      set(kOGMaxProp, kOGMax, var);
   }
}

void Style::setFgMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _fgMin = var;
      set(kFGMinProp, kFGMin, var);
   }
}

void Style::setFgMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _fgMax = var;
      set(kFGMaxProp, kFGMax, var);
   }
}

void Style::setIbuMin( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _ibuMin = var;
      set(kIBUMinProp, kIBUMin, var);
   }
}

void Style::setIbuMax( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _ibuMax = var;
      set(kIBUMaxProp, kIBUMax, var);
   }
}

void Style::setColorMin_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _colorMin_srm = var;
      set(kColorMinProp, kColorMin, var);
   }
}

void Style::setColorMax_srm( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _colorMax_srm = var;
      set(kColorMaxProp, kColorMax, var);
   }
}

void Style::setCarbMin_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _carbMin_vol = var;
      set(kCarbMinProp, kCarbMin, var);
   }
}

void Style::setCarbMax_vol( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      _carbMax_vol = var;
      set(kCarbMaxProp, kCarbMax, var);
   }
}

void Style::setAbvMin_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      _abvMin_pct = var;
      set(kABVMinProp, kABVMin, var);
   }
}

void Style::setAbvMax_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      _abvMax_pct = var;
      set(kABVMaxProp, kABVMax, var);
   }
}

void Style::setNotes( const QString& var )
{
   _notes = var;
   set(kNotesProp, kNotes, var);
}

void Style::setProfile( const QString& var )
{
   _profile = var;
   set(kProfileProp, kProfile, var);
}

void Style::setIngredients( const QString& var )
{
   _ingredients = var;
   set(kIngredientsProp, kIngredients, var);
}

void Style::setExamples( const QString& var )
{
   _examples = var;
   set(kExamplesProp, kExamples, var);
}

//============================="GET" METHODS====================================
QString Style::category() const { return _category; }
QString Style::categoryNumber() const { return _categoryNumber; }
QString Style::styleLetter() const { return _styleLetter; }
QString Style::styleGuide() const { return _styleGuide; }
QString Style::notes() const { return _notes; }
QString Style::profile() const { return _profile; }
QString Style::ingredients() const { return _ingredients; }
QString Style::examples() const { return _examples; }
const Style::Type Style::type() const { return _type; }
const QString Style::typeString() const { return _typeStr; }

double Style::ogMin() const { return _ogMin; }
double Style::ogMax() const { return _ogMax; }
double Style::fgMin() const { return _fgMin; }
double Style::fgMax() const { return _fgMax; }
double Style::ibuMin() const { return _ibuMin; }
double Style::ibuMax() const { return _ibuMax; }
double Style::colorMin_srm() const { return _colorMin_srm; }
double Style::colorMax_srm() const { return _colorMax_srm; }
double Style::carbMin_vol() const { return _carbMin_vol; }
double Style::carbMax_vol() const { return _carbMax_vol; }
double Style::abvMin_pct() const { return _abvMin_pct; }
double Style::abvMax_pct() const { return _abvMax_pct; }

bool Style::isValidType( const QString &str )
{
   return types.contains( str );
}

