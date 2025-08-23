/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Style.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "model/Style.h"

#include <QDebug>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Style.cpp"
#endif

QString Style::localisedName() { return tr("Style"); }
QString Style::localisedName_abvMax_pct       () { return tr("Max ABV"           ); }
QString Style::localisedName_abvMin_pct       () { return tr("Min ABV"           ); }
QString Style::localisedName_appearance       () { return tr("Appearance"        ); }
QString Style::localisedName_aroma            () { return tr("Aroma"             ); }
QString Style::localisedName_carbMax_vol      () { return tr("Max Carb"          ); }
QString Style::localisedName_carbMin_vol      () { return tr("Min Carb"          ); }
QString Style::localisedName_category         () { return tr("Category"          ); }
QString Style::localisedName_categoryNumber   () { return tr("Category Number"   ); }
QString Style::localisedName_colorMax_srm     () { return tr("Max Color"         ); }
QString Style::localisedName_colorMin_srm     () { return tr("Min Color"         ); }
QString Style::localisedName_examples         () { return tr("Examples"          ); }
QString Style::localisedName_fgMax            () { return tr("Max FG"            ); }
QString Style::localisedName_fgMin            () { return tr("Min FG"            ); }
QString Style::localisedName_flavor           () { return tr("Flavor"            ); }
QString Style::localisedName_ibuMax           () { return tr("Max IBU"           ); }
QString Style::localisedName_ibuMin           () { return tr("Min IBU"           ); }
QString Style::localisedName_ingredients      () { return tr("Ingredients"       ); }
QString Style::localisedName_mouthfeel        () { return tr("Mouthfeel"         ); }
QString Style::localisedName_notes            () { return tr("Notes"             ); }
QString Style::localisedName_ogMax            () { return tr("Max OG"            ); }
QString Style::localisedName_ogMin            () { return tr("Min OG"            ); }
QString Style::localisedName_overallImpression() { return tr("Overall Impression"); }
QString Style::localisedName_styleGuide       () { return tr("Style Guide"       ); }
QString Style::localisedName_styleLetter      () { return tr("Style Letter"      ); }
QString Style::localisedName_type             () { return tr("Type"              ); }
QString Style::localisedName_typeString       () { return tr("Type String"       ); }

EnumStringMapping const Style::typeStringMapping {
   {Style::Type::Beer    , "beer"    },
   {Style::Type::Cider   , "cider"   },
   {Style::Type::Mead    , "mead"    },
   {Style::Type::Kombucha, "kombucha"},
   {Style::Type::Soda    , "soda"    },
   {Style::Type::Wine    , "wine"    },
   {Style::Type::Other   , "other"   },
};
EnumStringMapping const Style::typeDisplayNames{
   {Style::Type::Beer    , tr("Beer"    )},
   {Style::Type::Cider   , tr("Cider"   )},
   {Style::Type::Mead    , tr("Mead"    )},
   {Style::Type::Kombucha, tr("Kombucha")},
   {Style::Type::Soda    , tr("Soda"    )},
   {Style::Type::Wine    , tr("Wine"    )},
   {Style::Type::Other   , tr("Other"   )},
};

bool Style::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Style const & rhs = static_cast<Style const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_category         , PropertyNames::Style::category         , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_categoryNumber   , PropertyNames::Style::categoryNumber   , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_styleLetter      , PropertyNames::Style::styleLetter      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_styleGuide       , PropertyNames::Style::styleGuide       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_type             , PropertyNames::Style::type             , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_ogMin            , PropertyNames::Style::ogMin            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_ogMax            , PropertyNames::Style::ogMax            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_fgMin            , PropertyNames::Style::fgMin            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_fgMax            , PropertyNames::Style::fgMax            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_ibuMin           , PropertyNames::Style::ibuMin           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_ibuMax           , PropertyNames::Style::ibuMax           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_colorMin_srm     , PropertyNames::Style::colorMin_srm     , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_colorMax_srm     , PropertyNames::Style::colorMax_srm     , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_carbMin_vol      , PropertyNames::Style::carbMin_vol      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_carbMax_vol      , PropertyNames::Style::carbMax_vol      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_abvMin_pct       , PropertyNames::Style::abvMin_pct       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_abvMax_pct       , PropertyNames::Style::abvMax_pct       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_notes            , PropertyNames::Style::notes            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_ingredients      , PropertyNames::Style::ingredients      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_examples         , PropertyNames::Style::examples         , propertiesThatDiffer) &&
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      AUTO_PROPERTY_COMPARE(this, rhs, m_aroma            , PropertyNames::Style::aroma            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_appearance       , PropertyNames::Style::appearance       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_flavor           , PropertyNames::Style::flavor           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_mouthfeel        , PropertyNames::Style::mouthfeel        , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_overallImpression, PropertyNames::Style::overallImpression, propertiesThatDiffer)
   );
}

