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

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Folder { inline BtStringConst const property{#property}; }
AddPropertyName(fullPath)
AddPropertyName(path)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

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
class Folder : public NamedEntity {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_path();
   static QString localisedName_fullPath();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Folder(QString const & fullPath = "");
   Folder(NamedParameterBundle const & namedParameterBundle);
   Folder(Folder const & other);

   virtual ~Folder();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(QString path     READ path     WRITE setPath    )

   /**
    * \brief \c fullPath is just \c path plus \c name, so this property is merely a convenience
    */
   Q_PROPERTY(QString fullPath READ fullPath WRITE setFullPath)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QString path() const;
   QString fullPath() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setPath(QString var);
   void setFullPath(QString var);

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   QString m_path = "";

};

#endif
