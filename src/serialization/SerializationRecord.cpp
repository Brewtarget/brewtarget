/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/SerializationRecord.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#include "serialization/SerializationRecord.h"

#include <QRegularExpression>

SerializationRecord::SerializationRecord() :
   m_namedParameterBundle{NamedParameterBundle::OperationMode::NotStrict},
   m_namedEntity{nullptr},
   m_includeInStats{true} {
   return;
}

SerializationRecord::~SerializationRecord() = default;

NamedParameterBundle const & SerializationRecord::getNamedParameterBundle() const {
   return this->m_namedParameterBundle;
}

std::shared_ptr<NamedEntity> SerializationRecord::getNamedEntity() const {
   return this->m_namedEntity;
}

void SerializationRecord::constructNamedEntity() {
   // Base class does not have a NamedEntity or a container, so nothing to do
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
///   qCritical() <<
///      Q_FUNC_INFO << this->m_recordDefinition.m_namedEntityClassName << "this->m_namedParameterBundle:" <<
///      this->m_namedParameterBundle;
   qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
   Q_ASSERT(false && "Trying to construct named entity for base record");
   return;
}

int SerializationRecord::storeNamedEntityInDb() {
   Q_ASSERT(false && "Trying to store named entity for base record");
   return -1;
}

void SerializationRecord::deleteNamedEntityFromDb() {
   Q_ASSERT(false && "Trying to delete named entity for base record");
   return;
}

[[nodiscard]] bool SerializationRecord::isDuplicate() {
   // Base class does not have a NamedEntity so nothing to check
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to check for duplicate NamedEntity when there is none");
   return false;
}

void SerializationRecord::normaliseName() {
   // Base class does not have a NamedEntity so nothing to normalise
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to normalise name of NamedEntity when there is none");
   return;
}

void SerializationRecord::setContainingEntity([[maybe_unused]] std::shared_ptr<NamedEntity> containingEntity) {
   // Base class does not have a NamedEntity or a container, so nothing to do
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to set containing entity when there is none");
   return;
}

void SerializationRecord::modifyClashingName(QString & candidateName) {
   //
   // First, see whether there's already a (n) (ie "(1)", "(2)" etc) at the end of the name (with or without
   // space(s) preceding the left bracket.  If so, we want to replace this with " (n+1)".  If not, we try " (1)".
   //
   int duplicateNumber = 1;
   QRegularExpression const & nameNumberMatcher = NamedEntity::getDuplicateNameNumberMatcher();
   QRegularExpressionMatch match = nameNumberMatcher.match(candidateName);
   QString matchedValue = match.captured(1);
   if (matchedValue.size() > 0) {
      // There's already some integer in brackets at the end of the name, extract it, add one, and truncate the name
      duplicateNumber = matchedValue.toInt() + 1;
      candidateName.truncate(match.capturedStart(1));
   }
   candidateName += QString(" (%1)").arg(duplicateNumber);
   return;
}