ObjectStore & Style::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Style>::getInstance();
}

TypeLookup const Style::typeLookup {
   "Style",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, category         , m_category         ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, categoryNumber   , m_categoryNumber   ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, styleLetter      , m_styleLetter      ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, styleGuide       , m_styleGuide       ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, type             , m_type             , ENUM_INFO(Style::type)                    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, ogMin            , m_ogMin            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, ogMax            , m_ogMax            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, fgMin            , m_fgMin            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, fgMax            , m_fgMax            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, ibuMin           , m_ibuMin           , Measurement::PhysicalQuantity::Bitterness ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, ibuMax           , m_ibuMax           , Measurement::PhysicalQuantity::Bitterness ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, colorMin_srm     , m_colorMin_srm     , Measurement::PhysicalQuantity::Color      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, colorMax_srm     , m_colorMax_srm     , Measurement::PhysicalQuantity::Color      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, carbMin_vol      , m_carbMin_vol      , Measurement::PhysicalQuantity::Carbonation),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, carbMax_vol      , m_carbMax_vol      , Measurement::PhysicalQuantity::Carbonation),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, abvMin_pct       , m_abvMin_pct       ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, abvMax_pct       , m_abvMax_pct       ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, notes            , m_notes            ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, ingredients      , m_ingredients      ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, examples         , m_examples         ,           NonPhysicalQuantity::String     ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, aroma            , m_aroma            ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, appearance       , m_appearance       ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, flavor           , m_flavor           ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, mouthfeel        , m_mouthfeel        ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Style, overallImpression, m_overallImpression,           NonPhysicalQuantity::String     ),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Style>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Style>, Style>::value);

//====== Constructors =========

Style::Style(QString name) :
   NamedEntity        {name},
   FolderBase<Style>  {},
   m_category         {""               },
   m_categoryNumber   {""               },
   m_styleLetter      {""               },
   m_styleGuide       {""               },
   m_type             {Style::Type::Beer},
   m_ogMin            {0.0              },
   m_ogMax            {0.0              },
   m_fgMin            {0.0              },
   m_fgMax            {0.0              },
   m_ibuMin           {0.0              },
   m_ibuMax           {0.0              },
   m_colorMin_srm     {0.0              },
   m_colorMax_srm     {0.0              },
   m_carbMin_vol      {std::nullopt     },
   m_carbMax_vol      {std::nullopt     },
   m_abvMin_pct       {std::nullopt     },
   m_abvMax_pct       {std::nullopt     },
   m_notes            {""               },
   m_ingredients      {""               },
   m_examples         {""               },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_aroma            {""               },
   m_appearance       {""               },
   m_flavor           {""               },
   m_mouthfeel        {""               },
   m_overallImpression{""               } {

   CONSTRUCTOR_END
   return;
}

