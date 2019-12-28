/*
 * MiscEditor.h is part of Brewtarget, and is Copyright the following
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

#ifndef _MISCEDITOR_H
#define   _MISCEDITOR_H

#include <QDialog>
#include "ui_miscEditor.h"
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Misc;

/*!
 * \class MiscEditor
 * \author Philip G. Lee
 *
 * \brief View/controller dialog for editing miscs.
 */
class MiscEditor : public QDialog, private Ui::miscEditor
{
   Q_OBJECT

public:
   MiscEditor( QWidget *parent=nullptr );
   virtual ~MiscEditor() {}
   //! Set the misc we wish to view/edit.
   void setMisc( Misc* m );

public slots:
   //! Save changes.
   void save();
   //! Clear dialog and close.
   void clearAndClose();
   void changed(QMetaProperty,QVariant);
//   void updateField();

private:
   Misc* obsMisc;
   /*! Updates the UI elements effected by the \b metaProp of
    *  the misc we are watching. If \b metaProp is null,
    *  then update all the UI elements at once.
    */
   void showChanges(QMetaProperty* metaProp = nullptr);
};

#endif   /* _MISCEDITOR_H */

