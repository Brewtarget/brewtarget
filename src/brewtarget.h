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

   //! \brief The log level of a message.
   enum LogType{
          //! Just a warning.
          LogType_WARNING,
          //! Full-blown error.
          LogType_ERROR
   };
   //! \brief The formula used to get beer color.
   enum ColorType {MOSHER, DANIEL, MOREY};
   //! \brief The units to display color in.
   enum ColorUnitType {SRM, EBC};
   //! \brief Units for density
   enum DensityUnitType {SG,PLATO};
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
   enum DBTable{
      //! None of the tables. 0
      NOTABLE,
      //! In the BrewNote table. 1
      BREWNOTETABLE,
      //! In the Equipment table. 2
      EQUIPTABLE,
      //! In the Fermentable table. 3
      FERMTABLE,
      //! In the Hop table. 4
      HOPTABLE,
      //! In the Instruction table. 5
      INSTRUCTIONTABLE,
      //! In the MashStep table. 6
      MASHSTEPTABLE,
      //! In the Mash table. 7
      MASHTABLE,
      //! In the Misc table. 8
      MISCTABLE,
      //! In the Recipe table. 9
      RECTABLE,
      //! In the Style table. 10
      STYLETABLE,
      //! In the Water table. 11
      WATERTABLE,
      //! In the Yeast table. 12
      YEASTTABLE,

     //! In the Fermentable Inventory table. 13
      FERMINVTABLE,
      //! In the Hop Inventory table. 14
      HOPINVTABLE,
      //! In the Misc Inventory table. 15
      MISCINVTABLE,
     //! In the Yeast Inventory table. 16
      YEASTINVTABLE,

      //! In the Fermentable Parent Child Relationship table. 17
      FERMCHILDTABLE,
      //! In the Hop Parent Child Relationship table. 18
      HOPCHILDTABLE,
      //! In the Misc Parent Child Relationship table. 19
      MISCCHILDTABLE,
     //! In the Yeast Parent Child Relationship table. 20
      YEASTCHILDTABLE

   };

   //! \return the data directory
   static QString getDataDir();
   //! \return the doc directory
   static QString getDocDir();
   //! \return the config directory
   static QString getConfigDir(bool* success = 0);
   //! \return user-specified directory where the database files reside.
   static QString getUserDataDir();
   //! \brief Blocking call that starts the application.
   static int run();

   static double toDouble(QString text, bool* ok = 0);
   static double toDouble(const BeerXMLElement* element, QString attribute, QString caller);
   static double toDouble(QString text, QString caller);

   //! \brief Log a message.
   static void log( LogType lt, QString message );
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
   //! \brief Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg, bool showUnits=true );
   //! \brief Appropriate thickness units will be placed in \c *volumeUnit and \c *weightUnit.
   static void getThicknessUnits( Unit** volumeUnit, Unit** weightUnit );

   static QPair<double,double> displayRange(BeerXMLElement* element, QObject *object, QString attribute, RangeType _type = DENSITY);
   static QPair<double,double> displayRange(QObject *object, QString attribute, double min, double max, RangeType _type = DENSITY);

   //! \return SI amount for the string
   static double qStringToSI( QString qstr, Unit* unit, Unit::unitDisplay dispUnit = Unit::noUnit, bool force = false);

   //! \brief return the bitterness formula's name
   static QString ibuFormulaName();
   //! \brief return the color formula name
   static QString colorFormulaName();

   // One method to rule them all, and in darkness bind them
   static UnitSystem* findUnitSystem(Unit* unit, Unit::unitDisplay display);
   static QString colorUnitName(Unit::unitDisplay display);

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
   static void removeOption(QString attribute);

   static QString generateName(QString attribute, const QString section, iUnitOps ops);

   // Grr. Shortcuts never, ever pay  off
   static QMenu* setupColorMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupDateMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupDensityMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupMassMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale = Unit::noScale, bool generateScale = true);
   static QMenu* setupTemperatureMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupVolumeMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale = Unit::noScale, bool generateScale = true);
   static QMenu* setupTimeMenu(QWidget* parent, Unit::unitScale scale);
   static void generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal, QActionGroup* qgrp = 0);

   //! \return the main window.
   static MainWindow* mainWindow();

private:
   static MainWindow* _mainWindow;
   static QDomDocument* optionsDoc;
   static QTranslator* defaultTrans;
   static QTranslator* btTrans;
   static QFile* logFile;
   static QTextStream* logStream;
   static QString currentLanguage;
   static QSettings btSettings;
   static bool userDatabaseDidNotExist;
   static QFile pidFile;

   //! \brief If this option is false, do not bother the user about new versions.
   static bool checkVersion;

   /*! Stores the date that we last asked the user to merge the
    *  data-space database to the user-space database.
    */
   static QDateTime lastDbMergeRequest;

   //! \brief Where the user says the database files are
   static QString userDataDir;

   // Options to be edited ONLY by the OptionDialog============================
   // Whether or not to display plato instead of SG.
//   static bool usePlato;

   static iUnitSystem weightUnitSystem;
   static iUnitSystem volumeUnitSystem;

   // Sigh. You knew this was coming right? But I think I can clean a lot of
   // shit up with some clever work.
   static QHash<int, UnitSystem*> thingToUnitSystem;

   static TempScale tempScale;
   static ColorType colorFormula;
   static ColorUnitType colorUnit;
   static DensityUnitType densityUnit;
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
   static bool initialize();
   /*!
    * \brief Run after QApplication exits to clean up shit, close database, etc.
    */
   static void cleanup();

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
   static bool copyDataFiles(QString newPath);

   //! \brief Ensure our directories exist.
   static bool ensureDirectoriesExist();
   //! \brief Ensure the datafiles exist.
   static bool ensureDataFilesExist();
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
