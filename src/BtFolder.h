/*
 * BtFolder.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#ifndef BTFOLDER_H
#define BTFOLDER_H

class BtFolder;

#include <QSharedPointer>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QWidget>
#include <QVector>
#include <QObject>


/*!
 * \class BtFolder
 * \author Mik Firestone
 *
 * \brief Item needed to implement folders in the trees
 *
 * This provides a generic item from which the trees are built. Since most of
 * the actions required are the same regardless of the item being stored (e.g.
 * hop or equipment), this class considers them all the same. 
 *
 * A few notes, just so I don't have to rethink all of this. This class
 * generates NO signals. It catches signals from cahnges made in the database,
 * but I currently don't think it needs to signal anything itself. I reserve
 * the right to chnage this just as soon as I actually start working the
 * trees.
 *
 */
class BtFolder : public QObject
{

   Q_OBJECT

public:

   BtFolder();
   BtFolder(BtFolder const& other);

   virtual ~BtFolder() {}

   // Getters
   QString name() const;
   QString path() const;
   QString fullPath() const;

   //Setter
   void setName( QString var );
   void setPath( QString var );
   void setfullPath( QString var );

   //! \brief do some tests to see if the provided name is mine
   bool isFolder( QString name );

private:
   QString _name;
   QString _path;
   QString _fullPath;

};

#endif /* BTFOLDER_H */
