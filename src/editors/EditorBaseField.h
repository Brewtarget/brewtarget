/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/EditorBaseField.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef EDITORS_EDITORBASEFIELD_H
#define EDITORS_EDITORBASEFIELD_H
#pragma once

#include <variant>

#include "widgets/BtBoolComboBox.h"
#include "widgets/BtComboBox.h"

//
// This is only included from one place -- editors/EditorBase.h -- but I think it's a big enough block that there is
// benefit in having it as a separate file.
//

/**
 * \brief Most fields are written together.  However, some are marked 'Late' because they need to be written after
 *        the object is created.
 *
 *        Logically this belongs inside EditorBaseField, but that's templated, so it would get a bit hard to refer to if
 *        we put it there.
 */
enum class WhenToWriteField {
   Normal,
   Late,
   Never
};

/**
 * \brief Field info for a field of a subclass of \c EditorBase.
 *
 *        Note that we can't put this inside the \c EditorBase class declaration as we also want to use it there, and
 *        we'd get errors about "invalid use of incomplete type ‘class EditorBase<Derived, NE>’".
 *
 *        We template on both label and edit field types.  This is partly so we call the right overload of
 *        \c SmartAmounts::Init, partly because \c QLineEdit and \c QTextEdit don't have a useful common base class, and
 *        partly because we want to access member functions of \c SmartLineEdit that don't exist on \c QLineEdit or
 *        \c QTextEdit.
 *
 *        Note that the member functions are mostly \c const because they are not modifying this struct -- merely things
 *        referenced by the struct.
 */
template<class LabelType, class EditFieldType>
struct EditorBaseField {

   char const * labelName;
   LabelType * label;
   EditFieldType * editField;
   BtStringConst const & property;
   // This field isn't used for all values of EditFieldType, but we don't try to make it conditional for the same
   // reasons as EditorBase::m_liveEditItem below.
   std::optional<int> precision = std::nullopt;
   WhenToWriteField whenToWrite = WhenToWriteField::Normal;
   bool hasControlledField = false;

   /**
    * \brief Constructor for when we don't have a SmartLineEdit or similar
    *
    *        NB: Both \c precision and \c whenToWrite have defaults, but precision is the one that more often needs
    *        something other than default to be specified, so we put it first in the argument list.
    */
   EditorBaseField([[maybe_unused]] char const * const editorClass,
                   char const * const labelName,
                   [[maybe_unused]] char const * const labelFqName,
                   LabelType * label,
                   [[maybe_unused]] char const * const editFieldName,
                   [[maybe_unused]] char const * const editFieldFqName,
                   EditFieldType * editField,
                   BtStringConst const & property,
                   [[maybe_unused]] TypeInfo const & typeInfo,
                   std::optional<int> precision = std::nullopt,
                   WhenToWriteField whenToWrite = WhenToWriteField::Normal)
   requires (!std::same_as<EditFieldType, SmartLineEdit > &&
             !std::same_as<EditFieldType, BtComboBox    > &&
             !std::same_as<EditFieldType, BtBoolComboBox>) :
      labelName  {labelName  },
      label      {label      },
      editField  {editField  },
      property   {property   },
      precision  {precision  },
      whenToWrite{whenToWrite} {
      return;
   }

   //! Constructor for when we have a SmartLineEdit
   EditorBaseField(char const * const editorClass,
                   char const * const labelName,
                   char const * const labelFqName,
                   LabelType * label,
                   char const * const editFieldName,
                   char const * const editFieldFqName,
                   SmartLineEdit * editField,
                   BtStringConst const & property,
                   TypeInfo const & typeInfo,
                   std::optional<int> precision = std::nullopt,
                   WhenToWriteField whenToWrite = WhenToWriteField::Normal)
   requires (std::same_as<EditFieldType, SmartLineEdit>) :
      labelName  {labelName  },
      label      {label      },
      editField  {editField  },
      property   {property   },
      precision  {precision  },
      whenToWrite{whenToWrite} {
      SmartAmounts::Init(editorClass,
                         labelName,
                         labelFqName,
                         *label,
                         editFieldName,
                         editFieldFqName,
                         *editField,
                         typeInfo,
                         precision);
      return;
   }

