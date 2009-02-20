/*
 * YeastEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#ifndef _YEASTEDITOR_H
#define	_YEASTEDITOR_H

class YeastEditor;

#include "ui_yeastEditor.h"
#include "yeast.h"
#include "observable.h"

class YeastEditor : public QDialog, private Ui::yeastEditor, Observer
{
   Q_OBJECT

public:
   YeastEditor( QWidget *parent=0 );
   void setYeast( Yeast* y );

public slots:
   void save();
   void clearAndClose();

private:
   Yeast* obsYeast;

   virtual void notify(Observable* notifier); // Inherited from Observer
   void showChanges();
};

#endif	/* _YEASTEDITOR_H */

