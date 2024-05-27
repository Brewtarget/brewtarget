/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartField.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mike Evans <mikee@saxicola.co.uk>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Scott Peshak <scott@peshak.net>
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
#ifndef WIDGETS_SMARTFIELD_H
#define WIDGETS_SMARTFIELD_H
#pragma once

#include <memory> // For PImpl
#include <optional>

#include <QString>

#include "BtFieldType.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "utils/BtStringConst.h"
#include "utils/TypeLookup.h"
#include "widgets/SmartAmounts.h"
#include "widgets/SmartBase.h"

class SmartAmountSettings;
class SmartLabel;
struct TypeInfo;

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
 *                                               ...         QLineEdit
 *                                              /                     \
 *                                             /        SmartBase      \
 *                                        QLabel        /      \        \
 *                                         \    \      /    SmartField   \
 *                                          \  SmartLabel    /      \     \
 *                                           \              /        \     \
 *                                          SmartDigitWidget        SmartLineEdit
 *
 *        A number of helper functions exist in the \c SmartAmounts namespace.
 *
 *        Note that although \c SmartLabel and \c SmartDigitWidget both inherit from \c QLabel, they serve different
 *        purposes.  \c SmartLabel tells you what a field is, eg "Target Boil Size" and, where appropriate, allows you
 *        to select the \c SystemOfMeasurement and/or \c RelativeScale for the field.  A \c SmartDigitWidget on the
 *        other hand shows the value of a field, eg the total amount of NaCl in a water profile.
 *
 *        A \c SmartLabel and a \c SmartField typically work in conjunction with each other, but there are a number of
 *        edge cases.  You cannot assume that a \c SmartLabel will always have a \c SmartField or vice versa.  See
 *        comment in \c widgets/SmartLabel.h for more details.  This is why \c SmartAmountSettings exists to hold the
 *        information that is usually in \c SmartLabel but can be held in \c SmartField if there is no \c SmartLabel.
 *        The \c SmartBase class mostly just provides a convenient way to access \c SmartAmountSettings member
 *        functions.
 *
 *        .:TBD:. There is still a certain amount of code duplication between \c SmartLabel and \c SmartField, which it
 *        would be nice to eliminate somehow:
 *              setForcedSystemOfMeasurement
 *              setForcedRelativeScale
 *              getForcedSystemOfMeasurement
 *              getForcedRelativeScale
 *              getScaleInfo
 *
 *              getPhysicalQuantity
 *              selectPhysicalQuantity
 *
 *        ××× SmartWidgetSettings or something.  One is the interface, other is the implementation/storage.  Logic is,
 *        if Field has a Label then ask the Label; otherwise use our own.  The storage is:
 *              TypeInfo const *          m_typeInfo
 *              std::optional<Measurement::PhysicalQuantity> m_currentPhysicalQuantity
 *
 *              Measurement::Unit const * m_fixedDisplayUnit       BUT ONLY IN SMART_fIELD
 */
class SmartField : public SmartBase<SmartField> {
public:
   SmartField();
   virtual ~SmartField();

   /**
    * \brief This needs to be called before the object is used, typically in constructor of whatever editor is using the
    *        widget.  As well as passing in a bunch of info that cannot easily be given to the constructor (per comment
    *        above), it also ensures, if necessary, that the \c changedSystemOfMeasurementOrScale signal from the
    *        \c SmartLabel buddy is connected to the \c lineChanged slot of this \c SmartField.
    *
    *        Note, in reality, you actually use the \c SMART_FIELD_INIT macro (see \c widgets/SmartAmounts.h).
    *
    * \param editorName Name of the owning editor (eg "FermentableEditor").  Together with \c fieldName, should
    *                   uniquely identify this field.
    *
    * \param fieldName Name of the member variable of this field in its owning editor (eg "field_color").
    *                  Together with \c editorName, should uniquely identify this field.
    *
    * \param fieldFqName This should uniquely identify this field in the application.  (Usually, it's a combination
    *                    of the owning widget and the member variable, eg "FermentableEditor->field_color".)
    *                    This is mainly used for logging, where it helps a lot with debugging.  (We have hundreds of
    *                    instances of this object and if we detect that one of them is misconfigured, it's very useful
    *                    to be able to log which one!)
    *                       We \b could construct this at run-time from \c editorName and \c fieldName, but, since we're
    *                    being invoked via a macro (\c SMART_FIELD_INIT etc), we might as well have the
    *                    compiler/preprocessor do the necessary concatenation at compile-time and hand the results in
    *                    via this parameter.
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
    *                  assume it's the same for everything, but allow modification via \c setPrecision.
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

   /**
    * \brief Maybe for consistency this should be \c getSettings() but that jars somewhat!
    */
   [[nodiscard]] SmartAmountSettings & settings();

