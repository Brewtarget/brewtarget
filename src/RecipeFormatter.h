/*
* RecipeFormatter.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2012.
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
#include <QPrinter>
#include <QPrintDialog>
#include <QWebView>
#include <QDialog>
#include <QFile>
#include "recipe.h"

/*!
 * \class RecipeFormatter
 * \author Philip G. Lee
 *
 * \brief View class that creates various text versions of a recipe.
 */
class RecipeFormatter : public QObject
{
   Q_OBJECT
public:

   enum { PRINT, PREVIEW, HTML, NUMACTIONS };
   
   RecipeFormatter(QObject* parent=0);
   ~RecipeFormatter();
   //! Set the recipe to view.
   void setRecipe(Recipe* recipe);
   //! Get a plaintext view.
   QString getTextFormat();
   //! Get an html view.
   QString getHTMLFormat();
   //! Get a BBCode view.
   QString getBBCodeFormat();
   //! Get the maximum number of characters in a list of strings.
   unsigned int getMaxLength( QStringList* list );
   //! Prepend a string with spaces until its final length is the given length.
   QString padToLength( QString str, unsigned int length );
   //! Same as \b padToLength but with multiple strings.
   void padAllToMaxLength( QStringList* list );
   
   //! Send a printable version to the printer.
   void print(QPrinter *mainPrinter, QPrintDialog* dialog, int action = PRINT, QFile* outFile=0);

public slots:
   //! Put the plaintext view onto the clipboard.
   void toTextClipboard();
   
private:
   QString getTextSeparator();

   QString buildTitleTable();
   QString buildFermentableTable();
   QString buildHopsTable();
   QString buildYeastTable();
   QString buildMashTable();
   QString buildMiscTable();
   QString buildNotes();
   QString buildInstructionTable();
   QString buildBrewNotes();
   QString getCSS();

   QList<Hop*> sortHopsByTime(Recipe* rec);
   QList<Fermentable*> sortFermentablesByWeight(Recipe* rec);
   
   QString* textSeparator;
   Recipe* rec;

   QPrinter* printer;
   QWebView* doc;
   QDialog* docDialog;
   QString cssName;

private slots:
   bool loadComplete(bool ok);
};

#endif /*RECIPE_FORMATTER_H*/
