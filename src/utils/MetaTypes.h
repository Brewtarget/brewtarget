/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/MetaTypes.h is part of Brewtarget, and is copyright the following authors 2023:
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
#ifndef UTILS_METATYPES_H
#define UTILS_METATYPES_H
#pragma once

#include <optional>

#include <QDate>
#include <QString>

#include "measurement/Amount.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"

//
// It is useful in various places to be able to store member variables in QVariant objects.
//
// Where we define a strongly-typed enum, we usually just need a corresponding Q_ENUM declaration in the same header.
// This works with generic serialisation code (eg to and from database or BeerJSON) because you can safely static_cast
// between the strongly typed enum and an integer, so the generic code can use integers (via EnumStringMapping) and the
// class-specific code can use the strongly-typed enums and everything just work.
//
// HOWEVER, when the enum is optional (ie stored in memory inside std::optional, stored in the DB as a nullable field,
// only an optional field in BeerJSON, etc) then we cannot rely on casting.  You cannot, eg, static_cast between
// std::optional<int> and std::optional<Fermentable::GrainGroup>.  So inside NamedParameterBundle, we always store
// std::optional<int> for optional enum fields inside QVariant.  We need this Q_DECLARE_METATYPE macro here to allow
// this to  happen.
//
// We then put template wrappers in NamedParameterBundle so things aren't too clunky in the class-specific code.
//
// Similarly, for other nullable fields, we need to declare that we want to store std::optional<fieldType> inside
// QVariant.  This is a convenient place to do it because this header gets included not only by all the model classes
// but also by all the different serialisation code (Database, XML, JSON).
//
// Note that Qt MOC will error if you repeat a Q_DECLARE_METATYPE() declaration for the same type, which is another
// reason to put them all in one central place rather than try to declare as needed individually.
//
Q_DECLARE_METATYPE(std::optional<bool        >)
Q_DECLARE_METATYPE(std::optional<double      >)
Q_DECLARE_METATYPE(std::optional<int         >)
Q_DECLARE_METATYPE(std::optional<QDate       >)
Q_DECLARE_METATYPE(std::optional<QString     >)
Q_DECLARE_METATYPE(std::optional<unsigned int>)

//
// We need to be able to pass Measurement::Amount through the Qt Properties system.  Note that we do *not* pass
// Measurement::ConstrainedAmount classes such as MassOrVolumeAmt, MassVolumeOrCountAmt or MassOrVolumeConcentrationAmt.
// This is because the Qt Meta Object Compiler (MOC) that parses Q_DECLARE_METATYPE doesn't understand new types and so,
// eg, would not know that:
//    MassOrVolumeAmt
// is the same as:
//    Measurement::ConstrainedAmount<Measurement::ChoiceOfPhysicalQuantity const,
//                                   Measurement::ChoiceOfPhysicalQuantity::Mass_Volume>
// This is a problem with templated classes such as IngredientAmount, where we want to be able to refer to
// Measurement::ConstrainedAmount classes using the template parameters of IngredientAmount.
//
// So, for any Measurement::ConstrainedAmount class, we always declare its Qt Property in terms of Measurement::Amount,
// and we then rely on caller/getter/setter to do any necessary casting.  These casts are safe because
// Measurement::ConstrainedAmount classes only constrain the behaviour of Measurement::Amount and do not add or change
// any member variables.
//
Q_DECLARE_METATYPE(Measurement::Amount               )
Q_DECLARE_METATYPE(std::optional<Measurement::Amount>)

// Normally, we would just declare enums with Q_ENUM, but that doesn't work outside of a QObject class, so we have to
// do it here and use Q_DECLARE_METATYPE instead.
Q_DECLARE_METATYPE(Measurement::PhysicalQuantity)
Q_DECLARE_METATYPE(Measurement::ChoiceOfPhysicalQuantity)

// Measurement::Unit does not inherit from QObject, so we need this for Measurement::Units::unitStringMapping to work
Q_DECLARE_METATYPE(Measurement::Unit const *)

/**
 * \brief Just to keep us on our toes, there is an additional requirement that certain new types be registered at
 *        run-time, otherwise you'll get a "Unable to handle unregistered datatype" error and eg \c QObject::property
 *        will return a \c QVariant that is not valid (ie for which \c isValid() returns \c false).
 *
 *        The Qt doco (https://doc.qt.io/qt-6/qmetatype.html#qRegisterMetaType-2) says:
 *
 *           To use the type T in QMetaType, QVariant, or with the QObject::property() API, registration is not
 *           necessary.
 *              To use the type T in queued signal and slot connections, qRegisterMetaType<T>() must be called before
 *           the first connection is established. That is typically done in the constructor of the class that uses T,
 *           or in the main() function.
 *
 *        Again, we choose to do all this run-time registration in one place, viz this function, which should be called
 *        from \c main before invoking \c Application::run().  Of course, for the unit tests to work properly, it
 *        \b also needs to be called from the constructor of \c Testing.
 */
void registerMetaTypes();

#endif
