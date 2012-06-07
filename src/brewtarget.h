/*
 * brewtarget.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BREWTARGET_H
#define _BREWTARGET_H

// need to use this to turn on mac keyboard shortcuts (see http://doc.qt.nokia.com/4.7-snapshot/qtglobal.html#qt_set_sequence_auto_mnemonic)
extern void qt_set_sequence_auto_mnemonic(bool b);

class Brewtarget;

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
#include "UnitSystem.h"


// Forward declarations.
class MainWindow;
class Unit;
class BeerXMLElement;

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
public:
   Brewtarget();

   enum LogType {WARNING, ERROR};
   enum ColorType {MOSHER, DANIEL, MOREY};
   enum ColorUnitType {SRM, EBC};
   enum IbuType {TINSETH, RAGER};
   enum iUnitOps {
      NOOP = -1 ,
      SCALE, 
      UNIT
   };

   enum DBTable{ NOTABLE, BREWNOTETABLE, EQUIPTABLE, FERMTABLE, HOPTABLE, INSTRUCTIONTABLE,
                 MASHSTEPTABLE, MASHTABLE, MISCTABLE, RECTABLE, STYLETABLE, WATERTABLE, YEASTTABLE  };
   //! \return the data directory
   static QString getDataDir();
   //! \return the doc directory
   static QString getDocDir();
   //! \return the config directory
   static QString getConfigDir(bool* success = 0);
   //! \return user-specified directory where the database files reside.
   static QString getUserDataDir();
   //! \return blocking call that starts the application.
   static int run();
   //! Log a message.
   static void log( LogType lt, QString message );
   //! Log an error message.
   static void logE( QString message );
   //! Log a warning message.
   static void logW( QString message );

   /*!
    * Produces the appropriate string for 'amount' which has units 'units'.
    * Variable 'precision' controls how many decimal places. I've overloaded
    * it to aid in the transition
    */
   static QString displayAmount( double amount, Unit* units=0, int precision=3, 
                                 unitDisplay displayUnit = noUnit, unitScale displayScale = noScale );
   static QString displayAmount( BeerXMLElement* element, QObject* object, QString attribute, Unit* units=0, int precision=3 );

   //! Display date correctly depending on locale.
   static QString displayDate( QDate const& date );
   //! Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg, bool showUnits=true );
   //! Appropriate thickness units will be placed in *volumeUnit and *weightUnit.
   static void getThicknessUnits( Unit** volumeUnit, Unit** weightUnit );
   //! Display original gravity appropriately.
   static QString displayOG( double og, unitDisplay displayUnit = noUnit, bool showUnits=false);
   static QString displayOG( BeerXMLElement* element, QObject* object, QString attribute, bool showUnits=false);

   //! Display final gravity appropriately.
   static QString displayFG( double fg, double og, unitDisplay displayUnit = noUnit, bool showUnits=false );
   static QString displayFG(QPair<QString, BeerXMLElement*> fg, QPair<QString, BeerXMLElement*> og, QObject* object, bool showUnits = false);

   //! Display color appropriately.
   static QString displayColor( double srm, unitDisplay displayUnit = noUnit, bool showUnits=false);
   static QString displayColor(  BeerXMLElement* element, QObject* object, QString attribute, bool showUnits=false);

   //! \return SI amount for weight string. I.e. 0.454 for "1 lb".
   static double weightQStringToSI( QString qstr );
   //! \return SI amount for volume string.
   static double volQStringToSI( QString qstr );
   //! \return SI amount for temperature string.
   static double tempQStringToSI( QString qstr );
   //! \return SI amount for time string.
   static double timeQStringToSI( QString qstr );
   //! \return SI amount for color string.
   static double colorQStringToSI(QString qstr);

   static bool hasUnits(QString qstr);
   //! \return the weight system
   static iUnitSystem getWeightUnitSystem();
   //! \return the volume system
   static iUnitSystem getVolumeUnitSystem();
   //! \return the temperature scale
   static TempScale getTemperatureScale();
   //! \return the color units
   static unitDisplay getColorUnit();
   
   //! Read options from file.
   static void readPersistentOptions();
   //! Save options to file.
   static void savePersistentOptions();

   /*!
    * Loads the brewtarget translator with two letter ISO 639-1 code
    * 'twoLetterLanguage'. For example, for spanish, it would
    * be 'es'.
    * Currently, this does NO checking to make sure the locale
    * code is acceptable.
    */
   static void setLanguage(QString twoLetterLanguage);
   /*!
    * Gets the 2-letter ISO 639-1 language code we are currently using.
    */
   static const QString& getCurrentLanguage();
   /*!
    * Gets the ISO 639-1 language code for the system.
    */
   static const QString& getSystemLanguage();

   static bool  hasOption(QString attribute, const QObject* object = 0, iUnitOps ops = NOOP);
   static void  setOption(QString attribute, QVariant value, const QObject* object = 0, iUnitOps ops = NOOP);
   static QVariant option(QString attribute, QVariant default_value, const QObject* object = 0, iUnitOps = NOOP);

   static QString generateName(QString attribute, const QObject* object, iUnitOps ops);

   // Grr. Shortcuts never, ever pay  off
   static QMenu* setupColorMenu(QWidget* parent, QVariant unit);
   static QMenu* setupGravityMenu(QWidget* parent, QVariant unit);
   static QMenu* setupMassMenu(QWidget* parent, QVariant unit, QVariant scale = -1);
   static QMenu* setupTemperatureMenu(QWidget* parent, QVariant unit);
   static QMenu* setupVolumeMenu(QWidget* parent, QVariant unit, QVariant scale = -1);
   static void generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal);

   //! \return the main window.
   static MainWindow* getMainWindow();

