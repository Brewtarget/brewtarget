/*
* MashComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2012.
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

/*!
 * \class MashComboBox
 * \author Philip G. Lee
 *
 * \brief A combobox that is a view class for a list of mashes.
 *
 * Well, it's not exactly
 * a strict view class, since it contains model-related methods, so we should
 * prune out the model methods at some point.
 */
class MashComboBox : public QComboBox
{
  Q_OBJECT
  
   public:
      MashComboBox(QWidget* parent=0);
      virtual ~MashComboBox() {}
      //! Add a mash to the internal model's list.
      void addMash(Mash* m);
      //! Remove a mash from the internal model's list.
      void removeMash(Mash* m);
      //! Set the current index to that which corresponds to \b m.
      void setIndexByMash(Mash* m);
      //! Set the index.
      void setIndex(int ndx);
      //! Remove all mashs from the internal model.
      void removeAllMashs();
      //! Populate the internal model with all the database mashs.
      void repopulateList();
      
      //! \return the selected mash.
      Mash* getSelectedMash();
      
   public slots:
      void changed(QMetaProperty, QVariant);
   private:
      QList<Mash*> mashObs;
};

#endif
