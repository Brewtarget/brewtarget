/*
 * WaterEditor.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef WATEREDITOR_H
#define WATEREDITOR_H

#include <QDialog>
#include "ui_waterEditor.h"
#include "observable.h"
#include "water.h"

class WaterEditor : public QDialog, public Ui::waterEditor, public Observer
{
    Q_OBJECT
public:
    WaterEditor(QWidget *parent = 0);
    ~WaterEditor();

    /*!
     * Sets the water we want to observe.
     */
    void setWater(Water* water);

    virtual void notify(Observable *notifier, QVariant info); // From Observer.

 public slots:
    void showChanges();
    void saveAndClose();

private:
    Water* obs; // Observed water.
};

#endif // WATEREDITOR_H
