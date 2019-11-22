/*
 * YeastEditor.h is part of Brewtarget, and is Copyright the following
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

#ifndef _YEASTEDITOR_H
#define   _YEASTEDITOR_H

class YeastEditor;

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_yeastEditor.h"

// Forward declarations.
class Yeast;

/*!
 * \class YeastEditor
 * \author Philip G. Lee
 *
 * \brief View/controller dialog for modifying yeasts.
 */
class YeastEditor : public QDialog, private Ui::yeastEditor
{
   Q_OBJECT

public:
   YeastEditor( QWidget *parent=nullptr );
   virtual ~YeastEditor() {}
   //! Set the yeast we want to modify.
   void setYeast( Yeast* y );

public slots:
   void save();
   void clearAndClose();
   void changed(QMetaProperty,QVariant);

private:
   Yeast* obsYeast;

   void showChanges(QMetaProperty* prop = nullptr);
};

#endif   /* _YEASTEDITOR_H */

