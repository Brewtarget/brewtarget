/*
 * MaltinessWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QSize>
#include <QPainter>
#include <QSizePolicy>
#include <QFrame>
#include "MaltinessWidget.h"
#include "brewtarget.h"
#include "recipe.h"

MaltinessWidget::MaltinessWidget(QWidget* parent) : QLabel(parent), recObs(0)
{
   setup();
}

void MaltinessWidget::setup()
{
   QSize size(110,50);

   // Want to specify a minimum size and have it expand if able.
   //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
   // Align text in center, vertically and horizontally.
   setAlignment(Qt::AlignCenter);
   // Add a border.
   setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

   //palette = palette();

   // Set size policy of the MaltinessWidget
   //sPolicy = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
   setMinimumSize(size);
   setMaximumSize(size);
}

void MaltinessWidget::observeRecipe(Recipe* recipe)
{
   if( recObs )
      disconnect( recObs, 0, this, 0 );
   
   recObs = recipe;
   if( recObs )
   {
      connect( recObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      updateInfo();
      update();
   }
}

QColor MaltinessWidget::bgColor()
{
   switch(region())
   {
      case CLOYING:
         return QColor::fromRgb(252,91,10);
      case EXTRAMALTY:
         return QColor::fromRgb(252,134,10);
      case SLIGHTLYMALTY:
         return QColor::fromRgb(246,195,6);
      case BALANCED:
         return QColor::fromRgb(235,248,11);
      case SLIGHTLYHOPPY:
         return QColor::fromRgb(148,251,12);
      case EXTRAHOPPY:
         return QColor::fromRgb(12,248,26);
      case HARSH:
         return QColor::fromRgb(12,178,13);
      default:
         return QColor::fromRgb(255,255,255);
   }
}

QString MaltinessWidget::fgText()
{
   switch(region())
   {
      case CLOYING:
         return tr("<b>Cloying</b>");
      case EXTRAMALTY:
         return tr("<b>Extra malty</b>");
      case SLIGHTLYMALTY:
         return tr("<b>Slightly malty</b>");
      case BALANCED:
         return tr("<b>Balanced</b>");
      case SLIGHTLYHOPPY:
         return tr("<b>Slightly hoppy</b>");
      case EXTRAHOPPY:
         return tr("<b>Extra hoppy</b>");
      case HARSH:
         return tr("<b>Way hoppy</b>");
      default:
         return QString("");
   }
}

int MaltinessWidget::region()
{
   double ibu;
   double points;

   if( recObs == 0 )
      return -1;

   ibu = recObs->IBU();
   points = (recObs->og() - 1)*1000;

   if( (11./3.)*ibu-5./3. < points )
      return CLOYING;
   else if( 3*ibu-5 < points && points <= (11./3.)*ibu-5./3. )
      return EXTRAMALTY;
   else if( (7./3.)*ibu-5./3. < points && points <= 3*ibu-5 )
      return SLIGHTLYMALTY;
   else if( 2*ibu-5 < points && points <= (7./3.)*ibu-5./3. )
      return BALANCED;
   else if( (5./3.)*ibu-10./3. < points && points <= 2*ibu-5 )
      return SLIGHTLYHOPPY;
   else if( (5./4.)*ibu-3.75 < points && points <= (5./3.)*ibu-10./3. )
      return EXTRAHOPPY;
   else
      return HARSH;
}

void MaltinessWidget::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() != recObs )
      return;

   QString propName(prop.name());
   if( propName == "IBU" || propName == "og" )
   {
      updateInfo();
      update();
   }
}

// Changes the text/color based on recipe statistics.
void MaltinessWidget::updateInfo()
{
   QColor bg = bgColor();

   QString colorstring = QString("%1%2%3").arg(bg.red(),2,16,QChar('0')).arg(bg.green(),2,16,QChar('0')).arg(bg.blue(),2,16,QChar('0'));

   setStyleSheet(QString("QLabel { background: #%1 }").arg(colorstring).toAscii());
   setText(fgText());
}
