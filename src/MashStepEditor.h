/*
 * MashStepEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MASHSTEPEDITOR_H
#define   _MASHSTEPEDITOR_H

class MashStepEditor;

#include <QDialog>
#include <QWidget>
#include <QVariant>
#include "observable.h"
#include "mashstep.h"
#include "ui_mashStepEditor.h"

class MashStepEditor : public QDialog, public Ui::mashStepEditor, public Observer
{
   Q_OBJECT
public:
   MashStepEditor(QWidget* parent=0);
   virtual ~MashStepEditor() {}
   virtual void notify(Observable *notifier, QVariant info=QVariant());

public slots:
   void saveAndClose();
   void setMashStep(MashStep* step);
   void close();
   void grayOutStuff(const QString& text);

private:
   void showChanges();
   void clear();
   MashStep* obs;
};

#endif   /* _MASHSTEPEDITOR_H */

