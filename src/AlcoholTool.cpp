/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * AlcoholTool.cpp is is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Ryan Hoobler <rhoob@yahoo.com>
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
#include "AlcoholTool.h"

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

#include "Algorithms.h"
#include "widgets/SmartLabel.h"
#include "widgets/SmartLineEdit.h"
#include "Localization.h"
#include "PersistentSettings.h"
#include "measurement/SystemOfMeasurement.h"
#include "widgets/ToggleSwitch.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_AlcoholTool.cpp"
#endif

// Settings we only use in this file under the PersistentSettings::Sections::alcoholTool section
#define AddSettingName(name) namespace { BtStringConst const name{#name}; }
AddSettingName(advancedInputsEnabled)
AddSettingName(hydrometerCalibrationTemperatureInC)
#undef AddSettingName


// This private implementation class holds all private non-virtual members of AlcoholTool
class AlcoholTool::impl {
public:
   /**
    * Constructor
    */
   impl(AlcoholTool & self) :
      self                         {self},
      label_reading                {new QLabel       (&self)},
      label_temperature            {new SmartLabel   (&self)},
      label_corrected              {new QLabel       (&self)},
      enableAdvancedInputs         {new ToggleSwitch (&self)},
      label_og                     {new SmartLabel   (&self)},
      input_og                     {new SmartLineEdit(&self)},
      input_og_temperature         {new SmartLineEdit(&self)},
      corrected_og                 {new QLabel       (&self)},
      label_fg                     {new SmartLabel   (&self)},
      input_fg                     {new SmartLineEdit(&self)},
      input_fg_temperature         {new SmartLineEdit(&self)},
      corrected_fg                 {new QLabel       (&self)},
      label_calibration_temperature{new SmartLabel   (&self)},
      input_calibration_temperature{new SmartLineEdit(&self)},
      label_result                 {new QLabel       (&self)},
      output_result                {new QLabel       (&self)},
      gridLayout                   {new QGridLayout  (&self)} {

      SMART_FIELD_INIT_FS(AlcoholTool, label_og                     , input_og                     , double, Measurement::PhysicalQuantity::Density    );
      SMART_FIELD_INIT_FS(AlcoholTool, label_fg                     , input_fg                     , double, Measurement::PhysicalQuantity::Density    );
      SMART_FIELD_INIT_FS(AlcoholTool, label_temperature            , input_og_temperature         , double, Measurement::PhysicalQuantity::Temperature);
      SMART_FIELD_INIT_FS(AlcoholTool, label_temperature            , input_fg_temperature         , double, Measurement::PhysicalQuantity::Temperature);
      SMART_FIELD_INIT_FS(AlcoholTool, label_calibration_temperature, input_calibration_temperature, double, Measurement::PhysicalQuantity::Temperature);

      this->restoreSettings();
      this->enableAdvancedInputs->setFont(QFont("Roboto medium", 13));
      this->output_result->setText("%");
      this->doLayout();

      this->connectSignals();
      return;
   }

   /**
    * Destructor
    *
    * Not much for us to do in the destructor.  Per https://doc.qt.io/qt-5/objecttrees.html, "When you create a QObject
    * with another object as parent, it's added to the parent's children() list, and is deleted when the parent is."
    *
    * I think, for similar reasons, we also do not need to delete QSpacerItem objects after they have been added to a
    * layout.
    */
   ~impl() = default;

   void doLayout() {
      this->input_og->setMinimumSize(QSize(80, 0));
      this->input_fg->setMinimumSize(QSize(80, 0));

      this->label_result->setObjectName(QStringLiteral("label_results"));
      this->label_result->setContextMenuPolicy(Qt::CustomContextMenu);

      this->output_result->setMinimumSize(QSize(80, 0));
      this->output_result->setObjectName(QStringLiteral("output_result"));

      this->gridLayout->addWidget(this->label_reading, 0, 1);
      this->gridLayout->addWidget(this->label_temperature, 0, 2);
      this->gridLayout->addWidget(this->label_corrected, 0, 3);
      this->gridLayout->addWidget(this->enableAdvancedInputs, 0, 4);

      this->gridLayout->addWidget(this->label_og, 1, 0);
      this->gridLayout->addWidget(this->input_og, 1, 1);
      this->gridLayout->addWidget(this->input_og_temperature, 1, 2);
      this->gridLayout->addWidget(this->corrected_og, 1, 3);

      this->gridLayout->addWidget(this->label_fg, 2, 0);
      this->gridLayout->addWidget(this->input_fg, 2, 1);
      this->gridLayout->addWidget(this->input_fg_temperature, 2, 2);
      this->gridLayout->addWidget(this->corrected_fg, 2, 3);

      this->gridLayout->addWidget(this->label_result, 3, 0);
      this->gridLayout->addWidget(this->output_result, 3, 1);

      this->gridLayout->addWidget(this->label_calibration_temperature, 1, 4);
      this->gridLayout->addWidget(this->input_calibration_temperature, 2, 4);

      this->showOrHideAdvancedControls();

      this->retranslateUi();
      return;
   }

