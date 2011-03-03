/*
 * MiscEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MISCEDITOR_H
#define	_MISCEDITOR_H

#include "ui_miscEditor.h"
#include "misc.h"
#include "observable.h"

class MiscEditor : public QDialog, private Ui::miscEditor, Observer
{
   Q_OBJECT

public:
   MiscEditor( QWidget *parent=0 );
   void setMisc( Misc* m );
   
public slots:
   void save();
   void clearAndClose();
   
private:
   Misc* obsMisc;
   
   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
   void showChanges();
};

#endif	/* _MISCEDITOR_H */

