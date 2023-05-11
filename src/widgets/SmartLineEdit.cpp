/*
 * widgets/SmartLineEdit.cpp is part of Brewtarget, and is copyright the following authors 2009-2023:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mike Evans <mikee@saxicola.co.uk>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "widgets/SmartLineEdit.h"

#include <QDebug>
#include <QFontMetrics>
#include <QMargins>
#include <QRect>
#include <QStyle>

#include "measurement/Measurement.h"
#include "widgets/SmartField.h"
#include "utils/OptionalHelpers.h"
#include "utils/TypeLookup.h"
#include "widgets/SmartLabel.h"

namespace {
   int const min_text_size = 8;
   int const max_text_size = 50;
}

// This private implementation class holds all private non-virtual members of SmartLineEdit
class SmartLineEdit::impl {
public:
   impl(SmartLineEdit & self) :
      m_self{self},
      m_desiredWidthInPixels{0} {
      return;
   }
   ~impl() = default;

   void calculateDisplaySize(QString const & maximalDisplayString) {
      //
      // By default, some, but not all, boxes have a min and max width of 100 pixels, but this is not wide enough on a
      // high DPI display.  We instead calculate width here based on font-size - but without reducing any existing
      // minimum width.
      //
      // Unfortunately, for a QLineEdit object, calculating the width is hard because, besides the text, we need to
      // allow for the width of padding and frame, which is non-trivial to discover.  Eg, typically:
      //   marginsAroundText() and contentsMargins() both return 0 for left and right margins
      //   contentsRect() and frameSize() both give the same width as width()
      // AFAICT, the best option is to query via pixelMetric() calls to the widget's style, but we need to check this
      // works in practice on a variety of different systems.
      //
      QFontMetrics displayFontMetrics(this->m_self.font());
      QRect minimumTextRect = displayFontMetrics.boundingRect(maximalDisplayString);
      QMargins marginsAroundText = this->m_self.textMargins();
      auto myStyle = this->m_self.style();
      // NB: 2× frame width as on left and right; same for horizontal spacing
      int totalWidgetWidthForMaximalDisplayString = minimumTextRect.width() +
                                                   marginsAroundText.left() +
                                                   marginsAroundText.right() +
                                                   (2 * myStyle->pixelMetric(QStyle::PM_DefaultFrameWidth)) +
                                                   (2 * myStyle->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));

      this->m_desiredWidthInPixels = qMax(this->m_self.minimumWidth(), totalWidgetWidthForMaximalDisplayString);
      return;
   }

   void setDisplaySize(bool recalculate = false) {
      if (recalculate) {
         QString sizingString = this->m_self.text();

         // this is a dirty bit of cheating. If we do not reset the minimum
         // width, the field only ever gets bigger. This forces the resize I
         // want, but only when we are instructed to force it
         this->m_self.setMinimumWidth(0);
         if (sizingString.length() < min_text_size) {
            sizingString = QString(min_text_size,'a');
         } else if (sizingString.length() > max_text_size) {
            sizingString = QString(max_text_size,'a');
         }
         this->calculateDisplaySize(sizingString);
      }
      this->m_self.setFixedWidth(this->m_desiredWidthInPixels);
      return;
   }

   /**
    * \brief You can't pass in std::nullopt to setAmount as it's of type std::nullopt_t, so this saves us repeating some
    *        code.
    */
   void setNoAmount() {
      // What the field is measuring doesn't matter as it's not set
      this->m_self.QLineEdit::setText("");
      this->setDisplaySize();
      return;
   }

   SmartLineEdit &                    m_self;
   int                                m_desiredWidthInPixels;
};

SmartLineEdit::SmartLineEdit(QWidget * parent) :
   QLineEdit(parent),
   SmartField{},
   pimpl{std::make_unique<impl>(*this)} {
   connect(this, &QLineEdit::editingFinished, this, &SmartLineEdit::onLineChanged);
   return;
}

SmartLineEdit::~SmartLineEdit() = default;

QString SmartLineEdit::getRawText() const {
   return this->text();
}

void SmartLineEdit::setRawText(QString const & text) {
   this->QLineEdit::setText(text);
   this->pimpl->setDisplaySize();
   return;
}

void SmartLineEdit::connectSmartLabelSignal(SmartLabel & smartLabel) {
   connect(&smartLabel, &SmartLabel::changedSystemOfMeasurementOrScale, this, &SmartLineEdit::lineChanged);
   return;
}

void SmartLineEdit::doPostInitWork() {
   // We can work out (and store) our display size here, but we don't yet set it.  The way the Designer UI Files work is
   // to generate code that calls setters such as setMaximumWidth() etc, which would override anything we do too early
   // on in the life of the object.  To be safe therefore, we set our size when setText() is called.
   this->pimpl->calculateDisplaySize(this->getMaximalDisplayString());
   return;
}

void SmartLineEdit::onLineChanged() {
   Q_ASSERT(this->isInitialised());
   qDebug() << Q_FUNC_INFO;

   if (std::holds_alternative<NonPhysicalQuantity>(*this->getTypeInfo().fieldType)) {
      // The field is not measuring a physical quantity so there are no units or unit conversions to handle
      // However, for anything other than a string, we still want to parse out the numeric part of the input
      this->correctEnteredText();

      if (sender() == this) {
         emit textModified();
      }
      return;
   }

   // The field must be measuring a physical quantity, because we dealt with the other case above!
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->getTypeInfo().fieldType));

   auto const previousScaleInfo = this->getScaleInfo();

   this->lineChanged(previousScaleInfo);
   return;
}

void SmartLineEdit::lineChanged(SmartAmounts::ScaleInfo previousScaleInfo) {
   Q_ASSERT(this->isInitialised());
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->getTypeInfo().fieldType));

   // editingFinished happens on focus being lost, regardless of anything
   // being changed. I am hoping this short circuits properly and we do
   // nothing if nothing changed.
   if (this->sender() == this && !this->isModified()) {
      qDebug() << Q_FUNC_INFO << this->getFqFieldName() << ": Nothing changed; field holds" << this->text();
      return;
   }

   this->correctEnteredText(previousScaleInfo);

   if (sender() == this) {
      emit textModified();
   }

   return;
}
