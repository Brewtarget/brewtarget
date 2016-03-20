/*
 * BrewDayWidget.h is part of Brewtarget, and is Copyright the following
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

#ifndef _BREWDAYWIDGET_H
#define _BREWDAYWIDGET_H

class BrewDayWidget;

#include "ui_brewDayWidget.h"
#include <QWidget>
#include <QSize>
#include <QTextBrowser>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Recipe;

/*!
 * \class BrewDayWidget
 * \author Philip G. Lee
 *
 * \brief Widget that displays the brewday info. Deprecated I believe.
 */
class BrewDayWidget : public QWidget, public Ui::brewDayWidget
{
   Q_OBJECT
public:
   BrewDayWidget(QWidget* parent=0);
   virtual ~BrewDayWidget() {}
   void setRecipe(Recipe* rec);

   virtual QSize sizeHint() const; // From QWidget

public slots:
   void insertInstruction();
   void removeSelectedInstruction();
   void pushInstructionUp();
   void pushInstructionDown();
   void pushInstructionPrint();
   void pushInstructionPreview();
   void changed(QMetaProperty, QVariant);

private:
   void showChanges();
   void repopulateListWidget();
   void clear();
   QString buildTitleTable();
   QString buildInstructionTable();
   QString buildFooterTable();
   QString getCSS();
   
   Recipe* recObs;
   QPrinter* printer;
   QTextBrowser* doc;

   QString cssName;

private slots:
   bool loadComplete(bool ok);
};

#endif  /* _BREWDAYWIDGET_H */

