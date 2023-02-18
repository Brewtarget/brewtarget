/*
 * BtDigitWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * = Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "BtDigitWidget.h"

#include <iostream>

#include <QDebug>
#include <QFrame>
#include <QSettings>

#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "PersistentSettings.h"

// This private implementation class holds all private non-virtual members of BtDigitWidget
class BtDigitWidget::impl {
public:
   /**
    * Constructor
    */
   impl(BtDigitWidget & self) :
      self{self},
      m_rgblow{0x0000d0},
      m_rgbgood{0x008000},
      m_rgbhigh{0xd00000},
      m_lowLim{0.0},
      m_highLim{1.0},
      m_styleSheet{QString("QLabel { font-weight: bold; color: #%1 }")},
      m_constantColor{false},
      m_lastNum{1.5},
      m_lastPrec{3},
      m_low_msg{BtDigitWidget::tr("Too low for style.")},
      m_good_msg{BtDigitWidget::tr("In range for style.")},
      m_high_msg{BtDigitWidget::tr("Too high for style.")} {
      this->self.setStyleSheet(m_styleSheet.arg(0,6,16,QChar('0')));
      this->self.setFrameStyle(QFrame::Box);
      this->self.setFrameShadow(QFrame::Sunken);
      return;
   }


   /**
    * Destructor
    */
   ~impl() = default;

   void setTextStyleAndToolTip(QString str) {
      QString style{this->m_styleSheet};
      if ((!this->m_constantColor && (this->m_lastNum < this->m_lowLim)) ||
         (this->m_constantColor && this->m_color == LOW)) {
         style = this->m_styleSheet.arg(this->m_rgblow, 6, 16, QChar('0'));
         self.setToolTip(this->m_constantColor ? "" : this->m_low_msg);
      } else if ((!this->m_constantColor && (this->m_lastNum <= this->m_highLim)) ||
               (this->m_constantColor && this->m_color == GOOD)) {
         style = this->m_styleSheet.arg(this->m_rgbgood, 6, 16, QChar('0'));
         self.setToolTip(this->m_constantColor ? "" : this->m_good_msg);
      } else {
         if (this->m_constantColor && this->m_color == BLACK) {
            style = this->m_styleSheet.arg(0, 6, 16, QChar('0'));
         } else {
            style = this->m_styleSheet.arg(this->m_rgbhigh, 6, 16, QChar('0'));
            self.setToolTip(this->m_high_msg);
         }
      }

      this->self.setStyleSheet(style);
      this->self.QLabel::setText(str);
      return;
   }

   void adjustColors() {
      this->setTextStyleAndToolTip(self.displayAmount(this->m_lastNum, this->m_lastPrec));

      return;
   }

   // Member variables for impl
   BtDigitWidget & self;

   unsigned int m_rgblow;
   unsigned int m_rgbgood;
   unsigned int m_rgbhigh;
   double m_lowLim;
   double m_highLim;
   QString m_styleSheet;
   bool m_constantColor;
   ColorType m_color;
   double m_lastNum;
   int m_lastPrec;

   QString m_low_msg;
   QString m_good_msg;
   QString m_high_msg;

};

BtDigitWidget::BtDigitWidget(QWidget *parent,
                             BtFieldType fieldType,
                             Measurement::Unit const * units) :
   QLabel(parent),
   UiAmountWithUnits(parent, fieldType, units),
   pimpl{std::make_unique<impl>(*this)} {
   return;
}

BtDigitWidget::~BtDigitWidget() = default;

QString BtDigitWidget::getWidgetText() const {
   return this->text();
}

void BtDigitWidget::setWidgetText(QString text) {
   this->QLabel::setText(text);
   return;
}

void BtDigitWidget::display(QString str) {
   static bool converted;

   this->pimpl->m_lastNum = Localization::toDouble(str, &converted);
   this->pimpl->m_lastPrec = str.length() - str.lastIndexOf(Localization::getLocale().decimalPoint()) - 1;
   if (converted) {
      this->display(this->pimpl->m_lastNum, this->pimpl->m_lastPrec);
   } else {
      qWarning() << Q_FUNC_INFO << "Could not convert" << str << "to double";
      QLabel::setText("-");
   }
   return;
}

