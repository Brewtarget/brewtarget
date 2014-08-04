/*
 * WaterEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_waterEditor.h"

// Forward declarations.
class Water;

/*!
 * \class WaterEditor
 * \author Philip G. Lee
 *
 * \brief View/controller class for modifying water records.
 */
class WaterEditor : public QDialog, public Ui::waterEditor
{
    Q_OBJECT
public:
    WaterEditor(QWidget *parent = 0);
    virtual ~WaterEditor() {}

    /*!
     * Sets the water we want to observe.
     */
    void setWater(Water* water);

 public slots:
    void showChanges(QMetaProperty* prop = 0);
    void saveAndClose();
    void changed(QMetaProperty,QVariant);

private:
    Water* obs; // Observed water.
};

#endif // WATEREDITOR_H
