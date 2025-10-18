/*======================================================================================================================
 * utils/PropertyHelper.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "utils/PropertyHelper.h"

#include "utils/EnumStringMapping.h"
#include "utils/OptionalHelpers.h"

QVariant PropertyHelper::readDataFromPropertyValue(
   QVariant modelData,
   TypeInfo const & typeInfo,
   int const role,
   bool const showChoiceOfPhysicalQuantity,
   std::optional<Measurement::SystemOfMeasurement> const forcedSystemOfMeasurement,
   std::optional<Measurement::UnitSystem::RelativeScale> const forcedRelativeScale
) {
   // Uncomment this log statement if asserts below are firing
//   qDebug() <<
//      Q_FUNC_INFO << "TypeInfo:" << typeInfo << ", modelData:" << modelData << ", role:" << role <<
//      ", showChoiceOfPhysicalQuantity:" << showChoiceOfPhysicalQuantity;

   //
   // Following notes apply to table models:
   //
   //    - Unlike in an editor, in the table model, the edit control is only shown when you are actually editing a
   //      field.  Normally there's a separate control flow for just displaying the modelData otherwise.  We'll get
   //      called in both cases, but the modelData of `role` will be different.
   //
   //    - For Qt::EditRole, we're being called from ItemDelegate::readDataFromModel (see
   //      qtModels/tableModels/ItemDelegate.h), which will handle any special display requirements for enums and bools
   //      (where, in both cases, we show combo boxes), because it is feeding directly into the appropriate editor
   //      widget.  For other types, we want to hand back something that can be converted to QString.
   //
   //    - For Qt::DisplayRole, we're typically being called from QSortFilterProxyModel::data which is, in turn, called
   //      by QItemDelegate::paint.  We don't want to override QItemDelegate::paint in ItemDelegate, because it would be
   //      overkill.  So, here, we just need to make sure we're returning something that can sensibly be converted to
   //      QString.
   //

   //
   // First handle the cases where ItemDelegate::readDataFromModel wants "raw" data
   //
   if (role == Qt::EditRole && std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
      auto const nonPhysicalQuantity = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);
      if (nonPhysicalQuantity == NonPhysicalQuantity::Enum ||
          nonPhysicalQuantity == NonPhysicalQuantity::Bool) {
         return modelData;
      }
   }

   bool hasValue = false;
   if (typeInfo.isOptional()) {
      // This does the right thing even for enums - see comment in utils/OptionalHelpers.cpp
      Optional::removeOptionalWrapper(modelData, typeInfo, &hasValue);
   } else {
      hasValue = true;
   }

   if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
      auto const nonPhysicalQuantity = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);

      if (!hasValue) {
         if (role == Qt::UserRole &&
             nonPhysicalQuantity != NonPhysicalQuantity::Enum &&
             nonPhysicalQuantity != NonPhysicalQuantity::Bool) {
            return modelData;
         }
         return QString{""};
      }

      if (nonPhysicalQuantity == NonPhysicalQuantity::Enum) {
         // We assert that we cannot be editing this type of field (because it's a combo box, so user just selects
         // one of the values in the list).
         Q_ASSERT(role != Qt::EditRole);

         Q_ASSERT(typeInfo.displayAs);
         Q_ASSERT(std::holds_alternative<DisplayInfo::Enum>(*typeInfo.displayAs));
         DisplayInfo::Enum const & enumInfo = std::get<DisplayInfo::Enum>(*typeInfo.displayAs);
         Q_ASSERT(modelData.canConvert<int>());
         int const enumValue = modelData.toInt();
//            qDebug() << Q_FUNC_INFO << "Enum value:" << enumValue;
         std::optional<QString> displayText = enumInfo.displayNames.enumAsIntToValue(enumValue);
         // It's a coding error if we couldn't find something to display!
         Q_ASSERT(displayText);
         return *displayText;
      }

      if (nonPhysicalQuantity == NonPhysicalQuantity::Bool) {
         // As with Enum, we cannot edit a Bool, just select one of the predetermined values for it.
         Q_ASSERT(role != Qt::EditRole);

         Q_ASSERT(typeInfo.displayAs);
         Q_ASSERT(std::holds_alternative<DisplayInfo::Bool>(*typeInfo.displayAs));
         DisplayInfo::Bool const & info = std::get<DisplayInfo::Bool>(*typeInfo.displayAs);
         Q_ASSERT(modelData.canConvert<bool>());
         return modelData.toBool() ? info.setDisplay : info.unsetDisplay;
      }

      if (role == Qt::UserRole) {
         // For sorting, we need the actual amount
         return modelData;
      }

      if (nonPhysicalQuantity == NonPhysicalQuantity::Percentage) {
         unsigned int precision = 3;
         if (typeInfo.displayAs) {
            Q_ASSERT(std::holds_alternative<DisplayInfo::Precision>(*typeInfo.displayAs));
            DisplayInfo::Precision const & precisionInfo = std::get<DisplayInfo::Precision>(*typeInfo.displayAs);
            precision = precisionInfo.precision;
         }
         // We assert that percentages are numbers and therefore either are double or convertible to double
         Q_ASSERT(modelData.canConvert<double>());
         return QVariant(Measurement::displayQuantity(modelData.toDouble(), precision, nonPhysicalQuantity));
      }

      //
      // Other non-physical quantities can just be returned as-is (at the bottom of this function)
      //

   } else {
      //
      // Since it's not NonPhysicalQuantity, it must be either Measurement::ChoiceOfPhysicalQuantity or
      // Measurement::PhysicalQuantity.
      //
      // Most of the handling for these two cases is the same.
      //
      Q_ASSERT(
         std::holds_alternative<Measurement::PhysicalQuantity        >(*typeInfo.fieldType) ||
         std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType)
      );

      //
      // Since we already handled bool or enum above, the typeInfo.displayAs must either be empty or hold precision
      // info.
      //
      Q_ASSERT(!typeInfo.displayAs || std::holds_alternative<DisplayInfo::Precision>(*typeInfo.displayAs));

      //
      // Deal with null values first
      //
      if (!hasValue) {
         if (role == Qt::UserRole) {
            return modelData;
         }
         return QString{""};
      }

      //
      // We don't need this in every case below, but we might as well pull it out here to save repeating ourselves in
      // different branches below.
      //
      unsigned int const precision {
         typeInfo.displayAs ? std::get<DisplayInfo::Precision>(*typeInfo.displayAs).precision : 3
      };

      //
      // A field marked Measurement::PhysicalQuantity can be a double or Measurement::Amount (or a subclass thereof).
      //
      // If it's a double, then we know it must be measured in canonical units of that PhysicalQuantity.
      //
      if (typeInfo.typeIndex == typeid(double)) {
         Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*typeInfo.fieldType));

         Q_ASSERT(modelData.canConvert<double>());
         double const rawValue = modelData.value<double>();
         //
         // This is one of the points where it's important that NamedEntity classes always store data in canonical
         // units.  For any properties where that's _not_ the case, we need either to:
         //   - ensure we're passing Measurement::Amount, ie the units are always included; or
         //   - specify the non-canonical unit in TypeInfo.
         //
         auto const physicalQuantity = std::get<Measurement::PhysicalQuantity>(*typeInfo.fieldType);
         Measurement::Unit const & unit{
            typeInfo.unit ? *typeInfo.unit : Measurement::Unit::getCanonicalUnit(physicalQuantity)
         };
         Measurement::Amount const amount{rawValue, unit};

         // For sorting, we need the actual amount
         if (role == Qt::UserRole) {
            return QVariant::fromValue(amount);
         }

         // For display or edit, we return the string representation
         return QVariant(Measurement::displayAmount(amount,
                                                    precision,
                                                    forcedSystemOfMeasurement,
                                                    forcedRelativeScale));
      }

      //
      // Since the field value is not a double, it must be Measurement::Amount or a subclass thereof -- ie an
      // instantiation of the Measurement::ConstrainedAmount template (eg MassOrVolumeAmt).
      //
      // NB: per the comments in utils/MetaTypes.h, QVariant does not understand subclasses, so a QVariant holding
      //     MassOrVolumeAmt would return false from canConvert<Measurement::Amount>().  Therefore, we actually always
      //     set the QProperty type to Measurement::Amount,  rather than a Measurement::ConstrainedAmount instantiation
      //     such as MassOrVolumeAmt.  Casting back and forth between these types is safe because
      //     Measurement::ConstrainedAmount does not add any member variables to Measurement::Amount.
      //
      Q_ASSERT(modelData.canConvert<Measurement::Amount>());

      //
      // Depending on whether showChoiceOfPhysicalQuantity is set, we're going to want either the amount itself or its
      // physical quantity.  (Per the comments in qtModels/tableModels/ItemDelegate.h, the latter is when we need to
      // show a drop-down for the PhysicalQuantity of the amount.)
      //
      // In both cases, we start by getting the amount from the model.
      //
      Measurement::Amount const amount = modelData.value<Measurement::Amount>();
      if (!amount.isValid()) {
         // It's a coding error if we get here
         qCritical() <<
            Q_FUNC_INFO << /* columnInfo.columnFqName << */ "Invalid amount for" << /* columnInfo.propertyPath << */
            "TypeInfo:" << typeInfo << ", modelData:" << modelData;
         Q_ASSERT(false);
      }

      if (showChoiceOfPhysicalQuantity) {
         //
         // This is the drop-down for the PhysicalQuantity of the Amount
         //
         Measurement::PhysicalQuantity const physicalQuantity = amount.unit->getPhysicalQuantity();
         if (role == Qt::EditRole) {
            // For edit, we just want the actual PhysicalQuantity
            return QVariant::fromValue(static_cast<int>(physicalQuantity));
         }

         // For display or sort we want to map the PhysicalQuantity to its user-friendly name string
         // (It's a coding error if we couldn't find something to display, which will result in an exception,
         // terminating the program and prompting us to fix the bug.)
         return Measurement::physicalQuantityDisplayNames[physicalQuantity];
      }

      // This is the Amount itself
      if (role == Qt::UserRole) {
         // For sorting, we need the actual amount
         return QVariant::fromValue(amount);
      }

      //
      // For display or edit, we return the string representation
      //
      return QVariant(
         Measurement::displayAmount(amount,
                                    precision,
                                    forcedSystemOfMeasurement,
                                    forcedRelativeScale)
      );
   }

   // If we got here, there's no special handling required - ie the data has no units or special formatting
   // requirements, so we can just return as-is.
   return modelData;
}

