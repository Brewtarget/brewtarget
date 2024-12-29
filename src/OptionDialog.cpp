/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * OptionDialog.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Idar Lund <idarlund@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "OptionDialog.h"

#include <optional>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QIcon>
#include <QMap>
#include <QMessageBox>
#include <QSizePolicy>
#include <QString>
#include <QVector>
#include <QWidget>

#include "config.h"
#include "database/Database.h"
#include "Localization.h"
#include "Logging.h"
#include "MainWindow.h"
#include "measurement/ColorMethods.h"
#include "measurement/IbuMethods.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "PersistentSettings.h"

//
// Anonymous namespace for constants, global variables and functions used only in this file
//
namespace {

   enum DbConnectionTestStates {
      NO_CHANGE,
      NEEDS_TEST,
      TEST_FAILED,
      TEST_PASSED
   };

   /**
    * \brief For a given QComboBox, save the UnitSystem it has selected
    *
    * \param comboBox
    * \param comboBoxName Used only for logging
    * \param physicalQuantity
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool saveComboBoxChoiceOfUnitSystem(QComboBox const & comboBox,
                                       char const * const comboBoxName,
                                       Measurement::PhysicalQuantity physicalQuantity) {
      // A QVariant can always be converted to a QString (albeit sometimes an empty one!) so there is no success/failure
      // notification from QVariant::toString()
      QString selection = comboBox.itemData(comboBox.currentIndex()).toString();
      auto unitSystems = Measurement::UnitSystem::getUnitSystems(physicalQuantity);
      for (auto unitSystem : unitSystems) {
         if (selection == unitSystem->uniqueName) {
            qDebug() <<
               Q_FUNC_INFO << "Setting UnitSystem for" << physicalQuantity << "to" << unitSystem->uniqueName;
            Measurement::setDisplayUnitSystem(physicalQuantity, *unitSystem);
            return true;
         }
      }

      qWarning() <<
         Q_FUNC_INFO << "Unable to interpret value " << selection << "of" << comboBoxName << "as UnitSystem name for" <<
         physicalQuantity;
      return false;
   }

}

// This private implementation class holds all private non-virtual members of OptionDialog
class OptionDialog::impl {
public:

   /**
    * Constructor
    */
   impl(OptionDialog & self) :
      m_self                     {self},
      qFileDialog                {&self},
      label_pgHostname           {self.groupBox_dbConfig},
      input_pgHostname           {self.groupBox_dbConfig},
      label_pgPortNum            {self.groupBox_dbConfig},
      input_pgPortNum            {self.groupBox_dbConfig},
      label_pgSchema             {self.groupBox_dbConfig},
      input_pgSchema             {self.groupBox_dbConfig},
      label_pgDbName             {self.groupBox_dbConfig},
      input_pgDbName             {self.groupBox_dbConfig},
      label_pgUsername           {self.groupBox_dbConfig},
      input_pgUsername           {self.groupBox_dbConfig},
      label_pgPassword           {self.groupBox_dbConfig},
      input_pgPassword           {self.groupBox_dbConfig},
      checkBox_savePgPassword    {self.groupBox_dbConfig},
      label_userDataDir          {self.groupBox_dbConfig},
      input_userDataDir          {self.groupBox_dbConfig},
      pushButton_browseDataDir   {self.groupBox_dbConfig},
      label_backupDir            {self.groupBox_dbConfig},
      input_backupDir            {self.groupBox_dbConfig},
      pushButton_browseBackupDir {self.groupBox_dbConfig},
      label_numBackups           {self.groupBox_dbConfig},
      spinBox_numBackups         {self.groupBox_dbConfig},
      label_frequency            {self.groupBox_dbConfig},
      spinBox_frequency          {self.groupBox_dbConfig} {
      //
      // Optimise the select file dialog to select directories
      //
      this->qFileDialog.setFileMode(QFileDialog::Directory);  // <- User can only select directories
      this->qFileDialog.setOptions(
         QFileDialog::ShowDirsOnly |        // <- Only show directories in the dialog
         QFileDialog::DontUseNativeDialog | // <- Use the Qt dialog for selecting directories as it's usually better at
         //    respecting all the other settings than the native dialog
         QFileDialog::HideNameFilterDetails // <- Don't have the file-types selector active, as it serves no purpose
      );                                    //    for selecting a directory
      this->qFileDialog.setFilter(QDir::AllDirs | QDir::Hidden); // <- We don't hide any directories from the user

      // PostgresSQL settings UI
      this->label_pgHostname.setObjectName(QStringLiteral("label_pgHostname"));
      this->input_pgHostname.setObjectName(QStringLiteral("input_pgHostname"));
      this->label_pgPortNum.setObjectName(QStringLiteral("label_pgPortNum"));
      this->input_pgPortNum.setObjectName(QStringLiteral("input_pgPortNum"));
      this->label_pgSchema.setObjectName(QStringLiteral("label_pgSchema"));
      this->input_pgSchema.setObjectName(QStringLiteral("input_pgSchema"));
      this->label_pgDbName.setObjectName(QStringLiteral("label_pgDbName"));
      this->input_pgDbName.setObjectName(QStringLiteral("input_pgDbName"));
      this->label_pgUsername.setObjectName(QStringLiteral("label_pgUsername"));
      this->input_pgUsername.setObjectName(QStringLiteral("input_pgUsername"));
      this->label_pgPassword.setObjectName(QStringLiteral("label_pgPassword"));
      this->input_pgPassword.setObjectName(QStringLiteral("input_pgPassword"));
      this->input_pgPassword.setEchoMode(QLineEdit::Password);
      this->checkBox_savePgPassword.setObjectName(QStringLiteral("checkBox_savePgPassword"));
      this->postgresVisible(false);

      // SQLite settings UI
      this->label_userDataDir.setObjectName(QStringLiteral("label_userDataDir"));
      this->input_userDataDir.setObjectName(QStringLiteral("input_userDataDir"));
      this->pushButton_browseDataDir.setObjectName(QStringLiteral("button_browseDataDir"));
      this->label_backupDir.setObjectName(QStringLiteral("label_backupDir"));
      this->input_backupDir.setObjectName(QStringLiteral("input_backupDir"));
      this->pushButton_browseBackupDir.setObjectName(QStringLiteral("button_browseBackupDir"));
      this->label_numBackups.setObjectName(QStringLiteral("label_numBackups"));
      this->spinBox_numBackups.setObjectName(QStringLiteral("spinBox_numBackups"));
      this->spinBox_numBackups.setMinimum(-1);
      this->spinBox_numBackups.setMaximum(9999);
      this->label_frequency.setObjectName(QStringLiteral("label_frequency"));
      this->spinBox_frequency.setObjectName(QStringLiteral("spinBox_frequency"));
      this->spinBox_frequency.setMinimum(1); // Couldn't make any semantic difference between 0 and 1. So start at 1
      this->spinBox_frequency.setMaximum(10);
      this->sqliteVisible(false);

      return;
   }

