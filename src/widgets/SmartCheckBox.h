/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartCheckBox.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef WIDGETS_SMARTCHECKBOX_H
#define WIDGETS_SMARTCHECKBOX_H
#pragma once

#include <memory> // For PImpl

#include <QCheckBox>

class QLabel;
class SmartLineEdit;
struct TypeInfo;

/**
 * \class SmartCheckBox
 *
 * \brief Used for controlling the measurement type of another field -- eg mass vs volume
 */
class SmartCheckBox : public QCheckBox {
Q_OBJECT

public:
   SmartCheckBox(QWidget* parent = nullptr);
   virtual ~SmartCheckBox();

   /**
    * \brief This needs to be called before the object is used, typically in constructor of whatever editor is using the
    *        widget.  Similar to the equivalent functions in \c SmartField.  See widgets/SmartField.h for more extensive
    *        comments.
    *
    *        Note, in reality, you actually use the \c SMART_CHECK_BOX_INIT macro (see below).
    *
    * \param editorName
    * \param checkBoxName
    * \param checkBoxFqName
    * \param buddyLabel
    * \param controlledField
    * \param typeInfo
    */
   void init(char const *  const   editorName,
             char const *  const   checkBoxName,
             char const *  const   checkBoxlFqName,
             QLabel              & buddyLabel,
             SmartLineEdit       & controlledField,
             TypeInfo      const & typeInfo);

public slots:
   void onToggled(bool const state);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a field object
   SmartCheckBox(SmartCheckBox const&) = delete;
   //! No assignment operator, as never want anyone, not even our friends, to make copies of a field object
   SmartCheckBox& operator=(SmartCheckBox const&) = delete;
   //! No move constructor
   SmartCheckBox(SmartCheckBox &&) = delete;
   //! No move assignment
   SmartCheckBox & operator=(SmartCheckBox &&) = delete;
};

/**
 * \brief Instead of calling:
 *           this->checkBox_dmsPIsMassPerVolume->init("FermentableEditor",
 *                                                    "checkBox_dmsPIsMassPerVolume",
 *                                                    "FermentableEditor->checkBox_dmsPIsMassPerVolume",
 *                                                    *this->label_dmsPIsMassPerVolume,
 *                                                    *this->lineEdit_dmsP,
 *                                                    Fermentable,
 *                                                    PropertyNames::Fermentable::dmsPIsMassPerVolume)
 *        You call:
 *           SMART_CHECK_BOX_INIT(FermentableEditor,
 *                                checkBox_dmsPIsMassPerVolume,
 *                                label_dmsPIsMassPerVolume,
 *                                lineEdit_dmsP,
 *                                Fermentable,
 *                                dmsPIsMassPerVolume)
 *
 *        NOTE: We are more concise here than in \c SMART_FIELD_INIT and related macros because none of the combo boxes
 *              need to access inherited properties.  Eg, in \c HopEditor, all the properties for combo boxes are going
 *              to be \c PropertyNames::Hop::somethingOrOther, which is not always the case for other types of field.
 */
#define SMART_CHECK_BOX_INIT(editorClass, checkBoxName, labelName, controlledFieldName, modelClass, propertyName) \
   this->checkBoxName->init(#editorClass, \
                            #checkBoxName, \
                            #editorClass "->" #checkBoxName, \
                            *this->labelName, \
                            *this->controlledFieldName, \
                            modelClass::typeLookup.getType(PropertyNames::modelClass::propertyName))

#endif
