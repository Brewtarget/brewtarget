/*======================================================================================================================
 * measurement/Qualitative.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "measurement/Qualitative.h"

#include <QDebug>

#include "utils/EnumStringMapping.h"

EnumStringMapping const Measurement::Qualitative::wortClarityStringMapping {
   {WortClarity::VeryClear      , "very clear"      },
   {WortClarity::Clear          , "clear"           },
   {WortClarity::SomewhatClear  , "somewhat clear"  },
   {WortClarity::Average        , "average"         },
   {WortClarity::SomewhatCloudy , "somewhat cloudy" },
   {WortClarity::Cloudy         , "cloudy"          },
   {WortClarity::VeryCloudy     , "very cloudy"     },
   {WortClarity::ExtremelyCloudy, "extremely cloudy"},
};

EnumDisplayNameMapping const Measurement::Qualitative::wortClarityDisplayNames {"QObject", {
   {WortClarity::VeryClear      , QT_TRANSLATE_NOOP("QObject", "Very Clear"      )},
   {WortClarity::Clear          , QT_TRANSLATE_NOOP("QObject", "Clear"           )},
   {WortClarity::SomewhatClear  , QT_TRANSLATE_NOOP("QObject", "Somewhat Clear"  )},
   {WortClarity::Average        , QT_TRANSLATE_NOOP("QObject", "Average"         )},
   {WortClarity::SomewhatCloudy , QT_TRANSLATE_NOOP("QObject", "Somewhat Cloudy" )},
   {WortClarity::Cloudy         , QT_TRANSLATE_NOOP("QObject", "Cloudy"          )},
   {WortClarity::VeryCloudy     , QT_TRANSLATE_NOOP("QObject", "Very Cloudy"     )},
   {WortClarity::ExtremelyCloudy, QT_TRANSLATE_NOOP("QObject", "Extremely Cloudy")},
}};

template<class S>
S & operator<<(S & stream, Measurement::Qualitative::WortClarity const val) {
   stream <<
      "WortClarity #" << static_cast<int>(val) << ": (" <<
      Measurement::Qualitative::wortClarityStringMapping[val] << ")";
   return stream;
}
template QDebug      & operator<<(QDebug      & stream, Measurement::Qualitative::WortClarity const val);
template QTextStream & operator<<(QTextStream & stream, Measurement::Qualitative::WortClarity const val);