   void retranslateLanguageComboBox() {
      QVector<Localization::LanguageInfo> const & languageInfos {Localization::languageInfo()};
      for (int ii = 0; ii < languageInfos.size(); ++ii) {
         QString nameInCurrentLang = tr(languageInfos[ii].nameInEnglish);
         this->m_self.comboBox_lang->setItemText(ii, nameInCurrentLang);
      }
      return;
   }

   void initLangs() {
      QVector<Localization::LanguageInfo> const & languageInfos {Localization::languageInfo()};
      for (auto langInfo : languageInfos) {
         QString nameInCurrentLang = tr(langInfo.nameInEnglish);
         this->m_self.comboBox_lang->addItem(langInfo.countryFlag, nameInCurrentLang, langInfo.iso639_1Code);
      }

      //
      // Default icon size is 16 × 16, which, besides being the wrong shape for flags, is really too small on HDPI
      // screens.  (Most country flags are 3:2 width:height, or thereabouts.  And even those that are 2:1 width:height
      // will usually still be pretty recognisable in 3:2 format.
      //
      // We probably should adapt the icon size to the display resolution by creating a font in a given (display
      // independent) point size and then querying its actual display size in pixels etc etc, as is done in
      // RangedSlider.cpp.  But in the meantime, hopefully 36 wide × 24 high is an OK compromise for the various
      // different screen resolutions commonly in use.
      //
      this->m_self.comboBox_lang->setIconSize(QSize(36, 24));

      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   void postgresVisible(bool const canSee) {
      this->label_pgHostname.setVisible(canSee);
      this->input_pgHostname.setVisible(canSee);
      this->label_pgPortNum.setVisible(canSee);
      this->input_pgPortNum.setVisible(canSee);
      this->label_pgSchema.setVisible(canSee);
      this->input_pgSchema.setVisible(canSee);
      this->label_pgDbName.setVisible(canSee);
      this->input_pgDbName.setVisible(canSee);
      this->label_pgUsername.setVisible(canSee);
      this->input_pgUsername.setVisible(canSee);
      this->label_pgPassword.setVisible(canSee);
      this->input_pgPassword.setVisible(canSee);
      this->checkBox_savePgPassword.setVisible(canSee);
      this->label_pgPassword.setVisible(canSee);
      return;
   }

   void sqliteVisible(bool const canSee) {
      this->label_userDataDir.setVisible(canSee);
      this->input_userDataDir.setVisible(canSee);
      this->pushButton_browseDataDir.setVisible(canSee);
      this->label_backupDir.setVisible(canSee);
      this->input_backupDir.setVisible(canSee);
      this->pushButton_browseBackupDir.setVisible(canSee);
      this->label_numBackups.setVisible(canSee);
      this->spinBox_numBackups.setVisible(canSee);
      this->label_frequency.setVisible(canSee);
      this->spinBox_frequency.setVisible(canSee);
      return;
   }

   /**
    *
    */
   void clearLayout() {
      QLayoutItem * child;
      while ((child = this->m_self.gridLayout->takeAt(0)) != nullptr) {
         this->m_self.gridLayout->removeItem(child);
      }
      return;
   }

   /**
    * Determine which set of DB config params to show, based on whether PostgresSQL or SQLite is selected
    */
   void setDbDialog(Database::DbType db) {
      qDebug() <<
         Q_FUNC_INFO << "Set " << (db == Database::DbType::PGSQL ? "PostgresSQL" : "SQLite") << " config params visible";
      this->m_self.groupBox_dbConfig->setVisible(false);

      this->clearLayout();
      if (db == Database::DbType::PGSQL) {
         this->postgresVisible(true);
         this->sqliteVisible(false);

         this->m_self.gridLayout->addWidget(&this->label_pgHostname, 0, 0);
         this->m_self.gridLayout->addWidget(&this->input_pgHostname, 0, 1, 1, 2);

         this->m_self.gridLayout->addWidget(&this->label_pgPortNum, 0, 3);
         this->m_self.gridLayout->addWidget(&this->input_pgPortNum, 0, 4);

         this->m_self.gridLayout->addWidget(&this->label_pgSchema, 1, 0);
         this->m_self.gridLayout->addWidget(&this->input_pgSchema, 1, 1);

         this->m_self.gridLayout->addWidget(&this->label_pgDbName, 2, 0);
         this->m_self.gridLayout->addWidget(&this->input_pgDbName, 2, 1);

         this->m_self.gridLayout->addWidget(&this->label_pgUsername, 3, 0);
         this->m_self.gridLayout->addWidget(&this->input_pgUsername, 3, 1);

         this->m_self.gridLayout->addWidget(&this->label_pgPassword, 4, 0);
         this->m_self.gridLayout->addWidget(&this->input_pgPassword, 4, 1);

         this->m_self.gridLayout->addWidget(&this->checkBox_savePgPassword, 4, 4);

      } else {
         this->postgresVisible(false);
         this->sqliteVisible(true);

         this->m_self.gridLayout->addWidget(&this->label_userDataDir, 0, 0);
         this->m_self.gridLayout->addWidget(&this->input_userDataDir, 0, 1, 1, 2);
         this->m_self.gridLayout->addWidget(&this->pushButton_browseDataDir, 0, 3);

         this->m_self.gridLayout->addWidget(&this->label_backupDir, 1, 0);
         this->m_self.gridLayout->addWidget(&this->input_backupDir, 1, 1, 1, 2);
         this->m_self.gridLayout->addWidget(&this->pushButton_browseBackupDir, 1, 3);

         this->m_self.gridLayout->addWidget(&this->label_numBackups, 3, 0);
         this->m_self.gridLayout->addWidget(&this->spinBox_numBackups, 3, 1);

         this->m_self.gridLayout->addWidget(&this->label_frequency, 4, 0);
         this->m_self.gridLayout->addWidget(&this->spinBox_frequency, 4, 1);
      }
      this->m_self.groupBox_dbConfig->setVisible(true);
      return;
   }

   void retranslateDbDialog() {
      //PostgreSQL stuff
      this->label_pgHostname.setText(QApplication::translate("optionsDialog", "Hostname", nullptr));
      this->label_pgPortNum.setText(QApplication::translate("optionsDialog", "Port", nullptr));
      this->label_pgSchema.setText(QApplication::translate("optionsDialog", "Schema", nullptr));
      this->label_pgDbName.setText(QApplication::translate("optionsDialog", "Database", nullptr));
      this->label_pgUsername.setText(QApplication::translate("optionsDialog", "Username", nullptr));
      this->label_pgPassword.setText(QApplication::translate("optionsDialog", "Password", nullptr));
      this->checkBox_savePgPassword.setText(QApplication::translate("optionsDialog", "Save password", nullptr));

      // SQLite things
      this->label_userDataDir.setText(QApplication::translate("optionsDialog", "Data Directory", nullptr));
      this->pushButton_browseDataDir.setText(QApplication::translate("optionsDialog", "Browse", nullptr));
      this->label_backupDir.setText(QApplication::translate("optionsDialog", "Backup Directory", nullptr));
      this->pushButton_browseBackupDir.setText(QApplication::translate("optionsDialog", "Browse", nullptr));
      this->label_numBackups.setText(QApplication::translate("optionsDialog", "Number of Backups", nullptr));
      this->label_frequency.setText(QApplication::translate("optionsDialog", "Frequency of Backups", nullptr));

      // set up the tooltips if we are using them
#ifndef QT_NO_TOOLTIP
      this->input_pgHostname.setToolTip(QApplication::translate("optionsDialog", "PostgresSQL's host name or IP address",
                                                                nullptr));
      this->input_pgPortNum.setToolTip(QApplication::translate("optionsDialog", "Port the PostgreSQL is listening on",
                                                               nullptr));
      this->input_pgSchema.setToolTip(QApplication::translate("optionsDialog", "The schema containing the database",
                                                              nullptr));
      this->input_pgUsername.setToolTip(QApplication::translate("optionsDialog", "User with create/delete table access",
                                                                nullptr));
      this->input_pgPassword.setToolTip(QApplication::translate("optionsDialog", "Password for the user", nullptr));
      this->input_pgDbName.setToolTip(QApplication::translate("optionsDialog", "The name of the database", nullptr));
      this->label_userDataDir.setToolTip(QApplication::translate("optionsDialog", "Where your database file is", nullptr));
      this->label_backupDir.setToolTip(QApplication::translate("optionsDialog", "Where to save your backups", nullptr));
      this->label_numBackups.setToolTip(QApplication::translate("optionsDialog",
                                                                "Number of backups to keep: -1 means never remove, 0 means never backup", nullptr));
      // Actually the backups happen after every X times the program is closed, but the tooltip is already long enough!
      this->label_frequency.setToolTip(QApplication::translate("optionsDialog",
                                                               "How many times %1 needs to be run to trigger another backup: 1 means always backup", nullptr).arg(CONFIG_APPLICATION_NAME_UC));
#endif
      return;
   }

   /**
    * \brief Update UI strings according to current language.
    */
   void retranslate() {
      // Let the Ui take care of its business
      this->m_self.retranslateUi(&this->m_self);
      this->retranslateDbDialog();

      this->retranslateLanguageComboBox();

      return;
   }

   void changeColors() {
      // Yellow when the test is needed
      // Red when the test failed
      // Green when the test passed
      // Black otherwise.

      switch (dbConnectionTestState) {
         case NEEDS_TEST:
            this->m_self.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            this->m_self.pushButton_testConnection->setEnabled(true);
            this->m_self.pushButton_testConnection->setStyleSheet("color:rgb(240,225,25)");
            break;
         case TEST_FAILED:
            this->m_self.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            this->m_self.pushButton_testConnection->setStyleSheet("color:red");
            break;
         case TEST_PASSED:
            this->m_self.pushButton_testConnection->setStyleSheet("color:green");
            this->m_self.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            this->m_self.pushButton_testConnection->setEnabled(false);
            break;
         case NO_CHANGE:
            this->m_self.pushButton_testConnection->setStyleSheet("color:grey");
            this->m_self.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            this->m_self.pushButton_testConnection->setEnabled(false);
            break;
      }
      return;
   }

   /**
    * \brief Update dialog with current options.
    */
   void showChanges() {
      // Set the right language
      int index = this->m_self.comboBox_lang->findData(Localization::getCurrentLanguage());
      if (index >= 0) {
         this->m_self.comboBox_lang->setCurrentIndex(index);
      }

      this->m_self.weightComboBox->setCurrentIndex(
         this->m_self.weightComboBox->findData(
            Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Mass).uniqueName
         )
      );
      this->m_self.temperatureComboBox->setCurrentIndex(
         this->m_self.temperatureComboBox->findData(
            Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Temperature).uniqueName
         )
      );
      this->m_self.volumeComboBox->setCurrentIndex(
         this->m_self.volumeComboBox->findData(
            Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume).uniqueName
         )
      );
      this->m_self.gravityComboBox->setCurrentIndex(
         this->m_self.gravityComboBox->findData(
            Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Density).uniqueName
         )
      );
      this->m_self.dateComboBox->setCurrentIndex(this->m_self.dateComboBox->findData(Localization::getDateFormat()));
      this->m_self.colorComboBox->setCurrentIndex(
         this->m_self.colorComboBox->findData(
            Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Color).uniqueName
         )
      );
      this->m_self.diastaticPowerComboBox->setCurrentIndex(
         this->m_self.diastaticPowerComboBox->findData(
            Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::DiastaticPower).uniqueName
         )
      );

      this->m_self.colorFormulaComboBox->setCurrentIndex(
         this->m_self.colorFormulaComboBox->findData(ColorMethods::colorFormula)
      );
      this->m_self.ibuFormulaComboBox->setCurrentIndex(
         this->m_self.ibuFormulaComboBox->findData(static_cast<int>(IbuMethods::ibuFormula))
      );

      // User data directory
      this->input_userDataDir.setText(PersistentSettings::getUserDataDir().absolutePath());

      // Backup stuff
      // By default backups go in the same directory as the DB
      this->input_backupDir.setText(PersistentSettings::value(PersistentSettings::Names::directory,
                                                              PersistentSettings::getUserDataDir().absolutePath(),
                                                              PersistentSettings::Sections::backups).toString());
      this->spinBox_numBackups.setValue(PersistentSettings::value(PersistentSettings::Names::maximum,
                                                                  10,
                                                                  PersistentSettings::Sections::backups).toInt());
      this->spinBox_frequency.setValue(PersistentSettings::value(PersistentSettings::Names::frequency,
                                                                 4,
                                                                 PersistentSettings::Sections::backups).toInt());

      // The IBU modifications. These will all be calculated from a 60 min boil. This is gonna get confusing.
      double amt = Localization::toDouble(
         PersistentSettings::value(PersistentSettings::Names::mashHopAdjustment, 0).toString(),
         Q_FUNC_INFO
      );
      this->m_self.ibuAdjustmentMashHopDoubleSpinBox->setValue(amt * 100);

      amt = Localization::toDouble(
         PersistentSettings::value(PersistentSettings::Names::firstWortHopAdjustment, 1.1).toString(),
         Q_FUNC_INFO
      );
      this->m_self.ibuAdjustmentFirstWortDoubleSpinBox->setValue(amt * 100);

      // Database stuff -- this looks weird, but trust me. We want SQLITE to be
      // the default for this field
      int tmp = PersistentSettings::value(PersistentSettings::Names::dbType,
                                          static_cast<int>(Database::DbType::SQLITE)).toInt() - 1;
      this->m_self.comboBox_engine->setCurrentIndex(tmp);

      this->input_pgHostname.setText(PersistentSettings::value(PersistentSettings::Names::dbHostname, "localhost").toString());
      this->input_pgPortNum.setText(PersistentSettings::value(PersistentSettings::Names::dbPortnum, "5432").toString());
      this->input_pgSchema.setText(PersistentSettings::value(PersistentSettings::Names::dbSchema, "public").toString());
      this->input_pgDbName.setText(PersistentSettings::value(PersistentSettings::Names::dbName, CONFIG_APPLICATION_NAME_LC).toString());
      this->input_pgUsername.setText(PersistentSettings::value(PersistentSettings::Names::dbUsername, CONFIG_APPLICATION_NAME_LC).toString());
      this->input_pgPassword.setText(PersistentSettings::value(PersistentSettings::Names::dbPassword, "").toString());
      this->checkBox_savePgPassword.setChecked(PersistentSettings::contains(PersistentSettings::Names::dbPassword));

      this->dbConnectionTestState = NO_CHANGE;
      this->changeColors();

      if (RecipeHelper::getAutomaticVersioningEnabled()) {
         this->m_self.checkBox_versioning->setCheckState(Qt::Checked);
         this->m_self.groupBox_deleteBehavior->setEnabled(true);
         switch (PersistentSettings::value(PersistentSettings::Names::deletewhat, Recipe::DESCENDANT).toInt()) {
            case Recipe::ANCESTOR:
               this->m_self.radioButton_deleteAncestor->setChecked(true);
               break;
            default:
               this->m_self.radioButton_deleteDescendant->setChecked(true);
               break;
         }
      } else {
         this->m_self.checkBox_versioning->setCheckState(Qt::Unchecked);
         this->m_self.groupBox_deleteBehavior->setEnabled(false);
      }

      if (PersistentSettings::value(PersistentSettings::Names::showsnapshots, false).toBool()) {
         this->m_self.checkBox_alwaysShowSnaps->setCheckState(Qt::Checked);
      } else {
         this->m_self.checkBox_alwaysShowSnaps->setCheckState(Qt::Unchecked);
      }

      return;
   }

   // =========================================== Member variables for impl ============================================
   OptionDialog & m_self;

   // Used for selecting directories
   QFileDialog qFileDialog;

   // UI stuff to make this work as I want
   // Postgres things
   QLabel    label_pgHostname;
   QLineEdit input_pgHostname;
   QLabel    label_pgPortNum;
   QLineEdit input_pgPortNum;
   QLabel    label_pgSchema;
   QLineEdit input_pgSchema;
   QLabel    label_pgDbName;
   QLineEdit input_pgDbName;
   QLabel    label_pgUsername;
   QLineEdit input_pgUsername;
   QLabel    label_pgPassword;
   QLineEdit input_pgPassword;
   QCheckBox checkBox_savePgPassword;
   // SQLite things
   QLabel      label_userDataDir;
   QLineEdit   input_userDataDir;
   QPushButton pushButton_browseDataDir;
   QLabel      label_backupDir;
   QLineEdit   input_backupDir;
   QPushButton pushButton_browseBackupDir;
   QLabel      label_numBackups;
   QSpinBox    spinBox_numBackups;
   QLabel      label_frequency;
   QSpinBox    spinBox_frequency;

   DbConnectionTestStates dbConnectionTestState;

};

