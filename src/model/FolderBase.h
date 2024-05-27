/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/FolderBase.h is part of Brewtarget, and is copyright the following authors 2024:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef MODEL_FOLDERBASE_H
#define MODEL_FOLDERBASE_H
#pragma once

#include <QString>

#include "model/NamedParameterBundle.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeLookup.h"

//╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::FolderBase { BtStringConst const property{#property}; }
AddPropertyName(folder)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌

/**
 * \brief Adds a "folder" property to a class.  Typically this is used only on classes that are not dependent on others
 *        (see comments in model/NamedEntity.h).
 */
template<class Derived> class FolderBasePhantom;
template<class Derived>
class FolderBase : public CuriouslyRecurringTemplateBase<FolderBasePhantom, Derived> {

protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   FolderBase() :
      m_folder{""} {
      return;
   }

   FolderBase(NamedParameterBundle const & namedParameterBundle) :
      SET_REGULAR_FROM_NPB(m_folder, namedParameterBundle, PropertyNames::FolderBase::folder, "") {
      return;
   }

   FolderBase(FolderBase const & other) :
      m_folder{other.m_folder} {
      return;
   }

   ~FolderBase() = default;

   QString const & getFolder() const {
      return this->m_folder;
   }

   void doSetFolder(QString const & val) {
      this->derived().setAndNotify(PropertyNames::FolderBase::folder, this->m_folder, val);
      return;
   }

protected:
   QString m_folder;
};

template<class Derived>
TypeLookup const FolderBase<Derived>::typeLookup {
   "FolderBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::FolderBase::folder,
       TypeInfo::construct<decltype(FolderBase<Derived>::m_folder)>(
          PropertyNames::FolderBase::folder,
          TypeLookupOf<decltype(FolderBase<Derived>::m_folder)>::value
       )}
   },
   // Parent class lookup: none as we are at the top of this arm of the inheritance tree
   {}
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define FOLDER_BASE_DECL(Derived) \
   /* This allows FolderBase to call protected and private members of Derived. */           \
   friend class FolderBase<Derived>;                                                        \
                                                                                            \
   public:                                                                                  \
   /*=========================== FB "GETTER" MEMBER FUNCTIONS ===========================*/ \
   virtual QString const & folder() const;                                                  \
   /*=========================== FB "SETTER" MEMBER FUNCTIONS ===========================*/ \
   virtual void setFolder(QString const & val);                                             \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define FOLDER_BASE_COMMON_CODE(Derived) \
   /*====================================== FB "GETTER" MEMBER FUNCTIONS ======================================*/ \
   QString const & Derived::folder() const { return this->getFolder(); }                                          \
   /*====================================== FB "SETTER" MEMBER FUNCTIONS ======================================*/ \
   void Derived::setFolder(QString const & val) { this->doSetFolder(val); return; }                               \

#endif
