/*
 * BrewDayWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#ifndef _BREWDAYWIDGET_H
#define	_BREWDAYWIDGET_H

class BrewDayWidget;

#include "ui_brewDayWidget.h"
#include <QWidget>
#include <QSize>
#include "observable.h"
#include "recipe.h"

class BrewDayWidget : public QWidget, public Ui::brewDayWidget, public Observer
{
   Q_OBJECT
public:
   BrewDayWidget(QWidget* parent=0);
   void setRecipe(Recipe* rec);

   virtual QSize sizeHint() const; // From QWidget
   virtual void notify(Observable *notifier, QVariant info); // From Observer.

public slots:
   void insertInstruction();
   void removeSelectedInstruction();

private:
   void showChanges();
   void repopulateListWidget();
   void clear();
   
   Recipe* recObs;
};

#endif	/* _BREWDAYWIDGET_H */

