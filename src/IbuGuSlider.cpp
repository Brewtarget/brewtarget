/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * IbuGuSlider.cpp is part of Brewtarget, and is copyright the following authors 2009-2014:
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

#include "IbuGuSlider.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_IbuGuSlider.cpp"

IbuGuSlider::IbuGuSlider(QWidget* parent)
   : RangedSlider(parent)
{
   setRange(0,1);
   setPreferredRange(0,0);
   setPrecision(2);

   QLinearGradient bgGrad( QPointF(0,0), QPointF(1,0) );
   bgGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
   //bgGrad.setColorAt( 0, QColor(255,255,255) );
   bgGrad.setColorAt( (.28+.36)/2.0, QColor(252,144,48) );
   bgGrad.setColorAt( (.36+.44)/2.0, QColor(252,204,4) );
   bgGrad.setColorAt( (.44+.53)/2.0, QColor(243,252,4) );
   bgGrad.setColorAt( (.53+.64)/2.0, QColor(185,240,120) );
   bgGrad.setColorAt( (.64+.85)/2.0, QColor(121,201,121) );
   //bgGrad.setColorAt( .85, QColor(255,255,255) );

   setBackgroundBrush(bgGrad);
   setMarkerBrush(QColor(0,0,0));
   setTickMarks(0,0);
}

void IbuGuSlider::setValue(double value)
{
   QString text;

   if( value < 0.28 )
      text = tr("Cloying");
   else if( value < 0.36 )
      text = tr("Extra Malty");
   else if( value < 0.44 )
      text = tr("Slightly Malty");
   else if( value < 0.53 )
      text = tr("Balanced");
   else if( value < 0.64 )
      text = tr("Slightly Hoppy");
   else if( value < 0.85 )
      text = tr("Extra Hoppy");
   else
      text = tr("Way Hoppy");

   setMarkerText(text);
   RangedSlider::setValue(value);
}
