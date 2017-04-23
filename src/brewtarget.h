/*
 * brewtarget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
 * - Samuel Östling <MrOstling@gmail.com>
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

#ifndef _BREWTARGET_H
#define _BREWTARGET_H

// I think this will make things restart. I hope this will make things
// restart.
#define RESTART_CODE 0x1000

// need to use this to turn on mac keyboard shortcuts (see http://doc.qt.nokia.com/4.7-snapshot/qtglobal.html#qt_set_sequence_auto_mnemonic)
extern void qt_set_sequence_auto_mnemonic(bool b);

#include <QObject>
#include <QApplication>
#include <QString>
#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QTranslator>
#include <QTextStream>
#include <QDateTime>
#include <QSettings>
#include <QMenu>
#include <QMetaProperty>
#include <QList>
#include "UnitSystem.h"
#include "Log.h"

class BeerXMLElement;
class MainWindow;

// Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
Q_DECLARE_METATYPE( QMetaProperty )

/*!
 * \class Brewtarget
 * \author Philip G. Lee
 *
 * \brief The main class. Figures out stuff from the system, formats things appropriately, handles translation, etc.
 */
class Brewtarget : public QObject
{
   Q_OBJECT
   Q_ENUMS(DBTypes)

   friend class OptionDialog;
   friend class IbuMethods;
   friend class ColorMethods;
   friend class RecipeFormatter;
   friend class Unit;
   friend class Database;
   friend class MainWindow;
   friend class Testing;

public:
   Brewtarget();

   //! \brief The formula used to get beer color.
   enum ColorType {MOSHER, DANIEL, MOREY};
   //! \brief The units to display color in.
   enum ColorUnitType {SRM, EBC};
   //! \brief Units for density
   enum DensityUnitType {SG,PLATO};
   //! \brief The units for the diastatic power.
   enum DiastaticPowerUnitType {LINTNER, WK};
   //! \brief The formula used to get IBUs.
   enum IbuType {TINSETH, RAGER, NOONAN};
   //! \brief Controls how units and scales are stored in the options file
   enum iUnitOps {
      NOOP = -1 ,
      SCALE,
      UNIT
   };

   enum RangeType {
      DENSITY,
      COLOR
   };

   //! \brief The database tables.
   //! \brief You know. I need all the db tables, and I need them in a
   //  specific order. I need these constants defined in the EXACT order the
   //  tables are created by DatabaseSchemaHelper::create. Do not modify this
   //  unless you understand the relationship and fix all sides
   enum DBTable{
      //! None of the tables. 0
      NOTABLE,
      // Meta tables first
      BTALLTABLE,
      SETTINGTABLE,

      // BeerXML tables next
      EQUIPTABLE,
      FERMTABLE,
      HOPTABLE,
      MISCTABLE,
      STYLETABLE,
      YEASTTABLE,
      WATERTABLE,
      MASHTABLE,
      MASHSTEPTABLE,
      RECTABLE,
      BREWNOTETABLE,
      INSTRUCTIONTABLE,

      // then the bt_* tables
      BT_EQUIPTABLE,
      BT_FERMTABLE,
      BT_HOPTABLE,
      BT_MISCTABLE,
      BT_STYLETABLE,
      BT_YEASTTABLE,
      BT_WATERTABLE,

      // then the *_in_recipe tables
      FERMINRECTABLE,
      HOPINRECTABLE,
      MISCINRECTABLE,
      WATERINRECTABLE,
      YEASTINRECTABLE,
      INSTINRECTABLE,

      // then the child tables
      EQUIPCHILDTABLE,
      FERMCHILDTABLE,
      HOPCHILDTABLE,
      MISCCHILDTABLE,
      RECIPECHILDTABLE,
      STYLECHILDTABLE,
      WATERCHILDTABLE,
      YEASTCHILDTABLE,

