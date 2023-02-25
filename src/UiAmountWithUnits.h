/*
 * UiAmountWithUnits.h is part of Brewtarget, and is copyright the following
 * authors 2009-2022:
 * - Brian Rower <brian.rower@gmail.com>
 * - Mark de Wever <koraq@xs4all.nl>
 * - Matt Young <mfsy@yahoo.com>
 * - Mike Evans <mikee@saxicola.co.uk>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Scott Peshak <scott@peshak.net>
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
#ifndef UIAMOUNTWITHUNITS_H
#define UIAMOUNTWITHUNITS_H
#pragma once

#include <optional>

#include <QString>

#include "BtFieldType.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"

class QWidget;

struct PreviousScaleInfo {
   Measurement::SystemOfMeasurement oldSystemOfMeasurement;
   std::optional<Measurement::UnitSystem::RelativeScale> oldForcedScale = std::nullopt;
};

/**
 * \class UiAmountWithUnits A base class, suitable for combining with \c QLabel, \c QLineEdit, etc, that handles all the
 *                          unit transformation such a widget would need to do.
 */
class UiAmountWithUnits {
public:
   /**
    * \param
    * \param
    * \param units
    */
   UiAmountWithUnits(QWidget * parent,
                     BtFieldType fieldType,
                     Measurement::Unit const * units = nullptr);
   virtual ~UiAmountWithUnits();

   /**
    * \brief A class inheriting from this class is also expected to also inherit from a \c QWidget such as \c QLabel or
    *        \c QLineEdit.  We would like to be able to access the text() member function of that parent class in parts
    *        of our own implementation.  This is a bit tricky as \c QLabel::text() and \c QLineEdit::text() are actually
    *        unrelated, despite both having the same signature.  We therefore require child classes to implement this
    *        wrapper function that returns the value of \c text() from their other superclass.
    */
   virtual QString getWidgetText() const = 0;

   /**
    * \brief Similar to \c getText(), this allows this base class to access \c QLabel::setText() or
    *        \c QLineEdit::setText() in the subclass that also inherits from \c QLabel or \c QLineEdit.
    */
   virtual void setWidgetText(QString text) = 0;

   /**
    * \brief Returns what type of field this is - except that, if it is \c Measurement::PhysicalQuantity::Mixed, will
    *        return either \c Measurement::PhysicalQuantity::Mass or \c Measurement::PhysicalQuantity::Volume depending
    *        on the value of \c this->units.
    */
   BtFieldType getFieldType() const;

   void setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement);
   void setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> relativeScale);

   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement() const;
   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale() const;

   /**
    * \brief QString version of \c setForcedSystemOfMeasurement to work with code generated from .ui files (via
    *        Q_PROPERTY declared in subclass of this class)
    */
   void setForcedSystemOfMeasurementViaString(QString systemOfMeasurementAsString);

   /**
    * \brief QString version of \c getForcedSystemOfMeasurement to work with code generated from .ui files (via Q_PROPERTY
    *        declared in subclass of this class)
    */
   QString getForcedSystemOfMeasurementViaString() const;

   /**
    * \brief QString version of \c setForcedRelativeScale to work with code generated from .ui files (via Q_PROPERTY
    *        declared in subclass of this class)
    */
   void setForcedRelativeScaleViaString(QString relativeScaleAsString);

   /**
    * \brief QString version of \c getForcedRelativeScale to work with code generated from .ui files (via Q_PROPERTY
    *        declared in subclass of this class)
    */
   QString getForcedRelativeScaleViaString() const;

   // By defining the setters/getters, we can remove the need for initializeProperties.
   void    setEditField(QString editField);
   QString getEditField() const;

   void    setConfigSection(QString configSection);
   QString getConfigSection();

   /**
    * \brief Converts the numeric part of the input field to a double, ignoring any string suffix.  So "5.5 gal" will
    *        give 5.5, "20L" will return 20.0, and so on.
    */
   double toDoubleRaw(bool * ok = nullptr) const;

   /**
    * \brief Returns the field converted to canonical units for the relevant \c Measurement::PhysicalQuantity
    */
   Measurement::Amount toCanonical();

   /**
    * \brief Use this when you want to do something with the returned QString
    */
   QString displayAmount(double amount, int precision = 3);

protected:
   /**
    * \brief
    */
   void textOrUnitsChanged(PreviousScaleInfo previousScaleInfo);

   /**
    * \brief Returns the contents of the field converted, if necessary, to SI units
    *
    * \param oldSystemOfMeasurement
    * \param oldScale (optional)
    */
   Measurement::Amount convertToSI(PreviousScaleInfo previousScaleInfo);

private:
   QWidget * parent;
   /**
    * \brief Even inside the class (or any subclasses), this should never be accessed directly but always through
    *        \c this->getFieldType, as there is special case handling for \c Measurement::PhysicalQuantity::Mixed.
    */
   BtFieldType const fieldType;

protected:
   /**
    * \brief If \c fieldType is a \c Measurement::PhysicalQuantity, this is the \c Measurement::Unit that should be used
    *        to store the amount of this field.  This is normally fixed as our "standard" (normally metric) unit for the
    *        \c Measurement::PhysicalQuantity of the field -- eg kilograms for Mass, liters for Volume,
    *        celsius for Temperature, minutes for Time, etc.  However, for \c fieldType of
    *        \c Measurement::PhysicalQuantity::Mixed, this will need to vary between \c Measurement::Units::kilograms
    *        and \c Measurement::Units::liters depending on whether the current field is to be measured by weight or
    *        volume.
    *
    *        If \c fieldType is not a \c Measurement::PhysicalQuantity, this will be \c nullptr
    */
   Measurement::Unit const * units;
   QString editField;
   QString configSection;
};
#endif