OptionDialog::OptionDialog(QWidget * parent) : QDialog{},
   Ui::optionsDialog{},
   pimpl{std::make_unique<impl>(*this)} {

   // I need a lot of control over what is displayed on the DbConfig dialog.
   // Maybe designer can do it? No idea. So I did this hybrid model, and I
   // think it will end up biting my ...
   // anyway. It isn't pretty
   this->setupUi(this);
   this->pimpl->initLangs();

   if (parent != nullptr) {
      setWindowIcon(parent->windowIcon());
   }

   // populate the combo boxes on the units tab
   configure_unitCombos();

   // populate the combo boxes on the formulas tab
   configure_formulaCombos();

   // populate the combo boxes on the logging tab
   configure_logging();

   // database panel stuff
   comboBox_engine->addItem(tr("SQLite (default)"), QVariant(static_cast<int>(Database::DbType::SQLITE)));
   comboBox_engine->addItem(tr("PostgreSQL"), QVariant(static_cast<int>(Database::DbType::PGSQL)));

   // figure out which database we have
   int idx = comboBox_engine->findData(PersistentSettings::value(PersistentSettings::Names::dbType,
                                                                 static_cast<int>(Database::DbType::SQLITE)).toInt());
   this->pimpl->setDbDialog(static_cast<Database::DbType>(idx));

   // connect all the signals
   connect_signals();

   pushButton_testConnection->setEnabled(false);
}

