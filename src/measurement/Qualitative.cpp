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

EnumStringMapping const Measurement::Qualitative::wortClarityDisplayNames {
   {WortClarity::VeryClear      , QObject::tr("Very Clear"      )},
   {WortClarity::Clear          , QObject::tr("Clear"           )},
   {WortClarity::SomewhatClear  , QObject::tr("Somewhat Clear"  )},
   {WortClarity::Average        , QObject::tr("Average"         )},
   {WortClarity::SomewhatCloudy , QObject::tr("Somewhat Cloudy" )},
   {WortClarity::Cloudy         , QObject::tr("Cloudy"          )},
   {WortClarity::VeryCloudy     , QObject::tr("Very Cloudy"     )},
   {WortClarity::ExtremelyCloudy, QObject::tr("Extremely Cloudy")},
};

template<class S>
S & operator<<(S & stream, Measurement::Qualitative::WortClarity const val) {
   stream <<
      "WortClarity #" << static_cast<int>(val) << ": (" <<
      Measurement::Qualitative::wortClarityStringMapping[val] << ")";
   return stream;
}
template QDebug      & operator<<(QDebug      & stream, Measurement::Qualitative::WortClarity const val);
template QTextStream & operator<<(QTextStream & stream, Measurement::Qualitative::WortClarity const val);