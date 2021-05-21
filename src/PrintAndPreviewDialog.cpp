
/*
 * PrintAndPreviewDialog.cpp is part of Brewtarget, and is Copyright the following
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
#include "PrintAndPreviewDialog.h"
#include <QDebug>
#include <QPainter>
#include <QFont>

PrintAndPreviewDialog::PrintAndPreviewDialog ( QWidget *parent)
   : QDialog(parent)
{
   setupUi(this);

   _printer = new QPrinter();
   previewWidget = new QPrintPreviewWidget( _printer , this);
   PrintAndPreviewDialog::verticalLayout_PrintPreviewWidget->addWidget ( previewWidget );

   previewWidget->setFont(QFont("Arial", 18, QFont::Bold));

   connect(previewWidget, &QPrintPreviewWidget::paintRequested, this, &PrintAndPreviewDialog::printDocument);
   previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   previewWidget->setZoomMode(QPrintPreviewWidget::FitInView);
   previewWidget->show();
}

void PrintAndPreviewDialog::printDocument(QPrinter * printer){
   QPainter painter(printer);
   painter.setFont(QFont("Arial", 32, QFont::Bold));
   painter.drawText(20,60,QString("PAGE 1, PAGE 1, PAGE 1, PAGE 1"));
   printer->newPage();
   painter.setFont(QFont("Arial", 32, QFont::Bold));
   painter.drawText(20,60,QString("PAGE 2, PAGE 2, PAGE 2, PAGE 2"));
   painter.end();
}
