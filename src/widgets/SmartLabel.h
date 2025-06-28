/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartLabel.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef WIDGETS_SMARTLABEL_H
#define WIDGETS_SMARTLABEL_H
#pragma once

#include <memory> // For PImpl
#include <optional>

#include <QAction>
#include <QHash>
#include <QLabel>
#include <QMenu>
#include <QPoint>

#include "measurement/UnitSystem.h"
#include "widgets/SmartAmounts.h"
#include "widgets/SmartBase.h"

class SmartField;

/*!
 * \class SmartLabel
 *
 * \brief Performs the necessary magic to select display units for any label.  Specifically, this allows the user to
 *        right-click on the label for a field and select
 *           (a) which unit system to use for that field (eg US Customary (mass), Imperial (mass) or Metric/SI (mass)
 *               for a weight field) - all done via \c Measurement::SystemOfMeasurement
 *           (b) which units within that system to use for the field (eg kg, g, mg if the user has selected Metric/SI on
 *               a weight field) - all done via \c Measurement::UnitSystem::RelativeScale.
 *        Moreover, the settings for each label‡ are remembered (via \c PersistentSettings) for future times the program
 *        is run.
 *
 *        This has been a rather hidden feature of the program as there were no visual clues that right-clicking on a
 *        field label would bring up a useful menu (and it is not common behaviour in other software).  Where possible,
 *        we have now made it so that
 *          • mouseover on the label underlines the label text (hopefully making the user think of a clickable link),
 *          • where left-clicking would otherwise have no effect, it now has the same effect as right-click.
 *
 *        A \c SmartLabel will usually, but not always, have a corresponding \c SmartField.  (HOWEVER, per the notes
 *        below, there are some cases where there is \b no corresponding \c SmartField and some cases where there is
 *        more than one corresponding \c SmartField.)  These \c SmartLabel and the \c SmartField (or one of them
 *        if there is more than one) will be Qt buddies, which mostly just means that the \c SmartField accepts the
 *        input focus on behalf of the \c SmartLabel when the user types the label's shortcut key combination.  (TBD: It
 *        also means we don't have to store a bunch of info in this object that we can just get from our buddy.  Eg
 *        \c QuantityFieldType is stored in \c SmartField, so we don't also need to store it here in \c SmartLabel.)
 *
 *        When the \c SmartLabel needs to tell the \c SmartField and/or other widgets (eg a range slider) that the
 *        \c UnitSystem etc has changed, it sends a \c changedUnitSystemOrScale signal.   (Previously this signal was
 *        called \c labelChanged.)
 *
 *        NOTE: There are some circumstances where a \c SmartLabel is associated with more than one \c SmartField.
 *              Typically this is where we have a min/max range (eg on a \c Style record in \c StyleEditor).  The
 *              underlying \c QLabel can only have a single "buddy", which is the last one passed in via
 *              \c QLabel::setBuddy (typically done via the property system from code generated from a .ui file).
 *              HOWEVER by calling \c SmartLabel::addBuddy instead, then all buddies can be remembered by \c SmartLabel
 *              (even though the base \c QLabel will only retain the last set one).  This means a change to forced
 *              \c SystemOfMeasurement or \c RelativeScale on the \c SmartLabel will be correctly remembered (via
 *              \c PersistentSettings) and applied to the multiple \c SmartField objects.
 *                 IN PRACTICE, you don't need to worry about this, provided you call \c SMART_FIELD_INIT (or
 *              similar) for each \c SmartField, everything will be handled correctly.
 *
 *        NOTE: There are also cases where a \c SmartLabel has no buddy.  Eg, in \c MainWindow, we have a number of
 *              sliders that show predicted gravity for the \c Recipe along with the gravity range for the \c Style.
 *
 *        NOTE: There is another set of edge cases where we have a \c SmartField that does not have a \c SmartLabel
 *              (just a \c QLabel).  This is either because there is no scale or unit system to change (eg because the
 *              field is a \c NonPhysicalQuantity such as \c NonPhysicalQuantity::Percentage) or, because the field is
 *              being used in a conversion tool where we want the GUI to accept/show only one type of units, but we
 *              still want to leverage all the magical conversion that \c SmartField knows how to do.
 *
 *        NOTE: Finally (I hope!) there is the case of \c BtTableModel subclasses.  In such table models, some of the
 *              columns will have user-selectable \c SystemOfMeasurement and/or \c RelativeScale.  In this case, the
 *              \c BtTableModel::ColumnInfo mini-class "owns" the settings.
 *
 *        ‡ Previously, we stored the settings not "per \c SmartLabel" but "per \c SmartField".  This is logical for
 *          the mainline case of (the \c SmartLabel has one \c SmartField).  However, it makes things a bit
 *          complicated for the two edge cases: (a) the \c SmartLabel has more than one \c SmartField and (b) the
 *          \c SmartLabel has more than no \c SmartField
 *
 *
 * This is extra work when
 *          a \c SmartLabel has two \c SmartField buddies, and creates the need for \c SmartLabel::addBuddy etc
 *          above.  In some ways it would be simpler to use the name of the \c SmartLabel as the lookup key in
 *          \c PersistentSettings.  For the moment, we do not as it would require extra complexity elsewhere.  Eg, the
 *          \c SmartLabel does not currently know its name, so we'd need to inject it via an additional parameter on
 *          \c SmartField::init and some more work in the \c SMART_FIELD_INIT and related macros.
 */
