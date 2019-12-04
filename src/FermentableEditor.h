/*
 * FermentableEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FERMENTABLEEDITOR_H
#define   _FERMENTABLEEDITOR_H

class FermentableEditor;

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_fermentableEditor.h"

// Forward declarations.
class Fermentable;

/*!
 * \class FermentableEditor
 * \author Philip G. Lee
 *
 * \brief Fermentable view/controller dialog that allows you to edit Fermentables.
 */
class FermentableEditor : public QDialog, private Ui::fermentableEditor
{
   Q_OBJECT

public:
   FermentableEditor( QWidget *parent=nullptr );
   virtual ~FermentableEditor() {}
   void setFermentable( Fermentable* f );

public slots:
   void save();
   void clearAndClose();

private:
   Fermentable* obsFerm;
   /*! Updates the UI elements effected by the \b metaProp of
    *  the fermentable we are watching. If \b metaProp is null,
    *  then update all the UI elements at once.
    */
   void showChanges(QMetaProperty* metaProp = nullptr);
};

#endif   /* _FERMENTABLEEDITOR_H */
