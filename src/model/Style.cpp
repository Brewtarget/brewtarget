/*
 * model/Style.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/Style.h"

#include <QDebug>

#include "brewtarget.h"
#include "database.h"
#include "StyleSchema.h"
#include "TableSchemaConst.h"

QStringList Style::m_types = QStringList() << "Lager" << "Ale" << "Mead" << "Wheat" << "Mixed" << "Cider";

bool Style::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Style const & rhs = static_cast<Style const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_category       == rhs.m_category       &&
      this->m_categoryNumber == rhs.m_categoryNumber &&
      this->m_styleLetter    == rhs.m_styleLetter    &&
      this->m_styleGuide     == rhs.m_styleGuide     &&
      this->m_type           == rhs.m_type
   );
}

QString Style::classNameStr()
{
   static const QString name("Style");
   return name;
}

//====== Constructors =========

// suitable for something that will be written to the db later
Style::Style(QString t_name, bool cacheOnly)
   : NamedEntity(Brewtarget::STYLETABLE, cacheOnly, t_name, true),
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
     m_examples(QString()) {
   return;
}

// suitable for creating a Style from a database record
Style::Style(TableSchema* table, QSqlRecord rec, int t_key)
   : NamedEntity(table, rec, t_key) {
     m_category = rec.value( table->propertyToColumn( PropertyNames::Style::category)).toString();
     m_categoryNumber = rec.value( table->propertyToColumn( PropertyNames::Style::categoryNumber)).toString();
     m_styleLetter = rec.value( table->propertyToColumn( PropertyNames::Style::styleLetter)).toString();
     m_styleGuide = rec.value( table->propertyToColumn( PropertyNames::Style::styleGuide)).toString();
     m_typeStr = rec.value( table->propertyToColumn( PropertyNames::Style::type)).toString();
     m_ogMin = rec.value( table->propertyToColumn( PropertyNames::Style::ogMin)).toDouble();
     m_ogMax = rec.value( table->propertyToColumn( PropertyNames::Style::ogMax)).toDouble();
     m_fgMin = rec.value( table->propertyToColumn( PropertyNames::Style::fgMin)).toDouble();
     m_fgMax = rec.value( table->propertyToColumn( PropertyNames::Style::fgMax)).toDouble();
     m_ibuMin = rec.value( table->propertyToColumn( PropertyNames::Style::ibuMin)).toDouble();
     m_ibuMax = rec.value( table->propertyToColumn( PropertyNames::Style::ibuMax)).toDouble();
     m_colorMin_srm = rec.value( table->propertyToColumn( PropertyNames::Style::colorMin_srm)).toDouble();
     m_colorMax_srm = rec.value( table->propertyToColumn( PropertyNames::Style::colorMax_srm)).toDouble();
     m_carbMin_vol = rec.value( table->propertyToColumn( PropertyNames::Style::carbMin_vol)).toDouble();
     m_carbMax_vol = rec.value( table->propertyToColumn( PropertyNames::Style::carbMax_vol)).toDouble();
     m_abvMin_pct = rec.value( table->propertyToColumn( PropertyNames::Style::abvMin_pct)).toDouble();
     m_abvMax_pct = rec.value( table->propertyToColumn( PropertyNames::Style::abvMax_pct)).toDouble();
     m_notes = rec.value( table->propertyToColumn( PropertyNames::Style::notes)).toString();
     m_profile = rec.value( table->propertyToColumn( PropertyNames::Style::profile)).toString();
     m_ingredients = rec.value( table->propertyToColumn( PropertyNames::Style::ingredients)).toString();
     m_examples = rec.value( table->propertyToColumn( PropertyNames::Style::examples)).toString();

     m_type = static_cast<Style::Type>(m_types.indexOf(m_typeStr));
}

//==============================="SET" METHODS==================================
void Style::setCategory( const QString& var )
{
   m_category = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::category, var );
   }
}

void Style::setCategoryNumber( const QString& var )
{
   m_categoryNumber = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::categoryNumber, var );
   }
}

void Style::setStyleLetter( const QString& var )
{
   m_styleLetter = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::styleLetter, var );
   }
}

void Style::setStyleGuide( const QString& var )
{
   m_styleGuide = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::styleGuide, var );
   }
}

void Style::setType( Type t )
{

   if ( t < Type::Lager || t > Type::Cider ) {
      qCritical() << t << " cannot be converted to a type";
      m_type = Type::Cider;
   }
   else {
      m_type = t;
   }

   m_typeStr = m_types.at(t);

   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::type, m_typeStr);
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
         setEasy( PropertyNames::Style::ogMin, var);
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
         setEasy( PropertyNames::Style::ogMax, var);
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
         setEasy( PropertyNames::Style::fgMin, var);
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
         setEasy( PropertyNames::Style::fgMax, var);
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
         setEasy( PropertyNames::Style::ibuMin, var);
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
         setEasy( PropertyNames::Style::ibuMax, var);
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
         setEasy( PropertyNames::Style::colorMin_srm, var);
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
         setEasy( PropertyNames::Style::colorMax_srm, var);
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
         setEasy( PropertyNames::Style::carbMin_vol, var);
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
         setEasy( PropertyNames::Style::carbMax_vol, var);
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
         setEasy( PropertyNames::Style::abvMin_pct, var);
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
         setEasy( PropertyNames::Style::abvMax_pct, var);
      }
   }
}

void Style::setNotes( const QString& var )
{
    m_notes = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::notes, var);
   }
}

void Style::setProfile( const QString& var )
{
    m_profile = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::profile, var);
   }
}

void Style::setNamedEntitys( const QString& var )
{
    m_ingredients = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::ingredients, var);
   }
}

void Style::setExamples( const QString& var )
{
    m_examples = var;
   if ( ! m_cacheOnly ) {
      setEasy( PropertyNames::Style::examples, var);
   }
}

//============================="GET" METHODS====================================
QString Style::category() const { return m_category; }
QString Style::categoryNumber() const { return m_categoryNumber; }
QString Style::styleLetter() const { return m_styleLetter; }
QString Style::styleGuide() const { return m_styleGuide; }
QString Style::notes() const { return m_notes; }
QString Style::profile() const { return m_profile; }
QString Style::ingredients() const { return m_ingredients; }
QString Style::examples() const { return m_examples; }
Style::Type Style::type() const { return m_type; }
const QString Style::typeString() const { return m_typeStr; }

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

NamedEntity * Style::getParent() {
   Style * myParent = nullptr;

   // If we don't already know our parent, look it up
   if (!this->parentKey) {
      this->parentKey = Database::instance().getParentNamedEntityKey(*this);
   }

   // If we (now) know our parent, get a pointer to it
   if (this->parentKey) {
      myParent = Database::instance().style(this->parentKey);
   }

   // Return whatever we got
   return myParent;
}

int Style::insertInDatabase() {
   return Database::instance().insertStyle(this);
}

void Style::removeFromDatabase() {
   Database::instance().remove(this);
}
