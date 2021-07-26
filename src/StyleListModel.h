/*
 * StyleListModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#ifndef STYLELISTMODEL_H
#define STYLELISTMODEL_H
#pragma once

#include <memory>

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Style;
class Recipe;

/*!
 * \class StyleListModel
 *
 * \brief Model for a list of styles.
 */
class StyleListModel : public QAbstractListModel {
   Q_OBJECT

public:
   StyleListModel(QWidget* parent = 0);

   //! Reimplemented from QAbstractListModel.
   virtual int rowCount( QModelIndex const& parent = QModelIndex() ) const;
   //! Reimplemented from QAbstractListModel.
   virtual QVariant data( QModelIndex const& index, int role = Qt::DisplayRole ) const;
   //! Reimplemented from QAbstractListModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

   void addStyles(QList<Style*> s);
   void removeAll();

   Style* at(int ndx);
   int indexOf(Style* s);
   void remove(Style*);

public slots:
   void styleChanged(QMetaProperty,QVariant);
   void addStyle(int styleId);
   void removeStyle(int styleId, std::shared_ptr<QObject> object);

private:
   QList<Style*> styles;
   Recipe* recipe;

   void repopulateList();
};

#endif
