/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/EditorBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef EDITORS_EDITORBASE_H
#define EDITORS_EDITORBASE_H
#pragma once

#include <memory>
#include <variant>
#include <vector>

#include <QAbstractButton>
#include <QInputDialog>
#include <QString>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedEntity.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief Field info for a field of a subclass of \c EditorBase.
 *
 *        Note that we can't put this inside the \c EditorBase class declaration as we also want to use it there, and
 *        we'd get errors about "invalid use of incomplete type ‘class EditorBase<Derived, NE>’".
 */
struct EditorBaseField {
   /**
    * \brief Most fields are written together.  However, some are marked 'Late' because they need to be written after
    *        the object is created.
    */
   enum class WhenToWrite {
     Normal,
     Late
   };

   char const * labelName;
   std::variant<QLabel *, SmartLabel *> label;
   // We need to know what type the field is, partly because QLineEdit and QTextEdit don't have a useful common base
   // class, and partly because we want to access member functions of SmartLineEdit that don't exist on QLineEdit or
   // QTextEdit.
   std::variant<QLineEdit *, QTextEdit *, SmartLineEdit *> editField;
   BtStringConst const & property;
   // Both the next two fields have defaults, but precision is the one that more often needs something other than
   // default to be specified, so we put it first.
   std::optional<int> precision = std::nullopt;
   EditorBaseField::WhenToWrite whenToWrite = WhenToWrite::Normal;

   //! Constructor for when we don't have a SmartLineEdit
   template<class LabelType, class EditFieldType>
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
                   EditorBaseField::WhenToWrite whenToWrite = WhenToWrite::Normal) :
      labelName  {labelName  },
      label      {label      },
      editField  {editField  },
      property   {property   },
      precision  {precision  },
      whenToWrite{whenToWrite} {
      return;
   }

   //! Constructor for when we have a SmartLineEdit
   template<class LabelType>
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
                   EditorBaseField::WhenToWrite whenToWrite = WhenToWrite::Normal) :
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

};

/**
 * \brief This macro is similar to SMART_FIELD_INIT, but allows us to pass the EditorBaseField constructor.
 *
 *        We assume that, where Foo is some subclass of NamedEntity, then the editor class for Foo is always called
 *        FooEditor.
 */
#define EDITOR_FIELD(modelClass, label, editField, property, ...) \
   EditorBaseField{\
      #modelClass "Editor", \
      #label, \
      #modelClass "Editor->" #label, \
      label, \
      #editField, \
      #modelClass "Editor->" #editField, \
      editField, \
      property, \
      modelClass ::typeLookup.getType(property) \
      __VA_OPT__(, __VA_ARGS__) \
   }

/**
 * \class EditorBase
 *
 * \brief As in other places where we want to use class templating, we have to use multiple inheritance because we can't
 *        template a class that ultimately inherits from \c QObject.  However, with the magic of the Curiously Recurring
 *        Template Pattern, we can get past some of the limitations and avoid too much copy-and-paste code duplication.
 *        Eg:
 *
 *              QObject     Ui::hopEditor
 *                   \           |
 *                   ...         |
 *                     \         |     EditorBase<Hop, HopEditor>
 *                     QDialog   |    /
 *                           \   |   /
 *                            \  |  /
 *                           HopEditor
 *
 *        Besides inheriting from \c QDialog, the derived class (eg \c HopEditor in the example above) needs to
 *        implement the following trivial slots:
 *
 *          - \c void \c save() public slot, which should call \c EditorBase::doSave
 *          - \c void \c clearAndClose() public slot, which should call \c EditorBase::doClearAndClose
 *          - \c void \c changed(\c QMetaProperty, \c QVariant) public slot, which should call \c EditorBase::doChanged
 *          - \c void \c clickedNew() public slot, which should call \c EditorBase::newEditItem
 *
 *        The code for the definition of these slot functions (which is "the same" for all editors) can be inserted in
 *        the implementation file using the EDITOR_COMMON_CODE macro.  Eg, in HopEditor, we need:
 *
 *          (HopEditor)
 *
 *        Note that we cannot do the equivalent for the header file declarations because the Qt MOC does not expand
 *        non-Qt macros.
 *
 *        The derived class also needs to implement the following substantive member functions that \c EditorBase will
 *        call:
 *          - \c void \c writeFieldsToEditItem -- Writes most fields from the editor GUI fields into the object being
 *                                                edited
 *          - \c void \c writeLateFieldsToEditItem -- Writes any fields that must wait until the object definitely
 *                                                    exists in the DB
 *          - \c void \c readFieldsFromEditItem -- (Re)read one or all fields from the object into the relevant GUI
 *                                                 field(s).
 *
 *        Finally, derived class needs to have the following QPushButton members (typically defined in the .ui file):
 *           pushButton_new, pushButton_save, pushButton_cancel
 */
