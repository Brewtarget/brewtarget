/*======================================================================================================================
 * editors/InventoryFermentableEditor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef EDITORS_INVENTORYFERMENTABLEEDITOR_H
#define EDITORS_INVENTORYFERMENTABLEEDITOR_H
#pragma once

#include "ui_inventoryFermentableEditor.h"

#include <QDialog>
#include <QMetaProperty>
#include <QString>

#include "editors/EditorBase.h"
#include "model/InventoryFermentable.h"

#define InventoryFermentableEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
/*!
 * \class InventoryFermentableEditor
 *
 * \brief View/controller class for creating and editing InventoryFermentables.
 *
 *        See comment on EditorBase::connectSignalsAndSlots for why we need to have \c public, not \c private
 *        inheritance from the Ui base.
 */
class InventoryFermentableEditor :
   public QDialog,
   public Ui::inventoryFermentableEditor,
   public EditorBase<InventoryFermentableEditor, InventoryFermentable, InventoryFermentableEditorOptions> {

   Q_OBJECT

   EDITOR_COMMON_DECL(InventoryFermentable, InventoryFermentableEditorOptions)
};

#endif
