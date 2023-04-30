/*
 * SmartField.h is part of Brewtarget, and is copyright the following authors 2009-2023:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mike Evans <mikee@saxicola.co.uk>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Scott Peshak <scott@peshak.net>
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
#ifndef SMARTFIELD_H
#define SMARTFIELD_H
#pragma once

#include <memory> // For PImpl
#include <optional>

#include <QString>

#include "BtFieldType.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "utils/BtStringConst.h"
#include "SmartAmounts.h"

class QWidget;

class SmartLabel;
class TypeInfo;

/**
 * \class SmartField
 *
 * \brief Abstract base class that handles all the unit transformation and related logic for displaying a field.  Eg, if
 *        a field holds a volume amount then we need to be able to handle metric / US customary / Imperial systems of
 *        measurement and to deal with different scales (eg milliliters, liters, etc).  Can also deal with a
 *        \c NonPhysicalQuantity, including making sure that we add a '%' to \c NonPhysicalQuantity::Percentage.
 *
 *        The reason we need this base class, is there are two unrelated classes that need to be able to display amounts
 *        & units etc: \c SmartField, which allows amounts to be entered and edited, and \c SmartDigitWidget, which
 *        shows non-editable amounts (typically resulting from a calculation on other fields).
 *
 *                                                   QWidget
 *                                                  /       \
 *                                               ...        QLineEdit
 *                                               /                   \
 *                                              /       SmartField    \
 *                                        QLabel        /        \     \
 *                                       /       \     /          \     \
 *                             SmartLabel      SmartDigitWidget    SmartField
 *
 *        A number of helper functions exist in the \c SmartAmounts namespace.
 *
 *        Note that although \c SmartLabel and \c SmartDigitWidget both inherit from \c QLabel, they serve different
 *        purposes.  \c SmartLabel tells you what a field is, eg "Target Boil Size" and, where appropriate, allows you
 *        to select the \c SystemOfMeasurement and/or \c RelativeScale for the field.  A \c SmartDigitWidget on the
 *        other hand shows the value of a field, eg the total amount of NaCl in a water profile.
 *
 *        A \c SmartLabel and a \c SmartField typically work in conjunction with each other, but there are a number of
 *        edge cases.  See comment in \c widgets/SmartLabel.h for more details.
 *
 *        .:TBD:. There is still a certain amount of code duplication between \c SmartLabel and \c SmartField, which it
 *        would be nice to eliminate somehow.
 */
class SmartField {
public:
   SmartField();
   virtual ~SmartField();

   /**
    * \brief This needs to be called before the object is used, typically in constructor of whatever editor is using the
    *        widget.  As well as passing in a bunch of info that cannot easily be given to the constructor (per comment
    *        above), it also ensures, if necessary, that the \c changedSystemOfMeasurementOrScale signal from the
    *        \c SmartLabel buddy is connected to the \c lineChanged slot of this \c SmartField.
    *
    *        Note, in reality, you actually use the \c SMART_FIELD_INIT macro (see \c SmartAmounts.h).
    *
    * \param editorName Name of the owning editor (eg "FermentableEditor").  Together with \c fieldName, should
    *                   uniquely identify this field.
    *
    * \param fieldName Name of the member variable of this field in its owning editor (eg "field_color").
    *                     Together with \c editorName, should uniquely identify this field.
    *
    * \param fieldlFqName This should uniquely identify this field in the application.  (Usually, it's a combination
    *                        of the owning widget and the member variable, eg "FermentableEditor->field_color".)
    *                        This is mainly used for logging, where it helps a lot with debugging.  (We have hundreds of
    *                        instances of this object and if we detect that one of them is misconfigured, it's very
    *                        useful to be able to log which one!)
    *                           We \b could construct this at run-time from \c editorName and \c fieldName, but,
    *                        since we're being invoked via a macro (\c SMART_FIELD_INIT etc), we might as well have the
    *                        compiler/preprocessor do the necessary concatenation at compile-time and hand the results
    *                        in via this parameter.
    *
    * \param typeInfo Tells us what data type we use to store the contents of the field (when converted to canonical
    *                 units if it is a \c PhysicalQuantity) and, whether this is an optional field (in which case we
    *                 need to handle blank / empty string as a valid value).
    *
    * \param buddyLabel Usually needs to be \c QLabel if \c fieldType is a \c NonPhysicalQuantity and \c SmartLabel if
    *                   it is not.  However, a \c PhysicalQuantity (or \c Mixed2PhysicalQuantities) field can have a
    *                   \c QLabel (rather than a \c SmartLabel) where the user does \b not have a choice about units or
    *                   scales (even though they otherwise would for this sort of \c PhysicalQuantity).  This is
    *                   typically used on conversion dialogs, eg \c RefractoDialog, where we are asking the user to give
    *                   us inputs in specific units in order to convert them to other units measuring the same physical
    *                   quantity.
    *
    * \param precision For a decimal field, this determines the number of decimal places to show.  If not specified, we
    *                  show 3 decimal places.  TBD: IDK if one day we might need to be more sophisticated about this, ie
    *                  with number of decimal places dependent on the units that the user has chosen, but for now we
    *                  assume it's the same for everything.
    *
    * \param maximalDisplayString Used for determining the width of the widget (because a fixed pixel width isn't great
    *                             in a world where there are varying display DPIs).
    */
   template<class LabelType>
   void init(char const *                const   editorName,
             char const *                const   fieldName,
             char const *                const   fieldlFqName,
             LabelType                         & buddyLabel,
             TypeInfo                    const & typeInfo,
             std::optional<unsigned int> const   precision = std::nullopt,
             QString                     const & maximalDisplayString = "100.000 srm");

