/*======================================================================================================================
 * editors/SaltEditor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef EDITORS_SALTEDITOR_H
#define EDITORS_SALTEDITOR_H
#pragma once

#include "ui_saltEditor.h"

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>

#include "editors/EditorBase.h"
#include "model/Salt.h"

#define SaltEditorOptions EditorBaseOptions{ .liveEditItem = true, .idDisplay = true, .numRecipesUsing = true }
/*!
 * \class SaltEditor
 *
 * \brief View/controller dialog for editing salts.
 *
 *        See comment on EditorBase::connectSignalsAndSlots for why we need to have \c public, not \c private
 *        inheritance from the Ui base.
 */
class SaltEditor : public QDialog,
                   public Ui::saltEditor,
                   public EditorBase<SaltEditor, Salt, SaltEditorOptions> {
   Q_OBJECT
public:
   EDITOR_COMMON_DECL(Salt, SaltEditorOptions)

private:
   void postSetEditItem();
   void postInputFieldModified();
};

#endif
