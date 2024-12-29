/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtComboBoxEnum.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef WIDGETS_BTCOMBOBOXENUM_H
#define WIDGETS_BTCOMBOBOXENUM_H
#pragma once

#include <memory> // For PImpl
#include <optional>

#include <QComboBox>

#include "utils/EnumStringMapping.h"
#include "utils/TypeLookup.h"

class SmartLineEdit;

/**
 * \class BtComboBoxEnum extends \c QComboBox to handle strongly-typed enums more directly
 *
 *        Note that this class cannot be templated, as it will confuse the Qt Meta Object Compiler (MOC) but it is OK
 *        for it to have templated member functions.
 */
class BtComboBoxEnum : public QComboBox {
Q_OBJECT

public:
   BtComboBoxEnum(QWidget * parent);
   virtual ~BtComboBoxEnum();

   /**
    * \brief Post-construction initialisation.  Usually called via \c BT_COMBO_BOX_INIT macro
    *
    *        According to https://bugreports.qt.io/browse/QTBUG-50823 it is never going to be possible to specify the
    *        data (as opposed to display text) for a combo box via the .ui file.  So we have to do it in code instead.
    *        We could use the raw enum values as the data, but it would be a bit painful to debug if we ever had to, so
    *        for small extra effort we use the same serialisation strings that we use for BeerJSON and the DB.
    *
    * \param editorName
    * \param comboBoxName
    * \param comboBoxFqName     Fully qualified name.  Usually a combination of \c editorName and \c comboBoxName
    * \param nameMapping        String serialisations for the enum values
    * \param displayNameMapping Localised displayable names for the enum values
    * \param typeInfo           Mainly used to determine whether this is an optional enum
    * \param restrictTo         If specified, then, instead of offering all the enum values in \c nameMapping /
    *                           \c displayNameMapping, the combo box should show only the values in the supplied list.
    *                           This is useful eg for offering a choice of \c Measurement::PhysicalQuantity values for a
    *                           \c Measurement::ChoiceOfPhysicalQuantity value.
    * \param controlledField    If specified, then this is the field whose \c Measurement::PhysicalQuantity is
    *                           shown and controlled by this combo box.  (Note that we do \b not use this in table
    *                           models.
    */
   void init(char const * const editorName,
             char const * const comboBoxName,
             char const * const comboBoxFqName,
             EnumStringMapping const & nameMapping,
             EnumStringMapping const & displayNameMapping,
             TypeInfo          const & typeInfo,
             std::vector<int>  const * restrictTo = nullptr,
             SmartLineEdit *           controlledField = nullptr);

   /**
    * \brief For the case where we have a controlledField (see last parameter to \c init above), we want to be able to
    *        set the value of the combo box when the controlled field is first set (from reading the object being
    *        edited).
    *
    *        We could do this with additional signals etc but, for now, I think it's simpler just to have this function
    *        that the editor class calls just after initial population of the controlled field.
    */
   void autoSetFromControlledField();

   /**
    *
    */
   [[nodiscard]] bool isOptional() const;

   /**
    * \brief Set value of a combo box from an optional enum val
    *
    *        It looks a bit funky disabling this specialisation for a T that is optional, but the point is that we don't
    *        want the compiler to ever create a \c std::optional<std::optional<T>> type.  (Eg, we don't want to write
    *        `\c setAmount<std::optional<T>>(\c std::nullopt)` when we mean
    *        `\c setAmount<T>(\c std::optional<T>{std::nullopt})`.
    *
    * \param value
    */
   template<typename EE, typename = std::enable_if_t<is_non_optional<EE>::value> > void setValue(std::optional<EE> value) {
      Q_ASSERT(this->isOptional());
      if (!value) {
         this->setNull();
      } else {
         this->setValue(static_cast<int>(*value));
      }
      return;
   }

   /**
    * \brief Set value of a combo box from a non-optional enum val
    *
    * \param value
    */
   template<typename EE, typename = std::enable_if_t<is_non_optional<EE>::value> > void setValue(EE value) {
      Q_ASSERT(!this->isOptional());
      this->setValue(static_cast<int>(value));
      return;
   }

   /**
    * \brief Get value of a combo box for an optional enum val
    */
   template<typename EE> [[nodiscard]] std::optional<EE> getOptValue() const {
      Q_ASSERT(this->isOptional());
      auto value = this->getOptIntValue();
      if (!value) {
         return std::nullopt;
      }
      return static_cast<EE>(*value);
   }

