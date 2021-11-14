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
#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

namespace {
   QStringList const types{"Lager", "Ale", "Mead", "Wheat", "Mixed", "Cider"};
}

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

ObjectStore & Style::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Style>::getInstance();
}

//====== Constructors =========

// suitable for something that will be written to the db later
Style::Style(QString t_name, bool cacheOnly) :
   NamedEntity(-1, cacheOnly, t_name, true),
   m_category(QString()),
   m_categoryNumber(QString()),
   m_styleLetter(QString()),
   m_styleGuide(QString()),
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

Style::Style(Style const & other) :
   NamedEntity{other},
   m_category      {other.m_category      },
   m_categoryNumber{other.m_categoryNumber},
   m_styleLetter   {other.m_styleLetter   },
   m_styleGuide    {other.m_styleGuide    },
   m_type          {other.m_type          },
   m_ogMin         {other.m_ogMin         },
   m_ogMax         {other.m_ogMax         },
   m_fgMin         {other.m_fgMin         },
   m_fgMax         {other.m_fgMax         },
   m_ibuMin        {other.m_ibuMin        },
   m_ibuMax        {other.m_ibuMax        },
   m_colorMin_srm  {other.m_colorMin_srm  },
   m_colorMax_srm  {other.m_colorMax_srm  },
   m_carbMin_vol   {other.m_carbMin_vol   },
   m_carbMax_vol   {other.m_carbMax_vol   },
   m_abvMin_pct    {other.m_abvMin_pct    },
   m_abvMax_pct    {other.m_abvMax_pct    },
   m_notes         {other.m_notes         },
   m_profile       {other.m_profile       },
   m_ingredients   {other.m_ingredients   },
   m_examples      {other.m_examples      } {
   return;
}