private:
   static MainWindow* mainWindow;
   static QDomDocument* optionsDoc;
   static QTranslator* defaultTrans;
   static QTranslator* btTrans;
   static QFile* logFile;
   static QTextStream* logStream;
   static QString currentLanguage;
   static QSettings btSettings;
   static bool userDatabaseDidNotExist;

   /*! Helper to get option values. If \b hasOption is not null,
    *  is set to true iff the option exists in the document.
    */
   static QString getOptionValue(const QDomDocument& optionsDoc,
                                 const QString& option,
                                 bool* hasOption = 0);

   /*! Copies the user xml files to another directory. Returns
    *  false iff the copy is unsuccessful.
    */
   static bool copyDataFiles(QString newPath);

   //! Ensure our directories exist.
   static bool ensureDirectoriesExist();
   //! Ensure the datafiles exist.
   static bool ensureDataFilesExist();
   //! Ensure the option file exists.
   static bool ensureOptionFileExists();
   //! Load translation files.
   static void loadTranslations();
   //! Checks for a newer version and prompts user to download.
   static void checkForNewVersion(MainWindow* mw);
   
   //! If this option is false, do not bother the user about new versions.
   static bool checkVersion;

   /*! Stores the date that we last asked the user to merge the
    *  data-space database to the user-space database.
    */
   static QDateTime lastDbMergeRequest;

   //! Where the user says the .xml files are
   static QString userDataDir;

   // These are options that are ONLY to be edited by the OptionDialog.
   static bool usePlato; // Whether or not to display plato instead of SG.
   //
   static iUnitSystem weightUnitSystem;
   static iUnitSystem volumeUnitSystem;
   //
   static UnitSystem* weightSystem;
   static UnitSystem* volumeSystem;
   static UnitSystem* tempSystem;
   static UnitSystem* timeSystem;
   //
   static TempScale tempScale;
   static ColorType colorFormula;
   static ColorUnitType colorUnit;
   static IbuType ibuFormula;

   // Does this make any sense any longer?
   static UnitSystem* findVolumeUnitSystem(unitDisplay system);
   static UnitSystem* findMassUnitSystem(unitDisplay system);
   static UnitSystem* findTemperatureSystem(unitDisplay system);

};

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

