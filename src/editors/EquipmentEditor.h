/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/EquipmentEditor.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • David Grundberg <individ@acc.umu.se>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mike Evans <mikee@saxicola.co.uk>
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
#ifndef EDITORS_EQUIPMENTEDITOR_H
#define EDITORS_EQUIPMENTEDITOR_H
#pragma once

#include <vector>

#include <QMetaProperty>
#include <QVariant>

#include "ui_equipmentEditor.h"

#include "editors/EditorBase.h"
#include "model/Equipment.h"

#define EquipmentEditorOptions EditorBaseOptions{ .idDisplay = true, .numRecipesUsing = true }
/*!
 * \class EquipmentEditor
 *
 * \brief View/controller class for creating and editing \c Equipment records.
 *
 *        See comment on EditorBase::connectSignalsAndSlots for why we need to have \c public, not \c private
 *        inheritance from the Ui base.
 */
class EquipmentEditor : public QDialog,
                        public Ui::equipmentEditor,
                        public EditorBase<EquipmentEditor, Equipment, EquipmentEditorOptions> {
   Q_OBJECT

   EDITOR_COMMON_DECL(Equipment, EquipmentEditorOptions)

public slots:
   void hideOrShowOptionalVessels();
   void resetAbsorption();
   void updateDefaultEquipment();

public:
   bool validateBeforeSave();

private:
   void postReadFieldsFromEditItem(std::optional<QString> propName);
   void postWriteLateFieldsToEditItem();

   /**
    * \brief Remember, for each recipe using the equipment we are about to save, the batch size and pre-boil size that
    *        the equipment specified for it \b before the changes the user made in the editor -- so that, after saving,
    *        we can offer to update the recipes whose sizes matched (see \c offerToUpdateRecipeSizes).
    */
   void rememberRecipeSizesBeforeSave();

   /**
    * \brief For recipes using the equipment we just saved, whose batch size and/or pre-boil size matched what the
    *        equipment previously specified (and so, presumably, came from it), offer to update them to what the
    *        equipment now specifies.  See https://github.com/Brewtarget/brewtarget/issues/531.
    */
   void offerToUpdateRecipeSizes();

   struct RecipeSizesBeforeSave {
      Recipe * recipe;
      //! What the equipment specified for the recipe's batch size before the save
      double batchSize_l;
      //! What the equipment specified for the recipe's pre-boil size (for its boil time) before the save
      double preBoilSize_l;
   };
   std::vector<RecipeSizesBeforeSave> m_recipeSizesBeforeSave;
};

#endif