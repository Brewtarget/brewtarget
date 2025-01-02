/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Fermentation.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "model/Fermentation.h"

#include "model/NamedParameterBundle.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Fermentation.cpp"
#endif

QString Fermentation::localisedName() { return tr("Fermentation"); }

bool Fermentation::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentation const & rhs = static_cast<Fermentation const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_description == rhs.m_description &&
      this->m_notes       == rhs.m_notes       &&
      // Parent classes have to be equal too
      this->FolderBase<Fermentation>::doIsEqualTo(rhs) &&
      this->StepOwnerBase<Fermentation, FermentationStep>::doIsEqualTo(rhs)
   );
}

ObjectStore & Fermentation::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Fermentation>::getInstance();
}

TypeLookup const Fermentation::typeLookup {
   "Fermentation",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentation::description, Fermentation::m_description, NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentation::notes      , Fermentation::m_notes      , NonPhysicalQuantity::String),

      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Fermentation::primary          , Fermentation::primary          ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Fermentation::secondary        , Fermentation::secondary        ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Fermentation::tertiary         , Fermentation::tertiary         ),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Fermentation>::typeLookup),
    std::addressof(StepOwnerBase<Fermentation, FermentationStep>::typeLookup)
   }
};
static_assert(std::is_base_of<FolderBase<Fermentation>, Fermentation>::value);

//==================================================== CONSTRUCTORS ====================================================

Fermentation::Fermentation(QString name) :
   NamedEntity{name, true},
   FolderBase<Fermentation>{},
   StepOwnerBase<Fermentation, FermentationStep>{},
   m_description  {""},
   m_notes        {""} {

   CONSTRUCTOR_END
   return;
}

Fermentation::Fermentation(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity             {namedParameterBundle},
   FolderBase<Fermentation>{namedParameterBundle},
   StepOwnerBase<Fermentation, FermentationStep>{},
   SET_REGULAR_FROM_NPB (m_description, namedParameterBundle, PropertyNames::Fermentation::description),
   SET_REGULAR_FROM_NPB (m_notes      , namedParameterBundle, PropertyNames::Fermentation::notes      ) {

   CONSTRUCTOR_END
   return;
}

Fermentation::Fermentation(Fermentation const & other) :
   NamedEntity{other},
   FolderBase<Fermentation>{other},
   StepOwnerBase<Fermentation, FermentationStep>{other},
   m_description  {other.m_description},
   m_notes        {other.m_notes      } {

   CONSTRUCTOR_END
   return;
}

Fermentation::~Fermentation() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QString Fermentation::description() const { return this->m_description; }
QString Fermentation::notes      () const { return this->m_notes      ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Fermentation::setDescription (QString const & val) { SET_AND_NOTIFY(PropertyNames::Fermentation::description, this->m_description, val); return; }
void Fermentation::setNotes       (QString const & val) { SET_AND_NOTIFY(PropertyNames::Fermentation::notes      , this->m_notes      , val); return; }

std::shared_ptr<FermentationStep> Fermentation::primary  () const { return this->stepAt(1); }
std::shared_ptr<FermentationStep> Fermentation::secondary() const { return this->stepAt(2); }
std::shared_ptr<FermentationStep> Fermentation::tertiary () const { return this->stepAt(3); }
void Fermentation::setPrimary  (std::shared_ptr<FermentationStep> val) { this->setStepAt(val, 1); return; }
void Fermentation::setSecondary(std::shared_ptr<FermentationStep> val) { this->setStepAt(val, 2); return; }
void Fermentation::setTertiary (std::shared_ptr<FermentationStep> val) { this->setStepAt(val, 3); return; }

void Fermentation::acceptStepChange(QMetaProperty prop, QVariant val) {
   this->doAcceptStepChange(this->sender(), prop, val, {});
   return;
}

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Fermentation)

// Insert fermentationer-plate wrapper functions that call down to StepOwnerBase
STEP_OWNER_COMMON_CODE(Fermentation, fermentation)