bool PropertyHelper::isLessThan(QVariant const &  leftItem, QVariant const & rightItem, TypeInfo const & typeInfo) {
   // Normally leave this commented out as it generates far too much logging
//      qDebug() << Q_FUNC_INFO << "leftItem:" << leftItem << "; rightItem:" << rightItem;

   //
   // It's not crazy to have null come before valid values
   //
   if (leftItem.isNull()) {
      return true;
   }
   if (rightItem.isNull()) {
      return false;
   }

   if (typeInfo.fieldType) {
      QuantityFieldType const fieldType = *typeInfo.fieldType;
      if (std::holds_alternative<NonPhysicalQuantity>(fieldType)) {
         auto const nonPhysicalQuantity = std::get<NonPhysicalQuantity>(fieldType);
         switch (nonPhysicalQuantity) {
            case NonPhysicalQuantity::Date:
            case NonPhysicalQuantity::String:
            case NonPhysicalQuantity::Enum:
            case NonPhysicalQuantity::Bool:
               return leftItem.toString() < rightItem.toString();

            case NonPhysicalQuantity::Percentage:
            case NonPhysicalQuantity::Dimensionless:
               return leftItem.toDouble() < rightItem.toDouble();

            case NonPhysicalQuantity::OrdinalNumeral:
            case NonPhysicalQuantity::CardinalNumber:
               return leftItem.toInt() < rightItem.toInt();

            case NonPhysicalQuantity::Currency:
               return  leftItem.value<CurrencyAmount>() <
                        rightItem.value<CurrencyAmount>();

            case NonPhysicalQuantity::Objects:
               // We shouldn't be trying to compare lists of objects
               qWarning() << Q_FUNC_INFO << "Can't compare " << typeInfo;
               return false;

            // No default case as we want compiler to warn us if we missed a case above
         }
      } else if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
         return leftItem.value<Measurement::Amount>().toCanonical() <
               rightItem.value<Measurement::Amount>().toCanonical();
      } else {
         Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(fieldType));
         //
         // Per comment in readDataFromPropertyValue(), this field could be either the amount itself or a drop-down for
         // the PhysicalQuantity of the amount.  In the latter case, readDataFromPropertyValue() will have returned us
         // a QString, otherwise we'll have an Amount.
         //
         if (leftItem.typeId() == QMetaType::QString) {
            //
            // Field type should be determined only by column, not by row
            //
            Q_ASSERT(rightItem.typeId() == QMetaType::QString);
            return leftItem.toString() < rightItem.toString();
         }

         //
         // It's not instantly obvious how to sort a mixture of, eg, masses and volumes.  For the moment, we convert
         // everything to canonical units (kilograms for mass, liters for volume, etc) and then sort by the
         // quantities.
         //
         return leftItem.value<Measurement::Amount>().toCanonical() <
               rightItem.value<Measurement::Amount>().toCanonical();
      }

      //
      // It's a coding error if we didn't handle every case above
      //
      qWarning() << Q_FUNC_INFO << "Unhandled field type:" << fieldType;
   }

   return true;
}
