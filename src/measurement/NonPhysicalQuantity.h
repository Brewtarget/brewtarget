/*======================================================================================================================
 * measurement/NonPhysicalQuantity.h is part of Brewtarget, and is copyright the following authors 2022-2025:
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
 =====================================================================================================================*/
#ifndef MEASUREMENT_NONPHYSICALQUANTITY_H
#define MEASUREMENT_NONPHYSICALQUANTITY_H
#pragma once

#include <QString>

/**
 * \brief The types of value other than \c Measurement::PhysicalQuantity that can be shown in a UI field.
 *
 *        Note that there is intentionally \b no value here for \c none or similar.
 *
 *        See comment in \c measurement/PhysicalQuantity.h for why \c Count is a \c PhysicalQuantity rather than a
 *        \c NonPhysicalQuantity.
 */
enum class NonPhysicalQuantity {
   Date,
   String,
   Percentage,
   //! This usually means we need either a \c QCheckBox or a \c BtComboBoxBool
   Bool,
   //! This usually means we need a \c BtComboBoxEnum
   Enum,
   /**
    * This is for things like step number and other orderings where we have 1st, 2nd, 3rd etc (though its usually
    * presented as step #1, step #2, step #3, etc) and should usually be stored in an int (or unsigned int).
    *
    * This is also used for "number of times" something has been done or is to be done.  Eg number of times a yeast
    * sample has been cultured or reused.  Similarly, it's used for the number of fermentation stages in a Recipe.
    *
    * NOTE, however, this is NOT used for "number of things" (eg number of packets of yeast).  For that, we use
    * Measurement::PhysicalQuantity::Count, for the reasons explained in measurement/PhysicalQuantity.h.
    */
   OrdinalNumeral,
   /**
    * This is used for counts of things that will only ever be integers - eg \c NamedEntity::numRecipesUsing
    */
   CardinalNumber,
   /**
    * \brief This is for a number that has no units, not even pseudo ones.  It is currently a bit over-used -- ie there
    *        are places we are using this where we probably should be using a \c PhysicalQuantity.  We should fix these
    *        over time.
    */
   Dimensionless,
   //! Monetary value
   Currency,
   /**
    * \brief This is primarily used for \c OwnedSet::items.  (We mostly don't expose lists of object through Qt
    *        Properties.)
    */
   Objects,
};

/**
 * \brief Return the (untranslated) name of a \c NonPhysicalQuantity suitable for logging
 */
QString GetLoggableName(NonPhysicalQuantity nonPhysicalQuantity);


#endif
