/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Folder.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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

#include "database/ObjectStoreWrapper.h"
#include "model/NamedEntity.h"
#include "model/FolderPropertyBase.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::FolderCommon { inline BtStringConst const property{#property}; }
AddPropertyName(fullPath)
AddPropertyName(path)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \class FolderCommon
 *
 * \brief Non-templated base for \c Folder class (see below) needed to implement folders in the trees.
 */
class FolderCommon : public NamedEntity {
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

   explicit FolderCommon(QString const & name);
   explicit FolderCommon(NamedParameterBundle const & namedParameterBundle);
   FolderCommon(FolderCommon const & other);

   ~FolderCommon() override;

   //=================================================== PROPERTIES ====================================================
   //! The ID of the folder (if any) that contains this one
   Q_PROPERTY(int containedInFolderId   READ containedInFolderId   WRITE setContainedInFolderId)

   //! \brief The \c path is the names of all the parent folders concatenated together with '/'
   Q_PROPERTY(QString path     READ path     /*WRITE setPath*/    )

   //! \brief \c fullPath is just \c path plus \c name, so this property is merely a convenience
   Q_PROPERTY(QString fullPath READ fullPath /*WRITE setFullPath*/)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   [[nodiscard]] virtual int containedInFolderId() const = 0;
   [[nodiscard]] virtual QString path() const = 0;
   [[nodiscard]] virtual QString fullPath() const = 0;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   virtual void setContainedInFolderId(int const var) = 0;

protected:
   bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
};


template<class NE>
class Folder : public FolderCommon, public FolderPropertyBase<Folder<NE>, IsFolder::Yes> {
   //×××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××
   // We can't use the Q_OBJECT or Q_PROPERTY macros in this class (as it's a template).  Nor can we use the
   // FOLDER_BASE_DECL or FOLDER_BASE_COMMON_CODE macros (because template classes don't have the same declaration /
   // definition split as regular ones).  So we manually insert the same member functions here.
   friend class FolderPropertyBase<Folder<NE>, IsFolder::Yes>;
public:
   [[nodiscard]] int containedInFolderId() const override { return this->doContainedInFolderId(); }
   void setContainedInFolderId(int const val) override { this->doSetContainedInFolderId(val); return; }
   static std::shared_ptr<Folder<NE>> getById(int const id) { return ObjectStoreWrapper::getById<Folder<NE>>(id); }
   static Folder<NE> * getByIdRaw(int const id) { return ObjectStoreWrapper::getByIdRaw<Folder<NE>>(id); }
   //
   // NB: If we make changes to FOLDER_BASE_DECL and/or FOLDER_BASE_COMMON_CODE, the above needs to be kept in sync.
   //×××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××

   [[nodiscard]] static QString staticClassName() {
      return QString("Folder<%1>").arg(NE::staticMetaObject.className());
   }
   [[nodiscard]] QString className() const override {
      return Folder::staticClassName();
   }

   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   explicit Folder(QString const & name) :
      FolderCommon{name},
      FolderPropertyBase<Folder<NE>, IsFolder::Yes>{} {
      CONSTRUCTOR_END
      return;
   }

   explicit Folder(NamedParameterBundle const & namedParameterBundle) :
      FolderCommon{namedParameterBundle},
      FolderPropertyBase<Folder<NE>, IsFolder::Yes>{namedParameterBundle} {
      CONSTRUCTOR_END
      return;
   }

   Folder(Folder const & other) :
      FolderCommon{other},
      FolderPropertyBase<Folder<NE>, IsFolder::Yes>{other} {
      CONSTRUCTOR_END
      return;
   }

   [[nodiscard]] QString path() const override {
      return this->doPath("");
   }

   [[nodiscard]] QString fullPath() const override {
      return this->doPath(this->name());
   }

   [[nodiscard]] QString extraLogInfo() const override {
      return QString("%1 (Parent # %2)").arg(this->fullPath()).arg(this->containedInFolderId());
   }