class SmartLabel : public QLabel, public SmartBase<SmartLabel> {
   Q_OBJECT

public:
   /*!
    * \brief Initialize the SmartLabel with the parent and do some things with the type
    *
    * \param parent - QWidget* to the parent object
    *
    * \todo Not sure if I can get the name of the widget being created.
    *       Not sure how to signal the parent to redisplay
    */
   SmartLabel(QWidget * parent);

   /**
    * \brief We assert that one \c SmartLabel cannot be the parent of another.  At time of writing, on GCC on Linux,
    *        if you have a function that takes, eg \c SmartLabel& but you accidentally pass it \c SmartLabel*, the
    *        compiler will try to construct a temporary \c SmartLabel with the \c SmartLabel* as a parameter.  We'd much
    *        prefer a compiler error in this case, so we explicitly delete the constructor that the compiler is being
    *        overly-helpful in trying to call.
    */
   SmartLabel(SmartLabel * parent) = delete;

   virtual ~SmartLabel();

   /**
    * \brief Post-construction initialisation.  Usually called from \c SmartFieldInit.
    *        NOTE that it is OK for this to be called multiple times when there are multiple \c SmartField "buddies"
    *        for this \c SmartLabel.
    *
    * \param editorName
    * \param labelName
    * \param labelFqName Fully qualified label name.  Usually a combination of \c editorName and \c labelName.
    * \param smartField Can be \c nullptr if there is no corresponding \c SmartField (eg because a slider is used
    *                      instead).
    */
   void init(char const * const   editorName,
             char const * const   labelName,
             char const * const   labelFqName,
             SmartField *         smartField,
             TypeInfo     const & typeInfo);

   /**
    * \return \c true if \c init or \c initFixed has been called, \c false otherwise
    */
   [[nodiscard]] bool isInitialised() const;

   /**
    * \brief Maybe for consistency this should be \c getSettings() but that jars somewhat!
    */
   [[nodiscard]] SmartAmountSettings const & settings() const;

   /**
    * \brief This is called by \c SmartBase and just wraps \c changedSystemOfMeasurementOrScale
    */
   void correctEnteredText(SmartAmounts::ScaleInfo previousScaleInfo);

   /**
    * \brief Converts a measurement (aka amount) to its numerical equivalent in whatever units are configured for this
    *        field.
    */
   double getAmountToDisplay(double const canonicalValue) const;

   /**
    * \brief Converts a range (ie min/max pair) of measurements (aka amounts) to its numerical equivalent in whatever
    *        units are configured for this field.
    *
    * \param canonicalValueMin
    * \param canonicalValueMax
    *
    * \return
    */
   QPair<double,double> getRangeToDisplay(double const canonicalValueMin,
                                          double const canonicalValueMax) const;

   /**
    * \brief We override the \c QWidget event handlers \c enterEvent and \c leaveEvent to implement mouse-over effects
    *        on the label text - specifically to give the user a visual clue that the label text is (right)-clickable
    */
   virtual void enterEvent(QEnterEvent* event) override;
   virtual void leaveEvent(QEvent* event) override;

   /**
    * \brief We override the \c QWidget event handler \c mouseReleaseEvent to capture left mouse clicks on us.  (Right
    *        clicks get notified to us via the \c QWidget::customContextMenuRequested signal.)
    */
   virtual void mouseReleaseEvent(QMouseEvent * event) override;


private:
   void textEffect(bool enabled);

public slots:
   /**
    * \brief Shows the pop-up menu to allow the user to override the units and/or scale for this field
    */
   void popContextMenu(const QPoint &point);

signals:
   /**
    * \brief Signal to say we changed the forced system of measurement and/or scale for a field (or group of fields)
    *
    *        NB: This is mostly referenced in .ui files, which compile to string-based connection syntax (see
    *        https://doc.qt.io/qt-5/signalsandslots-syntaxes.html for difference from functor-based syntax that we
    *        generally prefer to use in .cpp files).  Note too that, if you are manually editing a .ui file rather than
    *        using Qt Designer, you must NOT put parameter names in the function declarations in the \<signal\> and
    *        \<slot\> tags inside the \<connection\> tag.
    *
    *        The idea is that fields affected by a change in forced system of measurement or scale (including to/from
    *        "default") can take current value, convert it to Metric/SI under the "old" settings, then redisplay it with
    *        whatever the new settings are.  Because the fields don't store the "old" settings, we have to send them.
    *        (They can get the new ones just by calling \c Measurement::getUnitSystemForField() etc.
    *
    *        There will always be an old \c SystemOfMeasurement, even if it's the global default for this field's
    *        \c PhysicalQuantity.  There might not be an old \c RelativeScale though, hence the \c std::optional.
    */
   void changedSystemOfMeasurementOrScale(SmartAmounts::ScaleInfo previousScaleInfo);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a label object
   SmartLabel(SmartLabel const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a label object
   SmartLabel& operator=(SmartLabel const&) = delete;
   //! No move constructor
   SmartLabel(SmartLabel &&) = delete;
   //! No move assignment
   SmartLabel & operator=(SmartLabel &&) = delete;
};

#endif
