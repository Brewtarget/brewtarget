/*
 * BrewDayScrollWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _BREWDAYSCROLLWIDGET_H
#define _BREWDAYSCROLLWIDGET_H

class BrewDayScrollWidget;

#include "ui_brewDayScrollWidget.h"
#include <QWidget>
#include <QSize>
#include <QWebView>
#include "observable.h"
#include "recipe.h"

class BrewDayScrollWidget : public QWidget, public Ui::brewDayScrollWidget, public Observer
{
   Q_OBJECT
public:
   BrewDayScrollWidget(QWidget* parent=0);
   void setRecipe(Recipe* rec);

   virtual QSize sizeHint() const; // From QWidget
   virtual void notify(Observable *notifier, QVariant info); // From Observer.

public slots:
   void generateInstructions();
   void insertInstruction();
   void removeSelectedInstruction();
   void pushInstructionUp();
   void pushInstructionDown();
   void pushInstructionPrint();
   void pushInstructionPreview();

private:
   void showChanges();
   void repopulateListWidget();
   void clear();
   void populateComboBox(QComboBox *comboBox_template);
   QString buildTitleTable();
   QString buildInstructionTable();
   QString buildFooterTable();
   QString getCSS();
   
   Recipe* recObs;
   QPrinter* printer;
   QWebView* doc;

   QString cssName;

private slots:
   bool loadComplete(bool ok);
   void comboSetCSS(const QString name);
};

#endif  /* _BREWDAYSCROLLWIDGET_H */

