/*
 * MashWizard.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MASHWIZARD_H
#define	_MASHWIZARD_H

class MashWizard;

#include "ui_mashWizard.h"
#include <QDialog>
#include "recipe.h"
#include "unit.h"

class MashWizard : public QDialog, public Ui::mashWizard
{
   Q_OBJECT
public:
   MashWizard(QWidget* parent=0);
   void setRecipe(Recipe* rec);

public slots:
   void wizardry(); // Do what the wizard is supposed to do.
   void show();

private:
   Recipe* recObs;
   Unit *weightUnit;
   Unit *volumeUnit;
};

#endif	/* _MASHWIZARD_H */