void OptionDialog::configure_unitCombos() {
   // Populate combo boxes on the "Units" tab
   weightComboBox->addItem(tr("Metric / SI units"),
                           QVariant(Measurement::UnitSystems::mass_Metric.uniqueName));
   weightComboBox->addItem(tr("US traditional units"),
                           QVariant(Measurement::UnitSystems::mass_UsCustomary.uniqueName));
   weightComboBox->addItem(tr("British imperial units"),
                           QVariant(Measurement::UnitSystems::mass_Imperial.uniqueName));

   temperatureComboBox->addItem(tr("Celsius"),
                                QVariant(Measurement::UnitSystems::temperature_MetricIsCelsius.uniqueName));
   temperatureComboBox->addItem(tr("Fahrenheit"),
                                QVariant(Measurement::UnitSystems::temperature_UsCustomaryIsFahrenheit.uniqueName));

   volumeComboBox->addItem(tr("Metric / SI units"),      QVariant(Measurement::UnitSystems::volume_Metric.uniqueName));
   volumeComboBox->addItem(tr("US traditional units"),   QVariant(Measurement::UnitSystems::volume_UsCustomary.uniqueName));
   volumeComboBox->addItem(tr("British imperial units"), QVariant(Measurement::UnitSystems::volume_Imperial.uniqueName));

   gravityComboBox->addItem(tr("20C/20C Specific Gravity"),
                            QVariant(Measurement::UnitSystems::density_SpecificGravity.uniqueName));
   gravityComboBox->addItem(tr("Plato/Brix/Balling"),
                            QVariant(Measurement::UnitSystems::density_Plato.uniqueName));

   dateComboBox->addItem(tr("mm-dd-YYYY"), QVariant(Localization::NumericDateFormat::MonthDayYear));
   dateComboBox->addItem(tr("dd-mm-YYYY"), QVariant(Localization::NumericDateFormat::DayMonthYear));
   dateComboBox->addItem(tr("YYYY-mm-dd"), QVariant(Localization::NumericDateFormat::YearMonthDay));

   colorComboBox->addItem(tr("SRM"), QVariant(Measurement::UnitSystems::color_StandardReferenceMethod.uniqueName));
   colorComboBox->addItem(tr("EBC"), QVariant(Measurement::UnitSystems::color_EuropeanBreweryConvention.uniqueName));
}

