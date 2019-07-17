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

#include "TableSchema.h"
#include "TableSchemaConst.h"
#include "StyleTableSchema.h"

QStringList Style::m_types = QStringList() << "Lager" << "Ale" << "Mead" << "Wheat" << "Mixed" << "Cider";

/*
// Maps from the XML tag to the object property
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

// Maps from the object property to the database column
QHash<QString,QString > Style::columnToProp = Style::columnToPropHash();
QHash<QString,QString> Style::columnToPropHash()
{
   QHash<QString,QString> propHash;
   propHash[kNameProp]           = kName;
   propHash[kCategoryProp]       = kCategory;
   propHash[kCategoryNumberProp] = kCategoryNumber;
   propHash[kStyleLetterProp]    = kStyleLetter;
   propHash[kStyleGuideProp]     = kStyleGuide;
   propHash[kTypeProp]           = kType;
   propHash[kOGMinProp]          = kOGMin;
   propHash[kOGMaxProp]          = kOGMax;
   propHash[kFGMinProp]          = kFGMin;
   propHash[kFGMaxProp]          = kFGMax;
   propHash[kIBUMinProp]         = kIBUMin;
   propHash[kIBUMaxProp]         = kIBUMax;
   propHash[kColorMinProp]       = kColorMin;
   propHash[kColorMaxProp]       = kColorMax;
   propHash[kCarbMinProp]        = kCarbMin;
   propHash[kCarbMaxProp]        = kCarbMax;
   propHash[kABVMinProp]         = kABVMin;
   propHash[kABVMaxProp]         = kABVMax;
   propHash[kNotesProp]          = kNotes;
   propHash[kProfileProp]        = kProfile;
   propHash[kIngredientsProp]    = kIngredients;
   propHash[kExamplesProp]       = kExamples;

   return propHash;
}
*/
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
   : BeerXMLElement(Brewtarget::STYLETABLE, -1, QString(), true),
     m_category(QString()),
     m_categoryNumber(QString()),
     m_styleLetter(QString()),
     m_styleGuide(QString()),
     m_typeStr(QString()),
     m_type(static_cast<Style::Type>(0)),
     m_ogMin(0.0),
     m_ogMax(0.0),
     m_fgMin(0.0),
     m_fgMax(0.0),
     m_ibuMin(0.0),
     m_ibuMax(0.0),
     m_colorMin_srm(0.0),
     m_colorMax_srm(0.0),
     m_carbMin_vol(0.0),
     m_carbMax_vol(0.0),
     m_abvMin_pct(0.0),
     m_abvMax_pct(0.0),
     m_notes(QString()),
     m_profile(QString()),
     m_ingredients(QString()),
     m_examples(QString()),
     m_cacheOnly(false)
{
}

Style::Style(QString t_name, bool cache)
   : BeerXMLElement(Brewtarget::STYLETABLE, -1, t_name, true),
     m_category(QString()),
     m_categoryNumber(QString()),
     m_styleLetter(QString()),
     m_styleGuide(QString()),
     m_typeStr(QString()),
     m_type(static_cast<Style::Type>(0)),
     m_ogMin(0.0),
     m_ogMax(0.0),
     m_fgMin(0.0),
     m_fgMax(0.0),
     m_ibuMin(0.0),
     m_ibuMax(0.0),
     m_colorMin_srm(0.0),
     m_colorMax_srm(0.0),
     m_carbMin_vol(0.0),
     m_carbMax_vol(0.0),
     m_abvMin_pct(0.0),
     m_abvMax_pct(0.0),
     m_notes(QString()),
     m_profile(QString()),
     m_ingredients(QString()),
     m_examples(QString()),
     m_cacheOnly(cache)
{
}

Style::Style(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool()),
     m_category(rec.value(kcolCategory).toString()),
     m_categoryNumber(rec.value(kcolCategoryNumber).toString()),
     m_styleLetter(rec.value(kcolStyleLetter).toString()),
     m_styleGuide(rec.value(kcolStyleGuide).toString()),
     m_typeStr(rec.value(kcolType).toString()),
     m_type(static_cast<Style::Type>(m_types.indexOf(m_typeStr))),
     m_ogMin(rec.value(kcolOGMin).toDouble()),
     m_ogMax(rec.value(kcolOGMax).toDouble()),
     m_fgMin(rec.value(kcolFGMin).toDouble()),
     m_fgMax(rec.value(kcolFGMax).toDouble()),
     m_ibuMin(rec.value(kcolIBUMin).toDouble()),
     m_ibuMax(rec.value(kcolIBUMax).toDouble()),
     m_colorMin_srm(rec.value(kcolColorMin).toDouble()),
     m_colorMax_srm(rec.value(kcolColorMax).toDouble()),
     m_carbMin_vol(rec.value(kcolCarbMin).toDouble()),
     m_carbMax_vol(rec.value(kcolCarbMax).toDouble()),
     m_abvMin_pct(rec.value(kcolABVMin).toDouble()),
     m_abvMax_pct(rec.value(kcolABVMax).toDouble()),
     m_notes(rec.value(kcolNotes).toString()),
     m_profile(rec.value(kcolProfile).toString()),
     m_ingredients(rec.value(kcolIngredients).toString()),
     m_examples(rec.value(kcolExamples).toString()),
     m_cacheOnly(false)
{
}

