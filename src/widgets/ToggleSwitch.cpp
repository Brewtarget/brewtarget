/*
 * widgets/ToggleSwitch.cpp is is part of Brewtarget, and is copyright the following
 * authors 2018-2021:
 * - Iman Ahmadvand <iman72411@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "widgets/ToggleSwitch.h"

#include <QtCore/qeasingcurve.h>

Q_DECL_IMPORT void qt_blurImage(QPainter * p, QImage & blurImage, qreal radius, bool quality, bool alphaOnly,
                                int transposed = 0); // src/widgets/effects/qpixmapfilter.cpp

namespace {
   constexpr auto CORNER_RADIUS = 8.0;
   constexpr auto THUMB_RADIUS = 14.5;
   constexpr auto SHADOW_ELEVATION = 2.0;

   bool leftToRight(QWidget * w) {
      if (nullptr != w) {
         return w->layoutDirection() == Qt::LeftToRight;
      }
      return false;
   }

   QPixmap drawShadowEllipse(qreal radius, qreal elevation, QColor const & color) {
      auto px = QPixmap(radius * 2, radius * 2);
      px.fill(Qt::transparent);

      {
         // draw ellipes
         QPainter p(&px);
         p.setBrush(color);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing, true);
         p.drawEllipse(QRectF(0, 0, px.size().width(), px.size().height()).center(), radius - elevation, radius - elevation);
      }

      QImage tmp(px.size(), QImage::Format_ARGB32_Premultiplied);
      tmp.setDevicePixelRatio(px.devicePixelRatioF());
      tmp.fill(0);
      QPainter tmpPainter(&tmp);
      tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
      tmpPainter.drawPixmap(QPointF(), px);
      tmpPainter.end();

      // blur the alpha channel
      QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
      blurred.setDevicePixelRatio(px.devicePixelRatioF());
      blurred.fill(0);
      {
         QPainter blurPainter(&blurred);
         qt_blurImage(&blurPainter, tmp, elevation * 4., true, false);
      }

      tmp = blurred;

      return QPixmap::fromImage(tmp);
   }

}

ToggleSwitch::ToggleSwitch(QWidget * parent) : SelectionControl(parent) {
   this->init();
   return;
}

ToggleSwitch::ToggleSwitch(QString const & text, QWidget * parent) : ToggleSwitch(parent) {
   this->setText(text);
   return;
}

ToggleSwitch::ToggleSwitch(QString const & text, QBrush const & brush, QWidget * parent) : ToggleSwitch(text, parent) {
   this->style.thumbOnColor = brush.color();
   this->style.trackOnColor = brush.color();
   return;
}

ToggleSwitch::~ToggleSwitch() = default;

void ToggleSwitch::init() {
//   this->setFont(style.font);
   this->setObjectName("ToggleSwitch");
   /* setup animations */
   this->thumbBrushAnimation = new Animator{ this, this };
   this->trackBrushAnimation = new Animator{ this, this };
   this->thumbPosAnimation = new Animator{ this, this };
   this->thumbPosAnimation->setup(style.thumbPosAnimation.duration, style.thumbPosAnimation.easing);
   this->trackBrushAnimation->setup(style.trackBrushAnimation.duration, style.trackBrushAnimation.easing);
   this->thumbBrushAnimation->setup(style.thumbBrushAnimation.duration, style.thumbBrushAnimation.easing);
   /* set init values */
   this->trackBrushAnimation->setStartValue(this->style.trackOffColor);
   this->trackBrushAnimation->setEndValue(this->style.trackOffColor);
   this->thumbBrushAnimation->setStartValue(this->style.thumbOffColor);
   this->thumbBrushAnimation->setEndValue(this->style.thumbOffColor);
   /* set standard palettes */
   auto p = palette();
   p.setColor(QPalette::Active, QPalette::ButtonText, style.textColor);
   p.setColor(QPalette::Disabled, QPalette::ButtonText, style.textColor);
   this->setPalette(p);
   this->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed));
   return;
}

QRect ToggleSwitch::indicatorRect() {
   auto const w = style.indicatorMargin.left() + style.height + style.indicatorMargin.right();
   return leftToRight(this) ? QRect(0, 0, w, style.height) : QRect(width() - w, 0, w, style.height);
}

QRect ToggleSwitch::textRect() {
   auto const w = style.indicatorMargin.left() + style.height + style.indicatorMargin.right();
   return leftToRight(this) ? rect().marginsRemoved(QMargins(w, 0, 0, 0)) : rect().marginsRemoved(QMargins(0, 0, w, 0));
}

QSize ToggleSwitch::sizeHint() const {
   auto h = style.height;
   auto w = style.indicatorMargin.left() + style.height + style.indicatorMargin.right() + fontMetrics().width(text());

   return QSize(w, h);
}