   //! Constructor for when we have a BtComboBox
   EditorBaseField(char const * const editorClass,
                   char const * const labelName,
                   [[maybe_unused]] char const * const labelFqName,
                   LabelType * label,
                   char const * const editFieldName,
                   char const * const editFieldFqName,
                   BtComboBox * editField,
                   BtStringConst const & property,
                   TypeInfo const & typeInfo,
                   EnumStringMapping const & nameMapping,
                   EnumStringMapping const & displayNameMapping,
                   std::vector<int>  const * restrictTo = nullptr,
                   SmartLineEdit *           controlledField = nullptr,
                   WhenToWriteField whenToWrite = WhenToWriteField::Normal)
   requires (std::same_as<EditFieldType, BtComboBox>) :
      labelName  {labelName  },
      label      {label      },
      editField  {editField  },
      property   {property   },
      precision  {std::nullopt},
      whenToWrite{whenToWrite} {
      editField->init(editorClass,
                      editFieldName,
                      editFieldFqName,
                      nameMapping,
                      displayNameMapping,
                      typeInfo,
                      restrictTo,
                      controlledField);
      if (controlledField) {
         this->hasControlledField = true;
      }
      return;
   }

   //! Constructor for when we have a BtBoolComboBox
   EditorBaseField(char const * const editorClass,
                   char const * const labelName,
                   [[maybe_unused]] char const * const labelFqName,
                   LabelType * label,
                   char const * const editFieldName,
                   char const * const editFieldFqName,
                   BtBoolComboBox * editField,
                   BtStringConst const & property,
                   TypeInfo const & typeInfo,
                   QString const & unsetDisplay = QObject::tr("No"),
                   QString const & setDisplay   = QObject::tr("Yes"),
                   WhenToWriteField whenToWrite = WhenToWriteField::Normal)
   requires (std::same_as<EditFieldType, BtBoolComboBox>) :
      labelName  {labelName  },
      label      {label      },
      editField  {editField  },
      property   {property   },
      precision  {std::nullopt},
      whenToWrite{whenToWrite} {
      // We could use BT_BOOL_COMBO_BOX_INIT here, but we'd be repeating a bunch of work we already did in EDITOR_FIELD
      editField->init(editorClass,
                      editFieldName,
                      editFieldFqName,
                      unsetDisplay,
                      setDisplay,
                      typeInfo);
      return;
   }

   //
   // You might think that in these connectFieldChanged, it would suffice to use QObject * as the type of context, but
   // this gave an error about "invalid conversion from ‘QObject*’ to
   // ‘const QtPrivate::FunctionPointer<void (WaterEditor::*)()>::Object*’ {aka ‘const WaterEditor*’} [-fpermissive]".
   // Rather than fight this, we just add another template parameter.
   //

   //! Simple case - the field tells us editing finished via the \c editingFinished signal
   template <typename Derived, typename Functor>
   void connectFieldChanged(Derived * context, Functor functor) const
   requires (std::same_as<EditFieldType, QLineEdit>) {
      // We ignore the defaulted parameter on connect, and its return value, as we don't need them
      context->connect(this->editField, &EditFieldType::editingFinished, context, functor, Qt::AutoConnection);
      return;
   }

   //! I don't know \c QPlainTextEdit does not have an \c editingFinished signal
   template <typename Derived, typename Functor>
   void connectFieldChanged(Derived * context, Functor functor) const
   requires (std::same_as<EditFieldType, QTextEdit> ||
             std::same_as<EditFieldType, QPlainTextEdit>) {
      context->connect(this->editField, &EditFieldType::textChanged, context, functor, Qt::AutoConnection);
      return;
   }

   //! \c SmartLineEdit uses \c editingFinished itself, and subsequently emits \c textModified after text corrections
   template <typename Derived, typename Functor>
   void connectFieldChanged(Derived * context, Functor functor) const
   requires (std::same_as<EditFieldType, SmartLineEdit>) {
      context->connect(this->editField, &EditFieldType::textModified, context, functor, Qt::AutoConnection);
      return;
   }

   //! Combo boxes are slightly different
   template <typename Derived, typename Functor>
   void connectFieldChanged(Derived * context, Functor functor) const
   requires (std::same_as<EditFieldType, BtComboBox> ||
             std::same_as<EditFieldType, BtBoolComboBox>) {
      // QOverload is needed on next line because the signal currentIndexChanged is overloaded in QComboBox - see
      // https://doc.qt.io/qt-5/qcombobox.html#currentIndexChanged
      context->connect(this->editField, QOverload<int>::of(&QComboBox::currentIndexChanged), context, functor, Qt::AutoConnection);
      return;
   }

