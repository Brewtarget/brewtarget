/*
 * model/NamedParameterBundle.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021-2023
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/NamedParameterBundle.h"

#include <string>
#include <stdexcept>
#include <sstream>

#include <boost/stacktrace.hpp>

#include <QDebug>
#include <QString>
#include <QTextStream>

NamedParameterBundle::NamedParameterBundle(NamedParameterBundle::OperationMode mode) :
   QHash<QString, QVariant>(),
   mode{mode} {
   return;
}

NamedParameterBundle::~NamedParameterBundle() = default;

NamedParameterBundle::iterator NamedParameterBundle::insert(BtStringConst const & parameterName,
                                                            QVariant const & value) {
   return this->QHash<QString, QVariant>::insert(QString{*parameterName}, value);
}


QVariant NamedParameterBundle::get(BtStringConst const & parameterName) const {
   if (!this->contains(*parameterName)) {
      QString errorMessage = QString("No value supplied for required parameter, %1.").arg(*parameterName);
      QTextStream errorMessageAsStream(&errorMessage);
      errorMessageAsStream << "  (Parameters in this bundle are ";
      bool wroteFirst = false;
      for (auto ii = this->constBegin(); ii != this->constEnd(); ++ii) {
         if (!wroteFirst) {
            wroteFirst = true;
         } else {
            errorMessageAsStream << ", ";
         }
         errorMessageAsStream << ii.key();
      }
      errorMessageAsStream << ")";
      if (this->mode == NamedParameterBundle::Strict) {
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
   QVariant returnValue = this->value(*parameterName);
   if (!returnValue.isValid()) {
      QString errorMessage =
         QString{"Invalid value (%1) supplied for required parameter, %2"}.arg(returnValue.toString(), *parameterName);
      qCritical() << Q_FUNC_INFO << errorMessage;
      throw std::invalid_argument(errorMessage.toStdString());
   }
   return returnValue;
}

template<class S>
S & operator<<(S & stream, NamedParameterBundle const & namedParameterBundle);

template<class S>
S & operator<<(S & stream, NamedParameterBundle const * namedParameterBundle);

template<class S>
S & operator<<(S & stream, NamedParameterBundle const & namedParameterBundle) {
   stream << namedParameterBundle.size() << "element NamedParameterBundle @" <<
   static_cast<void const *>(&namedParameterBundle) << " {";
   // QHash::constKeyValueBegin() and similar functions were not introduced until Qt 5.10, and
   // QKeyValueIterator::operator->() was not introduced until Qt 5.15.  For the moment, we are still supporting
   // Qt 5.9.5, so we need to do things differently for that.
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   for (auto ii = namedParameterBundle.cbegin(); ii != namedParameterBundle.cend(); ++ii) {
      stream << ii.key() << "->" << ii.value().toString() << " ";
   }
#else
   for (auto ii = namedParameterBundle.constKeyValueBegin(); ii != namedParameterBundle.constKeyValueEnd(); ++ii) {
      stream << ii->first << "->" << ii->second.toString() << " ";
   }
#endif
   stream << "}";
   return stream;
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
