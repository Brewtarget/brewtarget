/*======================================================================================================================
 * utils/PropertyHelper.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef UTILS_PROPERTYHELPER_H
#define UTILS_PROPERTYHELPER_H
#pragma once

#include <Qt>
#include <QVariant>

#include "measurement/Amount.h"
#include "measurement/Measurement.h"
#include "measurement/SystemOfMeasurement.h"
#include "measurement/UnitSystem.h"
#include "undoRedo/Undoable.h"
#include "utils/OptionalHelpers.h"
#include "utils/PropertyPath.h"
#include "utils/TypeInfo.h"

namespace PropertyHelper {

   /**
    * \brief Given a value returned from \c QObject::property (possibly via \c PropertyPath::getValue), convert it into
    *        something suitable for displaying, editing, sorting etc.
    *
    * \param modelData
    * \param typeInfo
    * \param role  A \c Qt::ItemDataRole value, often as passed to us by Qt.  The main values we care about are:
    *                 • \c Qt::DisplayRole means we return a \c QString suitable for display
    *                 • \c Qt::EditRole means we return a \c QString suitable for editing
    *                 • \c Qt::UserRole means we are being called from \c isLessThan, so we want to return whatever is
    *                                   needed for a sort comparison.  In some cases (eg an enum or bool), this is the
    *                                   the same QString as for \c Qt::DisplayRole, because an alphabetical sort of, eg
    *                                   a \c Hop::Form field, will make a lot more sense to the user than sorting it by
    *                                   the internal numerical value.  In other cases, eg where the underlying value is
    *                                   a \c Measurement::Amount, we want the raw value not the display text, so we can
    *                                   ensure 200g < 1kg etc.
    * \param showChoiceOfPhysicalQuantity  If this is set for an amount field then we want to get the physical quantity
    *                                      of that field (eg whether it is mass or volume).
    * \param forcedSystemOfMeasurement
    * \param forcedRelativeScale
    */
   QVariant readDataFromPropertyValue(
      QVariant modelData,
      TypeInfo const & typeInfo,
      int const role = Qt::DisplayRole,
      bool const showChoiceOfPhysicalQuantity = false,
      std::optional<Measurement::SystemOfMeasurement> const forcedSystemOfMeasurement = std::nullopt,
      std::optional<Measurement::UnitSystem::RelativeScale> const forcedRelativeScale = std::nullopt
   );

   /**
    *
    * \param physicalQuantity Needs to be supplied if and only if the type is \c Measurement::MixedPhysicalQuantities
    *
    * \return \c true if successful, \c false otherwise
    */
   template<typename NE>
   bool writeDataToProperty(NE & ne,
                            PropertyPath const & propertyPath,
                            TypeInfo const & typeInfo,
                            QVariant const & value,
                            std::optional<Measurement::ChoiceOfPhysicalQuantity>  const extras = std::nullopt,
                            std::optional<Measurement::SystemOfMeasurement>       const forcedSystemOfMeasurement = std::nullopt,
                            std::optional<Measurement::UnitSystem::RelativeScale> const forcedRelativeScale       = std::nullopt,
                            std::optional<Measurement::PhysicalQuantity> physicalQuantity = std::nullopt) {
      // Generally leave this commented out as it generates too much logging
//      qDebug() << Q_FUNC_INFO << propertyPath << ":" << value;
      // Uncomment this if one of the physicalQuantity-related asserts below is firing
//      qDebug().noquote() << Q_FUNC_INFO << "physicalQuantity: " << physicalQuantity << Logging::getStackTrace();
      // For all non physical quantities, including enums and bools, ItemDelegate::writeDataToModel will already have
      // created the right type of QVariant for us, including handling whether or not it is optional.
      QVariant processedValue;
      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         // It's a coding error if physicalQuantity was supplied for a field that is not a PhysicalQuantity
         Q_ASSERT(!physicalQuantity);
         processedValue = value;
      } else {
         // For physical quantities, we need to handle any conversions to and from canonical amounts, as well as deal
         // with optional values.
         //
         // ItemDelegate::writeDataToModel should have just given us a raw string
         Q_ASSERT(value.canConvert<QString>());

         //
         // For cases where we have an Amount and a drop-down chooser to select PhysicalQuantity (eg between Mass and
         // Volume), we have two columns with the same type.  The one that actually holds the amount is relatively
         // straightforward because that's what we're already holding in `value`.  The one that holds the drop-down
         // chooser also needs access to the amount, so it needs to get it from the model (which happens below).
         //
         bool const isPhysicalQuantityChooser =
           std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) &&
           extras;

         if (std::holds_alternative<Measurement::PhysicalQuantity>(*typeInfo.fieldType)) {
            // It's a coding error if physicalQuantity was supplied - because it's known in advance from the field type
            Q_ASSERT(!physicalQuantity);
            // Might seem a bit odd to overwrite the parameter here, but it allows us to share most of the code for
            // PhysicalQuantity and ChoiceOfPhysicalQuantity
            physicalQuantity = std::get<Measurement::PhysicalQuantity>(*typeInfo.fieldType);
         } else {
            // This should be the only possibility left
            Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType));
            // It's a coding error if physicalQuantity was not supplied for a non-Amount quantity column.  Equally it's
            // a coding error to supply it for the drop-down chooser column or for an Amount column.
            if (typeInfo.typeIndex == typeid(double)) {
               // For a double representing a ChoiceOfPhysicalQuantity, we need to be passed in the current
               // PhysicalQuantity because we don't know how to obtain it generically.
               Q_ASSERT(physicalQuantity);
            } else {
               Q_ASSERT(!physicalQuantity);
               if (!isPhysicalQuantityChooser) {
                  // If this is an Amount field, we just ask the model for the current amount and look at what
                  // PhysicalQuantity that is.
                  // As above, overwriting the physicalQuantity parameter here simplifies the code below
                  physicalQuantity =
                     QVariant(propertyPath.getValue(ne)).value<Measurement::Amount>().unit->getPhysicalQuantity();
               }
            }
         }

         // For the moment, I'm assuming any ChoiceOfPhysicalQuantity amount is never optional.  If we change our minds
         // about that in future then we'd need some additional logic here and in several other places.
         Q_ASSERT(isPhysicalQuantityChooser || physicalQuantity);
         Measurement::Amount amount{
            isPhysicalQuantityChooser ?
            // Drop-down
            QVariant(propertyPath.getValue(ne)).value<Measurement::Amount>() :
            // Amount itself
            Measurement::qStringToSI(value.toString(),
                                     *physicalQuantity,
                                     forcedSystemOfMeasurement,
                                     forcedRelativeScale)
         };
         if (typeInfo.typeIndex == typeid(double)) {
            processedValue = Optional::variantFromRaw(value.toString(), amount.quantity, typeInfo.isOptional());
         } else {
            // Comments above in readDataFromModel apply equally here.  You can cast between MassOrVolumeAmt and
            // Measurement::Amount, but not between QVariant<MassOrVolumeAmt> and QVariant<Measurement::Amount>, so
            // we have to do the casting before we wrap.
            if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) ||
                typeInfo.typeIndex == typeid(Measurement::Amount)) {

               // If this is the drop-down for the PhysicalQuantity of the amount, then changing it is just another way
               // of changing the amount itself.
               if (isPhysicalQuantityChooser) {
                  //
                  // There isn't an ideal way to convert an Amount from one PhysicalQuantity to another.  There is no
                  // generic way, eg, to convert from mass to volume because it depends on the density of the thing
                  // being measured, which we don't know.  The best we can do is convert from one canonical unit to
                  // another, eg from kilograms to liters in the case of mass to volume.
                  //
                  // This is easy to do because whatever amount we get back from the model will already be in canonical
                  // units because, by convention, we only store things in canonical units in the model and the DB.
                  //
                  Q_ASSERT(value.canConvert<int>());
                  Measurement::Unit const & newUnit = Measurement::Unit::getCanonicalUnit(
                     static_cast<Measurement::PhysicalQuantity>(value.value<int>())
                  );
                  amount.unit = &newUnit;
               }

               processedValue = Optional::variantFromRaw(value.toString(),
                                                         static_cast<Measurement::Amount>(amount),
                                                         typeInfo.isOptional());
            } else {
               // It's a coding error if we get here
               qCritical() <<
                  Q_FUNC_INFO << /*columnInfo.columnFqName << */ "Don't know how to parse" << propertyPath <<
                  "TypeInfo:" << typeInfo << ", value:" << value << ", amount:" << amount;
               Q_ASSERT(false);
            }
         }
      }

      Undoable::doOrRedoUpdate(
         ne,
         propertyPath,
         typeInfo,
         processedValue,
         NE::tr("Change %1 %2").arg(NE::staticMetaObject.className()).arg(typeInfo.localisedName())
      );

      return true;
   }

}

#endif
