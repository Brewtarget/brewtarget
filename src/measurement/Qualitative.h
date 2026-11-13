/*======================================================================================================================
 * measurement/Qualitative.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef MEASUREMENT_QUALITATIVE_H
#define MEASUREMENT_QUALITATIVE_H
#pragma once

class EnumStringMapping;

/**
 * Things that aren't (easily) quantifiable and are thus measured by judgement
 */
namespace Measurement::Qualitative {
   /**
    * \brief How clear the wort is when it is transferred to the fermenter, judged by eye.  Cloudier wort, with more hop
    *        and malt particles, loses more of its bitterness compounds during fermentation.  This is an input to the
    *        SMPH model of IBUs and is used in \c IbuMethods, \c Recipe::wortClarity (expected clarity) and
    *        \c BrewLog::measuredWortClarity (what was actually seen on brew day).
    *
    *        The values correspond one-to-one with the categories in John-Paul Hosom's SMPH calculator, and with the
    *        \c WortClarity type in dotBeer (see Recipe.beer.schema).
    */
   enum class WortClarity {
      VeryClear      ,
      Clear          ,
      SomewhatClear  ,
      Average        ,
      SomewhatCloudy ,
      Cloudy         ,
      VeryCloudy     ,
      ExtremelyCloudy,
   };
   // Note that we can't use the Q_ENUM macro here to allow storing the above enum class in a QVariant, because Q_ENUM
   // is designed to be used inside a class that derives from QObject.


   /*!
    * \brief Mapping between \c Recipe::WortClarity and string values suitable for serialisation in DB, dotBeer, etc
    */
   extern EnumStringMapping const wortClarityStringMapping;

   /*!
    * \brief Localised names of \c Recipe::WortClarity values suitable for displaying to the end user
    */
   extern EnumStringMapping const wortClarityDisplayNames;
}
/**
 * \brief Convenience functions for logging
 */
/**@{*/
template<class S> S & operator<<(S & stream, Measurement::Qualitative::WortClarity const val);


#endif