Style::Style(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   FolderBase<Style>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_category         , namedParameterBundle, PropertyNames::Style::category         ),
   SET_REGULAR_FROM_NPB (m_categoryNumber   , namedParameterBundle, PropertyNames::Style::categoryNumber   ),
   SET_REGULAR_FROM_NPB (m_styleLetter      , namedParameterBundle, PropertyNames::Style::styleLetter      ),
   SET_REGULAR_FROM_NPB (m_styleGuide       , namedParameterBundle, PropertyNames::Style::styleGuide       ),
   SET_REGULAR_FROM_NPB (m_type             , namedParameterBundle, PropertyNames::Style::type             ),
   SET_REGULAR_FROM_NPB (m_ogMin            , namedParameterBundle, PropertyNames::Style::ogMin            ),
   SET_REGULAR_FROM_NPB (m_ogMax            , namedParameterBundle, PropertyNames::Style::ogMax            ),
   SET_REGULAR_FROM_NPB (m_fgMin            , namedParameterBundle, PropertyNames::Style::fgMin            ),
   SET_REGULAR_FROM_NPB (m_fgMax            , namedParameterBundle, PropertyNames::Style::fgMax            ),
   SET_REGULAR_FROM_NPB (m_ibuMin           , namedParameterBundle, PropertyNames::Style::ibuMin           ),
   SET_REGULAR_FROM_NPB (m_ibuMax           , namedParameterBundle, PropertyNames::Style::ibuMax           ),
   SET_REGULAR_FROM_NPB (m_colorMin_srm     , namedParameterBundle, PropertyNames::Style::colorMin_srm     ),
   SET_REGULAR_FROM_NPB (m_colorMax_srm     , namedParameterBundle, PropertyNames::Style::colorMax_srm     ),
   SET_REGULAR_FROM_NPB (m_carbMin_vol      , namedParameterBundle, PropertyNames::Style::carbMin_vol      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_carbMax_vol      , namedParameterBundle, PropertyNames::Style::carbMax_vol      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_abvMin_pct       , namedParameterBundle, PropertyNames::Style::abvMin_pct       , std::nullopt),
   SET_REGULAR_FROM_NPB (m_abvMax_pct       , namedParameterBundle, PropertyNames::Style::abvMax_pct       , std::nullopt),
   SET_REGULAR_FROM_NPB (m_notes            , namedParameterBundle, PropertyNames::Style::notes            ),
   SET_REGULAR_FROM_NPB (m_ingredients      , namedParameterBundle, PropertyNames::Style::ingredients      ),
   SET_REGULAR_FROM_NPB (m_examples         , namedParameterBundle, PropertyNames::Style::examples         ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_aroma            , namedParameterBundle, PropertyNames::Style::aroma            , ""),
   SET_REGULAR_FROM_NPB (m_appearance       , namedParameterBundle, PropertyNames::Style::appearance       , ""),
   SET_REGULAR_FROM_NPB (m_flavor           , namedParameterBundle, PropertyNames::Style::flavor           , ""),
   SET_REGULAR_FROM_NPB (m_mouthfeel        , namedParameterBundle, PropertyNames::Style::mouthfeel        , ""),
   SET_REGULAR_FROM_NPB (m_overallImpression, namedParameterBundle, PropertyNames::Style::overallImpression, "") {

   CONSTRUCTOR_END
   return;
}

Style::Style(Style const & other) :
   NamedEntity{other},
   FolderBase<Style>{other},
   m_category         {other.m_category         },
   m_categoryNumber   {other.m_categoryNumber   },
   m_styleLetter      {other.m_styleLetter      },
   m_styleGuide       {other.m_styleGuide       },
   m_type             {other.m_type             },
   m_ogMin            {other.m_ogMin            },
   m_ogMax            {other.m_ogMax            },
   m_fgMin            {other.m_fgMin            },
   m_fgMax            {other.m_fgMax            },
   m_ibuMin           {other.m_ibuMin           },
   m_ibuMax           {other.m_ibuMax           },
   m_colorMin_srm     {other.m_colorMin_srm     },
   m_colorMax_srm     {other.m_colorMax_srm     },
   m_carbMin_vol      {other.m_carbMin_vol      },
   m_carbMax_vol      {other.m_carbMax_vol      },
   m_abvMin_pct       {other.m_abvMin_pct       },
   m_abvMax_pct       {other.m_abvMax_pct       },
   m_notes            {other.m_notes            },
   m_ingredients      {other.m_ingredients      },
   m_examples         {other.m_examples         },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_aroma            {other.m_aroma            },
   m_appearance       {other.m_appearance       },
   m_flavor           {other.m_flavor           },
   m_mouthfeel        {other.m_mouthfeel        },
   m_overallImpression{other.m_overallImpression} {

   CONSTRUCTOR_END
   return;
}