   QVariant getFieldValue() const requires (std::same_as<EditFieldType, QTextEdit     > ||
                                            std::same_as<EditFieldType, QPlainTextEdit>) {
      return this->editField->toPlainText();
   }

   QVariant getFieldValue() const requires (std::same_as<EditFieldType, QLineEdit>) {
      return this->editField->text();
   }

   QVariant getFieldValue() const requires (std::same_as<EditFieldType, SmartLineEdit > ||
                                            std::same_as<EditFieldType, BtComboBox    > ||
                                            std::same_as<EditFieldType, BtBoolComboBox>) {
      // Through the magic of templates, and naming conventions, one line suffices for all three types
      return this->editField->getAsVariant();
   }

   /**
    * \brief Set property on supplied object from edit field
    */
   void setPropertyFromEditField(QObject & object) const requires (std::same_as<EditFieldType, BtComboBox>) {
      //
      // The only "special case" we can't handle with template specialisation is where we have a combo-box that is
      // controlling the physical quantity for another field (eg whether an input field is mass or volume), there is
      // nothing to do here (because the Amount returned from the controlled field holds the units that we need to pass
      // to the object setter).
      //
      if (!this->hasControlledField) {
         object.setProperty(*property, this->getFieldValue());
      }
      return;
   }
   void setPropertyFromEditField(QObject & object) const requires (!std::same_as<EditFieldType, BtComboBox>) {
      object.setProperty(*property, this->getFieldValue());
      return;
   }

   void setEditFieldText(QString const & val) const requires (std::same_as<EditFieldType, QTextEdit     > ||
                                                              std::same_as<EditFieldType, QPlainTextEdit>) {
      this->editField->setPlainText(val);
      return;
   }

   void setEditFieldText(QString const & val) const requires (std::same_as<EditFieldType, QLineEdit    > ||
                                                              std::same_as<EditFieldType, SmartLineEdit>) {
      this->editField->setText(val);
      return;
   }

   void setEditField(QVariant const & val) const requires (std::same_as<EditFieldType, QTextEdit     > ||
                                                           std::same_as<EditFieldType, QPlainTextEdit> ||
                                                           std::same_as<EditFieldType, QLineEdit     >) {
      this->setEditFieldText(val.toString());
      return;
   }

   void setEditField(QVariant const & val) const requires (std::same_as<EditFieldType, SmartLineEdit > ||
                                                           std::same_as<EditFieldType, BtComboBox    > ||
                                                           std::same_as<EditFieldType, BtBoolComboBox>) {
      this->editField->setFromVariant(val);
      return;
   }

   //! This clears the field, or sets it to the default value
   void clearEditField()  const requires (std::same_as<EditFieldType, QTextEdit     > ||
                                          std::same_as<EditFieldType, QPlainTextEdit> ||
                                          std::same_as<EditFieldType, QLineEdit     >) {
      this->setEditFieldText("");
      return;
   }
   void clearEditField() const requires (std::same_as<EditFieldType, SmartLineEdit > ||
                                         std::same_as<EditFieldType, BtComboBox    > ||
                                         std::same_as<EditFieldType, BtBoolComboBox>) {
      this->editField->setDefault();
      return;
   }

   /**
    * \brief Set edit field from property on supplied object
    */
   void setEditFieldFromProperty(QObject & object) const requires (std::same_as<EditFieldType, BtComboBox>) {
      //
      // Similarly to setPropertyFromEditField, in the case of a combo-box that is controlling the physical quantity for
      // another field, we want to initialise from that controlled field.
      //
      if (this->hasControlledField) {
         this->editField->autoSetFromControlledField();
      } else {
         this->setEditField(object.property(*property));
      }
      return;
   }
   void setEditFieldFromProperty(QObject & object) const requires (!std::same_as<EditFieldType, BtComboBox>) {
      this->setEditField(object.property(*property));
      return;
   }

};

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, WhenToWriteField const & val) {
   switch (val) {
      case WhenToWriteField::Normal: stream << "WhenToWriteField::Normal"; break;
      case WhenToWriteField::Late  : stream << "WhenToWriteField::Late"  ; break;
      case WhenToWriteField::Never : stream << "WhenToWriteField::Never" ; break;
   }
   return stream;
}


