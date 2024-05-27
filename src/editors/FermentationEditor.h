/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/FermentationEditor.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef EDITORS_FERMENTATIONEDITOR_H
#define EDITORS_FERMENTATIONEDITOR_H
#pragma once

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_fermentationEditor.h"

// Forward declarations.
class Recipe;
class Fermentation;

/*!
 * \class FermentationEditor
 *
 * \brief View/controller dialog for editing a fermentation.
 *
 *        See also \c NamedFermentationEditor
 */
class FermentationEditor : public QDialog, public Ui::fermentationEditor {
   Q_OBJECT
public:
   FermentationEditor(QWidget * parent = nullptr);
   ~FermentationEditor();

public slots:
   void showEditor();
   void closeEditor();
   void saveAndClose();
   //! Set the fermentation we wish to view/edit.
   void setFermentation(std::shared_ptr<Fermentation> fermentation);
   void setRecipe(Recipe* r);

   void changed(QMetaProperty,QVariant);
private:
   void showChanges(QMetaProperty* prop = nullptr);
   void clear();
   Recipe* m_rec;
   std::shared_ptr<Fermentation> m_fermentationObs;
};

#endif
