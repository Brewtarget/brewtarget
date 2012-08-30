/*
 * EquipmentEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EQUIPMENTEDITOR_H
#define   _EQUIPMENTEDITOR_H

class EquipmentEditor;

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_equipmentEditor.h"

// Forward declarations
class Equipment;
class EquipmentListModel;

/*!
 * \class EquipmentEditor
 * \author Philip G. Lee
 *
 * \brief This is a dialog that edits an equipment record.
 */
class EquipmentEditor : public QDialog, public Ui::equipmentEditor
{
   Q_OBJECT

public:
   //! \param singleEquipEditor true if you do not want the necessary elements for viewing all the database elements.
   EquipmentEditor( QWidget *parent=0, bool singleEquipEditor=false );
   virtual ~EquipmentEditor() {}
   //! Edit the given equipment.
   void setEquipment( Equipment* e );

public slots:
   //! Save the changes to the equipment.
   void save();
   //! Create a new equipment record.
   void newEquipment();
   //! Delete the equipment from the database.
   void removeEquipment();
   //! Set the equipment to default values.
   void clear();
   //! Set defaults and close the dialog.
   void clearAndClose();
   //! Set absorption back to default.
   void resetAbsorption();

   //! Edit the equipment currently selected in our combobox.
   void equipmentSelected();
   //! Depending on the sender, set the correct equipment field to the appropriate value.
   void updateRecord();
   //! If state==Qt::Checked, set the "calculate boil volume" checkbox. Otherwise, unset.
   void updateCheckboxRecord(int state);
   //! Set a dirty bit if the notes change.
   void changedText();
   
   void changed(QMetaProperty,QVariant);

private:
   Equipment* obsEquip;
   EquipmentListModel* equipmentListModel;
   
   bool changeText;
   bool eventFilter(QObject *object, QEvent* event);
   void showChanges();
};

#endif   /* _EQUIPMENTEDITOR_H */

