/*
 * BtLineEdit.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023:
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef BTLINEEDIT_H
#define BTLINEEDIT_H
#pragma once

#include <QLineEdit>
#include <QString>
#include <QWidget>

#include "BtFieldType.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "UiAmountWithUnits.h"

class NamedEntity;

/*!
 * \class BtLineEdit
 *
 * \brief This extends QLineEdit such that the Object handles all the unit transformation we do, instead of each dialog.
 *        It makes the code much nicer and prevents more cut'n'paste code.
 *
 *        A \c BtLineEdit (or subclass thereof) will usually have a corresponding \c BtLabel (or subclass thereof).
 *        See comment in BtLabel.h for more details on the relationship between the two classes.
 *
 *        NB: Per https://doc.qt.io/qt-5/moc.html#multiple-inheritance-requires-qobject-to-be-first, "If you are using
 *        multiple inheritance, moc [Qt's Meta-Object Compiler] assumes that the first inherited class is a subclass of
 *        QObject. Also, be sure that only the first inherited class is a QObject."  In particular, this means we must
 *        put Q_PROPERTY declarations for UiAmountWithUnits attributes here rather than in UiAmountWithUnits itself.
 */
class BtLineEdit : public QLineEdit, public UiAmountWithUnits {
   Q_OBJECT

   Q_PROPERTY(QString configSection             READ getConfigSection                      WRITE setConfigSection                      STORED false)
   Q_PROPERTY(QString editField                 READ getEditField                          WRITE setEditField                          STORED false)
   Q_PROPERTY(QString forcedSystemOfMeasurement READ getForcedSystemOfMeasurementViaString WRITE setForcedSystemOfMeasurementViaString STORED false)
   Q_PROPERTY(QString forcedRelativeScale       READ getForcedRelativeScaleViaString       WRITE setForcedRelativeScaleViaString       STORED false)

public:
   /*!
    * \brief Initialize the BtLineEdit with the parent and do some things with the type
    *
    * \param parent - QWidget* to the parent object
    * \param lType - the type of label: none, gravity, mass or volume
    * \param maximalDisplayString - an example of the widest string this widget would be expected to need to display
    *
    * \todo Not sure if I can get the name of the widget being created.
    *       Not sure how to signal the parent to redisplay
    */
   BtLineEdit(QWidget* parent = nullptr,
              BtFieldType fieldType = NonPhysicalQuantity::String,
              Measurement::Unit const * units = nullptr,
              int const defaultPrecision = 3,
              QString const & maximalDisplayString = "100.000 srm");

   virtual ~BtLineEdit();

   virtual QString getWidgetText() const;
   virtual void setWidgetText(QString text);

   // Use one of these when you just want to set the text
   void setText(NamedEntity* element);
   void setText(NamedEntity* element, int precision);
   void setText(double amount);
   void setText(double amount, int precision);
   void setText(QString amount);
   void setText(QString amount, int precision);
   void setText(QVariant amount);
   void setText(QVariant amount, int precision);

   // Use this when you want to get the text as a number
   template<typename T> T getValueAs() const;

public slots:
   void onLineChanged();
   /**
    * \brief Received from \c BtLabel when the user has change \c UnitSystem
    *
    * This is mostly referenced in .ui files.  (NB this means that the signal connections are only checked at run-time.)
    */
   void lineChanged(PreviousScaleInfo previousScaleInfo);

signals:
   void textModified();

private:
   void calculateDisplaySize(QString const & maximalDisplayString);
   void setDisplaySize(bool recalculate = false);
   int const defaultPrecision;
   int desiredWidthInPixels;
};

//
// See comment in BtLabel.h for why we need all these trivial child classes to use in .ui files
//
// .:TODO:. We should change the inheritance hierarchy so that BtGenericEdit and BtStringEdit etc do not inherit from
//          UiAmountWithUnits.
//
class BtGenericEdit        : public BtLineEdit { Q_OBJECT public: BtGenericEdit       (QWidget* parent); };
class BtMassEdit           : public BtLineEdit { Q_OBJECT public: BtMassEdit          (QWidget* parent); };
class BtVolumeEdit         : public BtLineEdit { Q_OBJECT public: BtVolumeEdit        (QWidget* parent); };
class BtTimeEdit           : public BtLineEdit { Q_OBJECT public: BtTimeEdit          (QWidget* parent); };
class BtTemperatureEdit    : public BtLineEdit { Q_OBJECT public: BtTemperatureEdit   (QWidget* parent); };
class BtColorEdit          : public BtLineEdit { Q_OBJECT public: BtColorEdit         (QWidget* parent); };
class BtDensityEdit        : public BtLineEdit { Q_OBJECT public: BtDensityEdit       (QWidget* parent); };
class BtDiastaticPowerEdit : public BtLineEdit { Q_OBJECT public: BtDiastaticPowerEdit(QWidget* parent); };
class BtAcidityEdit        : public BtLineEdit { Q_OBJECT public: BtAcidityEdit       (QWidget* parent); };
class BtBitternessEdit     : public BtLineEdit { Q_OBJECT public: BtBitternessEdit    (QWidget* parent); };
class BtCarbonationEdit    : public BtLineEdit { Q_OBJECT public: BtCarbonationEdit   (QWidget* parent); };
class BtConcentrationEdit  : public BtLineEdit { Q_OBJECT public: BtConcentrationEdit (QWidget* parent); };
class BtViscosityEdit      : public BtLineEdit { Q_OBJECT public: BtViscosityEdit     (QWidget* parent); };
class BtStringEdit         : public BtLineEdit { Q_OBJECT public: BtStringEdit        (QWidget* parent); };
class BtPercentageEdit     : public BtLineEdit { Q_OBJECT public: BtPercentageEdit    (QWidget* parent); };
class BtDimensionlessEdit  : public BtLineEdit { Q_OBJECT public: BtDimensionlessEdit (QWidget* parent); };

// mixed objects are a pain.
class BtMixedEdit : public BtLineEdit {
   Q_OBJECT
public:
   BtMixedEdit(QWidget* parent);
public slots:
   void setIsWeight(bool state);
};

#endif
