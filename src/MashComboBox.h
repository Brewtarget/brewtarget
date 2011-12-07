/*
* MashComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MASHCOMBOBOX_H
#define  _MASHCOMBOBOX_H

class MashComboBox;

#include <QComboBox>
#include <QWidget>
#include <QList>
#include <QMetaProperty>
#include <QVariant>

// Forward declaration.
class Mash;

class MashComboBox : public QComboBox
{
  Q_OBJECT
  
   public:
      MashComboBox(QWidget* parent=0);
      virtual ~MashComboBox() {}
      void addMash(Mash* m);
      void setIndexByMash(Mash* m);
      void setIndex(int ndx);
      void removeAllMashs();
      void repopulateList();
      
      Mash* getSelectedMash();
      
   public slots:
      void changed(QMetaProperty, QVariant);
   private:
      QList<Mash*> mashObs;
};

#endif
