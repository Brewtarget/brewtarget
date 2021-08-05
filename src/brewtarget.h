/*
 * brewtarget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef BREWTARGET_H
#define BREWTARGET_H

#define CONFIG_VERSION 1

#define BTICON ":/images/brewtarget.svg"
//#define ICON96 ":/images/BrewtargetIcon_96.png"
#define GLASS ":/images/glass2.png"
#define SMALLBARLEY ":/images/smallBarley.svg"
#define SMALLHOP ":/images/smallHop.svg"
#define SMALLWATER ":/images/smallWater.svg"
#define SMALLYEAST ":/images/smallYeast.svg"
#define SMALLKETTLE ":/images/smallKettle.svg"
#define SMALLQUESTION ":/images/smallQuestion.svg"
#define SMALLSTYLE ":/images/smallStyle.svg"
#define SMALLPLUS ":/images/smallPlus.svg"
#define SMALLMINUS ":/images/smallMinus.svg"
#define SMALLARROW ":/images/smallArrow.svg"
#define SMALLINFO ":/images/smallInfo.svg"
#define SMALLOUTARROW ":/images/smallOutArrow.svg"
#define SHRED ":/images/editshred.svg"
#define EXITPNG ":/images/exit.svg"
#define SAVEPNG ":/images/filesave.svg"
#define SAVEDIRTYPNG ":/images/filesavedirty.svg"
#define CLOCKPNG ":/images/clock.svg"
#define SOUND ":/images/sound.png"
#define STOP ":/images/alarm_stop.png"

// need to use this to turn on Mac keyboard shortcuts (see https://doc.qt.io/qt-5/qkeysequence.html#qt_set_sequence_auto_mnemonic)
extern void qt_set_sequence_auto_mnemonic(bool b);

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QList>
#include <QMenu>
#include <QMetaProperty>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QTranslator>

#include "UnitSystem.h"
#include "Logging.h"
#include "utils/BtStringConst.h"

class NamedEntity;
class MainWindow;

// Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
Q_DECLARE_METATYPE( QMetaProperty )

/*!
 * \class Brewtarget
 *
 * \brief The main class. Figures out stuff from the system, formats things appropriately, handles translation, etc.
 *
 * TODO: Lots of things in this class belong elsewhere...
 */
class Brewtarget : public QObject
{
   Q_OBJECT
   Q_ENUMS(DbType)
   Q_ENUMS(delOptions)

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

   enum RangeType {
      DENSITY,
      COLOR
   };

   //! \return the data directory
   static QDir getDataDir();
   //! \return the doc directory
   static QDir getDocDir();
   //! \return the config directory
   static const QDir getConfigDir();
   //! \return user-specified directory where the database files reside.
   static QDir getUserDataDir();
   //! \return The System path for users applicationpath. on windows: c:\\users\\<USERNAME>\\AppData\\Roaming\\<APPNAME>
   static QDir getDefaultUserDataDir();
   /**
    * \return the resource directory where some files that ship with Brewtarget live (default DB, sounds, translations)
    *
    *         Most resources are compiled into the app with the Qt Resource System (see
    *         https://doc.qt.io/qt-5/resources.html) but, for some files, we want the user also to be able to access
    *         the file directly.  Such files are stored in this directory.
    */
   static QDir getResourceDir();

   /*!
    * \brief Blocking call that executes the application.
    * \param userDirectory If !isEmpty, overwrites the current settings.
    * \return Exit code from the application.
    */
   static int run();

   static double toDouble(QString text, bool* ok = nullptr);
   static double toDouble(const NamedEntity* element, BtStringConst const & propertyName, QString caller);
   static double toDouble(QString text, QString caller);

   /*!
    *  \brief Displays an amount in the appropriate units.
    *
    *  \param amount the amount to display
    *  \param units the units that \c amount is in
    *  \param precision how many decimal places
    *  \param unitDisplay which unit system to use, defaulting to "noUnit" which means use the system default
    *  \param Unit::unitScale which scale to use, defaulting to Unit::noScale which means use the largest scale that generates a value > 1
    */
   static QString displayAmount( double amount, Unit const * units=nullptr, int precision=3,
                                 Unit::unitDisplay displayUnit = Unit::noUnit, Unit::unitScale displayScale = Unit::noScale );
   /*!
    * \brief Displays an amount in the appropriate units.
    *
    * \param element Element whose amount we wish to display
    * \param object the GUI object doing the display, used to access configured unit&scale
    * \param propertyName the name of the property to display
    * \param units which unit system it is in
    * \param precision how many decimal places to use, defaulting to 3
    */
   static QString displayAmount( NamedEntity* element, QObject* object, BtStringConst const & propertyName, Unit const * units=nullptr, int precision=3 );

   /*!
    * \brief Displays an amount in the appropriate units.
    *
    * \param amount the amount to display
    * \param section the name of the object to reference to get units&scales from the config file
    * \param propertyName the property name to complete the lookup for units&scales
    * \param units which unit system it is in
    * \param precision how many decimal places to use, defaulting to 3
    */
   static QString displayAmount(double amount,
                                BtStringConst const & section,
                                BtStringConst const & propertyName,
                                Unit const * units=nullptr,
                                int precision = 3);