Style::~Style() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QString               Style::category         () const { return m_category         ; }
QString               Style::categoryNumber   () const { return m_categoryNumber   ; }
QString               Style::styleLetter      () const { return m_styleLetter      ; }
QString               Style::styleGuide       () const { return m_styleGuide       ; }
Style::Type           Style::type             () const { return m_type             ; }
double                Style::ogMin            () const { return m_ogMin            ; }
double                Style::ogMax            () const { return m_ogMax            ; }
double                Style::fgMin            () const { return m_fgMin            ; }
double                Style::fgMax            () const { return m_fgMax            ; }
double                Style::ibuMin           () const { return m_ibuMin           ; }
double                Style::ibuMax           () const { return m_ibuMax           ; }
double                Style::colorMin_srm     () const { return m_colorMin_srm     ; }
double                Style::colorMax_srm     () const { return m_colorMax_srm     ; }
std::optional<double> Style::carbMin_vol      () const { return m_carbMin_vol      ; }
std::optional<double> Style::carbMax_vol      () const { return m_carbMax_vol      ; }
std::optional<double> Style::abvMin_pct       () const { return m_abvMin_pct       ; }
std::optional<double> Style::abvMax_pct       () const { return m_abvMax_pct       ; }
QString               Style::notes            () const { return m_notes            ; }
QString               Style::ingredients      () const { return m_ingredients      ; }
QString               Style::examples         () const { return m_examples         ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
QString               Style::aroma            () const { return m_aroma            ; }
QString               Style::appearance       () const { return m_appearance       ; }
QString               Style::flavor           () const { return m_flavor           ; }
QString               Style::mouthfeel        () const { return m_mouthfeel        ; }
QString               Style::overallImpression() const { return m_overallImpression; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Style::setCategory         (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::category         , this->m_category         , val); }
void Style::setCategoryNumber   (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::categoryNumber   , this->m_categoryNumber   , val); }
void Style::setStyleLetter      (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::styleLetter      , this->m_styleLetter      , val); }
void Style::setStyleGuide       (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::styleGuide       , this->m_styleGuide       , val); }
void Style::setType             (Type                  const   val) { SET_AND_NOTIFY(PropertyNames::Style::type             , this->m_type             , val); }
void Style::setOgMin            (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::ogMin            , this->m_ogMin            , this->enforceMin(val, "og min"      )); }
void Style::setOgMax            (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::ogMax            , this->m_ogMax            , this->enforceMin(val, "og max"      )); }
void Style::setFgMin            (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::fgMin            , this->m_fgMin            , this->enforceMin(val, "fg min"      )); }
void Style::setFgMax            (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::fgMax            , this->m_fgMax            , this->enforceMin(val, "fg max"      )); }
void Style::setIbuMin           (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::ibuMin           , this->m_ibuMin           , this->enforceMin(val, "ibu min"     )); }
void Style::setIbuMax           (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::ibuMax           , this->m_ibuMax           , this->enforceMin(val, "ibu max"     )); }
void Style::setColorMin_srm     (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::colorMin_srm     , this->m_colorMin_srm     , this->enforceMin(val, "color min"   )); }
void Style::setColorMax_srm     (double                const   val) { SET_AND_NOTIFY(PropertyNames::Style::colorMax_srm     , this->m_colorMax_srm     , this->enforceMin(val, "color max"   )); }
void Style::setCarbMin_vol      (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Style::carbMin_vol      , this->m_carbMin_vol      , this->enforceMin(val, "carb vol min")); }
void Style::setCarbMax_vol      (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Style::carbMax_vol      , this->m_carbMax_vol      , this->enforceMin(val, "carb vol max")); }
void Style::setAbvMin_pct       (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Style::abvMin_pct       , this->m_abvMin_pct       , this->enforceMinAndMax(val, "min abv pct", 0.0, 100.0)); }
void Style::setAbvMax_pct       (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Style::abvMax_pct       , this->m_abvMax_pct       , this->enforceMinAndMax(val, "max abv pct", 0.0, 100.0)); }
void Style::setNotes            (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::notes            , this->m_notes            , val); }
///void Style::setProfile          (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::profile          , this->m_profile          , val); }
void Style::setIngredients      (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::ingredients      , this->m_ingredients      , val); }
void Style::setExamples         (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::examples         , this->m_examples         , val); }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Style::setAroma            (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::aroma            , this->m_aroma            , val); }
void Style::setAppearance       (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::appearance       , this->m_appearance       , val); }
void Style::setFlavor           (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::flavor           , this->m_flavor           , val); }
void Style::setMouthfeel        (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::mouthfeel        , this->m_mouthfeel        , val); }
void Style::setOverallImpression(QString               const & val) { SET_AND_NOTIFY(PropertyNames::Style::overallImpression, this->m_overallImpression, val); }

//=============================================== OTHER MEMBER FUNCTIONS ===============================================

// This class supports NamedEntity::numRecipesUsedIn
IMPLEMENT_NUM_RECIPES_USED_IN(Style)

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Style)
