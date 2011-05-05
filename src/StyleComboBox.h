/*
 * StyleComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _STYLECOMBOBOX_H
#define   _STYLECOMBOBOX_H

class StyleComboBox;

#include <QComboBox>
#include <QWidget>
#include <QVariant>
#include <QVector>
#include <string>
#include "observable.h"
#include "style.h"
#include "database.h"
#include "recipe.h"

class StyleComboBox : public QComboBox, public MultipleObserver
{
   Q_OBJECT

public:
   StyleComboBox(QWidget* parent=0);
   void startObservingDB();
   void observeRecipe(Recipe* rec);
   void addStyle(Style* style);
   void setIndexByStyleName(QString name);
   void removeAllStyles();
   void repopulateList();

   Style* getSelected();

   virtual void notify(Observable *notifier, QVariant info = QVariant()); // This will get called by observed whenever it changes.

private:
   QVector<Style*> styleObs;
   Recipe* recipeObs;
   Database* dbObs;
};

#endif   /* _STYLECOMBOBOX_H */

