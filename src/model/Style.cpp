/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Style.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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

QString Style::localisedName() { return tr("Style"); }

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

bool Style::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Style const & rhs = static_cast<Style const &>(other);
   // Base class will already have ensured names are equal
   return (
      Utils::AutoCompare(this->m_category      , rhs.m_category      ) &&
      Utils::AutoCompare(this->m_categoryNumber, rhs.m_categoryNumber) &&
      Utils::AutoCompare(this->m_styleLetter   , rhs.m_styleLetter   ) &&
      Utils::AutoCompare(this->m_styleGuide    , rhs.m_styleGuide    ) &&
      Utils::AutoCompare(this->m_type          , rhs.m_type          )
   );
}

ObjectStore & Style::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Style>::getInstance();
}

TypeLookup const Style::typeLookup {
   "Style",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::category         , Style::m_category         ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::categoryNumber   , Style::m_categoryNumber   ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::styleLetter      , Style::m_styleLetter      ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::styleGuide       , Style::m_styleGuide       ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::type             , Style::m_type             ,           NonPhysicalQuantity::Enum       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::ogMin            , Style::m_ogMin            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::ogMax            , Style::m_ogMax            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::fgMin            , Style::m_fgMin            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::fgMax            , Style::m_fgMax            , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::ibuMin           , Style::m_ibuMin           , Measurement::PhysicalQuantity::Bitterness ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::ibuMax           , Style::m_ibuMax           , Measurement::PhysicalQuantity::Bitterness ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::colorMin_srm     , Style::m_colorMin_srm     , Measurement::PhysicalQuantity::Color      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::colorMax_srm     , Style::m_colorMax_srm     , Measurement::PhysicalQuantity::Color      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::carbMin_vol      , Style::m_carbMin_vol      , Measurement::PhysicalQuantity::Carbonation),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::carbMax_vol      , Style::m_carbMax_vol      , Measurement::PhysicalQuantity::Carbonation),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::abvMin_pct       , Style::m_abvMin_pct       ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::abvMax_pct       , Style::m_abvMax_pct       ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::notes            , Style::m_notes            ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::ingredients      , Style::m_ingredients      ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::examples         , Style::m_examples         ,           NonPhysicalQuantity::String     ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::aroma            , Style::m_aroma            ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::appearance       , Style::m_appearance       ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::flavor           , Style::m_flavor           ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::mouthfeel        , Style::m_mouthfeel        ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Style::overallImpression, Style::m_overallImpression,           NonPhysicalQuantity::String     ),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Style>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Style>, Style>::value);

//====== Constructors =========

// suitable for something that will be written to the db later
Style::Style(QString name) :
   NamedEntity        {name, true       },
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
   SET_REGULAR_FROM_NPB (m_carbMin_vol      , namedParameterBundle, PropertyNames::Style::carbMin_vol      ),
   SET_REGULAR_FROM_NPB (m_carbMax_vol      , namedParameterBundle, PropertyNames::Style::carbMax_vol      ),
   SET_REGULAR_FROM_NPB (m_abvMin_pct       , namedParameterBundle, PropertyNames::Style::abvMin_pct       ),
   SET_REGULAR_FROM_NPB (m_abvMax_pct       , namedParameterBundle, PropertyNames::Style::abvMax_pct       ),
   SET_REGULAR_FROM_NPB (m_notes            , namedParameterBundle, PropertyNames::Style::notes            ),
   SET_REGULAR_FROM_NPB (m_ingredients      , namedParameterBundle, PropertyNames::Style::ingredients      ),
   SET_REGULAR_FROM_NPB (m_examples         , namedParameterBundle, PropertyNames::Style::examples         ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_aroma            , namedParameterBundle, PropertyNames::Style::aroma            ),
   SET_REGULAR_FROM_NPB (m_appearance       , namedParameterBundle, PropertyNames::Style::appearance       ),
   SET_REGULAR_FROM_NPB (m_flavor           , namedParameterBundle, PropertyNames::Style::flavor           ),
   SET_REGULAR_FROM_NPB (m_mouthfeel        , namedParameterBundle, PropertyNames::Style::mouthfeel        ),
   SET_REGULAR_FROM_NPB (m_overallImpression, namedParameterBundle, PropertyNames::Style::overallImpression) {
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

//==============================="SET" METHODS==================================
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

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Style)
