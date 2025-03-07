/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MashEditor.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef EDITORS_MASHEDITOR_H
#define EDITORS_MASHEDITOR_H
#pragma once

#include <memory>

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_mashEditor.h"

#include "editors/EditorBase.h"
#include "model/Mash.h"

#define MashEditorOptions EditorBaseOptions{ .recipe = true, .idDisplay = true }
/*!
 * \class MashEditor
 *
 * \brief View/controller dialog for editing a mash.
 *
 *        See also \c NamedMashEditor
 */
class MashEditor : public QDialog,
                   public Ui::mashEditor,
                   public EditorBase<MashEditor, Mash, MashEditorOptions> {
   Q_OBJECT

   EDITOR_COMMON_DECL(Mash, MashEditorOptions)

   //
   // TBD: We used to have the following function invoked by a button:
   //
   //    //! Get the tun mass and specific heat from the equipment.
   //    void fromEquipment();
   //
   // But, now that we are more explicitly allowing a Mash to be shared by more than one Recipe, this makes less sense.
   // We would either have to make the button only appear when the Mash is associated with exactly one Recipe, or we
   // would need to have some more complicated control to allow the user to select (either directly or indirectly) the
   // Equipment from which to copy the info.
   //
};

///// Forward declarations.
///class Recipe;
///class Mash;
///class Equipment;
///
////*!
/// * \class MashEditor
/// *
/// * \brief View/controller dialog for editing a mash.
/// *
/// *        See also \c NamedMashEditor
/// */
///class MashEditor : public QDialog, public Ui::mashEditor {
///   Q_OBJECT
///public:
///   MashEditor(QWidget * parent = nullptr);
///
///public slots:
///   void showEditor();
///   void closeEditor();
///   void saveAndClose();
///   //! Get the tun mass and specific heat from the equipment.
///   void fromEquipment();
///   //! Set the mash we wish to view/edit.
///   void setMash(std::shared_ptr<Mash> mash);
///   void setRecipe(Recipe* r);
///
///   void changed(QMetaProperty,QVariant);
///private:
///   void showChanges(QMetaProperty* prop = nullptr);
///   void clear();
///
///   Recipe *              m_recipe;
///   std::shared_ptr<Mash> m_mashObs;
///};

#endif