   QString const & getMaximalDisplayString() const;

   char const * getFqFieldName() const;

   /**
    * \brief Version of \c setQuantity, for an optional quantity.
    *
    *        It looks a bit funky disabling this specialisation for a T that is optional, but the point is that we don't
    *        want the compiler to ever create a \c std::optional<std::optional<T>> type.  (Eg, we don't want to write
    *        `\c setQuantity<std::optional<T>>(\c std::nullopt)` when we mean
    *        `\c setQuantity<T>(\c std::optional<T>{std::nullopt})`.
    *
    *        Note that, if you are explicitly providing std::nullopt as the parameter, you need to provide type
    *        information, eg myField->setQuantity<double>(std::nullopt);
    *
    * \param quantity is the quantity to display
    */
   template<typename T, typename = std::enable_if_t<is_non_optional<T>::value> > void setQuantity(std::optional<T> quantity);

   /**
    * \brief Set the quantity for a non-optional numeric field
    *
    * \param quantity is the quantity to display
    */
   template<typename T, typename = std::enable_if_t<is_non_optional<T>::value> > void setQuantity(T quantity);

   /**
    * \brief Usually a field is set via \c setQuantity because the units can be obtained from \c TypeInfo.  However, in
    *        certain circumstances, we need the caller to be able to supply both quantity and units, ie an Amount.  In
    *        particular, when the field type is \c Measurement::ChoiceOfPhysicalQuantity, it is not sufficient to call
    *        \c setQuantity as we will not be able determine units from field type.
    */
   void setAmount(Measurement::Amount const & amount);

   /**
    * \brief Normally, you set precision once when \c init is called via \c SMART_FIELD_INIT or similar.  However, if
    *        you really want to modify it on the fly, eg to have different precision for different units, this is what
    *        you call.  Note that you should call this before calling \c setQuantity.
    */
   void setPrecision(unsigned int const precision);
   [[nodiscard]] unsigned int getPrecision() const;

   /**
    * \brief If our field type is \b not \c NonPhysicalQuantity, then this returns the field converted to canonical
    *        units for the relevant \c Measurement::PhysicalQuantity.  (It is a coding error to call this function if
    *        our field type \c is \c NonPhysicalQuantity.)
    */
   Measurement::Amount getNonOptCanonicalAmt() const;

   /**
    * \brief As \c getNonOptCanonicalAmt but for optional fields
    */
   std::optional<Measurement::Amount> getOptCanonicalAmt() const;

   /**
    * \brief Same as calling \c quantity() on the result of \c getNonOptCanonicalAmt().
    */
   double getNonOptCanonicalQty() const;

   /**
    * \brief As \c getNonOptCanonicalQty but (with the obvious changes) for optional fields
    */
   std::optional<double> getOptCanonicalQty() const;


   /**
    * \brief Use this when you want to get the text as a number (and ignore any units or other trailling letters or
    *        symbols).
    *
    *        NOTE: If the field holds a \c PhysicalQuantity or \c PhysicalQuantities then this will return the same
    *              value as \c this->toCanonical().quantity().
    *
    *        This version is for non-optional (aka required) values.
    *
    *        Valid instantiations are \c int, \c unsigned \c int, \c double
    *
    * \param ok If set, used to return \c true if parsing of raw text went OK and \c false otherwise (in which case,
    *           function return value will be 0).
    */
   template<typename T> T getNonOptValue(bool * const ok = nullptr) const;

   /**
    * \brief As \c getNonOptValue but for std::optional values
    *
    *        NOTE: If the field holds a \c PhysicalQuantity or \c PhysicalQuantities then this will return the same
    *              value as \c this->toCanonical().quantity() for a non-blank field and \c std::nullopt for a blank
    *              field.
    *
    *        Valid instantiations are \c int, \c unsigned \c int, \c double
    *
    * \param ok If set, used to return \c true if parsing of raw text went OK and \c false otherwise (in which case,
    *           function return value will be \c std::nullopt).
    */
   template<typename T> std::optional<T> getOptValue(bool * const ok = nullptr) const;

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
