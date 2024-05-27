/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/ToggleSwitch.h is is part of Brewtarget, and is copyright the following authors 2018-2021:
 *   • Iman Ahmadvand <iman72411@gmail.com>
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
#ifndef WIDGETS_TOGGLESWITCH_H
#define WIDGETS_TOGGLESWITCH_H
#pragma once

#include <QtWidgets>

#include "BtColor.h"
#include "widgets/Animator.h"
#include "widgets/SelectionControl.h"

/**
 * \brief Switch widget following the Material design principles at https://material.io/components/switches
 */
class ToggleSwitch final : public SelectionControl {
   Q_OBJECT

public:
   struct AnimationSettings {
      QEasingCurve::Type easing;
      int duration;
      AnimationSettings() = default;
      AnimationSettings(QEasingCurve::Type easing, int duration) : easing{easing}, duration{duration} {
         return;
      }
   };

   struct ToggleSwitchStyle {
      int               height               = 36;
      QMargins          indicatorMargin      = QMargins(8, 8, 8, 8);
      BtColor           thumbOnColor         = BtColor("#00bcd4", 1);    // cyan500
      BtColor           trackOnColor         = BtColor("#00bcd4", 0.5);  // cyan500
      BtColor           thumbOffColor        = BtColor("#fafafa", 1);    // gray50
      BtColor           trackOffColor        = BtColor("#000000", 0.38); // black
      BtColor           thumbDisabledColor   = BtColor("#bdbdbd", 1);    // gray400
      BtColor           trackDisabledColor   = BtColor("#000000", 0.12); // black
      QColor            textColor            = Qt::black;
      double            disabledTextOpacity  = 0.26;
      AnimationSettings thumbBrushAnimation  = AnimationSettings(QEasingCurve::Type::Linear, 150);
      AnimationSettings trackBrushAnimation  = AnimationSettings(QEasingCurve::Type::Linear, 150);
      AnimationSettings thumbPosAnimation    = AnimationSettings(QEasingCurve::Type::InOutQuad, 150);
   };

   explicit ToggleSwitch(QWidget * parent = nullptr);
   ToggleSwitch(QString const & text, QWidget * parent = nullptr);
   ToggleSwitch(QString const & text, QBrush const & brush, QWidget * parent = nullptr);
   ~ToggleSwitch() override;

   QSize sizeHint() const override final;

protected:
   void paintEvent(QPaintEvent *) override final;
   void resizeEvent(QResizeEvent *) override final;
   void toggle(Qt::CheckState) override final;

   void init();
   QRect indicatorRect();
   QRect textRect();

private:
   ToggleSwitchStyle style;
   QPixmap shadowPixmap;
   QPointer<Animator> thumbBrushAnimation;
   QPointer<Animator> trackBrushAnimation;
   QPointer<Animator> thumbPosAnimation;
};

#endif
