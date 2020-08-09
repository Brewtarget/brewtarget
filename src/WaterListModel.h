/*
 * WaterListModel.h is part of Brewtarget, and is Copyright the following
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

#ifndef _WATERLISTMODEL_H
#define _WATERLISTMODEL_H
#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Water;
class Recipe;

/*!
 * \class WaterListModel
 * \author Philip G. Lee
 *
 * \brief Model for a list of waters.
 */
class WaterListModel : public QAbstractListModel
{
   Q_OBJECT
   
public:
   WaterListModel(QWidget* parent = nullptr);
   
   //! \brief Reimplemented from QAbstractListModel.
   virtual int rowCount( QModelIndex const& parent = QModelIndex() ) const;
   //! \brief Reimplemented from QAbstractListModel.
   virtual QVariant data( QModelIndex const& index, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractListModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

   void observeRecipe(Recipe* rec);
   //! \brief Add many waters to the list.
   void addWaters(QList<Water*> waters);
   //! \brief Remove all waters from the list.
   void removeAll();
   
   //! \brief Return the water at the index in the list.
   Water* at(int ndx);
   //! \brief Return the index of a particular water. DEPRECATED.
   int indexOf(Water* w);
   //! \brief Return the index of a particular water.
   QModelIndex find(Water* w);
   
public slots:
   void recChanged(QMetaProperty,QVariant);
   void waterChanged(QMetaProperty,QVariant);
   //! Add an water to the list.
   void addWater(Water* water);
   //! Remove an water from the list.
   void removeWater(Water* water);
   
private:
   QList<Water*> m_waters;
   Recipe* m_recipe;
   
   void repopulateList();
};

#endif /* _WATERLISTMODEL_H */