   /**
    * @return All the parents of the current folder in natural order (immediate parent will be last item, grandparent
    *         will be second last item, etc).
    */
   QList<Folder<NE> *> parentFolders() const {
      QList<Folder<NE> *> parents;
      // The folder() and folderRaw() functions return the folder (if any) to which the item belongs, including if the
      // item is itself a folder.
      for (Folder<NE> * parent = this->containedInFolderRaw(); parent; parent = parent->containedInFolderRaw()) {
         //
         // It would be more efficient to append to the list and have the caller deal with the fact that it's in
         // reverse order.  However, the lists are not large (eg a dozen nested folders would be a LOT for a typical
         // user), so the efficiency gain would not be worth the increased complexity IMHO.
         //
         qDebug() << Q_FUNC_INFO << "Inserting" << parent;
         parents.insert(0, parent);
      }
      return parents;
   }

   /**
    * @return As \c parentFolders, but with the current folder (ie this) appended
    */
   QList<Folder<NE> *> folderPath() const {
      QList<Folder<NE> *> folders = this->parentFolders();
      folders.append(const_cast<Folder<NE> *>(this));
      return folders;
   }

   /**
    * \brief Returns a \c Folder for the supplied full path (which must not be empty), creating any folders (and parent
    *        folders) as needed.
    *
    * @param path
    * @return
    */
   static Folder<NE> * createFromPath(QString const & path) {
      Folder<NE> * folder = nullptr;
      QStringList pieces = path.split("/", Qt::SkipEmptyParts);
      Q_ASSERT(!pieces.empty());
      while(!pieces.empty()) {
         auto const parentId = folder ? folder->key() : -1;
         QString const folderName = pieces.takeFirst();
         std::shared_ptr<Folder<NE>> subFolder = ObjectStoreWrapper::findFirstMatching<Folder<NE>>(
            [&](std::shared_ptr<Folder<NE>> val) {
               // Note that we are assuming that the "null" ID is always -1 here (rather than allowing any negative
               // value as null
               if (parentId != val->containedInFolderId()) {
                  return false;
               }
               // If parents match, we just need to compare names
               return (val->name() == folderName);
            }
         );
         // If we didn't find the sub-folder, we need to create it
         if (!subFolder) {
            subFolder = std::make_shared<Folder<NE>>(folderName);
            subFolder->setContainedInFolderId(folder ? folder->key() : -1);
            ObjectStoreWrapper::insert(subFolder);
         }
         folder = subFolder.get();
      }

      return folder;
   }

   /**
    * Get either the NE or the Folder<NE> children of this folder
    * @return
    */
   template<class ChildType>
   QList<std::shared_ptr<ChildType>> children() const requires (std::same_as<ChildType, NE> ||
                                                                std::same_as<ChildType, Folder<NE>>) {
      auto const folderId = this->key();
      return ObjectStoreWrapper::findAllMatching<ChildType>(
         [folderId] (std::shared_ptr<ChildType> obj) { return (obj->containedInFolderId() == folderId); }
      );
   }

   /**
    * Convenience wrappers for above
    */
   QList<std::shared_ptr<NE>        > childItems  () const { return this->children<NE        >(); }
   QList<std::shared_ptr<Folder<NE>>> childFolders() const { return this->children<Folder<NE>>(); }

protected:
   [[nodiscard]] ObjectStore & getObjectStoreTypedInstance() const override {
      return ObjectStoreTyped<Folder<NE>>::getInstance();
   }

private:

   /**
    *
    * @param startWith
    * @return
    */
   [[nodiscard]] QString doPath(QString const & startWith) const {
      QString myPath = startWith;
      for (auto myParent = this->containedInFolderRaw(); myParent; myParent = myParent->containedInFolderRaw()) {
         myPath = QString("%1/%2").arg(myParent->name()).arg(myPath);
      }
      return myPath;
   }

};

template<class NE>
TypeLookup const Folder<NE>::typeLookup {
   "Folder",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      // For the moment at least, we have no additional properties over those of our parents
      //
   },
   // Parent class lookups
   {&FolderCommon::typeLookup, &FolderPropertyBase<Folder<NE>>::typeLookup}
};

#endif
