/*
 * BtLabel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#ifndef BTLABEL_H
#define BTLABEL_H
#pragma once

#include <optional>

#include <QAction>
#include <QHash>
#include <QLabel>
#include <QMenu>
#include <QPoint>

#include "BtFieldType.h"
#include "measurement/UnitSystem.h"
#include "UiAmountWithUnits.h" // For PreviousScaleInfo

/*!
 * \class BtLabel
 *
 * \brief Performs the necessary magic to select display units for any label.  Specifically, this allows the user to
 *        right-click on the label for a field and select
 *           (a) which unit system to use for that field (eg US Customary (mass), Imperial (mass) or Metric/SI (mass)
 *               for a weight field)
 *           (b) which units within that system to use for the field (eg kg, g, mg if the user has selected Metric/SI on
 *               a weight field).
 *        Moreover, the settings for each label are remembered (via PersistentSettings) for future times the program is
 *        run.
 *
 *        This has been a rather hidden feature of the program as there were no visual clues that right-clicking on a
 *        field label would bring up a useful menu (and it is not common behaviour in other software).  Where possible,
 *        we have now made it so that
 *          • mouseover on the label underlines the label text (hopefully making the user think of a clickable link),
 *          • where left-clicking would otherwise have no effect, it now has the same effect as right-click.
 *
 *        A \c BtLabel (or subclass thereof) will usually have a corresponding \c BtLineEdit (or subclass thereof).
 *        These two widgets will be Qt buddies, which just means that the \c BtLineEdit accepts the input focus on
 *        behalf of the \c BtLabel when the user types the label's shortcut key combination.
 *
 *        When the \c BtLabel needs to tell the \c BtLineEdit that the \c UnitSystem etc has changed, it sends a
 *        \c changedUnitSystemOrScale signal.   (Previously this signal was called \c labelChanged.)
 */
class BtLabel : public QLabel {
   Q_OBJECT

public:
   /*!
    * \brief Initialize the BtLabel with the parent and do some things with the type
    *
    * \param parent - QWidget* to the parent object
    * \param lType - the type of label: none, gravity, mass or volume
    *
    * \todo Not sure if I can get the name of the widget being created.
    *       Not sure how to signal the parent to redisplay
    */
   BtLabel(QWidget * parent, BtFieldType fieldType);
   virtual ~BtLabel();

   /**
    * \brief We override the \c QWidget event handlers \c enterEvent and \c leaveEvent to implement mouse-over effects
    *        on the label text - specifically to give the user a visual clue that the label text is (right)-clickable
    */
   virtual void enterEvent(QEvent* event);
   virtual void leaveEvent(QEvent* event);

   /**
    * \brief We override the \c QWidget event handler \c mouseReleaseEvent to capture left mouse clicks on us.  (Right
    *        clicks get notified to us via the \c QWidget::customContextMenuRequested signal.)
    */
   virtual void mouseReleaseEvent (QMouseEvent * event);

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
    *
    *          .:TODO:. Fix this comment and/or the code
    *        Note that we are OK to use std::optional here as, per https://doc.qt.io/qt-5/signalsandslots.html, "Signals
    *        and slots can take any number of arguments of any type. They are completely type safe."  HOWEVER, when
    *        referring to the function signature in .ui files, we need to remember to escape '<' to "&lt;" and '>' to
    *        "&gt;" because .ui files are XML.
    */
   void changedSystemOfMeasurementOrScale(PreviousScaleInfo previousScaleInfo);

// Using protected instead of private allows me to not use the friends
// declaration
protected:
   BtFieldType fieldType;
   QString propertyName;
   QString configSection;
   QWidget *btParent;
   QMenu* contextMenu;

   void initializeSection();
   void initializeProperty();
   void initializeMenu();

};

//
// These are trivial specialisations of BtLabel that make it possible to use specific types of BtLabel in .ui files.
// It's a bit of a sledgehammer way to pass in a constructor parameter but seems necessary because of limitations in Qt.
//
// AFAIK there is no way to pass constructor parameters to an object in a .ui file.  (If you want to do that, the advice
// seems to be to build the layout manually in C++ code.)
//
// Similarly, we might think to template BtLabel, but the Qt Meta-Object Compiler (moc) doesn't understand C++
// templates, so we can't do that for classes that need to use the Q_OBJECT macro (required for classes that declare
// their own signals and slots or that use other services provided by Qt's meta-object system).
//
class BtColorLabel :          public BtLabel { Q_OBJECT public: BtColorLabel(QWidget* parent = nullptr); };
class BtDensityLabel :        public BtLabel { Q_OBJECT public: BtDensityLabel(QWidget* parent = nullptr); };
class BtMassLabel :           public BtLabel { Q_OBJECT public: BtMassLabel(QWidget* parent = nullptr); };
class BtTemperatureLabel :    public BtLabel { Q_OBJECT public: BtTemperatureLabel(QWidget* parent = nullptr); };
class BtVolumeLabel :         public BtLabel { Q_OBJECT public: BtVolumeLabel(QWidget* parent = nullptr); };
class BtTimeLabel :           public BtLabel { Q_OBJECT public: BtTimeLabel(QWidget* parent = nullptr); };
class BtMixedLabel :          public BtLabel { Q_OBJECT public: BtMixedLabel(QWidget* parent = nullptr); };
class BtDateLabel :           public BtLabel { Q_OBJECT public: BtDateLabel(QWidget* parent = nullptr); };
class BtDiastaticPowerLabel : public BtLabel { Q_OBJECT public: BtDiastaticPowerLabel(QWidget* parent = nullptr); };
#endif
