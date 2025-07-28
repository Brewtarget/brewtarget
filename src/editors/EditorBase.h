/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/EditorBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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

#include <concepts>
#include <memory>
#include <variant>
#include <vector>

#include <QAbstractButton>
#include <QInputDialog>
#include <QString>
#include <QPlainTextEdit>

#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "editors/EditorBaseField.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h" // Need to include this this to be able to cast Recipe to QObject
#include "undoRedo/Undoable.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

class StepOwner;

/**
 * \brief Extra base class for \c MashStepEditor, \c BoilStepEditor and \c FermentationStepEditor to inherit from
 *        \b before inheriting from \c EditorBase.  This provides the functionality that, if we are creating a new step,
 *        rather than editing an existing one, then we need to be able to set the owner (ie the \c Mash, \c Boil or
 *        \c Fermentation) when we click \b Save, because the steps have no independent existence without their owner.
 */
template<class Derived> class StepEditorPhantom;
template<class Derived, class NE>
class StepEditorBase : public CuriouslyRecurringTemplateBase<StepEditorPhantom, Derived> {
public:
   StepEditorBase() :
      m_stepOwner{nullptr} {
      return;
   }

   void setStepOwner(std::shared_ptr<typename NE::OwnerClass> stepOwner) {
      this->m_stepOwner = stepOwner;
      return;
   }

   /**
    * \brief If the step being edited is new, then, when save is clicked, this should be called
    */
   void addStepToStepOwner(std::shared_ptr<NE> step) {
      // It would be a coding error if, when showing MashStepEditor / BoilStepEditor / etc, the relevant
      // MashStep / BoilStep / etc were not set.
      Q_ASSERT(this->m_stepOwner);

      // It's a coding error if the step already has an owner (ie it's not new)
      Q_ASSERT(step->ownerId() < 0);

      // Going via Undoable::addStepToStepOwner makes the action undoable
      Undoable::addStepToStepOwner(*this->m_stepOwner, step);
      return;
   }

   /**
    * \brief I tried putting this in \c EditorBase and having \c NE::OwnerClass eavluated only when it is valid  (ie
    *        when HasStepOwner<editorBaseOptions> is true), and some replaced with some dummy type otherwise.  This
    *        seems quite to do.  Eg \c std::conditional requires both its class parameters to be valid.  So an extra
    *        CRTP base class was born.
    */
   std::shared_ptr<typename NE::OwnerClass> m_stepOwner;
};

/**
 * \brief This is used as a template parameter to turn on and off various \b small features in \c EditorBase (in
 *        conjunction with the concepts defined below).
 *
 * \sa EditorBase
 */
struct EditorBaseOptions {
   /**
    * \brief Enabling this turns on the temporary live copy of edit item, whose fields are updated straight away as
    *        edits are made.  This is useful for showing calculated values or drawing charts.
    */
   bool liveEditItem = false;
   /**
    * \brief Enabling this turns on the automatic update of the first tab to show the name of the item being edited.
    *        This relies on the \c Derived class having a \c QTabWidget called \c tabWidget_editor (and that the object
    *        name is provided by the \c PropertyNames::NamedEntity::name property).
    */
   bool nameTab = false;
   /**
    * \brief Enabling this turns on the observation of (the current) \c Recipe.  Typically it makes sense for us to be
    *        able to watch a \c Recipe when it can have at most one of the thing we are editing (\c Boil, \c Equipment,
    *        \c Fermentation, \c Mash, \c Style).
    */
   bool recipe = false;
   /**
    * \brief Enabling this means we show the edit item's ID (obtained from the \c NamedEntity::key() function) in a
    *        label field called \c label_id_value.
    */
   bool idDisplay = false;
   /**
    * \brief Enabling this shows a count of the number of recipes that use the thing we are editing.  The count is shown
    *        as descriptive text (eg "Used in 3 recipes") in a label field called \c label_numRecipesUsing
    *
    *        It would be nice if we could detect directly whether the \c Derived class has a member called
    *        \c label_numRecipesUsing, but I couldn't get the CREATE_HAS_MEMBER / HAS_MEMBER macros to work on the
    *        derived class from its CRTP base class.
    */
   bool numRecipesUsing = false;
};
template <EditorBaseOptions eb> struct has_LiveEditItem    : public std::integral_constant<bool, eb.liveEditItem   >{};
template <EditorBaseOptions eb> struct has_NameTab         : public std::integral_constant<bool, eb.nameTab        >{};
template <EditorBaseOptions eb> struct has_Recipe          : public std::integral_constant<bool, eb.recipe         >{};
template <EditorBaseOptions eb> struct has_IdDisplay       : public std::integral_constant<bool, eb.idDisplay      >{};
template <EditorBaseOptions eb> struct has_NumRecipesUsing : public std::integral_constant<bool, eb.numRecipesUsing>{};
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <EditorBaseOptions eb> concept CONCEPT_FIX_UP HasLiveEditItem    = has_LiveEditItem   <eb>::value;
template <EditorBaseOptions eb> concept CONCEPT_FIX_UP HasNameTab         = has_NameTab        <eb>::value;
template <EditorBaseOptions eb> concept CONCEPT_FIX_UP HasRecipe          = has_Recipe         <eb>::value;
template <EditorBaseOptions eb> concept CONCEPT_FIX_UP HasIdDisplay       = has_IdDisplay      <eb>::value;
template <EditorBaseOptions eb> concept CONCEPT_FIX_UP HasNumRecipesUsing = has_NumRecipesUsing<eb>::value;

