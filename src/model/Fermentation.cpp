/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Fermentation.cpp is part of Brewtarget, and is copyright the following authors 2023-2026:
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

#include "model/Folder.h"
#include "model/NamedParameterBundle.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Fermentation.cpp"
#endif

QString Fermentation::localisedName() { return tr("Fermentation"); }
QString Fermentation::localisedName_description    () { return tr("Description"     ); }
QString Fermentation::localisedName_krausenHandling() { return tr("Krausen Handling"); }
QString Fermentation::localisedName_notes      () { return tr("Notes"      ); }
QString Fermentation::localisedName_primary    () { return tr("Primary"    ); }
QString Fermentation::localisedName_secondary  () { return tr("Secondary"  ); }
QString Fermentation::localisedName_tertiary   () { return tr("Tertiary"   ); }

EnumStringMapping const Fermentation::krausenHandlingStringMapping {
   {Fermentation::KrausenHandling::MixedIn                     , "mixed in"                        },
   {Fermentation::KrausenHandling::MinorDeposits               , "minor deposits"                  },
   {Fermentation::KrausenHandling::MediumDeposits              , "medium deposits"                 },
   {Fermentation::KrausenHandling::HeavyDeposits               , "heavy deposits"                  },
   {Fermentation::KrausenHandling::VeryHeavyDeposits           , "very heavy deposits"             },
   {Fermentation::KrausenHandling::BlownOffSlowFermentation    , "blown off, slow fermentation"    },
   {Fermentation::KrausenHandling::BlownOffNormalFermentation  , "blown off, normal fermentation"  },
   {Fermentation::KrausenHandling::BlownOffVigorousFermentation, "blown off, vigorous fermentation"},
};

EnumStringMapping const Fermentation::krausenHandlingDisplayNames {
   {Fermentation::KrausenHandling::MixedIn                     , tr("Mixed In"                        )},
   {Fermentation::KrausenHandling::MinorDeposits               , tr("Minor Deposits on Fermenter"     )},
   {Fermentation::KrausenHandling::MediumDeposits              , tr("Medium Deposits on Fermenter"    )},
   {Fermentation::KrausenHandling::HeavyDeposits               , tr("Heavy Deposits on Fermenter"     )},
   {Fermentation::KrausenHandling::VeryHeavyDeposits           , tr("Very Heavy Deposits on Fermenter")},
   {Fermentation::KrausenHandling::BlownOffSlowFermentation    , tr("Blown Off, Slow Fermentation"    )},
   {Fermentation::KrausenHandling::BlownOffNormalFermentation  , tr("Blown Off, Normal Fermentation"  )},
   {Fermentation::KrausenHandling::BlownOffVigorousFermentation, tr("Blown Off, Vigorous Fermentation")},
};

bool Fermentation::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentation const & rhs = static_cast<Fermentation const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_description, PropertyNames::Fermentation::description, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_notes      , PropertyNames::Fermentation::notes      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_krausenHandling, PropertyNames::Fermentation::krausenHandling, propertiesThatDiffer) &&
      // Parent classes have to be equal too
      this->FolderPropertyBase<Fermentation>                     ::doCompareWith(rhs, propertiesThatDiffer) &&
      this->LockablePropertyBase<Fermentation>::doCompareWith(rhs, propertiesThatDiffer) &&
      this->StepOwnerBase<Fermentation, FermentationStep>::doCompareWith(rhs, propertiesThatDiffer)
   );
}

ObjectStore & Fermentation::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Fermentation>::getInstance();
}

TypeLookup const Fermentation::typeLookup {
   "Fermentation",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentation, description, m_description, NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentation, notes      , m_notes      , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentation, krausenHandling, m_krausenHandling, ENUM_INFO(Fermentation::krausenHandling)),
      PROPERTY_TYPE_LOOKUP_NO_MV(Fermentation, primary    , primary      ),
      PROPERTY_TYPE_LOOKUP_NO_MV(Fermentation, secondary  , secondary    ),
      PROPERTY_TYPE_LOOKUP_NO_MV(Fermentation, tertiary   , tertiary     ),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderPropertyBase<Fermentation>::typeLookup),
    std::addressof(LockablePropertyBase<Fermentation>::typeLookup),
    std::addressof(StepOwnerBase<Fermentation, FermentationStep>::typeLookup)
   }
};
static_assert(std::is_base_of<FolderPropertyBase<Fermentation>, Fermentation>::value);

