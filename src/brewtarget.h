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
          WARNING,
          //! Full-blown error.
          ERROR
   };
   //! \brief The formula used to get beer color.
   enum ColorType {MOSHER, DANIEL, MOREY};
   //! \brief The units to display color in.
   enum ColorUnitType {SRM, EBC};
   //! \brief The formula used to get IBUs.
   enum IbuType {TINSETH, RAGER};
   //! \brief Don't know what the fuck this is.
   //! \brief it helps simplify hasOption, option and setOption. It also helps
   //  make the calling code more understandable.
   enum iUnitOps {
      NOOP = -1 ,
      SCALE, 
      UNIT
   };

   enum RangeType {
      GRAVITY,
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
    */
   static QString displayAmount( double amount, Unit* units=0, int precision=3, 
                                 unitDisplay displayUnit = noUnit, unitScale displayScale = noScale );
   /*!
    * \brief Displays an amount in the appropriate units.
    * 
    * \param element Element whose amount we wish to display
    * \param attribute the \c QObject::property of \c element that returns the
    *        amount we wish to display
    */
   static QString displayAmount( BeerXMLElement* element, QObject* object, QString attribute, Unit* units=0, int precision=3 );

   //! \brief Display date formatted for the locale.
   static QString displayDate( QDate const& date );
   //! \brief Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg, bool showUnits=true );
   //! \brief Appropriate thickness units will be placed in \c *volumeUnit and \c *weightUnit.
   static void getThicknessUnits( Unit** volumeUnit, Unit** weightUnit );
   //! \brief Display original gravity appropriately.
   static QString displayOG( double og, unitDisplay displayUnit = noUnit, bool showUnits=false);
   //! \brief Display original gravity appropriately.
   static QString displayOG( BeerXMLElement* element, QObject* object, QString attribute, bool showUnits=false);

   /*!
    * \brief Display final gravity appropriately.
    * 
    * \param fg the final gravity in 20C/20C units.
    * \param og the original gravity in 20C/20C units. Necessary to have the
    *           \c og since some FG displays depend on the \c og.
    */
   static QString displayFG( double fg, double og, unitDisplay displayUnit = noUnit, bool showUnits=false );
   static QString displayFG(QPair<QString, BeerXMLElement*> fg, QPair<QString, BeerXMLElement*> og, QObject* object, bool showUnits = false);

   static QPair<double,double> displayRange(BeerXMLElement* element, QObject *object, QString attribute, RangeType _type = GRAVITY);
   static QPair<double,double> displayRange(QObject *object, QString attribute, double min, double max, RangeType _type = GRAVITY);

   //! \brief Display color appropriately.
   static QString displayColor( double srm, unitDisplay displayUnit = noUnit, bool showUnits=false);
   static QString displayColor(  BeerXMLElement* element, QObject* object, QString attribute, bool showUnits=false);

   //! \return SI amount for weight string. I.e. 0.454 for "1 lb".
   static double weightQStringToSI( QString qstr, unitDisplay dispUnit = noUnit, bool force=false);
   //! \return SI amount for volume string.
   static double volQStringToSI( QString qstr, unitDisplay dispUnit = noUnit, bool force=false );
   //! \return SI amount for temperature string.
   static double tempQStringToSI( QString qstr, unitDisplay dispUnit = noUnit, bool force=false);
   //! \return SI amount for time string.
   static double timeQStringToSI( QString qstr );
   //! \return SI amount for color string.
   static double colorQStringToSI(QString qstr);
   //! \return SI (sort of?) amount for density string.
   static double densityQStringToSI(QString qstr);

   //! \brief return the bitterness formula's name
   static QString ibuFormulaName();
   //! \brief return the color formula name
   static QString colorFormulaName();

   //! \return true iff the string has a valid unit substring at the end.
   static bool hasUnits(QString qstr);
   //! \return the weight system
   static iUnitSystem getWeightUnitSystem();
   //! \return the volume system
   static iUnitSystem getVolumeUnitSystem();
   //! \return the temperature scale
   static TempScale getTemperatureScale();
   //! \return the color units
   static unitDisplay getColorUnit();
   
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

   static bool  hasOption(QString attribute, const QObject* object = 0, iUnitOps ops = NOOP);
   static void  setOption(QString attribute, QVariant value, const QObject* object = 0, iUnitOps ops = NOOP);
   static QVariant option(QString attribute, QVariant default_value = QVariant(), const QObject* object = 0, iUnitOps = NOOP);
   static void removeOption(QString attribute);

   static QString generateName(QString attribute, const QObject* object, iUnitOps ops);

   // Grr. Shortcuts never, ever pay  off
   static QMenu* setupColorMenu(QWidget* parent, unitDisplay unit);
   static QMenu* setupGravityMenu(QWidget* parent, unitDisplay unit);
   static QMenu* setupMassMenu(QWidget* parent, unitDisplay unit, unitScale scale = noScale, bool generateScale = true);
   static QMenu* setupTemperatureMenu(QWidget* parent, unitDisplay unit);
   static QMenu* setupVolumeMenu(QWidget* parent, unitDisplay unit, unitScale scale = noScale, bool generateScale = true);
   static void generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal);

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
   static bool usePlato;
   
   static iUnitSystem weightUnitSystem;
   static iUnitSystem volumeUnitSystem;
   
   static UnitSystem* weightSystem;
   static UnitSystem* volumeSystem;
   static UnitSystem* tempSystem;
   static UnitSystem* timeSystem;
   
   static TempScale tempScale;
   static ColorType colorFormula;
   static ColorUnitType colorUnit;
   static IbuType ibuFormula;
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

   // Does this make any sense any longer?
   static UnitSystem* findVolumeUnitSystem(unitDisplay system);
   static UnitSystem* findMassUnitSystem(unitDisplay system);
   static UnitSystem* findTemperatureSystem(unitDisplay system);

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
