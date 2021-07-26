/*
 * MashComboBox.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef MASHCOMBOBOX_H
#define MASHCOMBOBOX_H
#pragma once

#include <memory>

#include <QComboBox>
#include <QList>
#include <QMetaProperty>
#include <QVariant>
#include <QWidget>

// Forward declaration.
class Mash;

/*!
 * \class MashComboBox
 *
 * \brief A combobox that is a view class for a list of mashes.
 *
 * Well, it's not exactly
 * a strict view class, since it contains model-related methods, so we should
 * prune out the model methods at some point.
 */
class MashComboBox : public QComboBox {
   Q_OBJECT

public:
   MashComboBox(QWidget * parent = 0);
   virtual ~MashComboBox() = default;

   //! Set the current index to that which corresponds to \b m.
   void setIndexByMash(Mash * m);
   //! Set the index.
   void setIndex(int ndx);
   //! Remove all mashs from the internal model.
   void removeAllMashs();
   //! Populate the internal model with all the database mashs.
   void repopulateList();

   //! \return the selected mash.
   Mash * getSelectedMash();

   //! Add a mash to the internal model's list.
   void add(Mash * m);
   //! Remove a mash from the internal model's list.
   void remove(Mash * m);

public slots:
   void changed(QMetaProperty, QVariant);
   //! Add a mash to the list.
   void addMash(int mashId);
   //! Remove a mash from the list.
   void removeMash(int mashId, std::shared_ptr<QObject> object);

private:
   QList<Mash *> mashObs;
};

#endif
