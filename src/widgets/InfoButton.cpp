/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/InfoButton.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "widgets/InfoButton.h"

#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QPainter>
#include <QStaticText>

#include "utils/Fonts.h"
#include "widgets/InfoText.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_InfoButton.cpp"

namespace {
   QColor const backgroundColorInfoShown {0x14, 0x82, 0x63};
   QColor const backgroundColorPressed   {0x63, 0x14, 0x82};
   QColor const backgroundColorInfoHidden{0x82, 0x63, 0x14};
   QColor const foregroundColorEnabled   {0xf8, 0xe0, 0x27};
   QColor const foregroundColorDisabled  {0x80, 0x80, 0x80};
   // In newer versions of Qt, we can use QColorConstants::Transparent, but it's not exactly hard to declare our own
   // transparent "color".  (Note it's only the fourth parameter that matters here.  Transparent black = transparent
   // white etc!
   QColor const backgroundTransparent    {0x00, 0x00, 0x00, 0x00};

   // In English, "i" stands for "info" (which is short for "information", which is short for "Please show me more
   // information about this setting").
   QString const buttonText = InfoButton::tr("i");

   /**
    * \brief Since the font is loaded from the resource bundle, we have to be careful about not trying to access it
    *        before the application is fully started up.
    */
   QFont const & getDisplayFont() {
      // QFonts are specified in point size, so the hard-coded numbers are fine here, even on HDPI displays
      // Note that if QFont::Black is specified as third parameter to QFont constructor, it is a weight (beyond
      // ExtraBold) not a colour!  Final (boolean) parameter is whether or not the font should be italic.
      static QFont const displayFont{Fonts::getInstance().TexGyreSchola_BoldItalic, 11, QFont::Bold, true};
      return displayFont;
   }
}

InfoButton::InfoButton(QWidget * parent) : QPushButton(parent), m_infoText{nullptr} {
   //
   // Shrink the button down from its default size, otherwise the 'ⓘ' will be surrounded by too much space.  Leave a
   // bit of space around the 'ⓘ' however, otherwise it looks cramped.  (Yes, we are adding 10% twice -- once to make
   // the circle 10% bigger than it otherwise would be, and once to add 10% space around the outside of the circle.)
   //
   // This is one of the few places where we really do want to use setFixedSize (rather than setMinimumSize or
   // setMinimumWidth).  It doesn't make sense to let Qt's layout manager grow the button just because the window it is
   // in is expanded.
   //
   double const diameter = static_cast<double>(this->getCircleDiameter());
   int const width = static_cast<int>(diameter * 1.1);
   int const height = width;
   this->setFixedSize(QSize(width, height));

   connect(this, &QAbstractButton::clicked, this, &InfoButton::doClick);

   return;
}

InfoButton::~InfoButton() = default;

void InfoButton::linkWith(InfoText * infoText) {
   // It's a coding error to pass in a null pointer
   Q_ASSERT(infoText);
   m_infoText = infoText;
   // Make sure the text starts out in expected state (ie not visible)
   m_infoText->hide();
   return;
}

void InfoButton::doClick() {
   // Nothing to do if we have no associated QLabel
   if (!m_infoText) {
      return;
   }

   if (m_infoText->isVisible()) {
      m_infoText->hide();
   } else {
      m_infoText->show();
   }
   return;
}

