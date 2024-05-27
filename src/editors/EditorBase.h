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

#include <QAbstractButton>
#include <QInputDialog>
#include <QString>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedEntity.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

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
 *        the implementation file using the EDITOR_COMMON_SLOT_DEFINITIONS macro.  Eg, in HopEditor, we need:
 *
 *          EDITOR_COMMON_SLOT_DEFINITIONS(HopEditor)
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
   EditorBase() :
      m_editItem{nullptr} {
      return;
   }
   virtual ~EditorBase() = default;

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
      this->derived().connect(this->derived().pushButton_save  , &QAbstractButton::clicked, &this->derived(), &Derived::save         );
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
         this->derived().readFieldsFromEditItem(std::nullopt);
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
    *        This is also called from \c BtTreeView::newNamedEntity.
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
///      if (!folder.isEmpty()) {
///         ne->setFolder(folder);
///      }

      this->setEditItem(ne);
      this->derived().show();
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
   void doSave() {
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

      this->derived().writeFieldsToEditItem();
      if (this->m_editItem->key() < 0) {
         ObjectStoreWrapper::insert(this->m_editItem);
      }
      this->derived().writeLateFieldsToEditItem();

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
    * \brief Subclass should call this from its \c changed slot
    *
    *        Note that \c QObject::sender has \c protected access specifier, so we can't call it from here, not even
    *        via the derived class pointer.  Therefore we have derived class call it and pass us the result.
    */
   void doChanged(QObject * sender, QMetaProperty prop, [[maybe_unused]] QVariant val) {
      if (this->m_editItem && sender == this->m_editItem.get()) {
         this->derived().readFieldsFromEditItem(prop.name());
      }
      return;
   }

protected:

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
      void save();                                                                   \
      void clearAndClose();                                                          \
      void changed(QMetaProperty, QVariant);                                         \
      void clickedNew();                                                             \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define EDITOR_COMMON_SLOT_DEFINITIONS(EditorName) \
   void EditorName::save() { this->doSave(); return; } \
   void EditorName::clearAndClose() { this->doClearAndClose(); return; } \
   void EditorName::changed(QMetaProperty prop, QVariant val) { this->doChanged(this->sender(), prop, val); return; } \
   void EditorName::clickedNew() { this->newEditItem(); return;}

#endif
