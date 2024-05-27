/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/BoilEditor.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef EDITORS_BOILEDITOR_H
#define EDITORS_BOILEDITOR_H
#pragma once

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_boilEditor.h"

// Forward declarations.
class Recipe;
class Boil;

/*!
 * \class BoilEditor
 *
 * \brief View/controller dialog for editing a boil.
 *
 *        See also \c NamedBoilEditor
 */
class BoilEditor : public QDialog, public Ui::boilEditor {
   Q_OBJECT
public:
   BoilEditor(QWidget * parent = nullptr);
   ~BoilEditor();

public slots:
   void showEditor();
   void closeEditor();
   void saveAndClose();
   //! Set the boil we wish to view/edit.
   void setBoil(std::shared_ptr<Boil> boil);
   void setRecipe(Recipe* r);

   void changed(QMetaProperty,QVariant);
private:
   void showChanges(QMetaProperty* prop = nullptr);
   void clear();
   Recipe* m_rec;
   std::shared_ptr<Boil> m_boilObs;
};

#endif
