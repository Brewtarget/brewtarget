/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Misc.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#ifndef MODEL_MISC_H
#define MODEL_MISC_H
#pragma once

#include <QString>
#include <QSqlRecord>

#include "model/Ingredient.h"
#include "model/IngredientBase.h"
#include "model/IngredientAmount.h"
#include "utils/EnumStringMapping.h"

class MiscCatalog;
class MiscEditor;
class InventoryMisc;
class RecipeAdditionMisc;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Misc { inline BtStringConst const property{#property}; }
AddPropertyName(notes    )
AddPropertyName(producer )
AddPropertyName(productId)
AddPropertyName(type     )
AddPropertyName(useFor   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Misc
 *
 * \brief Model for a misc record in the database.
 */
class Misc : public Ingredient, public IngredientBase<Misc> {
   Q_OBJECT

   INGREDIENT_BASE_DECL(Misc)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief The type of ingredient.
    */
   enum class Type {Spice      ,
                    Fining     ,
                    Water_Agent,
                    Herb       ,
                    Flavor     ,
                    Other      ,
                    // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
                    Wood       ,};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Misc::Type and string values suitable for serialisation in DB, BeerJSON, etc (but
    *        \b not BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c Misc::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   /**
    * \brief This is where we centrally define how \c Misc objects can be measured.
    */
   static constexpr auto validMeasures  = Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count;
   static constexpr auto defaultMeasure = Measurement::PhysicalQuantity::Mass;

   //
   // These aliases make it easier to template a number of functions that are essentially the same for a number of
   // different NamedEntity subclasses.
   //
   using CatalogClass        = MiscCatalog;
   using EditorClass         = MiscEditor;
   using InventoryClass      = InventoryMisc;
   using RecipeAdditionClass = RecipeAdditionMisc;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Misc(QString name = "");
   Misc(NamedParameterBundle const & namedParameterBundle);
   Misc(Misc const & other);

   virtual ~Misc();

   //! \brief The \c Type.
   Q_PROPERTY(Type type   READ type   WRITE setType)
   //! \brief What to use it for.
   Q_PROPERTY(QString useFor READ useFor WRITE setUseFor)
   //! \brief The notes.
   Q_PROPERTY(QString notes READ notes WRITE setNotes)
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(QString            producer          READ producer          WRITE setProducer       )
   Q_PROPERTY(QString            productId         READ productId         WRITE setProductId      )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Type    type     () const;
   QString useFor   () const;
   QString notes    () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString producer () const;
   QString productId() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setType     (Type    const   val);
   void setUseFor   (QString const & val);
   void setNotes    (QString const & val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setProducer (QString const & val);
   void setProductId(QString const & val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   Type    m_type     ;
   QString m_useFor   ;
   QString m_notes    ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString m_producer ;
   QString m_productId;
};

BT_DECLARE_METATYPES(Misc)

#endif