void BtDigitWidget::display(double num, int prec) {
   this->pimpl->m_lastNum = num;
   this->pimpl->m_lastPrec = prec;

   this->pimpl->setTextStyleAndToolTip(QString("%L1").arg(num,0,'f',prec));
   return;
}

void BtDigitWidget::setLowLim(double num) {
   if (num < this->pimpl->m_highLim) {
      this->pimpl->m_lowLim = num;
   }
   this->display(this->pimpl->m_lastNum, this->pimpl->m_lastPrec);
   return;
}

void BtDigitWidget::setHighLim(double num) {
   if (num > this->pimpl->m_lowLim) {
      this->pimpl->m_highLim = num;
   }
   this->display(this->pimpl->m_lastNum, this->pimpl->m_lastPrec);
   return;
}

void BtDigitWidget::setConstantColor(ColorType c) {
   this->pimpl->m_constantColor = (c == LOW || c == GOOD || c == HIGH || c == BLACK );
   this->pimpl->m_color = c;
   this->update(); // repaint.
   return;
}

void BtDigitWidget::setLimits(double low, double high) {
   if (low <  high) {
      this->pimpl->m_lowLim = low;
      this->pimpl->m_highLim = high;
   }
   this->pimpl->adjustColors();
   this->update(); // repaint.
   return;
}

void BtDigitWidget::setLowMsg( QString msg ) { this->pimpl->m_low_msg  = msg; update();}
void BtDigitWidget::setGoodMsg(QString msg ) { this->pimpl->m_good_msg = msg; update();}
void BtDigitWidget::setHighMsg(QString msg ) { this->pimpl->m_high_msg = msg; update();}

void BtDigitWidget::setMessages( QStringList msgs ) {
   if ( msgs.size() != 3 ) {
      qWarning() << Q_FUNC_INFO << "Wrong number of messages";
      return;
   }
   this->pimpl->m_low_msg = msgs[0];
   this->pimpl->m_good_msg = msgs[1];
   this->pimpl->m_high_msg = msgs[2];

   this->pimpl->adjustColors();
   return;
}

void BtDigitWidget::displayChanged(PreviousScaleInfo previousScaleInfo) {
   this->textOrUnitsChanged(previousScaleInfo);

   return;
}

void BtDigitWidget::setText(QString amount, int precision) {
   double amt;
   bool ok = false;

   this->setConfigSection("");
   auto const myFieldType = this->getFieldType();
   if (std::holds_alternative<NonPhysicalQuantity>(myFieldType) &&
       NonPhysicalQuantity::String == std::get<NonPhysicalQuantity>(myFieldType)) {
      QLabel::setText(amount);
   } else {
      amt = Localization::toDouble(amount, &ok);
      if ( !ok ) {
         qWarning() << QString("%1 could not convert %2 (%3:%4) to double")
               .arg(Q_FUNC_INFO)
               .arg(amount)
               .arg(this->configSection)
               .arg(this->editField);
      }
      this->pimpl->m_lastNum = amt;
      this->pimpl->m_lastPrec = precision;
      QLabel::setText(displayAmount(amt, precision));
   }
   return;
}

void BtDigitWidget::setText(double amount, int precision) {
   this->pimpl->m_lastNum = amount;
   this->pimpl->m_lastPrec = precision;
   this->setConfigSection("");
   QLabel::setText( displayAmount(amount,precision) );
   return;
}

BtMassDigit::BtMassDigit(QWidget* parent) :
   BtDigitWidget{parent, Measurement::PhysicalQuantity::Mass, &Measurement::Units::kilograms} {
   return;
}

BtGenericDigit::BtGenericDigit(QWidget* parent) :
   BtDigitWidget{parent, NonPhysicalQuantity::Count, nullptr} {
   return;
}
