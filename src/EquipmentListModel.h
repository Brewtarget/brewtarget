/*
 * EquipmentListModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef _EQUIPMENTLISTMODEL_H
#define _EQUIPMENTLISTMODEL_H
#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Equipment;
class Recipe;

/*!
 * \class EquipmentListModel
 * \author Philip G. Lee
 *
 * \brief Model for a list of equipments.
 */
class EquipmentListModel : public QAbstractListModel
{
   Q_OBJECT
   
public:
   EquipmentListModel(QWidget* parent = 0);
   
   //! \brief Reimplemented from QAbstractListModel.
   virtual int rowCount( QModelIndex const& parent = QModelIndex() ) const;
   //! \brief Reimplemented from QAbstractListModel.
   virtual QVariant data( QModelIndex const& index, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractListModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

   void observeRecipe(Recipe* rec);
   //! \brief Add many equipments to the list.
   void addEquipments(QList<Equipment*> equips);
   //! \brief Remove all equipments from the list.
   void removeAll();
   
   //! \brief Return the equipment at the index in the list.
   Equipment* at(int ndx);
   //! \brief Return the index of a particular equipment. DEPRECATED.
   int indexOf(Equipment* e);
   //! \brief Return the index of a particular equipment.
   QModelIndex find(Equipment* e);
   
public slots:
   void recChanged(QMetaProperty,QVariant);
   void equipChanged(QMetaProperty,QVariant);
   //! Add an equipment to the list.
   void addEquipment(Equipment* equipment);
   //! Remove an equipment from the list.
   void removeEquipment(Equipment* equipment);
   
private:
   QList<Equipment*> equipments;
   Recipe* recipe;
   
   void repopulateList();
};

#endif /* _EQUIPMENTLISTMODEL_H */
