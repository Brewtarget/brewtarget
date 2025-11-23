/*======================================================================================================================
 * widgets/BtLineEditCurrency.h is part of Brewtarget, and is copyright the following authors 2025:
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
 =====================================================================================================================*/
#ifndef WIDGETS_BTLINEEDITCURRENCY_H
#define WIDGETS_BTLINEEDITCURRENCY_H
#pragma once

#include <memory> // For PImpl

#include <QLineEdit>

#include "measurement/CurrencyAmount.h"
#include "utils/NoCopy.h"
#include "utils/TypeInfo.h"

/**
 * \brief Extends \c QLineEdit to handle \c CurrencyAmount.
 *
 *        Monetary amounts are sufficiently different from physical quantity measures that they merit their own edit
 *        class rather than adding more complexity to \c SmartLineEdit etc.
 */
class BtLineEditCurrency : public QLineEdit {
   Q_OBJECT

public:
   BtLineEditCurrency(QWidget* parent = nullptr);
   virtual ~BtLineEditCurrency();

   NO_COPY_DECLARATIONS(BtLineEditCurrency)

   /**
    * \brief Initialise a \c BtLineEditCurrency
    */
   void init(char const * const   editorName,
             char const * const   lineEditName,
             char const * const   lineEditFqName,
             TypeInfo     const & typeInfo);

   /**
    *
    */
   [[nodiscard]] bool isOptional() const;

   /**
    * \brief Set value of a combo box from a non-optional bool
    */
   void setValue(CurrencyAmount const & value);

   /**
    * \brief Set value of a combo box from an optional bool
    */
   void setValue(std::optional<CurrencyAmount> const & value);

   void setNull();

   void setDefault();

   /**
    * \brief Similar to \c SmartField::setFromVariant
    */
   void setFromVariant(QVariant const & value);

   /**
    * \brief Get value of a combo box for a non-optional CurrencyAmount
    */
   [[nodiscard]] CurrencyAmount getNonOptValue() const;

   /**
    * \brief Get value of a combo box for an optional CurrencyAmount
    */
   [[nodiscard]] std::optional<CurrencyAmount> getOptValue() const;

   /**
    * \brief Similar to \c SmartField::getAsVariant
    */
   [[nodiscard]] QVariant getAsVariant() const;

public slots:
   /**
    * \brief This slot receives the \c QLineEdit::editingFinished signal
    */
   void onLineChanged();

signals:
   /**
    * \brief Where we want "instant updates", this signal should be picked up by the editor or widget object using this
    *        input field so it can read the changed value and update the underlying data model.
    *
    *        Where we want to defer updating the underlying data model until the user clicks "Save" etc, then this
    *        signal will typically be ignored.
    */
   void textModified();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

};


#endif
