/*
 * AboutDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
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
#ifndef _ABOUTDIALOG_H
#define _ABOUTDIALOG_H

#include <QDialog>
#include <QEvent>
#include <QLabel>

/*!
 * \class AboutDialog
 *
 * \brief Simple "about" dialog for Brewtarget.
 */
class AboutDialog : public QDialog
{
   Q_OBJECT

public:
   AboutDialog(QWidget* parent=0);

   void changeEvent(QEvent* event);

   //! \name Public UI Variables
   //! @{
   QLabel* label;
   //! @}

private:
   void doLayout();
   void retranslateUi();
};

#endif   /* _ABOUTDIALOG_H */
