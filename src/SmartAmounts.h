/*
 * SmartAmounts.h is part of Brewtarget, and is copyright the following authors 2023:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef SMARTAMOUNTS_H
#define SMARTAMOUNTS_H
#pragma once

#include <optional>

#include <QString>

#include "measurement/SystemOfMeasurement.h"
#include "measurement/UnitSystem.h"

class QLabel;

class SmartLabel;
class SmartField;
class TypeInfo;

namespace SmartAmounts {
   /**
    * \brief Initialise a \c SmartLabel or \c QLabel and one of its \c SmartField buddies.  In the edge case that
    *        there are additional \c SmartField buddies, then this function should be called multiple times.
    *
    *        See comment in \c widgets/SmartLabel.h for more details.
    */
   template<class LabelType>
   void Init(char const * const editorName,
             char const * const labelName,
             char const * const labelFqName,
             LabelType &        label,
             char const * const fieldName,
             char const * const fieldlFqName,
             SmartField &       field,
             TypeInfo                    const & typeInfo,
             std::optional<unsigned int> const   precision = std::nullopt,
             QString                     const & maximalDisplayString = "100.000 srm");

   /**
    * \brief Alternate version of \c SmartAmounts::Init for when there is a \c SmartLabel but no \c SmartField
    */
   void InitNoSf(char const * const editorName,
                 char const * const labelName,
                 char const * const labelFqName,
                 SmartLabel &       label,
                 TypeInfo                    const & typeInfo,
                 std::optional<unsigned int> const   precision = std::nullopt,
                 QString                     const & maximalDisplayString = "100.000 srm");

   /**
    * \brief Alternate version of \c SmartAmounts::Init for when units are fixed.  (Note that this implies label is
    *        \c QLabel, not \c SmartLabel.)
    */
   void InitFixed(char const * const editorName,
                  QLabel &           label,
                  char const * const fieldName,
                  char const * const fieldlFqName,
                  SmartField &       field,
                  TypeInfo          const & typeInfo,
                  Measurement::Unit const & fixedDisplayUnit,
                  std::optional<unsigned int> const   precision = std::nullopt,
                  QString                     const & maximalDisplayString = "100.000 srm");

   /**
    * \brief Called by \c SmartLabel or subclasses of \c BtTableModel
    *
    * \param owningWindowName The class name of the editor (eg \c HopEditor) or \c BtTableModel (eg \c HopTableModel)
    * \param fieldName        The name of the member variable for a \c SmartLabel or of a \c ColumnIndex enum for a
    *                         \c BtTableModel column
    */
   void setForcedSystemOfMeasurement(char const * const owningWindowName,
                                     char const * const fieldName,
                                     std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement);

   /**
    * \brief Similar to setForcedSystemOfMeasurement
    */
   void setForcedRelativeScale(char const * const owningWindowName,
                               char const * const fieldName,
                               std::optional<Measurement::UnitSystem::RelativeScale> forcedScale);

   /**
    * \brief Called by \c SmartLabel or subclasses of \c BtTableModel
    *
    * \param owningWindowName The class name of the editor (eg \c HopEditor) or \c BtTableModel (eg \c HopTableModel)
    * \param fieldName        The name of the member variable for a \c SmartLabel or of a \c ColumnIndex enum for a
    *                         \c BtTableModel column
    */
   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement(char const * const owningWindowName,
                                                                                char const * const fieldName);

   /**
    * \brief Similar to getForcedSystemOfMeasurement
    */
   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale(char const * const owningWindowName,
                                                                                char const * const fieldName);

   /**
    * \brief Returns the forced \c SystemOfMeasurement or, if there is none, the \c SystemOfMeasurement of the
    *        globally-set \c UnitSystem for the \c PhysicalQuantity -- except that, if there are two values of
    *        \c PhysicalQuantity, we have to choose one arbitrarily.  The end result should be the same, because \c Mass
    *        & \c Volume share the same \c SystemOfMeasurement, as do \c MassConcentration & \c VolumeConcentration.
    */
   Measurement::SystemOfMeasurement getSystemOfMeasurement(char const * const owningWindowName,
                                                           char const * const fieldName,
                                                           Measurement::PhysicalQuantities physicalQuantities);

   /**
    *
    */
   Measurement::UnitSystem const & getUnitSystem(char const * const owningWindowName,
                                                 char const * const fieldName,
                                                 Measurement::PhysicalQuantity physicalQuantity);

   /**
    * \brief It's sometimes useful to have \c SystemOfMeasurement and \c UnitSystem::RelativeScale in a single variable
    */
   struct ScaleInfo {
      Measurement::SystemOfMeasurement systemOfMeasurement;
      std::optional<Measurement::UnitSystem::RelativeScale> relativeScale = std::nullopt;
   };

   /**
    * \brief Effectively combines calls to \c getSystemOfMeasurement and \c getForcedRelativeScale
    */
   ScaleInfo getScaleInfo(char const * const owningWindowName,
                          char const * const fieldName,
                          Measurement::PhysicalQuantities physicalQuantities);

}

