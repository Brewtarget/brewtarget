/*
* RecipeFormatter.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef RECIPE_FORMATTER_H
#define RECIPE_FORMATTER_H

class RecipeFormatter;

#include <QString>
#include <QStringList>
#include <QObject>
#include "recipe.h"

class RecipeFormatter : public QObject
{
   Q_OBJECT
public:
   RecipeFormatter();
   void setRecipe(Recipe* recipe);
   QString getTextFormat();
   QString getHTMLFormat();
   QString getBBCodeFormat();
   unsigned int getMaxLength( QStringList* list );
   QString padToLength( QString str, unsigned int length );
   void padAllToMaxLength( QStringList* list );
   
public slots:
   void toTextClipboard();
   
private:
   QString getTextSeparator();
   QString* textSeparator;
   Recipe* rec;
};

#endif /*RECIPE_FORMATTER_H*/