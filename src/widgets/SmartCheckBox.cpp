/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartCheckBox.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "widgets/SmartCheckBox.h"

#include <QDebug>
#include <QLabel>

#include "widgets/SmartLineEdit.h"
#include "utils/TypeLookup.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_SmartCheckBox.cpp"

// This private implementation class holds all private non-virtual members of SmartCheckBox
class SmartCheckBox::impl {
public:
   impl(SmartCheckBox & self) :
      m_self            {self},
      m_initialised     {false},
      m_editorName      {"Uninitialised m_editorName!" },
      m_checkBoxName    {"Uninitialised m_checkBoxName!"  },
      m_checkBoxFqName  {"Uninitialised m_checkBoxFqName!"},
      m_buddyLabel      {nullptr},
      m_controlledField {nullptr},
      m_typeInfo        {nullptr} {
      return;
   }

   ~impl() = default;

   SmartCheckBox &  m_self           ;
   bool             m_initialised    ;
   char const *     m_editorName     ;
   char const *     m_checkBoxName   ;
   char const *     m_checkBoxFqName ;
   QLabel *         m_buddyLabel     ;
   SmartLineEdit *  m_controlledField;
   TypeInfo const * m_typeInfo       ;
};

SmartCheckBox::SmartCheckBox(QWidget * parent) :
   QCheckBox(parent),
   pimpl{std::make_unique<impl>(*this)} {
   connect(this, &QAbstractButton::toggled, this, &SmartCheckBox::onToggled);
   return;
}

SmartCheckBox::~SmartCheckBox() = default;

void SmartCheckBox::init(char const *  const   editorName,
                         char const *  const   checkBoxName,
                         char const *  const   checkBoxFqName,
                         QLabel              & buddyLabel,
                         SmartLineEdit       & controlledField,
                         TypeInfo      const & typeInfo) {
   qDebug() << Q_FUNC_INFO << checkBoxFqName << ":" << typeInfo;

   // It's a coding error to call this function more than once!
   Q_ASSERT(!this->pimpl->m_initialised);

   this->pimpl->m_editorName      =  editorName     ;
   this->pimpl->m_checkBoxName    =  checkBoxName   ;
   this->pimpl->m_checkBoxFqName  =  checkBoxFqName ;
   this->pimpl->m_buddyLabel      = &buddyLabel     ;
   this->pimpl->m_controlledField = &controlledField;
   this->pimpl->m_typeInfo        = &typeInfo       ;

   // It's a coding error to try to initialise a SmartCheckBox before the SmartField it controls
   Q_ASSERT(this->pimpl->m_controlledField->isInitialised());

   // We want the checkbox and the controlled field to stay in sync.  In order for this to happen, they need to start
   // out in sync.  This is because the QAbstractButton::toggled signal is only emitted when the checkbox state CHANGES.
   // So, eg, if the checkbox is unchecked by default and the editor calls setChecked(false), then NO SIGNAL will be
   // generated, but as soon as either the user toggles the checkbox or someone calls setChecked in a way that DOES
   // change the checkbox value, we'll get the QAbstractButton::toggled signal.
   //
   // So, even though it seems a bit odd to sync the two fields before either one has had values set, it is necessary!
   this->pimpl->m_controlledField->selectPhysicalQuantity(this->isChecked());
   // TBD: Probably replace the above line with something along the following lines, but also need to look at onToggled below.
///   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));
///   auto const physicalQuantity = std::get<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType);
///   this->pimpl->m_controlledField->selectPhysicalQuantity(physicalQuantity);

   this->pimpl->m_initialised = true;
   return;
}

void SmartCheckBox::onToggled(bool const state) {
   Q_ASSERT(this->pimpl->m_initialised);

   // SmartField does all the heavy lifting here
   this->pimpl->m_controlledField->selectPhysicalQuantity(state);

   // Strictly, if we change, say, a Fermentable to be measured by mass instead of volume (or vice versa) we should also
   // somehow tell any other bit of the UI that is showing that Fermentable (eg a RecipeEditor or MainWindow) to
   // redisplay the relevant field.  Currently we don't do this, on the assumption that it's rare you will change how a
   // Fermentable is measured after you started using it in recipes.

   return;
}
