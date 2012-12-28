/*
* MashListModel.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Mash;
class Recipe;

/*!
 * \class MashListModel
 * \author Mik Firestone
 *
 * \brief Model for a list of named mashes
 */
class MashListModel : public QAbstractListModel
{
   Q_OBJECT
   
   public:
      MashListModel(QWidget* parent = 0);
      
      //! Reimplemented from QAbstractListModel.
      virtual int rowCount( QModelIndex const& parent = QModelIndex() ) const;
      //! Reimplemented from QAbstractListModel.
      virtual QVariant data( QModelIndex const& index, int role = Qt::DisplayRole ) const;
      //! Reimplemented from QAbstractListModel.
      virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
     
      //! \brief add the mashes named in the \c QList m 
      void addMashes(QList<Mash*> m);
      //! \brief removes all mashses from the model
      void removeAll();
     
      //! \brief \return the Mash at \c ndx 
      Mash* at(int ndx);
      //! \brief \returns the index of the named \c Mash
      int indexOf(Mash* m);

   public slots:
      void mashChanged(QMetaProperty,QVariant);
      void addMash(Mash*);
      void removeMash(Mash*);
      
   private:
      QList<Mash*> mashes;
      Recipe* recipe;
      
      void repopulateList();
};