      // finally the inventory tables
      FERMINVTABLE,
      HOPINVTABLE,
      MISCINVTABLE,
      YEASTINVTABLE
   };

   //! \brief Supported databases. I am not 100% sure I'm digging this
   //  solution, but this is more extensible than what I was doing previously
   enum DBTypes {
      NODB = -1,  // seems a popular choice with the cool enums
      SQLITE,     // compact, fast and a little loose
      PGSQL       // big, powerful, uptight and a little stodgy
   };

   //! \return the data directory
   static QDir getDataDir();
   //! \return the doc directory
   static QDir getDocDir();
   //! \return the config directory
   static const QDir getConfigDir();
   //! \return user-specified directory where the database files reside.
   static QDir getUserDataDir();
   /*!
    * \brief Blocking call that executes the application.
    * \param userDirectory If !isEmpty, overwrites the current settings.
    * \return Exit code from the application.
    */
   static int run(const QString &userDirectory = QString());

   static double toDouble(QString text, bool* ok = 0);
   static double toDouble(const BeerXMLElement* element, QString attribute, QString caller);
   static double toDouble(QString text, QString caller);

   //! \brief Log an error message.
   static void logE( QString message );
   //! \brief Log a warning message.
   static void logW( QString message );

   /*!
    *  \brief Displays an amount in the appropriate units.
    *
    *  \param amount the amount to display
    *  \param units the units that \c amount is in
    *  \param precision how many decimal places
    *  \param unitDisplay which unit system to use, defaulting to "noUnit" which means use the system default
    *  \param Unit::unitScale which scale to use, defaulting to Unit::noScale which means use the largest scale that generates a value > 1
    */
   static QString displayAmount( double amount, Unit* units=0, int precision=3,
                                 Unit::unitDisplay displayUnit = Unit::noUnit, Unit::unitScale displayScale = Unit::noScale );
   /*!
    * \brief Displays an amount in the appropriate units.
    *
    * \param element Element whose amount we wish to display
    * \param object the GUI object doing the display, used to access configured unit&scale
    * \param attribute the name of the attribute to display
    * \param units which unit system it is in
    * \param precision how many decimal places to use, defaulting to 3
    */
   static QString displayAmount( BeerXMLElement* element, QObject* object, QString attribute, Unit* units=0, int precision=3 );

   /*!
    * \brief Displays an amount in the appropriate units.
    *
    * \param amount the amount to display
    * \param section the name of the object to reference to get units&scales from the config file
    * \param attribute the attribute name to complete the lookup for units&scales
    * \param units which unit system it is in
    * \param precision how many decimal places to use, defaulting to 3
    */
   static QString displayAmount( double amount, QString section, QString attribute, Unit* units=0, int precision = 3);

   /*!
    *  \brief Displays an amount in the appropriate units.
    *
    *  \param amount the amount to display
    *  \param units the units that \c amount is in
    *  \param precision how many decimal places
    */
   static double amountDisplay( double amount, Unit* units=0, int precision=3,
                                 Unit::unitDisplay displayUnit = Unit::noUnit, Unit::unitScale displayScale = Unit::noScale );
   /*!
    * \brief Displays an amount in the appropriate units.
    *
    * \param element Element whose amount we wish to display
    * \param attribute the \c QObject::property of \c element that returns the
    *        amount we wish to display
    */
   static double amountDisplay( BeerXMLElement* element, QObject* object, QString attribute, Unit* units=0, int precision=3 );

   //! \brief Display date formatted for the locale.
   static QString displayDate( QDate const& date );
   //! \brief Display date formatted based on the user defined options.
   static QString displayDateUserFormated(QDate const &date);
   //! \brief Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg, bool showUnits=true );
   //! \brief Appropriate thickness units will be placed in \c *volumeUnit and \c *weightUnit.
   static void getThicknessUnits( Unit** volumeUnit, Unit** weightUnit );

