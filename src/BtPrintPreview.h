/*
 * BtPrintPreview.h is part of Brewtarget, and is copyright the following
 * authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef BTPRINTPREVIEW_H
#define BTPRINTPREVIEW_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>

class QEvent;
class QFile;
class QPrinter;
class QWidget;

/**
 * \brief Provides a print preview for a printable document
 */
class BtPrintPreview : public QDialog {
   Q_OBJECT
public:
   BtPrintPreview(QWidget * parent = nullptr);
   ~BtPrintPreview();

   /*!
    * \brief Set the content
    * \param content HTML marked-up text
    */
   void setContent(QString const & content);

   /*!
    * \brief Prints the content
    * \param printer The printer to print to, should not be \c NULL
    */
   void print(QPrinter* printer);

   /*!
    * \brief Exports the content to a file
    * \param file The output file opened for writing
    */
   void exportHtml(QFile* file);

   virtual void changeEvent(QEvent * event);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

};
#endif