using EditorBaseFieldVariant = std::variant<
   // Not all permutations are valid, hence why some are commented out
   EditorBaseField<QLabel, QLineEdit     >,
   EditorBaseField<QLabel, QTextEdit     >,
   EditorBaseField<QLabel, QPlainTextEdit>,
   EditorBaseField<QLabel, SmartLineEdit >,
   EditorBaseField<QLabel, BtComboBox    >,
   EditorBaseField<QLabel, BtBoolComboBox>,
   EditorBaseField<QWidget, QTextEdit     >, // This is for tabs such as tab_notes containing a single QTextEdit with no separate QLabel
   EditorBaseField<SmartLabel, QLineEdit     >,
//   EditorBaseField<SmartLabel, QTextEdit     >,
//   EditorBaseField<SmartLabel, QPlainTextEdit>,
   EditorBaseField<SmartLabel, SmartLineEdit >,
   EditorBaseField<SmartLabel, BtComboBox    >,
   EditorBaseField<SmartLabel, BtBoolComboBox>
>;

/**
 * \brief These macros are similar to SMART_FIELD_INIT, but allows us to pass the EditorBaseField constructor.
 *
 *        We assume that, where Foo is some subclass of NamedEntity, then the editor class for Foo is always called
 *        FooEditor.
 *
 *        Note that we can't just write decltype(*label) because (as explained at
 *        https://stackoverflow.com/questions/34231547/decltype-a-dereferenced-pointer-in-c), *label is actually a
 *        reference, and we can't have a member of EditorBaseField be a pointer to a reference.  Fortunately
 *        std::remove_pointer does what we want.
 *
 *        \c EDITOR_FIELD_NORM should be used for most fields
 *        \c EDITOR_FIELD_ENUM is for a combo box for an enum
 *        \c EDITOR_FIELD_COPQ is for a combo box that controls the physical quantity (eg mass/volume) of another field
 *
 *        To minimise errors, we try to keep the invocations of \c EDITOR_FIELD_NORM, \c EDITOR_FIELD_ENUM and
 *        \c EDITOR_FIELD_COPQ similar to each other -- within the limits of what we can do in macros.  This pushes us
 *        to using \c NamedEntity::name etc rather than \c PropertyNames::NamedEntity::name for the fourth parameter,
 *        because in the _ENUM version, we're also going to use this to get the StringMapping and DisplayNames lookups
 *        via the naming conventions we use for those.  (I did think about whether we should go beyond just a naming
 *        convention for those sets of look-ups, but it feels like it would be adding a reasonable amount of extra
 *        complexity for very small benefit.)
 *
 *        NOTE that we cannot completely check the first parameter at compile-time.  If you, eg, accidentally put
 *        \c Fermentation when you mean \c FermentationStep, the code will compile, but you will get an assert at
 *        run-time (when we try to look up a \c FermentationStep property type info on \c Fermentation).
 */
#define EDITOR_FIELD_BEGIN(modelClass, label, editField, property) \
   EditorBaseFieldVariant{ \
      EditorBaseField<std::remove_pointer<decltype(label)>::type, std::remove_pointer<decltype(editField)>::type>{\
         #modelClass "Editor", \
         #label, \
         #modelClass "Editor->" #label, \
         label, \
         #editField, \
         #modelClass "Editor->" #editField, \
         editField, \
         PropertyNames::property, \
         modelClass ::typeLookup.getType(PropertyNames::property)

#define EDITOR_FIELD_END(...) \
         __VA_OPT__(, __VA_ARGS__) \
      } \
   }

#define EDITOR_FIELD_NORM(modelClass, label, editField, property, ...) \
   EDITOR_FIELD_BEGIN(modelClass, label, editField, property) \
   EDITOR_FIELD_END(__VA_ARGS__)

#define EDITOR_FIELD_ENUM(modelClass, label, editField, property, ...) \
   EDITOR_FIELD_BEGIN(modelClass, label, editField, property), \
   property##StringMapping, \
   property##DisplayNames   \
   EDITOR_FIELD_END(__VA_ARGS__)

#define EDITOR_FIELD_COPQ(modelClass, label, editField, property, controlledField, ...) \
   EDITOR_FIELD_BEGIN(modelClass, label, editField, property), \
   Measurement::physicalQuantityStringMapping,                 \
   Measurement::physicalQuantityDisplayNames,                  \
   &Measurement::allPossibilitiesAsInt(modelClass::validMeasures), \
   controlledField \
   EDITOR_FIELD_END(__VA_ARGS__)

#endif
