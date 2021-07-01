/*
 * model/NamedParameterBundle.h is part of Brewtarget, and is Copyright the following
 * authors 2021
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
#ifndef MODEL_NAMEDPARAMETERBUNDLE_H
#define MODEL_NAMEDPARAMETERBUNDLE_H
#pragma once

#include <QHash>
#include <QVariant>

/**
 * \brief This allows constructors to be called without a long list of positional parameters and, more importantly, for
 *        those parameters to be data-driven, eg from a mapping of database column names to property names.
 */
class NamedParameterBundle : public QHash<char const * const, QVariant> {
public:
   NamedParameterBundle();
   ~NamedParameterBundle();

   /**
    * \brief Get the value of a parameter that is required to be present.  Throw an exception if it is not present.
    *        This is a convenience function to make the call to extract parameters concise.  (We don't want to use the
    *        operator[] of QHash because we want "parameter not found" to be an error.)
    */
   QVariant operator()(char const * const parameterName) const;

   /**
    * \brief Get the value of a parameter that is not required to be present
    *
    *        (NB: There is no general implementation of this templated function, just specific specialisations)
    *
    * \param parameterName
    * \param defaultValue  What to return if the parameter is not present in the bundle
    */
   template <class T> T operator()(char const * const parameterName, T const & defaultValue) const;
};

#endif