   static QPair<double,double> displayRange(BeerXMLElement* element, QObject *object, QString attribute, RangeType _type = DENSITY);
   static QPair<double,double> displayRange(QObject *object, QString attribute, double min, double max, RangeType _type = DENSITY);

   //! \return SI amount for the string
   static double qStringToSI( QString qstr, Unit* unit, 
         Unit::unitDisplay dispUnit = Unit::noUnit, Unit::unitScale dispScale = Unit::noScale);

   //! \brief return the bitterness formula's name
   static QString ibuFormulaName();
   //! \brief return the color formula name
   static QString colorFormulaName();

   // One method to rule them all, and in darkness bind them
   static UnitSystem* findUnitSystem(Unit* unit, Unit::unitDisplay display);
   static QString colorUnitName(Unit::unitDisplay display);
   static QString diastaticPowerUnitName(Unit::unitDisplay display);

   //! \return true iff the string has a valid unit substring at the end.
   static bool hasUnits(QString qstr);

   // You do know I will have to kill these too?
   //! \return the density units
   static Unit::unitDisplay getDensityUnit();
   //! \return the date format
   static Unit::unitDisplay getDateFormat();
   //! \return the volume system
   static iUnitSystem getVolumeUnitSystem();

   //! \brief Read options from file. This is deprecated, but we need it
   // around for the conversion
   static void convertPersistentOptions();
   //! \brief Read options from options. This replaces readPersistentOptions()
   static void readSystemOptions();
   //! \brief Writes the persisten options back to the options store
   static void saveSystemOptions();

   /*!
    *  \brief Loads the brewtarget translator with two letter ISO 639-1 code.
    *
    *  For example, for spanish, it would
    *  be 'es'. Currently, this does NO checking to make sure the locale
    *  code is acceptable.
    *
    *  \param twoLetterLanguage two letter ISO 639-1 code
    */
   static void setLanguage(QString twoLetterLanguage);
   /*!
    *  \brief Gets the 2-letter ISO 639-1 language code we are currently using.
    *  \returns current 2-letter ISO 639-1 language code.
    */
   static const QString& getCurrentLanguage();
   /*!
    *  \brief Gets the ISO 639-1 language code for the system.
    *  \returns current 2-letter ISO 639-1 system language code
    */
   static const QString& getSystemLanguage();

   static bool  hasOption(QString attribute, const QString section = QString(), iUnitOps ops = NOOP);
   static void  setOption(QString attribute, QVariant value, const QString section = QString(), iUnitOps ops = NOOP);
   static QVariant option(QString attribute, QVariant default_value = QVariant(), QString section = QString(), iUnitOps = NOOP);
   static void removeOption(QString attribute, QString section=QString());

   static QString generateName(QString attribute, const QString section, iUnitOps ops);

   // Grr. Shortcuts never, ever pay  off
   static QMenu* setupColorMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupDateMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupDensityMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupMassMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale = Unit::noScale, bool generateScale = true);
   static QMenu* setupTemperatureMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupVolumeMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale = Unit::noScale, bool generateScale = true);
   static QMenu* setupDiastaticPowerMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupTimeMenu(QWidget* parent, Unit::unitScale scale);
   static void generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal, QActionGroup* qgrp = 0);

   /*! 
    * \brief If we are supporting multiple databases, we need some way to
    * figure out which database we are using. I still don't know that this
    * will be the final implementation -- I can't help but think I should be
    * subclassing something
    */
   static Brewtarget::DBTypes dbType();
   /*!
    * \brief Different databases use different values for true and false.
    * These two methods handle that difference, in a marginally extensible way
    */
   static QString dbTrue(Brewtarget::DBTypes whichDb = Brewtarget::NODB);
   static QString dbFalse(Brewtarget::DBTypes whichDb = Brewtarget::NODB);

   //! \return the main window.
   static MainWindow* mainWindow();

