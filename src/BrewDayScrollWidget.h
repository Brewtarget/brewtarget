/*
 * BrewDayScrollWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef BREWDAYSCROLLWIDGET_H
#define BREWDAYSCROLLWIDGET_H
#pragma once

#include "ui_brewDayScrollWidget.h"

#include <QFile>
#include <QPrinter>
#include <QSize>
#include <QWidget>

#include "model/Recipe.h"
#include "BtPrintPreview.h"

/*!
 * \class BrewDayScrollWidget
 *
 *
 * \brief Widget that displays the brewday info in a scrollable area.
 */
class BrewDayScrollWidget : public QWidget, public Ui::brewDayScrollWidget {
   Q_OBJECT
public:
   BrewDayScrollWidget(QWidget* parent=nullptr);
   virtual ~BrewDayScrollWidget() = default;

   //! \brief Sets the observed recipe.
   void setRecipe(Recipe* rec);

   virtual QSize sizeHint() const; // From QWidget

   /*!
    * \brief Show the print preview
    */
   void printPreview();

   /*!
    * \brief Print
    * \param printer The printer to print to, should not be @c NULL
    */
   void print(QPrinter* printer);

   /*!
    * \brief Export to an HTML document
    * \param file The output file opened for writing
    */
   void exportHtml(QFile* file);


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
   void buildHtml(bool includeImage);

   Recipe* recObs;
   BtPrintPreview* btPrintPreview;
   //! Internal list of recipe instructions, always sorted by instruction number.
   QList<Instruction*> recIns;

   QString cssName;

private slots:
   void showInstruction(int insNdx);
   void saveInstruction();
};

#endif