   void showOrHideAdvancedControls() {
      bool visible = this->enableAdvancedInputs->isChecked();
      this->label_temperature            ->setVisible(visible);
      this->label_corrected              ->setVisible(visible);
      this->input_og_temperature         ->setVisible(visible);
      this->corrected_og                 ->setVisible(visible);
      this->input_fg_temperature         ->setVisible(visible);
      this->corrected_fg                 ->setVisible(visible);
      this->label_calibration_temperature->setVisible(visible);
      this->input_calibration_temperature->setVisible(visible);

      // The final ABV calculation depends on whether or not we are doing temperature correction, so we need to make
      // this call whenever we change the visibility of the advanced controls.
      this->updateCalculatedFields();
      return;
   }

   void updateCalculatedFields() {
      double og = this->input_og->getNonOptCanonicalQty();
      double fg = this->input_fg->getNonOptCanonicalQty();
      if (this->enableAdvancedInputs->isChecked()) {
         // User wants temperature correction
         double calibrationTempInC = this->input_calibration_temperature->getNonOptCanonicalQty();
         double ogReadTempInC          = this->input_og_temperature->getNonOptCanonicalQty();
         double fgReadTempInC          = this->input_fg_temperature->getNonOptCanonicalQty();
         if (0.0 == calibrationTempInC || 0.0 == ogReadTempInC) {
            og = 0.0;
            this->corrected_og->setText("? sg");
         } else {
            og = Algorithms::correctSgForTemperature(og, ogReadTempInC, calibrationTempInC);
            this->corrected_og->setText(Localization::getLocale().toString(og, 'f', 3).append(" sg"));
         }
         if (0.0 == calibrationTempInC || 0.0 == fgReadTempInC) {
            fg = 0.0;
            this->corrected_fg->setText("? sg");
         } else {
            fg = Algorithms::correctSgForTemperature(fg, fgReadTempInC, calibrationTempInC);
            this->corrected_fg->setText(Localization::getLocale().toString(fg, 'f', 3).append(" sg"));
         }
      }

      if (og != 0.0 && fg != 0.0 && og >= fg) {
         double abv = Algorithms::abvFromOgAndFg(og, fg);
         //
         // We want to show two decimal places so that the user has the choice about rounding.  In the UK, for instance,
         // for tax purposes, it is acceptable to truncate (rather than round) ABV to 1 decimal place, eg if your ABV is
         // 4.19% you declare it as 4.1% not 4.2%.
         //
         // Note that we do not use QString::number() as it does not honour the user's locale and instead always uses
         // QLocale::C, i.e., English/UnitedStates
         //
         // So, if ABV is, say, 5.179% the call to QLocale::toString() below will correctly round it to 5.18% and the user
         // can decide whether to use 5.1% or 5.2% on labels etc.
         //
         this->output_result->setText(Localization::getLocale().toString(abv, 'f', 2).append("%"));
         return;
      }

      this->output_result->setText("? %");
      return;
   }

   void connectSignals() {
      // If every input field triggers recalculation on modification then we don't need a "Convert" button
      connect(this->input_og                     , &SmartLineEdit::textModified, &self, &AlcoholTool::calculate);
      connect(this->input_fg                     , &SmartLineEdit::textModified, &self, &AlcoholTool::calculate);
      connect(this->input_og_temperature         , &SmartLineEdit::textModified, &self, &AlcoholTool::calculate);
      connect(this->input_fg_temperature         , &SmartLineEdit::textModified, &self, &AlcoholTool::calculate);
      connect(this->input_calibration_temperature, &SmartLineEdit::textModified, &self, &AlcoholTool::calculate);
      // This will also make the recalculation call after toggling the visibility of advanced controls
      connect(this->enableAdvancedInputs, &QAbstractButton::clicked, &self, &AlcoholTool::toggleAdvancedControls);

      return;
   }

