/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtOptionalDateEdit.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef WIDGETS_BTOPTIONALDATEEDIT_H
#define WIDGETS_BTOPTIONALDATEEDIT_H
#pragma once

#include <memory> // For PImpl
#include <optional>

#include <QDateEdit>

#include "utils/NoCopy.h"

/**
 * \brief Replacement for QDateEdit that allows date to be blank/unset
 *
 *        In an ideal world, there would be a simple way to obtain most of the functionality of \c QDateEdit without
 *        inheriting from it, since (a) we want to change rather than merely extend the interface and (b) even where we
 *        keep the interface but modify behaviour, not all of the functions we are overriding are virtual.  However, so
 *        far, it seems there is not an easy way to do this that avoids duplicating lots of the implementation of
 *        \c QDateEdit, so we live with a slight inelegance in our class and just have to remember that
 *        \c BtOptionalDateEdit should not be used where a \c QDateEdit is expected.
 */
class BtOptionalDateEdit : public QDateEdit {
   Q_OBJECT

   Q_PROPERTY(std::optional<QDate> optionalDate READ optionalDate WRITE setOptionalDate)

public:
   BtOptionalDateEdit(QWidget * parent = nullptr);
   ~BtOptionalDateEdit();

   std::optional<QDate> optionalDate() const;
   [[nodiscard]] QVariant getAsVariant() const;

   void setOptionalDate(std::optional<QDate> const val);

   /**
    * \brief Similar to \c SmartField::setFromVariant
    */
   void setFromVariant(QVariant const & value);

   QSize sizeHint() const;
   QSize minimumSizeHint() const;

signals:
   //! \brief Use this instead of void QDateTimeEdit::dateChanged(QDate date)
   void optionalDateChanged(std::optional<QDate> const val);

protected:
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    bool focusNextPrevChild(bool next);
    QValidator::State validate(QString &input, int &pos) const;

private slots:
   void dateChanged(QDate date);
   void clearDate();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   // Insert all the usual boilerplate to prevent copy/assignment/move
   NO_COPY_DECLARATIONS(BtOptionalDateEdit)

   // Delete some of the functions that we would otherwise inherit from base classes
   QDate date() = delete;
   void setDate(QDate date) = delete;
   QDateTime dateTime() = delete;
   void setDateTime(QDateTime const & dateTime) = delete;

   std::optional<QDate> m_optionalDate;
};

#endif
