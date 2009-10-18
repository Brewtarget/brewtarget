/*
 * MaltinessWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
#include <Qt>
#include <QFrame>
#include "MaltinessWidget.h"

MaltinessWidget::MaltinessWidget(QWidget* parent) : QWidget(parent)
{
   setup();
}

MaltinessWidget::MaltinessWidget(Recipe* recipe)
{
   setup();
   observeRecipe(recipe);
}

void MaltinessWidget::setup()
{
   QSize size(90,30);

   label = new QLabel(this);
   // Want to specify a minimum size and have it expand if able.
   label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
   // Align text in center, vertically and horizontally.
   label->setAlignment(Qt::AlignCenter);
   // Add a border.
   label->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

   palette = label->palette();

   // Set size policy of the MaltinessWidget
   //sPolicy = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
   setMinimumSize(size);
}

void MaltinessWidget::observeRecipe(Recipe* recipe)
{
   recObs = recipe;
   if( recipe != 0 )
      setObserved(recipe);

   update();
}

void MaltinessWidget::paintEvent(QPaintEvent*)
{
   if( recObs == 0 )
      return;

   QPainter painter(this);

   label->resize(size());
   
   palette.setColor(QPalette::Active, QPalette::Window, bgColor());
   label->setPalette(palette);
   label->setText(fgText());

   label->render(&painter);
}

QSize MaltinessWidget::sizeHint() const
{
   return label->sizeHint();
}

/*
QSizePolicy MaltinessWidget::sizePolicy() const
{
   return sPolicy;
}
*/

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
         return QString("<b>Cloying</b>");
      case EXTRAMALTY:
         return QString("<b>Extra malty</b>");
      case SLIGHTLYMALTY:
         return QString("<b>Slightly malty</b>");
      case BALANCED:
         return QString("<b>Balanced</b>");
      case SLIGHTLYHOPPY:
         return QString("<b>Slightly hoppy</b>");
      case EXTRAHOPPY:
         return QString("<b>Extra hoppy</b>");
      case HARSH:
         return QString("<b>Way hoppy</b>");
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

   ibu = recObs->getIBU();
   points = (recObs->getOg() - 1)*1000;

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

void MaltinessWidget::notify(Observable *notifier, QVariant /*info*/)
{
   if( notifier != recObs )
      return;

   update();
}