/**
 * \class EditorBase
 *
 * \brief CRTP class that provides the common functionality for free-standing editors - ie those for which we have a
 *        separate window.  The main entities for which we do NOT have free-standing editors are \c Recipe (see
 *        \c MainWindow), \c Instruction (see \c BrewDayScrollWidget, shown as a tab on \c MainWindow), \c BrewNote (see
 *        \c BrewNoteWidget, shown as zero or more tabs on \c MainWindow).
 *
 *        As in other places where we want to use class templating, we have to use multiple inheritance because we can't
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
 *        Additionally, derived class needs to have the following \c QPushButton members (typically defined in the .ui
 *        file):
 *           \c pushButton_new, \c pushButton_save, \c pushButton_cancel
 *
 *        The LiveEditItem template parameter determines whether we keep a "live" copy of whatever is being edited (ie
 *        a copy object to which edits will be applied in real time).  This is useful to show fields calculated by the
 *        NE object itself or (as in the case of \c WaterEditor) to feed data to a chart.  Subclasses that set
 *        \c editorBaseOptions.liveEditItem to \c true need the following additional private member functions:
 *           - \c void \c postInputFieldModified -- Update any chart following input field modification.
 */
template<class Derived> class EditorPhantom;
template<class Derived, class NE,
                        EditorBaseOptions editorBaseOptions>
class EditorBase : public CuriouslyRecurringTemplateBase<EditorPhantom, Derived> {
   friend Derived;
private:
   /**
    * \brief Constructor
    *
    *        Often with CRTP it's good to make the constructor private and Derived a friend, so that only Derived can
    *        call the CRTP base constructor.
    *
    *        Note that we cannot initialise this->m_fields here, as the parameters themselves won't get constructed
    *        until Derived calls setupUi().
    */
   EditorBase(QString const editorName) :
      m_editorName{editorName},
      m_fields{nullptr},
      m_editItem{nullptr},
      m_liveEditItem{nullptr} {
      return;
   }
public:
   ~EditorBase() = default;