/**
 * \brief Helper macro for \c SMART_FIELD_INIT.  Essentially does concatenation, using the magic, that, for the
 *        compiler, there is no difference between writing a string literal as:
 *           "foobarhumbug"
 *        and writing it as:
 *           "foo" "bar" "humbug"
 */
#define SFI_FQ_NAME(editorClass, fieldName) #editorClass "->" #fieldName

/**
 * \brief This macro saves a bit of copy-and-paste when invoking \c SmartFieldInit.  Eg instead of writing:
 *
 *           SmartFieldInit("FermentableEditor",
 *                          "label_color",
 *                          "FermentableEditor->label_color",
 *                          this->label_color,
 *                          "field_color",
 *                          "FermentableEditor->field_color",
 *                          this->field_color,
 *                          Fermentable::typeLookup.getType(PropertyNames::Fermentable::color_srm),
 *                          0);
 *
 *        you write:
 *
 *           SMART_FIELD_INIT(FermentableEditor, label_color, field_color, Fermentable, PropertyNames::Fermentable::color_srm, 0);
 *
 * \param editorClass The class name of the class holding the field we're initialising, eg \c HopEditor.  (In theory we
 *                    could pick this up via \c staticMetaObject.className(), but then we wouldn't be able to do the
 *                    macro concatenation here.)
 * \param modelClass The subclass of \c NamedEntity that we're editing.  Eg in \c HopEditor, this will be \c Hop
 * \param labelName  The name of the member variable for the corresponding label (\c QLabel or \c SmartLabel) for this
 *                   field.  NB we cannot always deduce this from fieldName, as sometimes two fields share a label, eg
 *                   for a min/max range on a \c Style.
 * \param fieldName  The name of the member variable for this field, eg in \c HopEditor, this could be \c field_name,
 *                   \c field_alpha, etc.  Note that:
 *                     - We deliberately don't try to do anything clever with automatically inserting the "field_"
 *                       prefix, as this would make the code harder to read and search.
 *                     - It is intentional that field names sometimes differ slightly from property names.  The latter
 *                       always include their canonical unit names (eg \c PropertyNames::Fermentable::color_srm) whereas
 *                       the former do not (eg \c field_color) because the user can enter data in any supported
 *                       units.
 * \param propertyName The name of the property to which this field relates, eg in \c HopEditor, this could be
 *                     \c PropertyNames::NamedEntity::name, \c PropertyNames::Hop::alpha_pct, etc.  (Note, as above, we
 *                     intentionally do \b not automatically insert the \c PropertyNames:: prefix.)
 * \param ...  Any remaining arguments are passed through to \c SmartField::init in fourth position and above
 *             Note that the introduction of __VA_OPT__ in C++20 makes our lives easier here.
 */
