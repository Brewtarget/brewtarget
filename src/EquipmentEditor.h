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
#include "ui_equipmentEditor.h"
#include "equipment.h"
#include "observable.h"

class EquipmentEditor : public QDialog, public Ui::equipmentEditor, public Observer
{
   Q_OBJECT

public:
   EquipmentEditor( QWidget *parent=0 );
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

private:
   Equipment* obsEquip;

   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
   void showChanges();
};

#endif   /* _EQUIPMENTEDITOR_H */