   void retranslateUi() {
      self.setWindowTitle(tr("Alcohol Tool"));
      this->label_og                     ->setText(tr("Original Gravity (OG)"));
      this->label_result                 ->setText(tr("ABV"));
      this->label_fg                     ->setText(tr("Final Gravity (FG)"));
      this->label_reading                ->setText(tr("Reading"));
      this->label_temperature            ->setText(tr("Temperature"));
      this->label_corrected              ->setText(tr("Corrected Reading"));
      this->enableAdvancedInputs         ->setText(tr("Advanced Mode"));
      this->label_calibration_temperature->setText(tr("Hydrometer Calibration Temperature"));

#ifndef QT_NO_TOOLTIP
      qDebug() << Q_FUNC_INFO << "Setting tooltips and What's This help texts";
      this->input_og->setToolTip(tr("Initial Reading"));
      this->input_fg->setToolTip(tr("Final Reading"));
      this->output_result->setToolTip(tr("Result"));
      this->output_result->setWhatsThis(
         tr("Calculated according to the formula set by the UK Laboratory of the Government Chemist")
      );
#else
      qDebug() << Q_FUNC_INFO << "Tooltips not enabled in this build";
#endif
      return;
   }

   // Restore any previous settings
   void restoreSettings() {
      // Whether to show the temperature correction fields -- off by default
      this->enableAdvancedInputs->setChecked(
         PersistentSettings::value(advancedInputsEnabled,
                                   false,
                                   PersistentSettings::Sections::alcoholTool).toBool()
      );

      // Hydrometer calibration temperature -- default is 20°C, or 68°F in the old money.
      // Working out which units to use is already solved elsewhere in the code base, but you just have to be careful
      // not to do the conversion twice (ie 20°C -> 68°F ... 68°C -> 154°F) as both SmartLineEdit::setAmount() and
      // Measurement::amountDisplay() take SI unit and convert them to whatever the user has chosen to display.  So you
      // just need SmartLineEdit::setAmount().
      this->input_calibration_temperature->setQuantity(
         PersistentSettings::value(hydrometerCalibrationTemperatureInC,
                                    20.0,
                                    PersistentSettings::Sections::alcoholTool).toDouble()
      );
      return;
   }

   // Save any settings that the user is likely to want to have for next time
   void saveSettings() {
      PersistentSettings::insert(advancedInputsEnabled,
                                 this->enableAdvancedInputs->isChecked(),
                                 PersistentSettings::Sections::alcoholTool);
      PersistentSettings::insert(hydrometerCalibrationTemperatureInC,
                                 this->input_calibration_temperature->getNonOptCanonicalQty(),
                                 PersistentSettings::Sections::alcoholTool);
      return;
   }

   // Member variables for impl
   AlcoholTool   & self;
   QLabel        * label_reading;
   SmartLabel    * label_temperature;
   QLabel        * label_corrected;
   ToggleSwitch  * enableAdvancedInputs;
   SmartLabel    * label_og;
   SmartLineEdit * input_og;
   SmartLineEdit * input_og_temperature;
   QLabel        * corrected_og;
   SmartLabel    * label_fg;
   SmartLineEdit * input_fg;
   SmartLineEdit * input_fg_temperature;
   QLabel        * corrected_fg;
   SmartLabel    * label_calibration_temperature;
   SmartLineEdit * input_calibration_temperature;
   QPushButton   * pushButton_convert;
   QLabel        * label_result;
   QLabel        * output_result;
   QGridLayout   * gridLayout;
};

AlcoholTool::AlcoholTool(QWidget* parent) : QDialog(parent),
                                            pimpl{std::make_unique<impl>(*this)} {
   return;
}

AlcoholTool::~AlcoholTool() = default;

void AlcoholTool::calculate() {
   this->pimpl->updateCalculatedFields();
   return;
}

void AlcoholTool::toggleAdvancedControls() {
   this->pimpl->showOrHideAdvancedControls();
   return;
}

void AlcoholTool::changeEvent(QEvent* event) {
   if (event->type() == QEvent::LanguageChange) {
      this->pimpl->retranslateUi();
   }
   // Let base class do its work too
   this->QDialog::changeEvent(event);
   return;
}

void AlcoholTool::done(int r) {
   this->pimpl->saveSettings();
   // Let base class do its work too
   this->QDialog::done(r);
   return;
}
