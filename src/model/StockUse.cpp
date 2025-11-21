/*======================================================================================================================
 * model/StockUse.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
 =====================================================================================================================*/
#include "model/StockUse.h"

#include "database/ObjectStoreWrapper.h"
#include "utils/AutoCompare.h"

QString StockUse::localisedName() { return tr("Stock Use"); }
QString StockUse::localisedName_brewNote    () { return tr("Brew Note"    ); }
QString StockUse::localisedName_brewNoteId  () { return tr("Brew Note ID" ); }
QString StockUse::localisedName_comment     () { return tr("Comment"      ); }
QString StockUse::localisedName_date        () { return tr("Date"         ); }
QString StockUse::localisedName_ownerId     () { return tr("Owner ID"     ); }
QString StockUse::localisedName_quantityUsed() { return tr("Quantity Used"); }
QString StockUse::localisedName_reason      () { return tr("Reason"       ); }

EnumStringMapping const StockUse::reasonStringMapping {
   {StockUse::Reason::Used    , "used"    },
   {StockUse::Reason::Lost    , "lost"    },
   {StockUse::Reason::Disposed, "disposed"},
};

EnumStringMapping const StockUse::reasonDisplayNames {
   {StockUse::Reason::Used    , tr("Used"    )},
   {StockUse::Reason::Lost    , tr("Lost"    )},
   {StockUse::Reason::Disposed, tr("Disposed")},
};


bool StockUse::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StockUse const & rhs = static_cast<StockUse const &>(other);
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_date        , PropertyNames::StockUse::date        , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_reason      , PropertyNames::StockUse::reason      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_quantityUsed, PropertyNames::StockUse::quantityUsed, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_comment     , PropertyNames::StockUse::comment     , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_brewNoteId  , PropertyNames::StockUse::brewNoteId  , propertiesThatDiffer)
   );
}

TypeLookup const StockUse::typeLookup {
   "StockUse",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(StockUse, date        , m_date        , NonPhysicalQuantity::Date         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockUse, reason      , m_reason      , ENUM_INFO(StockUse::reason)),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockUse, quantityUsed, m_quantityUsed, NonPhysicalQuantity::Dimensionless),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockUse, comment     , m_comment     , NonPhysicalQuantity::String       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockUse, brewNoteId  , m_brewNoteId  ),
      PROPERTY_TYPE_LOOKUP_NO_MV(StockUse, brewNote    , brewNote      ),
   },
   {&NamedEntity::typeLookup}
};

StockUse::StockUse(QString name) :
   NamedEntity{name},
   m_date             {QDate::currentDate()},
   m_reason           {StockUse::Reason::Used},
   m_quantityUsed     {0.0},
   m_comment          {""},
   m_brewNoteId       {-1} {
   return;
}

StockUse::StockUse(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   SET_REGULAR_FROM_NPB(m_date        , namedParameterBundle, PropertyNames::StockUse::date        ),
   SET_REGULAR_FROM_NPB(m_reason      , namedParameterBundle, PropertyNames::StockUse::reason      ),
   SET_REGULAR_FROM_NPB(m_quantityUsed, namedParameterBundle, PropertyNames::StockUse::quantityUsed),
   SET_REGULAR_FROM_NPB(m_comment     , namedParameterBundle, PropertyNames::StockUse::comment     ),
   SET_REGULAR_FROM_NPB(m_brewNoteId  , namedParameterBundle, PropertyNames::StockUse::brewNoteId  ) {
   return;
}

StockUse::StockUse(StockUse const & other) :
   NamedEntity{other},
   // NB: We do not copy m_cachedQuantityRemaining!
   m_date        {other.m_date        },
   m_reason      {other.m_reason      },
   m_quantityUsed{other.m_quantityUsed},
   m_comment     {other.m_comment     },
   m_brewNoteId  {other.m_brewNoteId  } {
   return;
}

StockUse::~StockUse() = default;

std::strong_ordering StockUse::operator<=>(StockUse const & other) const {
   // If two StockUses have the same Date, then we assume the one with the smaller ID is earlier
   auto const result = Utils::Auto3WayCompare(this->m_date, other.m_date,
                                 this->key()     , other.key());
///   qDebug() << Q_FUNC_INFO << "LHS:" << *this << result << "RHS:" << other;
   return result;
}

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QDate            StockUse::date        () const { return this->m_date        ;}
StockUse::Reason StockUse::reason      () const { return this->m_reason      ;}
double           StockUse::quantityUsed() const { return this->m_quantityUsed;}
QString          StockUse::comment     () const { return this->m_comment     ;}
int              StockUse::brewNoteId  () const { return this->m_brewNoteId  ;}

std::shared_ptr<BrewNote> StockUse::brewNote() const {
   return ObjectStoreWrapper::getRelational<BrewNote>(*this, this->m_brewNoteId);
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void StockUse::setDate        (QDate            const   val) { SET_AND_NOTIFY( PropertyNames::StockUse::date        , this->m_date        , val); }
void StockUse::setReason      (StockUse::Reason const   val) { SET_AND_NOTIFY( PropertyNames::StockUse::reason      , this->m_reason      , val); }
void StockUse::setQuantityUsed(double           const   val) { SET_AND_NOTIFY( PropertyNames::StockUse::quantityUsed, this->m_quantityUsed, val); }
void StockUse::setComment     (QString          const & val) { SET_AND_NOTIFY( PropertyNames::StockUse::comment     , this->m_comment     , val); }
void StockUse::setBrewNoteId  (int              const   val) { SET_AND_NOTIFY( PropertyNames::StockUse::brewNoteId  , this->m_brewNoteId  , val); }

void StockUse::setBrewNote(std::shared_ptr<BrewNote> const val) {
   // For now at least we don't care about the return value as we don't listen for any signals from the BrewNote
   ObjectStoreWrapper::setRelational(*this, val, this->m_brewNoteId);
   return;
}