template<class Derived> class EditorPhantom;
template<class Derived, class NE>
class EditorBase : public CuriouslyRecurringTemplateBase<EditorPhantom, Derived> {
public:

   /**
    * \brief Constructor
    *
    *        Note that we cannot initialise this->m_fields here, as the parameters themselves won't get constructed
    *        until Derived calls setupUi().
    */
   EditorBase() :
      m_fields{nullptr},
      m_editItem{nullptr} {
      return;
   }
   virtual ~EditorBase() = default;

   /**
    * \brief Derived should call this after calling setupUi
    */
   void postSetupUiInit(std::initializer_list<EditorBaseField> fields) {
      this->m_fields = std::make_unique<std::vector<EditorBaseField>>(fields);
      this->connectSignalsAndSlots();
      return;
   }

   /**
    * \brief Call this at the end of derived class's constructor (in particular, after the call to \c setupUi).
    *
    *        NOTE: This relies on derived classes having \c public, not the usual \c private, inheritance from the Ui
    *              base class (eg \c Ui::hopEditor in the example above), as otherwise \c pushButton_new etc would be
    *              inaccessible from this function.
    */
   void connectSignalsAndSlots() {
      // Standard editor slot connections
      this->derived().connect(this->derived().pushButton_new   , &QAbstractButton::clicked, &this->derived(), &Derived::clickedNew   );
      this->derived().connect(this->derived().pushButton_save  , &QAbstractButton::clicked, &this->derived(), &Derived::saveAndClose );
      this->derived().connect(this->derived().pushButton_cancel, &QAbstractButton::clicked, &this->derived(), &Derived::clearAndClose);
      return;
   }

   /**
    * \brief Edit the given Hop, Fermentable, etc.
    *
    *        Calling with no parameter clears the current item.
    */
   void setEditItem(std::shared_ptr<NE> editItem = nullptr) {
      if (this->m_editItem) {
         this->derived().disconnect(this->m_editItem.get(), nullptr, &this->derived(), nullptr);
      }
      this->m_editItem = editItem;
      if (this->m_editItem) {
         this->derived().connect(this->m_editItem.get(), &NamedEntity::changed, &this->derived(), &Derived::changed);
         this->readFromEditItem(std::nullopt);
      }
      return;
   }

   /**
    * \brief We don't want the compiler automatically constructing a shared_ptr for us if we accidentally call
    *        \c setEditItem with, say, a raw pointer, so this template trick ensures it can't.
    */
   template <typename D> void setEditItem(D) = delete;

   void setFolder(std::shared_ptr<NE> ne, QString const & folder) requires HasFolder<NE> {
      if (!folder.isEmpty()) {
         ne->setFolder(folder);
      }
      return;
   }

   void setFolder([[maybe_unused]] std::shared_ptr<NE> ne, [[maybe_unused]] QString const & folder) requires HasNoFolder<NE> {
      return;
   }