//==================================================== CONSTRUCTORS ====================================================

Fermentation::Fermentation(QString name) :
   NamedEntity{name},
   FolderPropertyBase<Fermentation>{},
   LockablePropertyBase<Fermentation>{},
   StepOwnerBase<Fermentation, FermentationStep>{},
   m_description    {""},
   m_notes          {""},
   m_krausenHandling{std::nullopt} {

   CONSTRUCTOR_END
   return;
}

Fermentation::Fermentation(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity             {namedParameterBundle},
   FolderPropertyBase<Fermentation>{namedParameterBundle},
   LockablePropertyBase<Fermentation>{namedParameterBundle},
   StepOwnerBase<Fermentation, FermentationStep>{},
   SET_REGULAR_FROM_NPB (m_description, namedParameterBundle, PropertyNames::Fermentation::description),
   SET_REGULAR_FROM_NPB (m_notes      , namedParameterBundle, PropertyNames::Fermentation::notes      ),
   SET_OPT_ENUM_FROM_NPB(m_krausenHandling, Fermentation::KrausenHandling, namedParameterBundle, PropertyNames::Fermentation::krausenHandling) {

   CONSTRUCTOR_END
   return;
}

Fermentation::Fermentation(Fermentation const & other) :
   NamedEntity{other},
   FolderPropertyBase<Fermentation>{other},
   LockablePropertyBase<Fermentation>{other},
   StepOwnerBase<Fermentation, FermentationStep>{other},
   m_description    {other.m_description    },
   m_notes          {other.m_notes          },
   m_krausenHandling{other.m_krausenHandling} {

   CONSTRUCTOR_END
   return;
}

Fermentation::~Fermentation() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QString Fermentation::description() const { return this->m_description; }
QString Fermentation::notes      () const { return this->m_notes      ; }
std::optional<Fermentation::KrausenHandling> Fermentation::krausenHandling     () const { return this->m_krausenHandling; }
std::optional<int>                           Fermentation::krausenHandlingAsInt() const { return Optional::toOptInt(this->m_krausenHandling); }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Fermentation::setDescription (QString const & val) { SET_AND_NOTIFY(PropertyNames::Fermentation::description, this->m_description, val); return; }
void Fermentation::setNotes       (QString const & val) { SET_AND_NOTIFY(PropertyNames::Fermentation::notes      , this->m_notes      , val); return; }
void Fermentation::setKrausenHandling     (std::optional<KrausenHandling> const val) { SET_AND_NOTIFY(PropertyNames::Fermentation::krausenHandling, this->m_krausenHandling, val); return; }
void Fermentation::setKrausenHandlingAsInt(std::optional<int>             const val) { this->setKrausenHandling(Optional::fromOptInt<KrausenHandling>(val)); return; }

std::shared_ptr<FermentationStep> Fermentation::primary  () const { return this->stepAt(1); }
std::shared_ptr<FermentationStep> Fermentation::secondary() const { return this->stepAt(2); }
std::shared_ptr<FermentationStep> Fermentation::tertiary () const { return this->stepAt(3); }
void Fermentation::setPrimary  (std::shared_ptr<FermentationStep> val) { this->setStepAt(val, 1); return; }
void Fermentation::setSecondary(std::shared_ptr<FermentationStep> val) { this->setStepAt(val, 2); return; }
void Fermentation::setTertiary (std::shared_ptr<FermentationStep> val) { this->setStepAt(val, 3); return; }

void Fermentation::acceptSetMemberChange(QMetaProperty prop, QVariant val) {
   this->doAcceptSetMemberChange(this->sender(), prop, val);
   return;
}

// This class supports NamedEntity::numRecipesUsedIn
IMPLEMENT_NUM_RECIPES_USED_IN(Fermentation)

// Boilerplate code for FolderPropertyBase
FOLDER_BASE_COMMON_CODE(Fermentation)

// Boilerplate code for LockablePropertyBase
LOCKABLE_BASE_COMMON_CODE(Fermentation)

// Insert fermentationer-plate wrapper functions that call down to StepOwnerBase
STEP_OWNER_COMMON_CODE(Fermentation, fermentation)