void ToggleSwitch::paintEvent(QPaintEvent *) {
   // For desktop usage we do not need Radial reaction

   QPainter p(this);

   const auto textRectangle = this->textRect();
   auto trackMargin = style.indicatorMargin;
   trackMargin.setTop(trackMargin.top() + 2);
   trackMargin.setBottom(trackMargin.bottom() + 2);
   QRectF trackRect = this->indicatorRect().marginsRemoved(trackMargin);

   if (isEnabled()) {
      p.setOpacity(1.0);
      p.setPen(Qt::NoPen);
      // Draw track
      p.setBrush(trackBrushAnimation->currentValue().value<QColor>());
      p.setRenderHint(QPainter::Antialiasing, true);
      p.drawRoundedRect(trackRect, CORNER_RADIUS, CORNER_RADIUS);
      p.setRenderHint(QPainter::Antialiasing, false);
      // Draw thumb
      trackRect.setX(trackRect.x() - trackMargin.left() - trackMargin.right() - 2 +
                     thumbPosAnimation->currentValue().toInt());
      auto thumbRect = trackRect;

      if (!shadowPixmap.isNull()) {
         p.drawPixmap(thumbRect.center() - QPointF(THUMB_RADIUS, THUMB_RADIUS - 1.0), shadowPixmap);
      }

      p.setBrush(thumbBrushAnimation->currentValue().value<QColor>());
      p.setRenderHint(QPainter::Antialiasing, true);
      // qDebug() << thumbRect << thumbPosAnimation->currentValue();
      p.drawEllipse(thumbRect.center(), THUMB_RADIUS - SHADOW_ELEVATION - 1.0, THUMB_RADIUS - SHADOW_ELEVATION - 1.0);
      p.setRenderHint(QPainter::Antialiasing, false);

      // Draw text
      if (!text().isEmpty()) {
         p.setOpacity(1.0);
         p.setPen(palette().color(QPalette::Active, QPalette::ButtonText));
         p.setFont(font());
         p.drawText(textRectangle, Qt::AlignLeft | Qt::AlignVCenter, text());
      }

   } else {
      p.setOpacity(this->style.trackDisabledColor.alphaF());
      p.setPen(Qt::NoPen);
      // Draw track
      p.setBrush(this->style.trackDisabledColor);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.drawRoundedRect(trackRect, CORNER_RADIUS, CORNER_RADIUS);
      p.setRenderHint(QPainter::Antialiasing, false);
      // Draw thumb
      p.setOpacity(1.0);
      if (!isChecked()) {
         trackRect.setX(trackRect.x() - trackMargin.left() - trackMargin.right() - 2);
      } else {
         trackRect.setX(trackRect.x() + trackMargin.left() + trackMargin.right() + 2);
      }
      auto thumbRect = trackRect;

      if (!shadowPixmap.isNull()) {
         p.drawPixmap(thumbRect.center() - QPointF(THUMB_RADIUS, THUMB_RADIUS - 1.0), shadowPixmap);
      }

      p.setOpacity(1.0);
      p.setBrush(this->style.trackDisabledColor);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.drawEllipse(thumbRect.center(), THUMB_RADIUS - SHADOW_ELEVATION - 1.0, THUMB_RADIUS - SHADOW_ELEVATION - 1.0);

      // Draw text
      if (!text().isEmpty()) {
         p.setOpacity(style.disabledTextOpacity);
         p.setPen(palette().color(QPalette::Disabled, QPalette::ButtonText));
         p.setFont(font());
         p.drawText(textRectangle, Qt::AlignLeft | Qt::AlignVCenter, text());
      }

   }
   return;
}

void ToggleSwitch::resizeEvent(QResizeEvent * e) {
   shadowPixmap = drawShadowEllipse(THUMB_RADIUS, SHADOW_ELEVATION, QColor(0, 0, 0, 70));
   SelectionControl::resizeEvent(e);
   return;
}

void ToggleSwitch::toggle(Qt::CheckState state) {
   if (state == Qt::Checked) {
      QVariant const posEnd = (style.indicatorMargin.left() + style.indicatorMargin.right() + 2) * 2;
      QVariant const thumbEnd = this->style.thumbOnColor;
      QVariant const trackEnd = this->style.trackOnColor;

      if (!isVisible()) {
         thumbPosAnimation->setCurrentValue(posEnd);
         thumbBrushAnimation->setCurrentValue(thumbEnd);
         trackBrushAnimation->setCurrentValue(trackEnd);
      } else {
         thumbPosAnimation->interpolate(0, posEnd);
         thumbBrushAnimation->interpolate(this->style.thumbOffColor, thumbEnd);
         trackBrushAnimation->interpolate(this->style.trackOffColor, trackEnd);
      }
   } else { // Qt::Unchecked
      QVariant const posEnd = 0;
      QVariant const thumbEnd = this->style.thumbOffColor;
      QVariant const trackEnd = this->style.trackOffColor;

      if (!isVisible()) {
         thumbPosAnimation->setCurrentValue(posEnd);
         thumbBrushAnimation->setCurrentValue(thumbEnd);
         trackBrushAnimation->setCurrentValue(trackEnd);
      } else {
         thumbPosAnimation->interpolate(thumbPosAnimation->currentValue().toInt(), posEnd);
         thumbBrushAnimation->interpolate(this->style.thumbOnColor, thumbEnd);
         trackBrushAnimation->interpolate(this->style.trackOnColor, trackEnd);
      }
   }
   return;
}