   /**
    * \brief Create a new Hop, Fermentable, etc.
    *
    *        This is also called from \c TreeView::newNamedEntity.
    */
   void newEditItem(QString folder = "") {
      QString name = QInputDialog::getText(&this->derived(),
                                           QString(QObject::tr("%1 name")).arg(NE::staticMetaObject.className()),
                                           QString(QObject::tr("%1 name:")).arg(NE::staticMetaObject.className()));
      if (name.isEmpty()) {
         return;
      }

      auto ne = std::make_shared<NE>(name);
      this->setFolder(ne, folder);

      this->setEditItem(ne);
      this->derived().show();
      return;
   }

   /**
    * \brief If \c fromEditItem is \c true supplied, sets the \c field.editField from the \c field.property of
    *        \c this->m_editItem -- ie populates/updates the UI input field from the model object.
    *        If \c fromEditItem is \c false, clears the edit field.
    */
   void getProperty(EditorBaseField const & field, bool const fromEditItem = true) {
      QVariant value;
      if (fromEditItem) {
         value = this->m_editItem->property(*field.property);
      } else {
         value = QString{""};
      }

      // Usually leave this debug log commented out unless trouble-shooting as it generates a lot of logging
//      qDebug() << Q_FUNC_INFO << field.labelName << "read from" << field.property << "as" << value;

      if (std::holds_alternative<QTextEdit *>(field.editField)) {
         std::get<QTextEdit *>(field.editField)->setPlainText(value.toString());
      } else if (std::holds_alternative<QLineEdit *>(field.editField)) {
         std::get<QLineEdit *>(field.editField)->setText(value.toString());
      } else {
         auto sle = std::get<SmartLineEdit *>(field.editField);
         if (fromEditItem) {
            sle->setFromVariant(value);
         } else {
            sle->setText(value.toString());
         }
      }
      return;
   }

   /**
    * \brief Sets \c field.property on \c this->m_editItem to the \c field.editField value -- ie writes back the UI
    *        value into the model object.
    */
   void setProperty(EditorBaseField const & field) {
      QVariant val;
      if (std::holds_alternative<QTextEdit *>(field.editField)) {
         val = QVariant::fromValue(std::get<QTextEdit *>(field.editField)->toPlainText());
      } else if (std::holds_alternative<QLineEdit *>(field.editField)) {
         val = QVariant::fromValue(std::get<QLineEdit *>(field.editField)->text());
      } else {
         auto sle = std::get<SmartLineEdit *>(field.editField);
         val = sle->getAsVariant();
      }

      this->m_editItem->setProperty(*field.property, val);
      return;
   }

   /**
    * \brief Subclass should override this if it needs to validate the form before saving happens.
    *
    * \return \c true if validation succeeded, \c false if it did not (and save should therefore be aborted)
    */
   bool validateBeforeSave() {
      return true;
   }

   /**
    * \brief Subclass should call this from its \c save slot
    */
   void doSaveAndClose() {
      if (!this->m_editItem) {
         this->derived().setVisible(false);
         return;
      }
      // Note that we have to call this->derived().validateBeforeSave(), not just this->validateBeforeSave(), in order
      // to allow the derived class to override validateBeforeSave().  But, because of the magic of the CRTP, there is
      // no need to make validateBeforeSave() virtual.
      if (!this->derived().validateBeforeSave()) {
         return;
      }

      this->writeNormalFields();
      if (this->m_editItem->key() < 0) {
         ObjectStoreWrapper::insert(this->m_editItem);
      }
      this->writeLateFields();

      this->derived().setVisible(false);
      return;
   }

   /**
    * \brief Subclass should call this from its \c clearAndClose slot
    */
   void doClearAndClose() {
      this->setEditItem();
      this->derived().setVisible(false); // Hide the window.
      return;
   }

