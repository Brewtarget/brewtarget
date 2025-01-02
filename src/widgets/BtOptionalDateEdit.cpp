/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtOptionalDateEdit.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "widgets/BtOptionalDateEdit.h"

#include <QCalendarWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionSpinBox>

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_BtOptionalDateEdit.cpp"

//
// This private implementation class holds all private non-virtual members of BtOptionalDateEdit
//
class BtOptionalDateEdit::impl {
public:
   /**
    * Constructor
    */
   impl(BtOptionalDateEdit & self) :
      m_self{self},
      m_pushButton_clear{new QPushButton(&m_self)},
      m_dateIsNull{true} {

      //
      // Set up the button that allows you to clear the date
      //
      this->m_pushButton_clear->setIcon(QIcon(":/images/clear.svg"));
      this->m_pushButton_clear->setFocusPolicy(Qt::NoFocus);
      //this->m_pushButton_clear->setFixedSize(17, this->m_pushButton_clear->sizeHint().height()-6);
      connect(this->m_pushButton_clear, &QAbstractButton::clicked, &this->m_self, &BtOptionalDateEdit::clearDate);
      this->m_pushButton_clear->setVisible(true);

      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   QLineEdit * getLineEdit() const {
      return this->m_self.findChild<QLineEdit *>("qt_spinbox_lineedit");
   }

   void updateUi() const {
      if (this->m_dateIsNull) {
         //
         // If we've made the date null, we want to clear the text in the box.  Unfortunately, clearing the text causes
         // the text box to send a QLineEdit::textChanged signal to QDate which will decide "" in the text box means
         // it should set the default date (2000-01-01).  That will then cause it to emit a QDateTimeEdit::dateChanged
         // signal, which we catch below in BtOptionalDateEdit::dateChanged.  If we're not careful, we then end up
         // setting the date back to valid and emitting BtOptionalDateEdit::optionalDateChanged with a date of
         // 2000-01-01 instead of std::nullopt.  (Ask me how I know.)
         //
         // So, we have to check for the text box being blank in BtOptionalDateEdit::dateChanged.
         //
         QLineEdit * lineEdit = this->getLineEdit();
         lineEdit->clear();
         // NB: Do not call lineEdit->repaint() here as (a) it is not necessary and (b) it will result in an endless
         //     loop!
      }

      this->m_pushButton_clear->setVisible(!this->m_dateIsNull);
      return;
   }

   bool dateIsNull() {
      auto date = this->m_self.QDateEdit::date();
      if (!date.isValid()) {
         this->m_dateIsNull = true;
      }
      return this->m_dateIsNull;
   }

   void setDateIsNull(bool const val) {
      this->m_dateIsNull = val;
      //
      // You might think it, if `val` is false, it would be a good idea to construct an invalid QDate here (eg using the
      // default constructor) and then set the date on the underlying control to that invalid date, eg:
      //    this->m_self.QDateEdit::setDate(QDate{});
      // However, when you call QDateEdit::setDate with an invalid date, it sets the stored date to the default value,
      // which is January 1, 2000.  It then, of course, emits a QDateTimeEdit::dateChanged signal, which we catch in
      // BtOptionalDateEdit::dateChanged and then, detecting the valid date of 2000-01-01, we call this function with
      // a true parameter, completing the circle and making it impossible to have a valid date.
      //
      // I wish I could say I worked this out a priori rather than the hard way, but I didn't.
      //
      this->updateUi();
      return;
   }

   //============================================== impl member variables ==============================================
   BtOptionalDateEdit & m_self;

   QPushButton * m_pushButton_clear;

private:
   bool m_dateIsNull;
};

BtOptionalDateEdit::BtOptionalDateEdit(QWidget * parent) :
   QDateEdit{parent},
   pimpl{std::make_unique<impl>(*this)} {

   //
   // Catch the base class signal about date changing so we can emit our own one
   //
   connect(this, &QDateTimeEdit::dateChanged, this, &BtOptionalDateEdit::dateChanged);

   this->update();

   return;
}

BtOptionalDateEdit::~BtOptionalDateEdit() = default;

std::optional<QDate> BtOptionalDateEdit::optionalDate() const {
   if (this->pimpl->dateIsNull()) {
      return std::nullopt;
   }

   return this->QDateEdit::date();
}

void BtOptionalDateEdit::setOptionalDate(std::optional<QDate> const val) {
   this->pimpl->setDateIsNull(!val);
   if (val) {
      this->QDateEdit::setDate(*val);
   }
   emit this->optionalDateChanged(this->optionalDate());
   return;
}


QSize BtOptionalDateEdit::sizeHint() const {
   QSize const baseClassSize = QDateEdit::sizeHint();
   return QSize(baseClassSize.width() + this->pimpl->m_pushButton_clear->width() + 3, baseClassSize.height());
}

QSize BtOptionalDateEdit::minimumSizeHint() const {
   QSize const baseClassSize = QDateEdit::minimumSizeHint();
   return QSize(baseClassSize.width() + this->pimpl->m_pushButton_clear->width() + 3, baseClassSize.height());
}

void BtOptionalDateEdit::showEvent(QShowEvent *event) {
   QDateEdit::showEvent(event);
   // If the date is null then we want to clear out whatever the base class wrote in the text box.  If it isn't null
   // then we want to show the "clear" button.
   this->pimpl->updateUi();
   return;
}

void BtOptionalDateEdit::paintEvent(QPaintEvent *event) {
   QDateEdit::paintEvent(event);
   // Logic from showEvent above applies here too.
   this->pimpl->updateUi();
   return;
}

void BtOptionalDateEdit::resizeEvent(QResizeEvent *event) {
   QStyleOptionSpinBox spinBoxStyle;
   this->initStyleOption(&spinBoxStyle);
   spinBoxStyle.subControls = QStyle::SC_SpinBoxUp;

   int xPos = this->style()->subControlRect(QStyle::CC_SpinBox,
                                            &spinBoxStyle,
                                            QStyle::SC_SpinBoxUp,
                                            this).left() - this->pimpl->m_pushButton_clear->width() - 3;
   this->pimpl->m_pushButton_clear->move(xPos, (this->height() - this->pimpl->m_pushButton_clear->height()) / 2);
   QDateEdit::resizeEvent(event);
   // Logic from showEvent above applies here too.
   this->pimpl->updateUi();
   return;
}

void BtOptionalDateEdit::keyPressEvent(QKeyEvent *event) {
   if (this->pimpl->dateIsNull() && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
      this->setOptionalDate(QDate::currentDate());
   } else if (event->key() == Qt::Key_Backspace ||
              event->key() == Qt::Key_Delete) {
      // If the whole date was selected then delete/backspace means delete the date
      QLineEdit *lineEdit = this->pimpl->getLineEdit();
      if (lineEdit->selectedText() == lineEdit->text()) {
         // Default constructed QDate is a "null" aka invalid date
         this->setOptionalDate(std::nullopt);
         event->accept();
         return;
      }
   }

   QDateEdit::keyPressEvent(event);
   return;
}

void BtOptionalDateEdit::mousePressEvent(QMouseEvent *event) {
   bool const dateIsNull = this->pimpl->dateIsNull();
   QDateEdit::mousePressEvent(event);
   if (dateIsNull && calendarWidget()->isVisible()) {
      this->setOptionalDate(QDate::currentDate());
   }
   return;
}

bool BtOptionalDateEdit::focusNextPrevChild(bool next) {
   if (this->pimpl->dateIsNull()) {
      return QAbstractSpinBox::focusNextPrevChild(next);
   }
   return QDateEdit::focusNextPrevChild(next);
}

QValidator::State BtOptionalDateEdit::validate(QString & input, int & pos) const {
   if (this->pimpl->dateIsNull()) {
      return QValidator::Acceptable;
   }
   return QDateEdit::validate(input, pos);
}

void BtOptionalDateEdit::clearDate() {
   this->pimpl->setDateIsNull(true);
   emit this->optionalDateChanged(this->optionalDate());
   return;
}

void BtOptionalDateEdit::dateChanged([[maybe_unused]] QDate date) {
   //
   // Per comment above in BtOptionalDateEdit::impl::updateUi, if we receive a date change signal and the text in the
   // QDateEdit's QLineEdit is empty, it means that date is null, and we should ignore the "default" date (2000-01-01)
   // that QDateEdit will be sending us here (and not propagate the signal).
   //
   QLineEdit * lineEdit = this->pimpl->getLineEdit();
   if (!lineEdit->text().isEmpty()) {
      //
      // I don't _think_ QDateEdit will ever send us an invalid date
      //
      this->pimpl->setDateIsNull(!date.isValid());
      emit this->optionalDateChanged(this->optionalDate());
   }

   return;
}
