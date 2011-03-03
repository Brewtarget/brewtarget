/*
 * HopEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _HOPEDITOR_H
#define	_HOPEDITOR_H

class HopEditor;

#include "ui_hopEditor.h"
#include "hop.h"
#include "observable.h"

class HopEditor : public QDialog, private Ui::hopEditor, Observer
{
   Q_OBJECT

public:
   HopEditor( QWidget *parent=0 );
   void setHop( Hop* h );

public slots:
   void save();
   void clearAndClose();

private:
   Hop* obsHop;

   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
   void showChanges();
};

#endif	/* _HOPEDITOR_H */

