/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartDigitWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "widgets/SmartDigitWidget.h"

#include <iostream>

#include <QDebug>
#include <QFrame>
#include <QSettings>

#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "PersistentSettings.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_SmartDigitWidget.cpp"
#endif

// This private implementation class holds all private non-virtual members of SmartDigitWidget
class SmartDigitWidget::impl {
public:
   /**
    * Constructor
    */
   impl(SmartDigitWidget & self) :
      m_self         {self},
      m_rgblow       {0x0000d0},
      m_rgbgood      {0x008000},
      m_rgbhigh      {0xd00000},
      m_lowLim       {0.0},
      m_highLim      {1.0},
      m_styleSheet   {QString("QLabel { font-weight: bold; color: #%1 }")},
      m_constantColor{false},
      m_low_msg      {SmartDigitWidget::tr("Too low for style.")},
      m_good_msg     {SmartDigitWidget::tr("In range for style.")},
      m_high_msg     {SmartDigitWidget::tr("Too high for style.")} {
      this->m_self.setStyleSheet(m_styleSheet.arg(0,6,16,QChar('0')));
      this->m_self.setFrameStyle(QFrame::Box);
      this->m_self.setFrameShadow(QFrame::Sunken);
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   void updateColors() {
      // If we don't hold NonPhysicalQuantity, then we need to ensure we're using canonical units to do the limit
      // comparisons.
      double const displayedValueAsCanonical{
         std::holds_alternative<NonPhysicalQuantity>(*this->m_self.getTypeInfo().fieldType) ?
            this->m_self.getNonOptValue<double>() : this->m_self.getNonOptCanonicalQty()
      };

      QString style{this->m_styleSheet};
      if ((!this->m_constantColor && (displayedValueAsCanonical < this->m_lowLim)) ||
          (this->m_constantColor && this->m_color == SmartDigitWidget::ColorType::Low)) {
         style = this->m_styleSheet.arg(this->m_rgblow, 6, 16, QChar('0'));
         m_self.setToolTip(this->m_constantColor ? "" : this->m_low_msg);
      } else if ((!this->m_constantColor && (displayedValueAsCanonical <= this->m_highLim)) ||
                 (this->m_constantColor && this->m_color == SmartDigitWidget::ColorType::Good)) {
         style = this->m_styleSheet.arg(this->m_rgbgood, 6, 16, QChar('0'));
         m_self.setToolTip(this->m_constantColor ? "" : this->m_good_msg);
      } else {
         if (this->m_constantColor && this->m_color == SmartDigitWidget::ColorType::Black) {
            style = this->m_styleSheet.arg(0, 6, 16, QChar('0'));
         } else {
            style = this->m_styleSheet.arg(this->m_rgbhigh, 6, 16, QChar('0'));
            m_self.setToolTip(this->m_high_msg);
         }
      }

      this->m_self.setStyleSheet(style);
      return;
   }

   // Member variables for impl
   SmartDigitWidget &          m_self;
   unsigned int                m_rgblow;
   unsigned int                m_rgbgood;
   unsigned int                m_rgbhigh;
   double                      m_lowLim;
   double                      m_highLim;
   QString                     m_styleSheet;
   bool                        m_constantColor;
   SmartDigitWidget::ColorType m_color;
   QString                     m_low_msg;
   QString                     m_good_msg;
   QString                     m_high_msg;
};

SmartDigitWidget::SmartDigitWidget(QWidget *parent) :
   SmartValueDisplay(parent),
   pimpl{std::make_unique<impl>(*this)} {
   return;
}

SmartDigitWidget::~SmartDigitWidget() = default;

void SmartDigitWidget::setRawText(QString const & text) {
   this->SmartValueDisplay::setRawText(text);
   this->pimpl->updateColors();
   return;
}

void SmartDigitWidget::setLowLim(double num) {
   if (num < this->pimpl->m_highLim) {
      this->pimpl->m_lowLim = num;
   }
   this->pimpl->updateColors();
   return;
}

void SmartDigitWidget::setHighLim(double num) {
   if (num > this->pimpl->m_lowLim) {
      this->pimpl->m_highLim = num;
   }
   this->pimpl->updateColors();
   return;
}

void SmartDigitWidget::setConstantColor(ColorType c) {
   this->pimpl->m_constantColor = (c != SmartDigitWidget::ColorType::None);
   this->pimpl->m_color = c;
   this->update(); // repaint.
   return;
}

void SmartDigitWidget::setLimits(double low, double high) {
   if (low <  high) {
      this->pimpl->m_lowLim = low;
      this->pimpl->m_highLim = high;
   }
   this->pimpl->updateColors();
   this->update(); // repaint.
   return;
}

void SmartDigitWidget::setLowMsg (QString msg) { this->pimpl->m_low_msg  = msg; this->update(); return; }
void SmartDigitWidget::setGoodMsg(QString msg) { this->pimpl->m_good_msg = msg; this->update(); return; }
void SmartDigitWidget::setHighMsg(QString msg) { this->pimpl->m_high_msg = msg; this->update(); return; }

void SmartDigitWidget::setMessages(QString lowMsg, QString goodMsg, QString highMsg) {
   this->pimpl->m_low_msg  = lowMsg ;
   this->pimpl->m_good_msg = goodMsg;
   this->pimpl->m_high_msg = highMsg;
   this->pimpl->updateColors();
   return;
}