   /**
    * \brief Get value of a combo box for a non-optional enum val
    */
   template<typename EE> [[nodiscard]] EE getNonOptValue() const {
      Q_ASSERT(!this->isOptional());
      return static_cast<EE>(this->getNonOptIntValue());
   }

   /**
    * \brief Get value of a combo box
    */
   [[nodiscard]] QVariant getAsVariant() const;

   /**
    * \brief Called from templated version of \c setValue, but also used in generic code (eg \c ItemDelegate) where we
    *        cannot use strongly-typed enums.
    */
   void setNull();

   /**
    * \brief Called from templated version of \c setValue, but also used in generic code (eg \c ItemDelegate) where we
    *        cannot use strongly-typed enums.
    */
   void setValue(int value);

   void setDefault();

   /**
    * \brief Similar to \c SmartField::setFromVariant
    */
   void setFromVariant(QVariant const & value);

   [[nodiscard]] std::optional<int> getOptIntValue() const;

   [[nodiscard]] int getNonOptIntValue() const;

public slots:
   void onIndexChanged(int const index);

private:

   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a label object
   BtComboBoxEnum(BtComboBoxEnum const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a label object
   BtComboBoxEnum& operator=(BtComboBoxEnum const&) = delete;
   //! No move constructor
   BtComboBoxEnum(BtComboBoxEnum &&) = delete;
   //! No move assignment
   BtComboBoxEnum & operator=(BtComboBoxEnum &&) = delete;
};

/**
 * \brief This macro saves a bit of copy-and-paste when invoking \c BtComboBoxEnum::Init.  Eg instead of writing:
 *
 *           this->comboBox_hopForm->init("HopEditor",
 *                                        "comboBox_hopForm",
 *                                        "HopEditor->comboBox_hopForm",
 *                                        Hop::formStringMapping,
 *                                        Hop::formDisplayNames,
 *                                        Hop::typeLookup.getType(PropertyNames::Hop::form)
 *
 *        you write:
 *
 *           BT_COMBO_BOX_INIT(HopEditor, comboBox_hopForm, Hop, form);
 *
 *        NOTE: We are more concise here than in \c SMART_FIELD_INIT and related macros because none of the combo boxes
 *              need to access inherited properties.  Eg, in \c HopEditor, all the properties for combo boxes are going
 *              to be \c PropertyNames::Hop::somethingOrOther, which is not always the case for other types of field.
 *
 *        Note that, as with other macros, string concatenation here uses the magic, that, for the compiler, there is no
 *        difference between writing a string literal as:
 *           "foobarhumbug"
 *        and writing it as:
 *           "foo" "bar" "humbug"
 */
#define BT_COMBO_BOX_INIT(editorClass, comboBoxName, modelClass, propertyName)                      \
   this->comboBoxName->init(#editorClass,                                                           \
                            #comboBoxName,                                                          \
                            #editorClass "->" #comboBoxName,                                        \
                            modelClass::propertyName##StringMapping,                                \
                            modelClass::propertyName##DisplayNames,                                 \
                            modelClass::typeLookup.getType(PropertyNames::modelClass::propertyName))

/**
 * \brief Alternate version of \c BT_COMBO_BOX_INIT for when we have Measurement::ChoiceOfPhysicalQuantity
 */
#define BT_COMBO_BOX_INIT_COPQ(editorClass, comboBoxName, modelClass, fqPropertyName, controlledField) \
   this->comboBoxName->init(#editorClass,                                                   \
                            #comboBoxName,                                                  \
                            #editorClass "->" #comboBoxName,                                \
                            Measurement::physicalQuantityStringMapping,                     \
                            Measurement::physicalQuantityDisplayNames,                      \
                            modelClass::typeLookup.getType(fqPropertyName),                 \
                            &Measurement::allPossibilitiesAsInt(modelClass::validMeasures), \
                            controlledField)

/**
 * \brief Alternate version of \c BT_COMBO_BOX_INIT for when the variable we are initialising is not a member variable
 *        (eg see FermentableItemDelegate::createEditor)
 */
#define BT_COMBO_BOX_INIT_NOMV(functionName, comboBoxName, modelClass, propertyName)          \
   comboBoxName->init(#functionName,                                                          \
                      #comboBoxName,                                                          \
                      #functionName "..." #comboBoxName,                                      \
                      modelClass::propertyName##StringMapping,                                \
                      modelClass::propertyName##DisplayNames,                                 \
                      modelClass::typeLookup.getType(PropertyNames::modelClass::propertyName))

#endif