private:
   static MainWindow* _mainWindow;
   static QDomDocument* optionsDoc;
   static QTranslator* defaultTrans;
   static QTranslator* btTrans;
   //! \brief OS-Agnostic RAII style Thread-safe Log file.
   static Log log;
   static QString currentLanguage;
   static QSettings btSettings;
   static bool userDatabaseDidNotExist;
   static QFile pidFile;
   static bool _isInteractive;

   static DBTypes _dbType;

   //! \brief If this option is false, do not bother the user about new versions.
   static bool checkVersion;

   /*! Stores the date that we last asked the user to merge the
    *  data-space database to the user-space database.
    */
   static QDateTime lastDbMergeRequest;

   //! \brief Where the user says the database files are
   static QDir userDataDir;

   // Options to be edited ONLY by the OptionDialog============================
   // Whether or not to display plato instead of SG.

   static iUnitSystem weightUnitSystem;
   static iUnitSystem volumeUnitSystem;

   // Sigh. You knew this was coming right? But I think I can clean a lot of
   // shit up with some clever work.
   static QHash<int, UnitSystem*> thingToUnitSystem;

   static TempScale tempScale;
   static ColorType colorFormula;
   static ColorUnitType colorUnit;
   static DensityUnitType densityUnit;
   static DiastaticPowerUnitType diastaticPowerUnit;
   static IbuType ibuFormula;
   static Unit::unitDisplay dateFormat;
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   /*!
    * \brief Run before showing MainWindow, does all system setup.
    *
    * Creates a PID file, sets config directory, reads system options,
    * ensures the data directories and files exist, loads translations,
    * and loads database.
    *
    * \returns false if anything goes awry, true if it's ok to start MainWindow
    */
   static bool initialize(const QString &userDirectory = QString());
   /*!
    * \brief Run after QApplication exits to clean up shit, close database, etc.
    */
   static void cleanup();


   /*!
    * \brief Checks if another instance is already running.
    *
    * Currently only works on Unix systems.
    */
   static bool instanceRunning();

   /*!
    * \brief If false, run Brewtarget in a way that requires no user interaction
    *
    * For example, if running a test case, ensure that no dialogs pop up that
    * prevent Brewtarget from starting
    */
   static bool isInteractive();
   //! \brief Set the mode to an interactive or non-interactive state
   static void setInteractive(bool val);

   /*!
    *  \brief Helper to get option values from XML.
    *
    *  If \b hasOption is not null,
    *  is set to true iff the option exists in the document.
    */
   static QString getOptionValue(const QDomDocument& optionsDoc,
                                 const QString& option,
                                 bool* hasOption = 0);

   /*!
    *  \brief Copies the user xml files to another directory.
    *  \returns false iff the copy is unsuccessful.
    */
   static bool copyDataFiles(const QDir newPath);

   //! \brief Ensure our directories exist.
   static bool ensureDirectoriesExist();
   //! \brief Create a directory if it doesn't exist, popping a error dialog if creation fails
   static bool createDir(QDir dir, QString errText = NULL);

   //! \brief Load translation files.
   static void loadTranslations();
   //! \brief Checks for a newer version and prompts user to download.
   static void checkForNewVersion(MainWindow* mw);

   static void loadMap();

   //! \return the weight system
   static iUnitSystem getWeightUnitSystem();
   //! \return the temperature scale
   static TempScale getTemperatureScale();
   //! \return the color units
   static Unit::unitDisplay getColorUnit();
   //! \return the diastatic power units
   static Unit::unitDisplay getDiastaticPowerUnit();
};

Q_DECLARE_METATYPE( Brewtarget::DBTable )

/*!
 * \mainpage Brewtarget Source Code Documentation
 *
 * \section secIntro Introduction
 *
 * Brewtarget is a cross-platform open source beer recipe software suite.
 * Our aim is to make "free as in beer" equal to "free as in speech" and
 * also to make a damn fine piece of software.
 *
 */

#endif   /* _BREWTARGET_H */
