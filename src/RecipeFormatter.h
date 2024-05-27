/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * RecipeFormatter.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef RECIPE_FORMATTER_H
#define RECIPE_FORMATTER_H
#pragma once

#include <memory> // For PImpl

#include <QList>
#include <QObject>

#include "model/Recipe.h"

class Style      ;
class Equipment  ;
class Fermentable;
class Hop        ;
class Misc       ;
class Yeast      ;
class Water      ;

/*!
 * \class RecipeFormatter
 *
 * \brief View class that creates various text versions of a recipe.
 */
class RecipeFormatter : public QObject {
   Q_OBJECT

public:
   RecipeFormatter(QWidget * parent = nullptr);
   virtual ~RecipeFormatter();

   //! Set the recipe to view.
   void setRecipe(Recipe* recipe);

   //! Get a whole mess of html views
   QString getHtmlFormat(QList<Recipe*> recipes);

   QString getHtmlFormat();
   QString buildHtmlHeader();
   QString buildHtmlFooter();

   //! Get a BBCode view. Why is this here?
   QString getBBCodeFormat();

   //! Generate a tooltip for a recipe
   QString getToolTip(Recipe      * rec);
   QString getToolTip(Style       * style);
   QString getToolTip(Equipment   * kit);
   QString getToolTip(Fermentable * ferm);
   QString getToolTip(Hop         * hop);
   QString getToolTip(Misc        * misc);
   QString getToolTip(Yeast       * yeast);
   QString getToolTip(Water       * water);

public slots:
   //! Put the plaintext view onto the clipboard.
   void toTextClipboard();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
