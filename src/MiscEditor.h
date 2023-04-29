/*
 * MiscEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef MISCEDITOR_H
#define MISCEDITOR_H
#pragma once

#include "ui_miscEditor.h"

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Misc;

/*!
 * \class MiscEditor
 *
 * \brief View/controller dialog for editing miscs.
 */
class MiscEditor : public QDialog, private Ui::miscEditor {
   Q_OBJECT

public:
   MiscEditor( QWidget *parent=nullptr );
   virtual ~MiscEditor();
   //! Set the misc we wish to view/edit.
   void setMisc( Misc* m );
  //! Create a misc with folders

public slots:
   //! Save changes.
   void save();
   //! Clear dialog and close.
   void clearAndClose();
   //! Add a new misc
   void newMisc(QString folder = "");
   void changed(QMetaProperty, QVariant);
   void setIsWeight(bool state);

private:
   Misc* obsMisc;
   /*! Updates the UI elements effected by the \b metaProp of
    *  the misc we are watching. If \b metaProp is null,
    *  then update all the UI elements at once.
    */
   void showChanges(QMetaProperty* metaProp = nullptr);
};

#endif