   //! No-op version
   void setupTabs() requires (!HasNameTab<editorBaseOptions>) { return; }
   //! Substantive version
   void setupTabs() requires (HasNameTab<editorBaseOptions>) {
      this->derived().tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);
      return;
   }

   /**
    * \brief Derived should call this after calling setupUi
    *
    *        NOTE that where two fields are linked to the same property (typically where we have an amount field and a
    *        combo box that controls whether that amount is mass/volume/etc) they \b must be adjacent in
    *        initializer_list.  (This is because of how we do early break out when only
    */
   void postSetupUiInit(std::initializer_list<EditorBaseFieldVariant> fields) {
      this->m_fields = std::make_unique<std::vector<EditorBaseFieldVariant>>(fields);
      this->setupTabs();
      this->connectSignalsAndSlots();
      return;
   }

   //! \brief No-op version
   void connectLiveEditSignalsAndSlots() requires (!HasLiveEditItem<editorBaseOptions>) {
      return;
   }

   /**
    * \brief When we have a live edit item, we want to know each time an input field has been modified so that we can
    *        update the corresponding property of the live edit item.  We connect the relevant signal to
    *        Derived::inputFieldModified, which then calls doInputFieldModified
    */
   void connectLiveEditSignalsAndSlots() requires HasLiveEditItem<editorBaseOptions> {
      if (this->m_fields) {
         for (auto const & field : *this->m_fields) {
            // Using std::visit and a lambda allows us to do generic things on std::variant
            std::visit(
               [this](auto&& fieldInfo) {
                  fieldInfo.connectFieldChanged(&this->derived(), &Derived::inputFieldModified);
               },
               field
            );
         }
      }
      return;
   }

   //! \brief No-op version
   void doInputFieldModified([[maybe_unused]] QObject const * const signalSender)
   requires (!HasLiveEditItem<editorBaseOptions>) {
      return;
   }

   //! \brief Substantive version
   void doInputFieldModified(QObject const * const signalSender) requires (HasLiveEditItem<editorBaseOptions>) {
      if (this->m_fields && this->m_liveEditItem && signalSender && signalSender->parent() == &this->derived()) {
         bool foundMatch = false;
         for (auto const & field : *this->m_fields) {
            // Using std::visit and a lambda allows us to do generic things on std::variant
            if (std::visit(
               // Lambda returns true if we matched the signal sender to this EditorBaseField, false otherwise
               [this, signalSender](auto&& fieldInfo) {
                  if (signalSender == fieldInfo.editField) {
                     fieldInfo.setPropertyFromEditField(*this->m_liveEditItem);
                     return true;
                  }
                  return false;
               },
               field
            )) {
               foundMatch = true;
               break;
            }
         }

         if (!foundMatch) {
            // If we get here, it's probably a coding error but there's no harm in soldiering on
            qWarning() << Q_FUNC_INFO << "Unrecognised signal sender";
            return;
         }

         this->derived().postInputFieldModified();
      }
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
      //
      this->connectLiveEditSignalsAndSlots();
      return;
   }

   //! \brief No-op version
   void setLiveEditItem() requires (!HasLiveEditItem<editorBaseOptions>) {
      return;
   }

   void setLiveEditItem() requires HasLiveEditItem<editorBaseOptions> {
      if (this->m_editItem) {
         this->m_liveEditItem = std::make_unique<NE>(*this->m_editItem);
      } else {
         this->m_liveEditItem.reset();
      }
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
         this->readFieldsFromEditItem(std::nullopt);
      }

      //
      // We detect when NE has a member StepClass (which should be a "using" alias) so we can show MashStep items in the
      // Mash editor, BoilStep items in the Boil editor, etc.  This requires the Derived class to have (via its .ui
      // file) a suitable subclass of StepsWidget (eg MashStepsWidget, BoilStepsWidget, etc) called stepsWidget.
      //
      if constexpr (std::is_base_of<StepOwner, NE>::value) {
         this->derived().stepsWidget->setStepOwner(editItem);
      }

      this->setLiveEditItem();

      // Comment below about calling this->derived().validateBeforeSave() also applies here
      this->derived().postSetEditItem();
      return;
   }

   /**
    * \brief \c Derived can override this if there is additional processing to do at the end of \c setEditItem
    *
    *        This is used, eg, in \c WaterEditor to set up the \c RadarChart
    */
   void postSetEditItem() {
      return;
   }

   /**
    * \brief We don't want the compiler automatically constructing a shared_ptr for us if we accidentally call
    *        \c setEditItem with, say, a raw pointer, so this template trick ensures it can't.
    */
   template <typename D> void setEditItem(D) = delete;

   void setFolderPath(std::shared_ptr<NE> ne, QString const & folderPath) requires HasFolder<NE> {
      if (!folderPath.isEmpty()) {
         ne->setFolderPath(folderPath);
      }
      return;
   }

   void setFolderPath([[maybe_unused]] std::shared_ptr<NE> ne, [[maybe_unused]] QString const & folderPath) requires HasNoFolder<NE> {
      return;
   }

   /**
    * \brief Create a new Hop, Fermentable, etc.
    *
    *        This is also called from \c TreeView::newNamedEntity.
    */
   void newEditItem(QString folderPath = "") {
      QString name = QInputDialog::getText(&this->derived(),
                                           QString(QObject::tr("%1 name")).arg(NE::staticMetaObject.className()),
                                           QString(QObject::tr("%1 name:")).arg(NE::staticMetaObject.className()));
      if (name.isEmpty()) {
         return;
      }

      auto ne = std::make_shared<NE>(name);
      this->setFolderPath(ne, folderPath);

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

   void updateStepOwnerIfNeeded() {
      if constexpr (std::is_base_of<StepEditorBase<Derived, NE>, Derived>::value) {
         if (this->m_editItem->ownerId() > 0) {
            // If the step already has an owner, then there is nothing to do here (and trying to re-add it to its
            // existing owner would be harmful.
            return;
         }

         // The member function we're calling is in StepEditorBase, so we have to access it via Derived (which inherits
         // from that, whereas EditorBase does not).
         this->derived().addStepToStepOwner(this->m_editItem);
      }
      return;
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

      this->writeNormalFieldsToEditItem();
      if (this->m_editItem->key() < 0) {
         ObjectStoreWrapper::insert(this->m_editItem);
      }
      this->writeLateFieldsToEditItem();
      this->updateStepOwnerIfNeeded();

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

   //! \brief No-op version
   void updateNameTabIfNeeded([[maybe_unused]] std::optional<QString> propName)
   requires (!HasNameTab<editorBaseOptions>) {
      return;
   }
   //! \brief Substantive version
   void updateNameTabIfNeeded(std::optional<QString> propName) requires (HasNameTab<editorBaseOptions>) {
      if (!propName || *propName == PropertyNames::NamedEntity::name) {
         this->derived().tabWidget_editor->setTabText(0, this->m_editItem->name());
      }
      return;
   }

   //! \brief No-op version
   void showId() requires (!HasIdDisplay<editorBaseOptions>) { return; }
   //! \brief Substantive version
   void showId() requires (HasIdDisplay<editorBaseOptions>) {
      // This label does not have an input field; it just shows the ID of the item
      this->derived().label_id_value->setText(QString::number(this->m_editItem->key()));
      return;
   }

   void showNumRecipesUsing() {
      if constexpr (HasNumRecipesUsing<editorBaseOptions>) {
         this->derived().label_numRecipesUsing->setText(Recipe::usedInRecipes(*this->m_editItem));
      }
      return;
   }

   //! Derived classes can override this for any extra behaviour
   void postReadFieldsFromEditItem([[maybe_unused]] std::optional<QString> propName) { return; }

   /**
    * \brief (Re)read either one field (if \c propName specified) or all (if it is \c std::nullopt) into the UI from the
    *        model item.
    */
   void readFieldsFromEditItem(std::optional<QString> propName) {
      if (this->m_editItem && this->m_fields) {
         bool matched = false;
         for (auto const & field : *this->m_fields) {
            if (std::visit(
               //
               // This lambda returns true if we should stop subsequent loop processing, or false if we should carry on
               // looking at subsequent fields.  Because the code inside the lambda cannot directly break out of the
               // for loop, we have to return this boolean to tell the calling code whether to break out.
               //
               [this, &propName, &matched](auto&& fieldInfo) {
                  //
                  // The update rule is simple -- we either update all fields (because no property name is supplied) or
                  // only the field(s) for the supplied property name.
                  //
                  // In most cases, there will only be one field per property name.  However, we also have to handle the
                  // case where we have a combo-box that is controlling the physical quantity for another field (eg
                  // whether an input field is mass or volume).  By convention, where there is more than one field for a
                  // property name, the records must be adjacent in the m_fields vector.  This makes our "break"
                  // criteria relatively simple:
                  //    - We have a property name
                  //    - We already matched it at least once
                  //    - The current field does not match
                  //
                  if (!propName || *propName == fieldInfo.property) {
                     // Normally leave this log statement commented out as it generates too many lines in the log file
//                     qDebug() << Q_FUNC_INFO << "Reading" << fieldInfo.property;
                     fieldInfo.setEditFieldFromProperty(*this->m_editItem);
                     if (propName) {
                        matched = true;
                     }
                  } else if (!propName && matched) {
                     return true;
                  }
                  return false;
               },
               field
            )) {
               break;
            }
         }
      }
      // The ID is not going to change unless we're reading in all fields
      if (!propName) {
         this->showId();
      }
      this->showNumRecipesUsing();
      this->updateNameTabIfNeeded(propName);
      // Note the need for derived() here to allow Derived to override
      this->derived().postReadFieldsFromEditItem(propName);
      return;
   }

   //! No-op version
   bool handleChangeFromRecipe([[maybe_unused]] QObject * sender) requires (!HasRecipe<editorBaseOptions>) {
      return false;
   }
   //! Substantive version
   bool handleChangeFromRecipe(QObject * sender) requires (HasRecipe<editorBaseOptions>) {
      if (this->m_recipeObs && sender == static_cast<QObject *>(this->m_recipeObs)) {
         this->readAllFields();
         return true;
      }
      return false;
   }
   /**
    * \brief Subclass should call this from its \c changed slot
    *
    *        Note that \c QObject::sender has \c protected access specifier, so we can't call it from here, not even
    *        via the derived class pointer.  Therefore we have derived class call it and pass us the result.
    */
   void doChanged(QObject * sender, QMetaProperty prop, [[maybe_unused]] QVariant val) {
      if (this->handleChangeFromRecipe(sender)) {
         return;
      }
      if (this->m_editItem && sender == this->m_editItem.get()) {
         this->readFieldsFromEditItem(prop.name());
      }
      return;
   }

   void doClearFields() {
      if (this->m_fields) {
         for (auto const & field : *this->m_fields) {
            std::visit(
               [](auto&& fieldInfo) {
                  fieldInfo.clearEditField();
               },
               field
            );
         }
      }
      return;
   }

   void readAllFields() {
      if (this->m_editItem) {
         this->readFieldsFromEditItem(std::nullopt);
      } else {
         this->doClearFields();
      }
      return;
   }

   void writeFields(WhenToWriteField const normalOrLate) {
      if (this->m_editItem && this->m_fields) {
         for (auto const & field : *this->m_fields) {
            std::visit(
               [this, normalOrLate](auto&& fieldInfo) {
                  // Normally leave this debug statement commented out as it generates too much logging
//                  qDebug() <<
//                     Q_FUNC_INFO << "Field:" << fieldInfo.labelName << ", property:" << *fieldInfo.property <<
//                     ", hasControlledField:" << fieldInfo.hasControlledField << ", whenToWrite:" <<
//                     fieldInfo.whenToWrite << ", value:" << fieldInfo.getFieldValue();
                  if (normalOrLate == fieldInfo.whenToWrite) {
                     fieldInfo.setPropertyFromEditField(*this->m_editItem);
                  }
               },
               field
            );
         }
      }
      return;

   }

   //! Derived classes can override this for any extra behaviour
   void postWriteNormalFieldsToEditItem() { return; }

   //! Write most fields from the editor GUI fields into the object being edited
   void writeNormalFieldsToEditItem() {
      this->writeFields(WhenToWriteField::Normal);
      // Note the need for derived() here to allow Derived to override
      this->derived().postWriteNormalFieldsToEditItem();
      return;
   }

   //! Derived classes can override this for any extra behaviour
   void postWriteLateFieldsToEditItem() { return; }

   //! Write any fields that must wait until the object definitely exists in the DB
   void writeLateFieldsToEditItem() {
      this->writeFields(WhenToWriteField::Late);
      // Note the need for derived() here to allow Derived to override
      this->derived().postWriteLateFieldsToEditItem();
      return;
   }

   void setRecipe(Recipe * recipe) requires HasRecipe<editorBaseOptions> {
      this->m_recipeObs = recipe;
      // TBD: We could automatically set the edit item as follows:
//   if (this->m_recipeObs) {
//      this->m_editItem = this->m_recipeObs->get<NE>()
//   }
      return;
   }

   //! \brief Show the editor and re-read the edit item.  Used for editors with Recipe.
   void doShowEditor() {
      this->readAllFields();
      this->derived().setVisible(true);
      return;
   }

   //! \brief Close (hide) the editor without saving anything.  Used for editors with Recipe.
   void doCloseEditor() {
      this->derived().setVisible(false);
      return;
   }

protected:
   /**
    * \brief Optionally an editor can have a "name" to add some context.  Eg for the Water editor, the water chemistry
    *        dialog allows you to have two of them open at once -- one "Base" and one "Target".
    */
   QString const m_editorName;

   /**
    * \brief Info about fields in this editor
    */
   std::unique_ptr<std::vector<EditorBaseFieldVariant>> m_fields;

   /**
    * \brief This is the \c NamedEntity subclass object we are creating or editing.  We are also "observing" it in the
    *        sense that, if any other part of the code changes its data, we'll get a signal so we can update our
    *        display.  Historically therefore this member variable was called \c obsHop, \c obsFermentable, etc in each
    *        of the editor classes.
    */
   std::shared_ptr<NE> m_editItem;

   /**
    * \brief Optionally, an editor can create a temporary copy of \c m_editItem to which to apply edits immediately.
    *        This is useful if we want to be able to show calculated values or if (as in the case of \c WaterEditor) we
    *        want to use a copy object to as input to a chart or graph showing live edits.  This object is discarded
    *        when the user clicks Save or Cancel.  (In the former case, the form values are applied to \c m_editItem; in
    *        the latter they are not.
    *
    *        There are various tricks where we could make the existence or type of this member variable depend on the
    *        LEI template parameter (see https://brevzin.github.io/c++/2021/11/21/conditional-members/) but it's
    *        currently a bit complicated, and should become easier with future reflection features.  So, for now, we
    *        we don't worry about the overhead of unnecessarily having this member when LEI is LiveEditItem::Disabled.
    */
   std::unique_ptr<NE> m_liveEditItem;

   /**
    * \brief The \c Recipe, if any, that we are "observing".
    */
   Recipe * m_recipeObs = nullptr;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define EDITOR_COMMON_DECL(NeName, Options)                                          \
   /* This allows EditorBase to call protected and private members of Derived */     \
   friend class EditorBase<NeName##Editor, NeName, Options>;                         \
                                                                                     \
   public:                                                                           \
      NeName##Editor(QWidget * parent = nullptr, QString const editorName = "");     \
      virtual ~NeName##Editor();                                                     \
                                                                                     \
      void readFieldsFromEditItem(std::optional<QString> propName);                  \
                                                                                     \
   public slots:                                                                     \
      /* Standard editor slots */                                                    \
      void saveAndClose();                                                           \
      void clearAndClose();                                                          \
      void changed(QMetaProperty, QVariant);                                         \
      void clickedNew();                                                             \
      void inputFieldModified();                                                     \
      /* Additional standard slots for editors with recipe */                        \
      void showEditor();                                                             \
      void closeEditor();                                                            \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define EDITOR_COMMON_CODE(NeName) \
   void NeName##Editor::saveAndClose() { this->doSaveAndClose(); return; }                                                \
   void NeName##Editor::clearAndClose() { this->doClearAndClose(); return; }                                              \
   void NeName##Editor::changed(QMetaProperty prop, QVariant val) { this->doChanged(this->sender(), prop, val); return; } \
   void NeName##Editor::clickedNew() { this->newEditItem(); return; }                                                     \
   void NeName##Editor::inputFieldModified() { this->doInputFieldModified(this->sender()); return; };                     \
   void NeName##Editor::showEditor() { this->doShowEditor(); }                                                            \
   void NeName##Editor::closeEditor() { this->doCloseEditor(); }                                                          \

#endif
