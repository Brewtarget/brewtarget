/*
 * WaterEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef WATEREDITOR_H
#define WATEREDITOR_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>

#include "ui_waterEditor.h"

// Forward declarations.
class Water;

/*!
 * \class WaterEditor
 *
 * \brief View/controller class for creating and modifying water records.
 */
class WaterEditor : public QDialog, public Ui::waterEditor {
   Q_OBJECT
public:
   WaterEditor(QWidget *parent = nullptr, QString const editorName = "Unnamed");
   virtual ~WaterEditor();

   /*!
    * Sets the water we want to observe.
    *
    * \param water If \c std::nullopt then stop observing
    */
   void setWater(std::optional<std::shared_ptr<Water>> water);

   void newWater(QString folder);

public slots:
   void showChanges(QMetaProperty const * prop = nullptr);
   void inputFieldModified();
   void changed(QMetaProperty, QVariant);
   void saveAndClose();
   void clearAndClose();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
