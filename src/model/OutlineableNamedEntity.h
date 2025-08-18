/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/OutlineableNamedEntity.h is part of Brewtarget, and is copyright the following authors 2024-2025:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef MODEL_OUTLINEABLENAMEDENTITY_H
#define MODEL_OUTLINEABLENAMEDENTITY_H
#pragma once

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::OutlineableNamedEntity { inline BtStringConst const property{#property}; }
AddPropertyName(outline)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \brief Abstract class that extends \c NamedEntity to provide the concept of an "outlineable" object.  This is needed
 *        in BeerJSON for \c Ingredient subclasses and \c Water.
 *
 *        In BeerJSON, a Hop/Fermentable/Misc/Yeast/Water can be defined in two different ways.  As a free-standing
 *        record (eg FermentableType) all the fields are included.  As part of a recipe-addition record (eg
 *        FermentableAdditionType), only a subset of the fields are used.  In BeerJSON, this subset is called the
 *        "Base", but we use the term "Outline" (as we're already using "base" for other things).  The following table
 *        (using BeerJSON field names) is a summary:
 *
 *                        ┌───────────────────┬─────────────────┬──────────────┬───────────────────────┬───────────────┐
 *                        │ Fermentable       │ Hop             │ Misc         │ Yeast (Culture)       │ Water         │
 *           ┌────────────┼───────────────────┼─────────────────┼──────────────┼───────────────────────┼───────────────┤
 *           │ Outline    │ • name            │ • name          │ • name       │ • name                │ • name        │
 *           │ Fields     │ • type            │ • producer      │ • producer   │ • type                │ • producer ‡  │
 *           │            │ • origin          │ • product_id    │ • product_id │ • form                │ • calcium     │
 *           │            │ • producer        │ • origin        │ • type       │ • producer            │ • bicarbonate │
 *           │            │ • product_id      │ • year          │              │ • product_id          │ • carbonate   │
 *           │            │ • grain_group     │ • form          │              │                       │ • potassium   │
 *           │            │ • yield †         │ • alpha_acid    │              │                       │ • iron        │
 *           │            │ • color           │ • beta_acid     │              │                       │ • nitrate     │
 *           │            │                   │                 │              │                       │ • nitrite     │
 *           │            │                   │                 │              │                       │ • flouride ⹋  │
 *           │            │                   │                 │              │                       │ • sulfate     │
 *           │            │                   │                 │              │                       │ • chloride    │
 *           │            │                   │                 │              │                       │ • sodium      │
 *           │            │                   │                 │              │                       │ • magnesium   │
 *           ├────────────┼───────────────────┼─────────────────┼──────────────┼───────────────────────┼───────────────┤
 *           │ Additional │ • notes           │ • type          │ • use_for    │ • temperature_range † │ • pH          │
 *           │ Full       │ • moisture        │ • notes         │ • notes      │ • alcohol_tolerance   │ • notes       │
 *           │ Record     │ • alpha_amylase   │ • percent_lost  │ • inventory  │ • flocculation        │               │
 *           │ Fields     │ • diastatic_power │ • substitutes   │              │ • attenuation_range † │               │
 *           │            │ • protein         │ • oil_content † │              │ • notes               │               │
 *           │            │ • kolbach_index   │ • inventory     │              │ • best_for            │               │
 *           │            │ • max_in_batch    │                 │              │ • max_reuse           │               │
 *           │            │ • recommend_mash  │                 │              │ • pof                 │               │
 *           │            │ • inventory       │                 │              │ • glucoamylase        │               │
 *           │            │ • glassy          │                 │              │ • inventory           │               │
 *           │            │ • plump           │                 │              │ • zymocide †          │               │
 *           │            │ • half            │                 │              │                       │               │
 *           │            │ • mealy           │                 │              │                       │               │
 *           │            │ • thru            │                 │              │                       │               │
 *           │            │ • friability      │                 │              │                       │               │
 *           │            │ • di_ph           │                 │              │                       │               │
 *           │            │ • viscosity       │                 │              │                       │               │
 *           │            │ • dms_p           │                 │              │                       │               │
 *           │            │ • fan             │                 │              │                       │               │
 *           │            │ • fermentability  │                 │              │                       │               │
 *           │            │ • beta_glucan     │                 │              │                       │               │
 *           └────────────┴───────────────────┴─────────────────┴──────────────┴───────────────────────┴───────────────┘
 *            † = compound field; ‡ = field we don't support; ⹋ sic -- see https://github.com/beerjson/beerjson/issues/214
 *
 *        What this means is that, when we read in a Hop/Fermentable/Misc/Yeast/Water inside a RecipeAddition/
 *        RecipeUseOf record (an XxxxAdditionType record in BeerJSON) then we need different logic when we "check for
 *        duplicates".  The record we read in will only have the Outline fields, so we don't want to compare the other
 *        fields when searching for existing records.
 *
 *        Eg, when reading a Recipe from BeerJSON, when we get to each FermentableAdditionType record, we would rather
 *        find a match for an existing Fermentable record than use the partial data from the BeerJSON outline/base
 *        record to create a new Fermentable.
 *
 *        The \c OutlineableNamedEntity class just adds an \c outline flag to make the necessary logic in BeerJSON
 *        processing easier.  It is not currently used anywhere else in the code.
 */
class OutlineableNamedEntity : public NamedEntity {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_outline();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   OutlineableNamedEntity(QString name);
   OutlineableNamedEntity(NamedParameterBundle const & namedParameterBundle);
   OutlineableNamedEntity(OutlineableNamedEntity const & other);

   virtual ~OutlineableNamedEntity();

   //! \brief Whether this is an outline record.  Note that we don't store this.
   Q_PROPERTY(bool outline   READ outline   WRITE setOutline   STORED false)

   bool outline() const;
   void setOutline(bool const val);

protected:
   bool m_outline;
};

#endif