void OptionDialog::configure_formulaCombos() {
   diastaticPowerComboBox->addItem(tr("Lintner"),
                                   QVariant(Measurement::UnitSystems::diastaticPower_Lintner.uniqueName));
   diastaticPowerComboBox->addItem(tr("WK"),
                                   QVariant(Measurement::UnitSystems::diastaticPower_WindischKolbach.uniqueName));

   // Populate combo boxes on the "Formulas" tab
   ibuFormulaComboBox->addItem(tr("Tinseth's approximation"), QVariant(static_cast<int>(IbuMethods::IbuFormula::Tinseth)));
   ibuFormulaComboBox->addItem(tr("Rager's approximation"  ), QVariant(static_cast<int>(IbuMethods::IbuFormula::Rager  )));
   ibuFormulaComboBox->addItem(tr("Noonan's approximation" ), QVariant(static_cast<int>(IbuMethods::IbuFormula::Noonan )));

   colorFormulaComboBox->addItem(tr("Mosher's approximation"), QVariant(ColorMethods::MOSHER));
   colorFormulaComboBox->addItem(tr("Daniel's approximation"), QVariant(ColorMethods::DANIEL));
   colorFormulaComboBox->addItem(tr("Morey's approximation" ), QVariant(ColorMethods::MOREY));
}

void OptionDialog::configure_logging() {
   //Populate options on the "Logging" tab
   for (auto ii : Logging::levelDetails) {
      loggingLevelComboBox->addItem(ii.description, QVariant(ii.level));
   }
   loggingLevelComboBox->setCurrentIndex(Logging::getLogLevel());
   checkBox_LogFileLocationUseDefault->setChecked(Logging::getLogInConfigDir());
   lineEdit_LogFileLocation->setText(Logging::getDirectory().absolutePath());
   this->setFileLocationState(Logging::getLogInConfigDir());
   return;
}

