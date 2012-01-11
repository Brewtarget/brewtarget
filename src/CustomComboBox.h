/*
* CustomComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2011.
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

#ifndef _CUSTOMCOMBOBOX_H
#define _CUSTOMCOMBOBOX_H

#include <QString>
#include <QComboBox>

/*!
* This class is a more flexible combo box. Does not show any text.
* \author Philip G. Lee (rocketman768@gmail.com).
*/
class CustomComboBox : public QComboBox
{
   Q_OBJECT
public:
   CustomComboBox(QWidget* parent = 0);
   virtual ~CustomComboBox(){}
   
   //! Reimplemented from QComboBox to allow the popup to be independently sized.
   void showPopup();
   
   //! Reimplemented from QComboBox to not show any text.
   virtual void paintEvent(QPaintEvent*);
};

/*
class CustomComboBox : public QPushButton
{
   Q_OBJECT
   
public:
   CustomComboBox(QWidget* parent = 0);
   virtual ~CustomComboBox(){};
   
   void setModel( QAbstractItemModel* model );
   int currentIndex();
   
   virtual void paintEvent(QPaintEvent*);
signals:
   //! Emitted when something is selected in the list.
   void activated(int index);
   //! Emitted when something is selected in the list.
   void activated(QString const& currentText);
   
public slots:
   
private slots:
   void handleToggle(bool);
   void handleRowChanged(int);
   void handleTextChanged(QString const&);

private:
   QListView* listView;
   QFrame* listFrame;
   QDesktopWidget* desktop;
   
   QRect computeListRect();
};
*/

#endif /*_CUSTOMCOMBOBOX_H*/