void InfoButton::paintEvent([[maybe_unused]] QPaintEvent * event) {
   // NB: We do not call QPaintEvent::paintEvent because it's not going to do anything helpful.  We do all the painting
   //     here (and we ignore any styles that might have been applied to the object, because we already know how we want
   //     it to look).

   //
   // By virtue of inheriting from QPushButton we're going to get given a default size.  This is too large for what we
   // want.  The button icon we draw will be smaller than the space we're allotted.  We don't want to specify an
   // absolute size, because this won't cope well with the fact that different displays have different resolutions.  We
   // have a couple of resolution-independent ways to get a sensible pixel size though: either a percentage size of the
   // default size we got from QPushButton, or take a font-size in points and obtain a bounding rectangle for some
   // suitable text.  In both cases, this gets Qt to do the work of adjusting the pixel size for the current display
   // dots-per-inch.
   //
   // I've gone with the font size approach because it's similar to what we do elsewhere, and likely to be more
   // consistent / platform-independent than whatever the default size of a button is.
   //
   int const diameter = this->getCircleDiameter();

   // If there is no infoText, then the button is disabled and is shown in grey (foregroundColorDisabled)
   QColor const & foregroundColor{m_infoText ? foregroundColorEnabled : foregroundColorDisabled};

   // The background color of a button is always transparent if it is disabled.  For an enabled buttin, it is one of
   // three colors: we show a "transition" color when the button is down - ie pressed but not released; otherwise we
   // show one color for when the info is displayed and another for when it is hidden.
   QColor const & backgroundColor{
      (
         !m_infoText ? backgroundTransparent : (
            this->isDown() ? backgroundColorPressed : (
               m_infoText->isVisible() ? backgroundColorInfoShown : backgroundColorInfoHidden
            )
         )
      )
   };

   // The coordinates below make a lot more sense if you know that origin (0,0) is in the middle of the button!

   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, false);
   painter.translate(this->width() / 2, this->height() / 2);

   // Set the color or stipple that is used for drawing lines or boundaries
   painter.setPen(QPen(foregroundColor, 2));
   // Set the color or pattern that is used for filling shapes
   painter.setBrush(QBrush(backgroundColor));

   painter.drawEllipse(QRect(0 - diameter/2, 0 - diameter/2, diameter, diameter));

   painter.setPen(QPen(foregroundColor, 4));
   painter.setFont(getDisplayFont());
   painter.drawText(0 - diameter/2, 0 - diameter/2, diameter, diameter, Qt::AlignCenter, tr("i"));

   return;
}

void InfoButton::resizeEvent(QResizeEvent * event) {
   QPushButton::resizeEvent(event);
   int const diameter = this->getCircleDiameter() + 4;
   int const xOff = (this->width() - diameter) / 2;
   int const yOff = (this->height() - diameter) / 2;
   this->setMask(QRegion(xOff, yOff, diameter, diameter, QRegion::Ellipse));
   return;
}

int InfoButton::getCircleDiameter() const {
   QFont const & displayFont = getDisplayFont();

   //
   // These are used for sizing the button according to the text -- see more substantive comment above in
   // InfoButton::paintEvent().  Here, we call QFontMetrics::tightBoundingRect rather than QFontMetrics::boundingRect
   // because the latter leaves a _lot_ of space around the 'i'.
   //
   // Note that we have to call this every time we want the diameter because, eg, the user might move the window between
   // two monitors with different native resolutions.
   //
   QFontMetrics const displayFontMetrics(displayFont);
   QRect const minimumTextRect = displayFontMetrics.tightBoundingRect(buttonText);

   // This gives us the length of the side of a square that would definitely contain the "i" (or equivalent in another
   // language).
   int const minSquareDiameter = std::max(minimumTextRect.width(), minimumTextRect.height());

   //
   // From a bounding square, we can get a bounding circle (albeit not a _minimal_ one) for the "i" character by drawing
   // a circle that passes through all four corners of the square.  The diameter of this circle is just √2 × the length
   // of a side of the square.  (It's the same as the distance between opposing corners of the square which, by
   // Pythagoras is √(s²+s²) = √(2s²) = √2s where s is the length of a side of the square.)
   //
   // In reality, although a circle of this diameter completely encloses the letter, it looks a bit cramped, so we make
   // it 10% bigger to give it a bit more "breathing space".
   //
   int const circleDiameter = static_cast<int>(std::sqrt(2.0) * static_cast<double>(minSquareDiameter) * 1.1);

   // Not expecting the available space in which we have to draw to ever be smaller than our calculated circle, but
   // handle that case anyway.
   int const availableDiameter = std::min(this->height(), this->width());
   return std::min(circleDiameter, availableDiameter);
}