//==============================="SET" METHODS==================================
void Style::setCategory( const QString& var )
{
   m_category = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropCategory, var );
   }
}

void Style::setCategoryNumber( const QString& var )
{
   m_categoryNumber = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropCategoryNumber, var );
   }
}

void Style::setStyleLetter( const QString& var )
{
   m_styleLetter = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropStyleLetter, var );
   }
}

void Style::setStyleGuide( const QString& var )
{
   m_styleGuide = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropStyleGuide, var );
   }
}

void Style::setType( Type t )
{
   m_type = t;
   m_typeStr = m_types.at(t);
   if ( ! m_cacheOnly ) {
      setEasy( kpropType, m_typeStr);
   }
}

void Style::setOgMin( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_ogMin = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropOGMin, var);
      }
   }
}

void Style::setOgMax( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_ogMax = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropOGMax, var);
      }
   }
}

void Style::setFgMin( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_fgMin = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropFGMin, var);
      }
   }
}

void Style::setFgMax( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_fgMax = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropFGMax, var);
      }
   }
}

void Style::setIbuMin( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_ibuMin = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropIBUMin, var);
      }
   }
}

void Style::setIbuMax( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_ibuMax = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropIBUMax, var);
      }
   }
}

void Style::setColorMin_srm( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_colorMin_srm = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropColorMin, var);
      }
   }
}

void Style::setColorMax_srm( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_colorMax_srm = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropColorMax, var);
      }
   }
}

void Style::setCarbMin_vol( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_carbMin_vol = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropCarbMin, var);
      }
   }
}

void Style::setCarbMax_vol( double var )
{
   if( var < 0.0 ) {
      return;
   }
   else
   {
      m_carbMax_vol = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropCarbMax, var);
      }
   }
}

void Style::setAbvMin_pct( double var )
{
   if( var < 0.0 || var > 100.0 ) {
      return;
   }
   else
   {
      m_abvMin_pct = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropABVMin, var);
      }
   }
}

void Style::setAbvMax_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
        m_abvMax_pct = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropABVMax, var);
      }
   }
}

void Style::setNotes( const QString& var )
{
    m_notes = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropNotes, var);
   }
}

void Style::setProfile( const QString& var )
{
    m_profile = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropProfile, var);
   }
}

void Style::setIngredients( const QString& var )
{
    m_ingredients = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropIngredients, var);
   }
}

void Style::setExamples( const QString& var )
{
    m_examples = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropExamples, var);
   }
}

void Style::setCacheOnly( const bool cache ) { m_cacheOnly = cache; }

//============================="GET" METHODS====================================
QString Style::category() const { return m_category; }
QString Style::categoryNumber() const { return m_categoryNumber; }
QString Style::styleLetter() const { return m_styleLetter; }
QString Style::styleGuide() const { return m_styleGuide; }
QString Style::notes() const { return m_notes; }
QString Style::profile() const { return m_profile; }
QString Style::ingredients() const { return m_ingredients; }
QString Style::examples() const { return m_examples; }
const Style::Type Style::type() const { return m_type; }
const QString Style::typeString() const { return m_typeStr; }

bool Style::cacheOnly() const { return m_cacheOnly; }
double Style::ogMin() const { return m_ogMin; }
double Style::ogMax() const { return m_ogMax; }
double Style::fgMin() const { return m_fgMin; }
double Style::fgMax() const { return m_fgMax; }
double Style::ibuMin() const { return m_ibuMin; }
double Style::ibuMax() const { return m_ibuMax; }
double Style::colorMin_srm() const { return m_colorMin_srm; }
double Style::colorMax_srm() const { return m_colorMax_srm; }
double Style::carbMin_vol() const { return m_carbMin_vol; }
double Style::carbMax_vol() const { return m_carbMax_vol; }
double Style::abvMin_pct() const { return m_abvMin_pct; }
double Style::abvMax_pct() const { return m_abvMax_pct; }

bool Style::isValidType( const QString &str )
{
   return m_types.contains( str );
}