   /**
    * \brief Read either one field (if \c propName specified) or all (if it is \c std::nullopt) into the UI from the
    *        model item.
    */
   void readFromEditItem(std::optional<QString> propName) {
      if (this->m_fields) {
         for (auto const & field : *this->m_fields) {
            if (!propName || *propName == field.property) {
               this->getProperty(field);
               if (propName) {
                  // Break out here if we were only updating one property
                  break;
               }
            }
         }
      }
      // TODO: For the moment, we still do this call, but ultimately we'll eliminate it.
      this->derived().readFieldsFromEditItem(propName);
      return;
   }

   /**
    * \brief Subclass should call this from its \c changed slot
    *
    *        Note that \c QObject::sender has \c protected access specifier, so we can't call it from here, not even
    *        via the derived class pointer.  Therefore we have derived class call it and pass us the result.
    */
   virtual void doChanged(QObject * sender, QMetaProperty prop, [[maybe_unused]] QVariant val) {
      if (this->m_editItem && sender == this->m_editItem.get()) {
         this->readFromEditItem(prop.name());
      }
      return;
   }

   void doClearFields() {
      if (this->m_fields) {
         for (auto const & field : *this->m_fields) {
            this->getProperty(field, false);
         }
      }
      return;
   }

   void readAllFields() {
      if (this->m_editItem) {
         this->readFromEditItem(std::nullopt);
      } else {
         this->doClearFields();
      }
      return;
   }

   void writeFields(EditorBaseField::WhenToWrite const normalOrLate) {
      if (this->m_fields) {
         for (auto const & field : *this->m_fields) {
            if (normalOrLate == field.whenToWrite) {
               this->setProperty(field);
            }
         }
      }
      return;

   }

   void writeNormalFields() {
      this->writeFields(EditorBaseField::WhenToWrite::Normal);
      // TODO: For the moment, we still do this call, but ultimately we'll eliminate it.
      this->derived().writeFieldsToEditItem();
      return;
   }

   void writeLateFields() {
      this->writeFields(EditorBaseField::WhenToWrite::Late);
      // TODO: For the moment, we still do this call, but ultimately we'll eliminate it.
      this->derived().writeLateFieldsToEditItem();
      return;
   }

protected:
   /**
    * \brief Info about fields in this editor
    */
   std::unique_ptr<std::vector<EditorBaseField>> m_fields;

   /**
    * \brief This is the \c NamedEntity subclass object we are creating or editing.  We are also "observing" it in the
    *        sense that, if any other part of the code changes its data, we'll get a signal so we can update our
    *        display.  Historically therefore this member variable was called \c obsHop, \c obsFermentable, etc in each
    *        of the editor classes.
    */
   std::shared_ptr<NE> m_editItem;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define EDITOR_COMMON_DECL(NeName)                                                   \
   /* This allows EditorBase to call protected and private members of Derived */     \
   friend class EditorBase<NeName##Editor, NeName>;                                  \
                                                                                     \
   public:                                                                           \
      NeName##Editor(QWidget * parent = nullptr);                                    \
      virtual ~NeName##Editor();                                                     \
                                                                                     \
      void writeFieldsToEditItem();                                                  \
      void writeLateFieldsToEditItem();                                              \
      void readFieldsFromEditItem(std::optional<QString> propName);                  \
                                                                                     \
   public slots:                                                                     \
      /* Standard editor slots */                                                    \
      void saveAndClose();                                                           \
      void clearAndClose();                                                          \
      void changed(QMetaProperty, QVariant);                                         \
      void clickedNew();                                                             \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define EDITOR_COMMON_CODE(EditorName) \
   void EditorName::saveAndClose() { this->doSaveAndClose(); return; } \
   void EditorName::clearAndClose() { this->doClearAndClose(); return; } \
   void EditorName::changed(QMetaProperty prop, QVariant val) { this->doChanged(this->sender(), prop, val); return; } \
   void EditorName::clickedNew() { this->newEditItem(); return;}

#endif
