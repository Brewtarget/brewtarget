/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Folder.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_FOLDER_H
#define MODEL_FOLDER_H
#pragma once

#include <QObject>
#include <QString>


/*!
 * \class Folder
 *
 * \brief Item needed to implement folders in the trees.
 *
 *        TODO: For the moment, this is implemented in a bit of a short-cut way where a folder exists by virtue of being
 *        referenced from a Fermentable/Hop/etc object.  Amongst other things this means empty folders are not possible
 *        and renaming folders is a bit tricky.  At some point we would like to address this and make Folders
 *        first-class descendants of \c NamedEntity with their own database table.
 *
 * This provides a generic item from which the trees are built. Since most of
 * the actions required are the same regardless of the item being stored (e.g.
 * hop or equipment), this class considers them all the same.
 *
 * A few notes, just so I don't have to rethink all of this. This class
 * generates NO signals. It catches signals from changes made in the database,
 * but I currently don't think it needs to signal anything itself. I reserve
 * the right to change this just as soon as I actually start working the
 * trees.
 *
 */
class Folder : public QObject {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   Folder(QString const & fullPath = "");
   Folder(Folder const & other);

   virtual ~Folder();

   // Getters
   QString name() const;
   QString path() const;
   QString fullPath() const;

   //Setter
   void setName(QString var);
   void setPath(QString var);
   void setfullPath(QString var);

   //! \brief do some tests to see if the provided name is mine
   bool isFolder(QString name);

private:
   QString m_name = "";
   QString m_path = "";
   QString m_fullPath = "";

};

#endif