#define SMART_FIELD_INIT(editorClass, labelName, fieldName, modelClass, propertyName, ...) \
   SmartAmounts::Init(#editorClass, \
                      #labelName, \
                      SFI_FQ_NAME(editorClass, labelName), \
                      *this->labelName, \
                      #fieldName, \
                      SFI_FQ_NAME(editorClass, fieldName), \
                      *this->fieldName, \
                      modelClass ::typeLookup.getType(propertyName) \
                      __VA_OPT__(, __VA_ARGS__))

/**
 *  \brief Alternate version of \c SMART_FIELD_INIT for when there is no \c SmartField
 */
#define SMART_FIELD_INIT_NO_SF(editorClass, labelName, modelClass, propertyName, ...) \
   SmartAmounts::InitNoSf(#editorClass, \
                          #labelName, \
                          SFI_FQ_NAME(editorClass, labelName), \
                          *this->labelName, \
                          modelClass ::typeLookup.getType(propertyName) \
                          __VA_OPT__(, __VA_ARGS__))

/**
 * \brief An alternate version of \c SMART_FIELD_INIT for use when there is no \c modelClass (eg in a free-standing
 *        calculation dialog that does not update the model).  Instead of writing:
 *
 *           static auto const typeInfoFor_field_temp = TypeInfo::construct<double>(Measurement::PhysicalQuantity::Temperature);
 *           this->field_temp->init("PrimingDialog->field_temp", typeInfoFor_field_temp, *this->label_temp, 1);
 *           SmartFieldInit("PrimingDialog",
 *                          "label_temp",
 *                          "PrimingDialog->label_temp",
 *                          this->label_temp,
 *                          "field_temp",
 *                          "PrimingDialog->field_temp",
 *                          this->field_temp,
 *                          typeInfoFor_field_temp,
 *                          1);
 *
 *       you write:
 *
 *           SMART_FIELD_INIT_FS(PrimingDialog, label_temp, field_temp, double, Measurement::PhysicalQuantity::Temperature, 1);
 *
 *       The _FS in the name stands for "free-standing".
 *
 * \param editorClass As for \c SMART_FIELD_INIT.
 * \param labelName   As for \c SMART_FIELD_INIT
 * \param fieldName   As for \c SMART_FIELD_INIT
 * \param nativeType  The native type in which this value is / would be stored, eg double
 * \param btFieldType The \c BtFieldType for this field.  Together with \c nativeType, this is used to construct a
 *                    static local \c TypeInfo struct to pass by reference to \c SmartField::init.
 * \param ...  Any remaining arguments are passed through to \c SmartField::init in fourth position and above
 */
#define SMART_FIELD_INIT_FS(editorClass, labelName, fieldName, nativeType, btFieldType, ...) \
   static auto const typeInfoFor_##fieldName = TypeInfo::construct<nativeType>(btFieldType); \
   SmartAmounts::Init(#editorClass, \
                      #labelName, \
                      SFI_FQ_NAME(editorClass, labelName), \
                      *this->labelName, \
                      #fieldName, \
                      SFI_FQ_NAME(editorClass, fieldName), \
                      *this->fieldName, \
                      typeInfoFor_##fieldName \
                      __VA_OPT__(, __VA_ARGS__))


/**
 * \brief An alternate version of \c SMART_FIELD_INIT_FS for use when there is no \c modelClass and display units are
 *        fixed.
 */
#define SMART_FIELD_INIT_FIXED(editorClass, labelName, fieldName, nativeType, fixedUnit, ...) \
   static auto const typeInfoFor_##fieldName = TypeInfo::construct<nativeType>(fixedUnit.getPhysicalQuantity()); \
   SmartAmounts::InitFixed(#editorClass, \
                           *this->labelName, \
                           #fieldName, \
                           SFI_FQ_NAME(editorClass, fieldName), \
                           *this->fieldName, \
                           typeInfoFor_##fieldName, \
                           fixedUnit \
                           __VA_OPT__(, __VA_ARGS__))

#endif
