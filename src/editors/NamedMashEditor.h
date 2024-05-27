/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/NamedMashEditor.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef EDITORS_NAMEDMASHEDITOR_H
#define EDITORS_NAMEDMASHEDITOR_H
#pragma once

#include <QDialog>
#include <QMetaProperty>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "ui_namedMashEditor.h"
#include "editors/MashStepEditor.h"
#include "listModels/MashListModel.h"
#include "listModels/EquipmentListModel.h"
#include "NamedEntitySortProxyModel.h"
#include "tableModels/MashStepTableModel.h"


// Forward declarations.
class Recipe;
class Mash;
class Equipment;

/*!
 * \class NamedMashEditor
 *
 * \brief View/controller dialog for editing a mash.
 *
 *        See also \c MashEditor
 */
class NamedMashEditor : public QDialog, public Ui::namedMashEditor {
   Q_OBJECT
public:
   NamedMashEditor(QWidget * parent = nullptr,
                   MashStepEditor* editor = nullptr,
                   bool singleMashEditor = false);

public slots:
   //! show the editor window
   void showEditor();
   //! close the editor window, abandoning changes
   void closeEditor();
   //! save the changes and close the editor
   void saveAndClose();
   //! Set the mash we wish to view/edit.
   void setMash(std::shared_ptr<Mash> mash);

   //! add a mash step to the mash
   void addMashStep();
   //! remove a mash step to the mash
   void removeMashStep();
   //! move a mash step up if you can
   void moveMashStepUp();
   //! move a mash step down if you can
   void moveMashStepDown();

   //! Get the tun mass and sp. heat from the equipment combobox
   void fromEquipment(QString const & name);
   //! set the current mash being edited according to the combobox
   void mashSelected(QString const & name);
   //! delete the mash
   void removeMash();
   //! Our standard changed slot
   void changed(QMetaProperty,QVariant);

private:
   //! The mash we are watching
   std::shared_ptr<Mash> m_mashObs;
   //! The mash list model for the combobox
   MashListModel * m_mashListModel;
   //! The table model
   MashStepTableModel * m_mashStepTableModel;
   //! and the mash step edit. Don't know if we need this one
   MashStepEditor * m_mashStepEditor;
   //! This is getting fun!
   EquipmentListModel* m_equipListModel;

   //! Show any changes made. This will get ugly, I am sure
   void showChanges(QMetaProperty * prop = nullptr);
   //! Clear the mash and delete all of its steps
   void clear();
   //! Convenience method to make sure just one item was selected
   bool justOne(QModelIndexList selected);

};

#endif
