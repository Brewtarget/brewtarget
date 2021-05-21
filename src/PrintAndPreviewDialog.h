/*
 * PrintAndPreviewDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Mattias Måhl <mattias@kejsarsten.com>
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
#ifndef _PRINTANDPREVIEW_H
#define _PRINTANDPREVIEW_H

#include <QDialog>
#include "ui_BTPrintAndPreview.h"
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QWidget>


/*!
 * \class PrintAndPreviewDialog
 * \author Mattias Måhl
 *
 * \brief Handle all printing and saving as PDF/HTML.
 */
class PrintAndPreviewDialog : public QDialog, private Ui::BtPrintAndPreview
{
   Q_OBJECT
public:
   PrintAndPreviewDialog( QWidget *parent );
   virtual ~PrintAndPreviewDialog() {}

   QPrintPreviewWidget* previewWidget;
private:
   QPrinter * _printer = nullptr;

public slots:
   void printDocument(QPrinter * printer);

};
#endif /* _PRINTANDPREVIEW_H */