   /**
    * \brief Alternate version of \c init for where the display units are fixed, ie not changeable by the user
    *        (typically for a dialog whose purpose is to convert from one unit to another).
    *
    * \param buddyLabel Always a \c QLabel
    */
   void initFixed(char const *                const   editorName,
                  char const *                const   fieldName,
                  char const *                const   fieldlFqName,
                  QLabel                            & buddyLabel,
                  TypeInfo                    const & typeInfo,
                  Measurement::Unit           const & fixedDisplayUnit,
                  std::optional<unsigned int> const   precision = std::nullopt,
                  QString                     const & maximalDisplayString = "100.000 srm");

   /**
    * \return \c true if \c init or \c initFixed has been called, \c false otherwise
    */
   [[nodiscard]] bool isInitialised() const;

   /**
    * \brief Both \c QLabel and \c QLineEdit have a \c text() member function but AFAICT not from the same base class.
    *        We want to be able to get the text of the widget, so subclasses of \c SmartField need to implement this
    *        wrapper function that calls \c QLabel::text or \c QLineEdit::text as appropriate.
    */
   virtual QString getRawText() const = 0;

   /**
    * \brief As with \c getRawText, this is a wrapper function that calls \c QLabel::setText or \c QLineEdit::setText as
    *        appropriate
    */
   virtual void setRawText(QString const & text) = 0;

   /**
    * \brief We need our subclasses to handle signal/slot connections for us.  (The \c SmartField class cannot inherit
    *        (directly or indirectly) from \c QObject as then the subclass would be inheriting twice from \c QObject,
    *        which is (a) bad in general and (b) not supported by the Qt meta-object compiler (MOC).
    *
    *        When we are dealing with a \c PhysicalQuantity, we want to receive the \c changedSystemOfMeasurementOrScale
    *        signal from \c SmartLabel.
    *
    *        This member function is called from the base class to tell the subclass to make that connection.
    */
   virtual void connectSmartLabelSignal(SmartLabel & smartLabel) = 0;

   /**
    * \brief Once the base class is initialised, it calls this to allow the derived class to do any of its own
    *        initialisation.  (This is simpler than having the derived class override \c init and \c initFixed.)
    */
   virtual void doPostInitWork() = 0;

   BtFieldType const getFieldType() const;

   TypeInfo const & getTypeInfo() const;

   QString const & getMaximalDisplayString() const;

   char const * getFqFieldName() const;

   /**
    * \brief If our field type is \b not \c NonPhysicalQuantity, then this returns the field converted to canonical
    *        units for the relevant \c Measurement::PhysicalQuantity.  (It is a coding error to call this function if
    *        our field type \c is \c NonPhysicalQuantity.)
    */
   Measurement::Amount toCanonical() const;

   /**
    * \brief Set the amount for a numeric field
    *
    * \param amount is the amount to display, but the field should be blank if this is \b std::nullopt
    */
   template<typename T> void setAmount(std::optional<T> amount);
   template<typename T> void setAmount(T                amount);

   void setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement);
   void setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> relativeScale);
   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement() const;
   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale() const;

   /**
    * \brief Get the current settings (which may come from system-wide defaults) for \c SystemOfMeasurement and
    *        \c RelativeScale.
    */
   SmartAmounts::ScaleInfo getScaleInfo() const;

   /**
    * \brief Use this when you want to get the text as a number (and ignore any units or other trailling letters or
    *        symbols)
    */
   template<typename T> T getValueAs() const;

   /**
    * \brief Returns what type of field this is - except that, if it is \c Mixed2PhysicalQuantities, will one of the two
    *        possible \c Measurement::PhysicalQuantity values depending on the value of \c this->units.
    *
    *        It is a coding error to call this function if our field type \c is \c NonPhysicalQuantity.)
    */
   Measurement::PhysicalQuantity getPhysicalQuantity() const;

   /**
    * \brief If the \c Measurement::PhysicalQuantities supplied in the \c init call was not a single
    *        \c Measurement::PhysicalQuantity, then this member function permits selecting the current
    *        \c Measurement::PhysicalQuantity from two in the \c Measurement::Mixed2PhysicalQuantities supplied in the
    *        constructor.
    */
   void selectPhysicalQuantity(Measurement::PhysicalQuantity const physicalQuantity);

   /**
    * \brief Returns the field converted to canonical units for the relevant \c Measurement::PhysicalQuantity
    *
    * \param rawValue field text to process
    * \return
    */
///   Measurement::Amount rawToCanonical(QString const & rawValue) const;

   /**
    * \brief Use this when you want to do something with the returned QString
    *
    * \param amount Must be in canonical units eg kilograms for mass, liters for volume
    */
   [[nodiscard]] QString displayAmount(double amount) const;

   /**
    * \brief When the user has finished entering some text, this function does the corrections, eg if the field is set
    *        to show US Customary volumes and user enters an amount in liters (aka litres) then we need to convert it to
    *        display in pints or quarts etc.
    *
    *        It is a coding error to call this version of the function when field type is \c NonPhysicalQuantity
    *
    * \param previousScaleInfo
    */
   void correctEnteredText(SmartAmounts::ScaleInfo previousScaleInfo);

   /**
    * \brief Version of \c correctEnteredText to call when field type is \c NonPhysicalQuantity
    */
   void correctEnteredText();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a field object
   SmartField(SmartField const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a field object
   SmartField& operator=(SmartField const&) = delete;
   //! No move constructor
   SmartField(SmartField &&) = delete;
   //! No move assignment
   SmartField & operator=(SmartField &&) = delete;
};

#endif
