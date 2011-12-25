/*
 * EquipmentEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

class EquipmentEditor : public QDialog, public Ui::equipmentEditor
{
   Q_OBJECT

public:
   //! \param singleEquipEditor true if you do not want the necessary elements for viewing all the database elements.
   EquipmentEditor( QWidget *parent=0, bool singleEquipEditor=false );
   virtual ~EquipmentEditor() {}
   void setEquipment( Equipment* e );

public slots:
   void save();
   void newEquipment();
   void removeEquipment();
   void clear();
   void clearAndClose();
   void resetAbsorption();

   void equipmentSelected( const QString& text );
   void updateRecord();
   void updateCheckboxRecord(int state);
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

