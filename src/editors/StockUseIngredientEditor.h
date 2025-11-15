/*======================================================================================================================
 * editors/StockUseIngredientEditor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef EDITORS_STOCKUSEINGREDIENTEDITOR_H
#define EDITORS_STOCKUSEINGREDIENTEDITOR_H
#pragma once

#include <QDialog>

#include "ui_stockUseIngredientEditor.h"

#include "editors/EditorBase.h"
#include "model/StockUseIngredient.h"

#define StockUseFermentableEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
class StockUseFermentableEditor :
   public QDialog,
   public Ui::stockUseIngredientEditor,
   public EnumeratedItemEditorBase<StockUseFermentableEditor, StockUseFermentable>,
   public EditorBase<StockUseFermentableEditor,
                     StockUseFermentable,
                     StockUseFermentableEditorOptions> {
   Q_OBJECT

public:
   void retranslateUi();

   EDITOR_COMMON_DECL(StockUseFermentable, StockUseFermentableEditorOptions)
};

#define StockUseHopEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
class StockUseHopEditor :
   public QDialog,
   public Ui::stockUseIngredientEditor,
   public EnumeratedItemEditorBase<StockUseHopEditor, StockUseHop>,
   public EditorBase<StockUseHopEditor,
                     StockUseHop,
                     StockUseHopEditorOptions> {
   Q_OBJECT

public:
   void retranslateUi();

   EDITOR_COMMON_DECL(StockUseHop, StockUseHopEditorOptions)
};

#define StockUseMiscEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
class StockUseMiscEditor :
   public QDialog,
   public Ui::stockUseIngredientEditor,
   public EnumeratedItemEditorBase<StockUseMiscEditor, StockUseMisc>,
   public EditorBase<StockUseMiscEditor,
                     StockUseMisc,
                     StockUseMiscEditorOptions> {
   Q_OBJECT

public:
   void retranslateUi();

   EDITOR_COMMON_DECL(StockUseMisc, StockUseMiscEditorOptions)
};

#define StockUseSaltEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
class StockUseSaltEditor :
   public QDialog,
   public Ui::stockUseIngredientEditor,
   public EnumeratedItemEditorBase<StockUseSaltEditor, StockUseSalt>,
   public EditorBase<StockUseSaltEditor,
                     StockUseSalt,
                     StockUseSaltEditorOptions> {
   Q_OBJECT

public:
   void retranslateUi();

   EDITOR_COMMON_DECL(StockUseSalt, StockUseSaltEditorOptions)
};

#define StockUseYeastEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
class StockUseYeastEditor :
   public QDialog,
   public Ui::stockUseIngredientEditor,
   public EnumeratedItemEditorBase<StockUseYeastEditor, StockUseYeast>,
   public EditorBase<StockUseYeastEditor,
                     StockUseYeast,
                     StockUseYeastEditorOptions> {
   Q_OBJECT

public:
   void retranslateUi();

   EDITOR_COMMON_DECL(StockUseYeast, StockUseYeastEditorOptions)
};

#endif
