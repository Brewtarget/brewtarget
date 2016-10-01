/*
 * BrewDayScrollWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _BREWDAYSCROLLWIDGET_H
#define _BREWDAYSCROLLWIDGET_H

class BrewDayScrollWidget;

#include "ui_brewDayScrollWidget.h"
#include <QWidget>
#include <QSize>
#include <QTextBrowser>
#include <QPrinter>
#include <QPrintDialog>
#include <QFile>
#include "recipe.h"

/*!
 * \class BrewDayScrollWidget
 * \author Philip G. Lee
 *
 * \brief Widget that displays the brewday info in a scrollable area.
 */
class BrewDayScrollWidget : public QWidget, public Ui::brewDayScrollWidget
{
   Q_OBJECT
public:
   enum { PRINT, PREVIEW, HTML, NUMACTIONS };

   BrewDayScrollWidget(QWidget* parent=0);
   virtual ~BrewDayScrollWidget() {}
   //! \brief Sets the observed recipe.
   void setRecipe(Recipe* rec);

   virtual QSize sizeHint() const; // From QWidget

   /*!
    *  \brief Prints a paper version of the info in this dialog.
    *  Should be moved to its own view class.
    */
   void print(QPrinter* mainPrinter, int action = PRINT, QFile* outFile = 0);

public slots:
   //! Automatically generate a new list of instructions.
   void generateInstructions();
   //! Insert a custom instruction into the recipe.
   void insertInstruction();
   //! Remove the instruction selected in the list view.
   void removeSelectedInstruction();
   //! Push selected instruction up.
   void pushInstructionUp();
   //! Push selected instruction down.
   void pushInstructionDown();

private slots:
   //! \brief Receive notifications from the recipe.
   void acceptChanges( QMetaProperty prop, QVariant value );
   //! \brief Receive changes from instructions.
   void acceptInsChanges( QMetaProperty prop, QVariant value );
   
private:
   //! Update the view.
   void showChanges();
   //! Repopulate the list widget with all the instructions.
   void repopulateListWidget();
   void clear();
   
   QString buildTitleTable(bool includeImage = true);
   QString buildInstructionTable();
   QString buildFooterTable();
   
   Recipe* recObs;
   QPrinter* printer;
   QTextBrowser* doc;
   //! Internal list of recipe instructions, always sorted by instruction number.
   QList<Instruction*> recIns;

   QString cssName;

private slots:
   bool loadComplete(bool ok);
   void showInstruction(int insNdx);
   void saveInstruction();
};

#endif  /* _BREWDAYSCROLLWIDGET_H */

