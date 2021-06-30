/*
 * RecipeFormatter.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef RECIPE_FORMATTER_H
#define RECIPE_FORMATTER_H

class RecipeFormatter;

#include <QString>
#include <QStringList>
#include <QObject>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextBrowser>
#include <QDialog>
#include <QFile>
#include <QList>
#include <QStringList>
#include "model/Recipe.h"
#include "PrintAndPreviewDialog.h"

/*!
 * \class RecipeFormatter
 * \author Philip G. Lee
 *
 * \brief View class that creates various text versions of a recipe.
 */
class RecipeFormatter : public QObject
{
   Q_OBJECT

   friend class PrintAndPreviewDialog;
public:

   RecipeFormatter(QObject* parent=nullptr);
   ~RecipeFormatter();
   //! Set the recipe to view.
   void setRecipe(Recipe* recipe);
   //! Get a plaintext view.
   QString getTextFormat();
   //! Get an html view.
   QString getHTMLFormat();
   //! Get a whole mess of html views
   QString getHTMLFormat( QList<Recipe*> recipes );
   //! Get a BBCode view. Why is this here?
   QString getBBCodeFormat();
   //! Generate a tooltip for a recipe
   QString getToolTip(Recipe* rec);
   QString getToolTip(Style* style);
   QString getToolTip(Equipment* kit);
   QString getToolTip(Fermentable* ferm);
   QString getToolTip(Hop* hop);
   QString getToolTip(Misc* misc);
   QString getToolTip(Yeast* yeast);
   QString getToolTip(Water* water);
   QString getLabelToolTip();
   //! Get the maximum number of characters in a list of strings.
   unsigned int getMaxLength( QStringList* list );
   //! Prepend a string with spaces until its final length is the given length.
   QString padToLength( const QString &str, unsigned int length );
   //! Same as \b padToLength but with multiple strings.
   void padAllToMaxLength( QStringList* list, unsigned int padding=2 );
   //! Return the text wrapped with the given length
   QString wrapText( const QString &text, int wrapLength );

public slots:
   //! Put the plaintext view onto the clipboard.
   void toTextClipboard();

private:
   QString getTextSeparator();

   QString buildHTMLHeader();
   QString buildStatTableHtml();
   QString buildStatTableTxt();
   QList<QStringList> buildStatList();
   QString buildFermentableTableHtml();
   QString buildFermentableTableTxt();
   QList<QStringList> buildFermentableList();
   QString buildHopsTableHtml();
   QString buildHopsTableTxt();
   QList<QStringList> buildHopsList();
   QString buildYeastTableHtml();
   QString buildYeastTableTxt();
   QList<QStringList> buildYeastList();
   QString buildMashTableHtml();
   QString buildMashTableTxt();
   QList<QStringList> buildMashList();
   QString buildMiscTableHtml();
   QString buildMiscTableTxt();
   QList<QStringList> buildMiscList();
   QString buildNotesHtml();
   QString buildInstructionTableTxt();
   /* I am not sure how I want to implement these yet.
    * I might just include the salts in the instructions table. Until I decide
    * these stay commented out
   QString buildWaterTableHtml();
   QString buildWaterTableTxt();
   QString buildSaltTableHtml();
   QString buildSaltTableTxt();
   */
   QString buildBrewNotesHtml();
   QString buildNotesString();
   QString buildTasteNotesString();
   QString buildBrewNotesTxt();
   QString buildHTMLFooter();

   QList<Hop*> sortHopsByTime(Recipe* rec);
   QList<Fermentable*> sortFermentablesByWeight(Recipe* rec);

   QString* textSeparator;
   Recipe* rec;
};

#endif /*RECIPE_FORMATTER_H*/