Style::Style(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity     {namedParameterBundle},
   m_category      {namedParameterBundle(PropertyNames::Style::category      ).toString()},
   m_categoryNumber{namedParameterBundle(PropertyNames::Style::categoryNumber).toString()},
   m_styleLetter   {namedParameterBundle(PropertyNames::Style::styleLetter   ).toString()},
   m_styleGuide    {namedParameterBundle(PropertyNames::Style::styleGuide    ).toString()},
   m_type          {static_cast<Style::Type>(namedParameterBundle(PropertyNames::Style::type).toInt())},
   m_ogMin         {namedParameterBundle(PropertyNames::Style::ogMin         ).toDouble()},
   m_ogMax         {namedParameterBundle(PropertyNames::Style::ogMax         ).toDouble()},
   m_fgMin         {namedParameterBundle(PropertyNames::Style::fgMin         ).toDouble()},
   m_fgMax         {namedParameterBundle(PropertyNames::Style::fgMax         ).toDouble()},
   m_ibuMin        {namedParameterBundle(PropertyNames::Style::ibuMin        ).toDouble()},
   m_ibuMax        {namedParameterBundle(PropertyNames::Style::ibuMax        ).toDouble()},
   m_colorMin_srm  {namedParameterBundle(PropertyNames::Style::colorMin_srm  ).toDouble()},
   m_colorMax_srm  {namedParameterBundle(PropertyNames::Style::colorMax_srm  ).toDouble()},
   m_carbMin_vol   {namedParameterBundle(PropertyNames::Style::carbMin_vol   ).toDouble()},
   m_carbMax_vol   {namedParameterBundle(PropertyNames::Style::carbMax_vol   ).toDouble()},
   m_abvMin_pct    {namedParameterBundle(PropertyNames::Style::abvMin_pct    ).toDouble()},
   m_abvMax_pct    {namedParameterBundle(PropertyNames::Style::abvMax_pct    ).toDouble()},
   m_notes         {namedParameterBundle(PropertyNames::Style::notes         ).toString()},
   m_profile       {namedParameterBundle(PropertyNames::Style::profile       ).toString()},
   m_ingredients   {namedParameterBundle(PropertyNames::Style::ingredients   ).toString()},
   m_examples      {namedParameterBundle(PropertyNames::Style::examples      ).toString()} {
   return;
}


//==============================="SET" METHODS==================================
void Style::setCategory(QString const & var) {
   this->setAndNotify(PropertyNames::Style::category, this->m_category, var);
}

void Style::setCategoryNumber(QString const & var) {
   this->setAndNotify(PropertyNames::Style::categoryNumber, this->m_categoryNumber, var);
}

void Style::setStyleLetter(QString const & var) {
   this->setAndNotify(PropertyNames::Style::styleLetter, this->m_styleLetter, var);
}

void Style::setStyleGuide(QString const & var) {
   this->setAndNotify(PropertyNames::Style::styleGuide, this->m_styleGuide, var);
}

void Style::setType(Type t) {
   this->setAndNotify(PropertyNames::Style::type, this->m_type, t);
}

void Style::setOgMin(double var) {
   this->setAndNotify(PropertyNames::Style::ogMin, this->m_ogMin, this->enforceMin(var, "og min"));
}

void Style::setOgMax(double var) {
   this->setAndNotify(PropertyNames::Style::ogMax, this->m_ogMax, this->enforceMin(var, "og max"));
}

void Style::setFgMin(double var) {
   this->setAndNotify(PropertyNames::Style::fgMin, this->m_fgMin, this->enforceMin(var, "fg min"));
}

void Style::setFgMax(double var) {
   this->setAndNotify(PropertyNames::Style::fgMax, this->m_fgMax, this->enforceMin(var, "fg max"));
}

void Style::setIbuMin(double var) {
   this->setAndNotify(PropertyNames::Style::ibuMin, this->m_ibuMin, this->enforceMin(var, "ibu min"));
}

void Style::setIbuMax(double var) {
   this->setAndNotify(PropertyNames::Style::ibuMax, this->m_ibuMax, this->enforceMin(var, "ibu max"));
}

void Style::setColorMin_srm(double var) {
   this->setAndNotify(PropertyNames::Style::colorMin_srm, this->m_colorMin_srm, this->enforceMin(var, "color min"));
}

void Style::setColorMax_srm(double var) {
   this->setAndNotify(PropertyNames::Style::colorMax_srm, this->m_colorMax_srm, this->enforceMin(var, "color max"));
}

void Style::setCarbMin_vol(double var) {
   this->setAndNotify(PropertyNames::Style::carbMin_vol, this->m_carbMin_vol, this->enforceMin(var, "carb vol min"));
}

void Style::setCarbMax_vol(double var) {
   this->setAndNotify(PropertyNames::Style::carbMax_vol, this->m_carbMax_vol, this->enforceMin(var, "carb vol max"));
}

void Style::setAbvMin_pct(double var) {
   this->setAndNotify(PropertyNames::Style::abvMin_pct, this->m_abvMin_pct, this->enforceMinAndMax(var, "min abv pct", 0.0, 100.0));
}

void Style::setAbvMax_pct(double var) {
   this->setAndNotify(PropertyNames::Style::abvMax_pct, this->m_abvMax_pct, this->enforceMinAndMax(var, "max abv pct", 0.0, 100.0));
}

void Style::setNotes(QString const & var) {
   this->setAndNotify(PropertyNames::Style::notes, this->m_notes, var);
}

void Style::setProfile(QString const & var) {
   this->setAndNotify(PropertyNames::Style::profile, this->m_profile, var);
}

void Style::setIngredients(QString const & var) {
   this->setAndNotify(PropertyNames::Style::ingredients, this->m_ingredients, var);
}

void Style::setExamples(QString const & var) {
   this->setAndNotify(PropertyNames::Style::examples, this->m_examples, var);
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
const QString Style::typeString() const { return types.at(m_type); }

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

Recipe * Style::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
