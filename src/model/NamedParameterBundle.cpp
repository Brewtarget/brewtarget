/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/NamedParameterBundle.cpp is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#include "model/NamedParameterBundle.h"

#include <string>
#include <stdexcept>
#include <sstream>

#include <boost/stacktrace.hpp>

#include <QDebug>
#include <QString>
#include <QTextStream>
#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

NamedParameterBundle::NamedParameterBundle(NamedParameterBundle::OperationMode mode) :
   m_parameters{},
   m_mode{mode},
   m_containedBundles{} {
   return;
}

NamedParameterBundle::~NamedParameterBundle() = default;

void NamedParameterBundle::insert(BtStringConst const & propertyName, QVariant const & value) {
   // std::map and std::unordered_map both need an extra set of braces on the call to insert, as we're actually passing
   // in one parameter (std::pair) rather than two.
   this->m_parameters.insert({QString{*propertyName}, value});
   return;
}

void NamedParameterBundle::insert(PropertyPath const & propertyPath, QVariant const & value) {
   NamedParameterBundle * bundle = this;
   auto const & properties = propertyPath.properties();
   for (auto const property : properties) {
      if (property == properties.last()) {
         bundle->insert(*property, value);
      } else {
         bool const newBundle { !bundle->m_containedBundles.contains(**property) };
         // Here, operator[]() silently inserts an item into bundle->m_containedBundles if no item exists with the same
         // key, which is exactly the behaviour we want, and hence why we don't explicitly need to call insert elsewhere
         // on this member variable.
         //
         // Note that property is pointer to BtStringConst, so we need one '*' to dereference it and another to extract the
         // wrapped string.
         bundle = &bundle->m_containedBundles[**property];
         if (newBundle) {
            // If we just created a new bundle, ensure it inherits the top level mode.
            bundle->m_mode = this->m_mode;
         }
      }
   }
   return;
}

bool NamedParameterBundle::contains(BtStringConst const & propertyName) const {
   return this->m_parameters.contains(*propertyName);
}

bool NamedParameterBundle::contains(PropertyPath const & propertyPath) const {
   NamedParameterBundle const * bundle = this;
   auto const & properties = propertyPath.properties();
   for (auto const property : properties) {
      if (property == properties.last()) {
         return bundle->contains(*property);
      }
      if (!bundle->m_containedBundles.contains(**property)) {
         return false;
      }
      bundle = &bundle->m_containedBundles.at(**property);
   }
   Q_UNREACHABLE(); // We should never get here
}

template<typename P>
void NamedParameterBundle::insertIfNotPresent(P const & propertyNameOrPath, QVariant const & value) {
   if (!this->contains(propertyNameOrPath)) {
      this->insert(propertyNameOrPath, value);
   }
   return;
}

//
// Instantiate the above template functions for the types that are going to use them
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template void NamedParameterBundle::insertIfNotPresent<BtStringConst>(BtStringConst const & propertyName, QVariant const & value);
template void NamedParameterBundle::insertIfNotPresent<PropertyPath >(PropertyPath  const & propertyPath, QVariant const & value);

std::size_t NamedParameterBundle::size() const noexcept {
   //
   // This function is only used for logging, so, for simplicitly, we'll count each contained bundle as 1, rather than
   // by the number of parameters it contains.
   //
   return this->m_parameters.size() + this->m_containedBundles.size();
}

bool NamedParameterBundle::isEmpty() const {
   return this->m_parameters.empty() && this->m_containedBundles.empty();
}

QVariant NamedParameterBundle::get(BtStringConst const & propertyName) const {
   if (!this->m_parameters.contains(*propertyName)) {
      QString errorMessage = QString("No value supplied for required parameter, %1.").arg(*propertyName);
      QTextStream errorMessageAsStream(&errorMessage);
      errorMessageAsStream << "  (Parameters in this bundle are ";
      bool wroteFirst = false;
      for (auto const & [key, value] : this->m_parameters) {
         if (!wroteFirst) {
            wroteFirst = true;
         } else {
            errorMessageAsStream << ", ";
         }
         errorMessageAsStream << key;
      }
      errorMessageAsStream << ")";
      if (this->m_mode == NamedParameterBundle::OperationMode::Strict) {
         //
         // We want to throw an exception here because it's a lot less code than checking a return value on every call
         // and, usually, missing required parameter is a coding error.
         //
         // Qt doesn't have its own exceptions, so we use a C++ Standard Library one, which in turn means using
         // std::string.
         //
         // C++ exceptions don't give you a stack trace by default, so we use Boost to include one, as that's going to
         // be pretty helpful in debugging.
         //
         std::ostringstream stacktrace;
         stacktrace << boost::stacktrace::stacktrace();
         errorMessageAsStream << "\nStacktrace:\n" << QString::fromStdString(stacktrace.str());

         qCritical().noquote() << Q_FUNC_INFO << errorMessage;
         throw std::invalid_argument(errorMessage.toStdString());
      }
      // In non-strict mode we'll just construct an empty QVariant and return that in the hope that its default value
      // (eg 0 for a numeric type, empty string for a QString) is OK.
      qInfo() << Q_FUNC_INFO << errorMessage << ", so using generic default";
      return QVariant{};
   }
   QVariant returnValue = this->m_parameters.at(*propertyName);
   if (!returnValue.isValid()) {
      QString errorMessage =
         QString{"Invalid value (%1) supplied for required parameter, %2"}.arg(returnValue.toString(), *propertyName);
      qCritical() << Q_FUNC_INFO << errorMessage;
      throw std::invalid_argument(errorMessage.toStdString());
   }
   return returnValue;
}

bool NamedParameterBundle::containsBundle(BtStringConst const & propertyName) const {
   return this->m_containedBundles.contains(*propertyName);
}

NamedParameterBundle const & NamedParameterBundle::getBundle(BtStringConst const & propertyName) const {
   return this->m_containedBundles.at(*propertyName);
}

template<class S>
S & NamedParameterBundle::writeToStream(S & stream, QString const indent) const {
   stream << indent << this->size() << "element NamedParameterBundle @" <<
   static_cast<void const *>(this) << " {\n";
   QString const newIndent{QString("   %1").arg(indent)};
   for (auto const & [key, value] : this->m_parameters) {
      stream << newIndent << key << "->" << value.typeName() << ":" << value.toString() << "\n";
   }

   for (auto const & [bundleName, bundle] : this->m_containedBundles) {
      stream << newIndent << bundleName << "->\n";
      bundle.writeToStream(stream, newIndent);
   }

   stream << indent << "}\n";
   return stream;
}

template<class S>
S & operator<<(S & stream, NamedParameterBundle const & namedParameterBundle) {
   return namedParameterBundle.writeToStream(stream, "");
}

template<class S>
S & operator<<(S & stream, NamedParameterBundle const * namedParameterBundle) {
   if (namedParameterBundle) {
      stream << *namedParameterBundle;
   } else {
      stream << "NULL";
   }
   return stream;
}

//
// Instantiate the above template functions for the types that are going to use them
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template QDebug &      operator<<(QDebug &      stream, NamedParameterBundle const & namedParameterBundle);
template QTextStream & operator<<(QTextStream & stream, NamedParameterBundle const & namedParameterBundle);
template QDebug &      operator<<(QDebug &      stream, NamedParameterBundle const * namedParameterBundle);
template QTextStream & operator<<(QTextStream & stream, NamedParameterBundle const * namedParameterBundle);