   /*!
    *  \brief Displays an amount in the appropriate units.
    *
    *  \param amount the amount to display
    *  \param units the units that \c amount is in
    *  \param precision how many decimal places
    */
   static double amountDisplay( double amount, Unit const * units=nullptr, int precision=3,
                                 Unit::unitDisplay displayUnit = Unit::noUnit, Unit::unitScale displayScale = Unit::noScale );
   /*!
    * \brief Displays an amount in the appropriate units.
    *
    * \param element Element whose amount we wish to display
    * \param propertyName the \c QObject::property of \c element that returns the
    *        amount we wish to display
    */
   static double amountDisplay( NamedEntity* element, QObject* object, BtStringConst const & propertyName, Unit const * units=nullptr, int precision=3 );

   //! \brief Display date formatted for the locale.
   static QString displayDate( QDate const& date );
   //! \brief Display date formatted based on the user defined options.
   static QString displayDateUserFormated(QDate const &date);
   //! \brief Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg, bool showUnits=true );
   //! \brief Appropriate thickness units will be placed in \c *volumeUnit and \c *weightUnit.
   static void getThicknessUnits( Unit const ** volumeUnit, Unit const ** weightUnit );

   static QPair<double,double> displayRange(NamedEntity* element,
                                            QObject *object,
                                            BtStringConst const & propertyNameMin,
                                            BtStringConst const & propertyNameMax,
                                            RangeType _type = DENSITY);
   static QPair<double,double> displayRange(QObject *object,
                                            BtStringConst const & propertyName,
                                            double min,
                                            double max,
                                            RangeType _type = DENSITY);

   //! \return SI amount for the string
   static double qStringToSI( QString qstr, Unit const * unit,
         Unit::unitDisplay dispUnit = Unit::noUnit, Unit::unitScale dispScale = Unit::noScale);

   //! \brief return the bitterness formula's name
   static QString ibuFormulaName();
   //! \brief return the color formula name
   static QString colorFormulaName();

   // One method to rule them all, and in darkness bind them
   static UnitSystem const * findUnitSystem(Unit const * unit, Unit::unitDisplay display);
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
   static SystemOfMeasurement getVolumeUnitSystem();

   //! \brief Read options from file. This is deprecated, but we need it
   // around for the conversion
//   static void convertPersistentOptions();
   //! \brief Every so often, we need to update the config file itself. This does that.
   static void updateConfig();
   //! \brief Read options from options. This replaces readPersistentOptions()
   static void readSystemOptions();
   //! \brief Writes the persisten options back to the options store
   static void saveSystemOptions();

   /*!
    *  \brief Loads the Brewtarget translator with two letter ISO 639-1 code.
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


   // Grr. Shortcuts never, ever pay  off
   static QMenu* setupColorMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupDateMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupDensityMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupMassMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale = Unit::noScale, bool generateScale = true);
   static QMenu* setupTemperatureMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupVolumeMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale = Unit::noScale, bool generateScale = true);
   static QMenu* setupDiastaticPowerMenu(QWidget* parent, Unit::unitDisplay unit);
   static QMenu* setupTimeMenu(QWidget* parent, Unit::unitScale scale);
   static void generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal, QActionGroup* qgrp = nullptr);

   //! \return the main window.
   static MainWindow* mainWindow();

private:
   static MainWindow* m_mainWindow;
   static QDomDocument* optionsDoc;
   static QTranslator* defaultTrans;
   static QTranslator* btTrans;
   static QString currentLanguage;
   static QSettings btSettings;
   static bool userDatabaseDidNotExist;
   static QFile pidFile;
   static bool _isInteractive;

   //! \brief If this option is false, do not bother the user about new versions.
   static bool checkVersion;

   //! \brief Where the user says the database files are
   static QDir userDataDir;

   // Options to be edited ONLY by the OptionDialog============================
   // Whether or not to display plato instead of SG.

   static SystemOfMeasurement weightUnitSystem;
   static SystemOfMeasurement volumeUnitSystem;

   // Sigh. You knew this was coming right? But I think I can clean a lot of
   // shit up with some clever work.
   static QHash<int, UnitSystem const *> thingToUnitSystem;

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
    * Creates a PID file, reads system options,
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

public:
   /*!
    * \brief If false, run Brewtarget in a way that requires no user interaction
    *
    * For example, if running a test case, ensure that no dialogs pop up that
    * prevent Brewtarget from starting
    */
   static bool isInteractive();
   //! \brief Set the mode to an interactive or non-interactive state
   static void setInteractive(bool val);

private:
   /*!
    *  \brief Helper to get option values from XML.
    *
    *  If \b hasOption is not null,
    *  is set to true iff the option exists in the document.
    */
/*   static QString getOptionValue(const QDomDocument& optionsDoc,
                                 const QString& option,
                                 bool* hasOption = nullptr);
*/
   /*!
    *  \brief Copies the SQLite database file to another directory.
    *  \returns false iff the copy is unsuccessful.
    */
   static bool copyDataFiles(const QDir newPath);


   //! \brief Load translation files.
   static void loadTranslations();
   //! \brief Checks for a newer version and prompts user to download.
   static void checkForNewVersion(MainWindow* mw);

   static void loadMap();

   //! \return the weight system
   static SystemOfMeasurement getWeightUnitSystem();
   //! \return the temperature scale
   static TempScale getTemperatureScale();
   //! \return the color units
   static Unit::unitDisplay getColorUnit();
   //! \return the diastatic power units
   static Unit::unitDisplay getDiastaticPowerUnit();
};


/*!
 * \mainpage Brewtarget Source Code Documentation
 *
 * \section secIntro Introduction
 *
 * Brewtarget is a cross-platform open source beer recipe software suite.
 */

#endif