void OptionDialog::connect_signals() {
   connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionDialog::saveAndClose);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &OptionDialog::cancel);

   // QOverload is needed on next line because the signal currentIndexChanged is overloaded in QComboBox - see
   // https://doc.qt.io/qt-5/qcombobox.html#currentIndexChanged
   connect(comboBox_engine, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OptionDialog::setEngine);
   connect(pushButton_testConnection, &QAbstractButton::clicked, this, &OptionDialog::testConnection);

   // figure out which database we have
   int idx = comboBox_engine->findData(PersistentSettings::value(PersistentSettings::Names::dbType,
                                                                 static_cast<int>(Database::DbType::SQLITE)).toInt());
   this->pimpl->setDbDialog(static_cast<Database::DbType>(idx));

   // Set the signals
   connect(&this->pimpl->checkBox_savePgPassword, &QAbstractButton::clicked, this, &OptionDialog::savePassword);
   connect(this->checkBox_LogFileLocationUseDefault, &QAbstractButton::clicked, this, &OptionDialog::setFileLocationState);

   connect(&this->pimpl->input_pgHostname, &QLineEdit::editingFinished, this, &OptionDialog::testRequired);
   connect(&this->pimpl->input_pgPortNum,  &QLineEdit::editingFinished, this, &OptionDialog::testRequired);
   connect(&this->pimpl->input_pgSchema,   &QLineEdit::editingFinished, this, &OptionDialog::testRequired);
   connect(&this->pimpl->input_pgDbName,   &QLineEdit::editingFinished, this, &OptionDialog::testRequired);
   connect(&this->pimpl->input_pgUsername, &QLineEdit::editingFinished, this, &OptionDialog::testRequired);
   connect(&this->pimpl->input_pgPassword, &QLineEdit::editingFinished, this, &OptionDialog::testRequired);

   connect(&this->pimpl->pushButton_browseDataDir, &QAbstractButton::clicked, this, &OptionDialog::setDataDir);
   connect(&this->pimpl->pushButton_browseBackupDir, &QAbstractButton::clicked, this, &OptionDialog::setBackupDir);
   connect(pushButton_resetToDefault, &QAbstractButton::clicked, this, &OptionDialog::resetToDefault);
   connect(pushButton_LogFileLocationBrowse, &QAbstractButton::clicked, this, &OptionDialog::setLogDir);
   pushButton_testConnection->setEnabled(false);

   connect(checkBox_versioning, &QAbstractButton::clicked, this, &OptionDialog::versioningChanged);
   connect(checkBox_alwaysShowSnaps, &QAbstractButton::clicked, this, &OptionDialog::signalAncestors);

   // Call this here to set up translatable strings.
   this->pimpl->retranslate();
   return;
}

