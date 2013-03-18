/*
 * BtTextEdit.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#ifndef BTTEXTEDIT_H
#define BTTEXTEDIT_H


#include <QPlainTextEdit>
#include <QFocusEvent>


/*!
 * \class BtTextEdit
 * \author Mik Firestone
 *
 * \brief This extend QPlainTextEdit such that it only signals when the widget
 * loses focus and the text has been modified within the widget. This, in
 * turn, reduces the number of needless writes we make to the database.
 * 
 */
class BtTextEdit : public QPlainTextEdit
{
   Q_OBJECT

public:

   BtTextEdit(QWidget* parent = 0);
   BtTextEdit(const QString &text, QWidget* parent = 0);

   bool isModified();

   void focusOutEvent(QFocusEvent *e);
   void setPlainText( const QString& text);

public slots:
   void textChanged();

signals:
   void textModified();

private:
   bool wasModified;

};


#endif
