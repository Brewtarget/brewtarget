/*
 * FermentableEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _FERMENTABLEEDITOR_H
#define   _FERMENTABLEEDITOR_H

class FermentableEditor;

#include <QDialog>
#include "ui_fermentableEditor.h"

// Forward declarations.
class Fermentable;

class FermentableEditor : public QDialog, private Ui::fermentableEditor
{
   Q_OBJECT

public:
   FermentableEditor( QWidget *parent=0 );
   virtual ~FermentableEditor() {}
   void setFermentable( Fermentable* f );

public slots:
   void save();
   void clearAndClose();
   void changed(QMetaProperty,QVariant);

private:
   Fermentable* obsFerm;
   void showChanges(QMetaProperty* metaProp = 0);
};

#endif   /* _FERMENTABLEEDITOR_H */