void OptionDialog::signalAncestors() {
   emit showAllAncestors(checkBox_alwaysShowSnaps->checkState() == Qt::Checked);
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
OptionDialog::~OptionDialog() = default;


void OptionDialog::show() {
   this->pimpl->showChanges();
   this->setVisible(true);
   return;
}

void OptionDialog::setDataDir() {
   this->pimpl->qFileDialog.setDirectory(this->pimpl->input_userDataDir.text());
   this->pimpl->qFileDialog.setWindowTitle(tr("Choose User Data Directory"));
   if (this->pimpl->qFileDialog.exec() == QDialog::Accepted && this->pimpl->qFileDialog.selectedFiles().size() > 0) {
      this->pimpl->input_userDataDir.setText(this->pimpl->qFileDialog.selectedFiles().value(0));
   }
   return;
}

void OptionDialog::setBackupDir() {
   this->pimpl->qFileDialog.setDirectory(this->pimpl->input_backupDir.text());
   this->pimpl->qFileDialog.setWindowTitle(tr("Choose Backups Directory"));
   if (this->pimpl->qFileDialog.exec() == QDialog::Accepted && this->pimpl->qFileDialog.selectedFiles().size() > 0) {
      this->pimpl->input_backupDir.setText(this->pimpl->qFileDialog.selectedFiles().value(0));
   }
   return;
}

void OptionDialog::setLogDir() {
   this->pimpl->qFileDialog.setDirectory(lineEdit_LogFileLocation->text());
   this->pimpl->qFileDialog.setWindowTitle(tr("Choose Logging Directory"));
   if (this->pimpl->qFileDialog.exec() == QDialog::Accepted && this->pimpl->qFileDialog.selectedFiles().size() > 0) {
      lineEdit_LogFileLocation->setText(this->pimpl->qFileDialog.selectedFiles().value(0));
   }
   return;
}

void OptionDialog::resetToDefault() {
   Database::DbType engine = static_cast<Database::DbType>(comboBox_engine->currentData().toInt());
   if (engine == Database::DbType::PGSQL) {
      this->pimpl->input_pgHostname.setText(QString("localhost"));
      this->pimpl->input_pgPortNum.setText(QString("5432"));
      this->pimpl->input_pgSchema.setText(QString("public"));
      this->pimpl->input_pgDbName.setText(QString(CONFIG_APPLICATION_NAME_LC));
      this->pimpl->input_pgUsername.setText(QString(CONFIG_APPLICATION_NAME_LC));
      this->pimpl->input_pgPassword.setText(QString(""));
      this->pimpl->checkBox_savePgPassword.setChecked(false);
   } else {
      this->pimpl->input_userDataDir.setText(PersistentSettings::getConfigDir().absolutePath());
      this->pimpl->input_backupDir.setText(PersistentSettings::getConfigDir().absolutePath());
      this->pimpl->spinBox_frequency.setValue(4);
      this->pimpl->spinBox_numBackups.setValue(10);
   }
}


void OptionDialog::cancel() {
   this->setVisible(false);
   return;
}


void OptionDialog::changeEvent(QEvent * e) {
   switch (e->type()) {
      case QEvent::LanguageChange:
         this->pimpl->retranslate();
         e->accept();
         break;
      default:
         QDialog::changeEvent(e);
         break;
   }
   return;
}

void OptionDialog::setEngine([[maybe_unused]] int selected) {

   QVariant data = comboBox_engine->currentData();
   Database::DbType newEngine = static_cast<Database::DbType>(data.toInt());

   this->pimpl->setDbDialog(newEngine);
   this->testRequired();
   return;
}

void OptionDialog::testConnection() {
   bool success;
   QString hostname, schema, database, username, password;
   int port;

   Database::DbType newType = static_cast<Database::DbType>(comboBox_engine->currentData().toInt());
   // Do nothing if nothing is required.
   if (this->pimpl->dbConnectionTestState == NO_CHANGE || this->pimpl->dbConnectionTestState == TEST_PASSED) {
      return;
   }

   switch (newType) {
      case Database::DbType::PGSQL:
         hostname = this->pimpl->input_pgHostname.text();
         schema   = this->pimpl->input_pgSchema.text();
         database = this->pimpl->input_pgDbName.text();
         username = this->pimpl->input_pgUsername.text();
         password = this->pimpl->input_pgPassword.text();
         port     = this->pimpl->input_pgPortNum.text().toInt();

         success = Database::verifyDbConnection(newType, hostname, port, schema, database, username, password);
         break;
      default:
         hostname = QString("%1/%2").arg(this->pimpl->input_userDataDir.text()).arg("database.sqlite");
         success = Database::verifyDbConnection(newType, hostname);
   }

   if (success) {
      QMessageBox::information(nullptr,
                               QObject::tr("Connection Test"),
                               QString(QObject::tr("Connection to database was successful")));
      this->pimpl->dbConnectionTestState = TEST_PASSED;
   } else {
      // Database::testConnection already popped the dialog
      this->pimpl->dbConnectionTestState = TEST_FAILED;
   }
   this->pimpl->changeColors();
   return;
}

void OptionDialog::testRequired() {
   this->pimpl->dbConnectionTestState = NEEDS_TEST;
   this->pimpl->changeColors();
   return;
}


void OptionDialog::savePassword(bool state) {
   if (state) {
      QMessageBox::warning(
         nullptr,
         QObject::tr("Plaintext"),
         QObject::tr("Passwords are saved in plaintext. We make no effort to hide, obscure or otherwise protect the "
                     "password. By enabling this option, you take full responsibility for any potential problems.")
      );
   }
   return;
}

void OptionDialog::setFileLocationState(bool state) {
   this->lineEdit_LogFileLocation->setEnabled(! state);
   this->pushButton_LogFileLocationBrowse->setEnabled(! state);
   return;
}

void OptionDialog::versioningChanged(bool state) {
   groupBox_deleteBehavior->setEnabled(state);
}

void OptionDialog::saveAndClose() {
   saveDatabaseConfig();
   saveDefaultUnits();
   saveFormulae();
   saveLoggingSettings();
   saveVersioningSettings();

   // Set the right language.
   Localization::setLanguage(this->comboBox_lang->currentData().toString());

   setVisible(false);
}

bool OptionDialog::saveDefaultUnits() {
   bool okay = true;

   okay &= saveWeightUnits();
   okay &= saveTemperatureUnits();
   okay &= saveVolumeUnits();
   okay &= saveGravityUnits();
   okay &= saveDateFormat();
   okay &= saveColorUnits();
   okay &= saveDiastaticUnits();

   return okay;
}

void OptionDialog::saveFormulae() {
   bool okay = false;

   int ndx = ibuFormulaComboBox->itemData(ibuFormulaComboBox->currentIndex()).toInt(&okay);
   IbuMethods::ibuFormula = static_cast<IbuMethods::IbuFormula>(ndx);
   ndx = colorFormulaComboBox->itemData(colorFormulaComboBox->currentIndex()).toInt(&okay);
   ColorMethods::colorFormula = static_cast<ColorMethods::ColorType>(ndx);

   PersistentSettings::insert(PersistentSettings::Names::mashHopAdjustment, ibuAdjustmentMashHopDoubleSpinBox->value() / 100);
   PersistentSettings::insert(PersistentSettings::Names::firstWortHopAdjustment, ibuAdjustmentFirstWortDoubleSpinBox->value() / 100);
}

void OptionDialog::saveLoggingSettings() {
   // Saving Logging Options to the Log object
   Logging::setLogLevel(static_cast<Logging::Level>(loggingLevelComboBox->currentData().toInt()));
   Logging::setDirectory(
      checkBox_LogFileLocationUseDefault->isChecked() ?
      std::optional<QDir>(std::nullopt) : std::optional<QDir>(lineEdit_LogFileLocation->text())
   );
   // Make sure the main window updates.
   MainWindow::instance().showChanges();
}

void OptionDialog::saveVersioningSettings() {
   // Save versioning options
   if (checkBox_versioning->checkState() == Qt::Checked) {
      RecipeHelper::setAutomaticVersioningEnabled(true);
      if (radioButton_deleteAncestor->isChecked()) {
         PersistentSettings::insert(PersistentSettings::Names::deletewhat, Recipe::ANCESTOR);
      } else {
         PersistentSettings::insert(PersistentSettings::Names::deletewhat, Recipe::DESCENDANT);
      }
   } else {
      // the default when versioning is off is to only delete descendant
      RecipeHelper::setAutomaticVersioningEnabled(false);
      PersistentSettings::insert(PersistentSettings::Names::deletewhat, Recipe::DESCENDANT);
   }

   PersistentSettings::insert(PersistentSettings::Names::showsnapshots, checkBox_alwaysShowSnaps->checkState() == Qt::Checked);

}

bool OptionDialog::saveDatabaseConfig() {

   bool saveDbConfig = true;

   // TODO:: FIX THIS UI. I am really not sure what the best approach is here.
   if (this->pimpl->dbConnectionTestState == NEEDS_TEST || this->pimpl->dbConnectionTestState == TEST_FAILED) {
      QMessageBox::critical(
         nullptr,
         tr("Test connection or cancel"),
         tr("Saving the options without testing the connection can cause %1 to not restart.  Your changes have "
            "been discarded, which is likely really, really crappy UX.  Please open a bug explaining exactly how you "
            "got to this message.").arg(CONFIG_APPLICATION_NAME_UC)
                           );
      return false;
   }

   // ask the user if they want to transfer data
   if (this->pimpl->dbConnectionTestState == TEST_PASSED) {
      saveDbConfig = transferDatabase();
   }

   if (saveDbConfig && this->pimpl->checkBox_savePgPassword.checkState() == Qt::Checked) {
      PersistentSettings::insert(PersistentSettings::Names::dbPassword, this->pimpl->input_pgPassword.text());
   } else {
      PersistentSettings::remove(PersistentSettings::Names::dbPassword);
   }

   Database::DbType dbEngine = static_cast<Database::DbType>(comboBox_engine->currentData().toInt());
   if (dbEngine == Database::DbType::SQLITE) {
      saveSqliteConfig();
   }

   return saveDbConfig;
}

bool OptionDialog::transferDatabase() {
   bool success = true;
   // This got unpleasant. There are multiple possible transfer paths.
   // SQLite->Pgsql, Pgsql->Pgsql and Pgsql->SQLite. This will ensure we
   // preserve the information required.
   try {
      QString theQuestion =
         tr("Would you like %1 to transfer your data to the new database? "
            "NOTE: If you've already loaded the data, say No").arg(CONFIG_APPLICATION_NAME_UC);
      if (QMessageBox::Yes == QMessageBox::question(this, tr("Transfer database"), theQuestion)) {
         Database::instance().convertDatabase(this->pimpl->input_pgHostname.text(),
                                              this->pimpl->input_pgDbName.text(),
                                              this->pimpl->input_pgUsername.text(),
                                              this->pimpl->input_pgPassword.text(),
                                              this->pimpl->input_pgPortNum.text().toInt(),
                                              static_cast<Database::DbType>(this->comboBox_engine->currentData().toInt()));
      }
      // Database engine stuff
      int engine = comboBox_engine->currentData().toInt();
      PersistentSettings::insert(PersistentSettings::Names::dbType, engine);
      // only write these changes when switching TO pgsql
      if (engine == static_cast<int>(Database::DbType::PGSQL)) {
         PersistentSettings::insert(PersistentSettings::Names::dbHostname, this->pimpl->input_pgHostname.text());
         PersistentSettings::insert(PersistentSettings::Names::dbPortnum,  this->pimpl->input_pgPortNum.text());
         PersistentSettings::insert(PersistentSettings::Names::dbSchema,   this->pimpl->input_pgSchema.text());
         PersistentSettings::insert(PersistentSettings::Names::dbName,     this->pimpl->input_pgDbName.text());
         PersistentSettings::insert(PersistentSettings::Names::dbUsername, this->pimpl->input_pgUsername.text());
      }
      QMessageBox::information(
         this,
         tr("Restart"),
         tr("Please restart %1 to connect to the new database").arg(CONFIG_APPLICATION_NAME_UC)
      );
   } catch (QString e) {
      qCritical() << Q_FUNC_INFO << e;
      success = false;
   }
   return success;
}

void OptionDialog::saveSqliteConfig() {
   // Check the new userDataDir.
   QString newUserDataDir = this->pimpl->input_userDataDir.text();
   QDir userDirectory(newUserDataDir);

   // I think this is redundant and could be handled as just a simple db
   // transfer using the TEST_PASSED loop above.
   if (userDirectory != PersistentSettings::getUserDataDir()) {
      // If there are no data files present...
      if (! QFileInfo(userDirectory, "database.sqlite").exists()) {
         // ...tell user we will copy old data files to new location.
         QMessageBox::information(this,
                                  tr("Copy Data"),
                                  tr("There do not seem to be any data files in this directory, so we will copy your old data here.")
                                 );
         Database::copyDataFiles(newUserDataDir);
      }

      PersistentSettings::setUserDataDir(newUserDataDir);
      QMessageBox::information(
         this,
         tr("Restart"),
         tr("Please restart %1.").arg(CONFIG_APPLICATION_NAME_UC)
      );
   }

   PersistentSettings::insert(PersistentSettings::Names::maximum,   this->pimpl->spinBox_numBackups.value(), PersistentSettings::Sections::backups);
   PersistentSettings::insert(PersistentSettings::Names::frequency, this->pimpl->spinBox_frequency.value(),  PersistentSettings::Sections::backups);
   PersistentSettings::insert(PersistentSettings::Names::directory, this->pimpl->input_backupDir.text(),     PersistentSettings::Sections::backups);

   return;
}

bool OptionDialog::saveWeightUnits() {
   return saveComboBoxChoiceOfUnitSystem(*weightComboBox, "weightComboBox", Measurement::PhysicalQuantity::Mass);
}

bool OptionDialog::saveTemperatureUnits() {
   return saveComboBoxChoiceOfUnitSystem(*temperatureComboBox, "temperatureComboBox", Measurement::PhysicalQuantity::Temperature);
}

bool OptionDialog::saveVolumeUnits() {
   return saveComboBoxChoiceOfUnitSystem(*volumeComboBox, "volumeComboBox", Measurement::PhysicalQuantity::Volume);
}

bool OptionDialog::saveGravityUnits() {
   return saveComboBoxChoiceOfUnitSystem(*gravityComboBox, "gravityComboBox", Measurement::PhysicalQuantity::Density);
}

bool OptionDialog::saveDateFormat() {
   bool okay = false;
   Localization::NumericDateFormat numericDateFormat =
      static_cast<Localization::NumericDateFormat>(dateComboBox->itemData(dateComboBox->currentIndex()).toInt(&okay));
   if (!okay) {
      qWarning() <<
         Q_FUNC_INFO << "Unable to interpret data format selection" <<
         dateComboBox->itemData(dateComboBox->currentIndex()) << "as integer";
      return false;
   }

   Localization::setDateFormat(numericDateFormat);
   return okay;
}

bool OptionDialog::saveColorUnits() {
   return saveComboBoxChoiceOfUnitSystem(*colorComboBox, "colorComboBox", Measurement::PhysicalQuantity::Color);
}

bool OptionDialog::saveDiastaticUnits() {
   return saveComboBoxChoiceOfUnitSystem(*diastaticPowerComboBox, "diastaticPowerComboBox", Measurement::PhysicalQuantity::DiastaticPower);
}
