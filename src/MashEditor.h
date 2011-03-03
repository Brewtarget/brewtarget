/*
 * MashEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MASHEDITOR_H
#define	_MASHEDITOR_H

class MashEditor;

#include <QDialog>
#include "ui_mashEditor.h"
#include "recipe.h"

class MashEditor : public QDialog, public Ui::mashEditor
{
   Q_OBJECT
public:
   MashEditor( QWidget *parent = 0 );

public slots:
   void showEditor();
   void closeEditor();
   void saveAndClose();
   void fromEquipment();
   void setRecipe(Recipe* recipe);

private:
   void showChanges();
   void clear();
   Recipe* rec;

};

#endif	/* _MASHEDITOR_H */

