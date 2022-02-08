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

// need to use this to turn on Mac keyboard shortcuts (see https://doc.qt.io/qt-5/qkeysequence.html#qt_set_sequence_auto_mnemonic)
extern void qt_set_sequence_auto_mnemonic(bool b);

#include <QDir>
#include <QMetaProperty>

class MainWindow;

// Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
Q_DECLARE_METATYPE( QMetaProperty )

/*!
 * \brief Figures out stuff from the system etc.
 *
 * TODO: The config & system options stuff probably belongs in a separate class, and the remainder of what's here might go in main or MainWindow...
 */
namespace Brewtarget {
   //! \return the data directory
   QDir getDataDir();
   //! \return the doc directory
   QDir getDocDir();
   //! \return the config directory
   const QDir getConfigDir();
   //! \return user-specified directory where the database files reside.
   QDir getUserDataDir();
   //! \return The System path for users applicationpath. on windows: c:\\users\\<USERNAME>\\AppData\\Roaming\\<APPNAME>
   QDir getDefaultUserDataDir();

   /**
    * \return the resource directory where some files that ship with Brewtarget live (default DB, sounds, translations)
    *
    *         Most resources are compiled into the app with the Qt Resource System (see
    *         https://doc.qt.io/qt-5/resources.html) but, for some files, we want the user also to be able to access
    *         the file directly.  Such files are stored in this directory.
    */
   QDir getResourceDir();

   /*!
    * \brief Blocking call that executes the application.
    * \param userDirectory If !isEmpty, overwrites the current settings.
    * \return Exit code from the application.
    */
   int run();

   //! \brief Every so often, we need to update the config file itself. This does that.
   void updateConfig();
   //! \brief Read options from options. This replaces readPersistentOptions()
   void readSystemOptions();
   //! \brief Writes the persisten options back to the options store
   void saveSystemOptions();

   //! \brief If this option is false, do not bother the user about new versions.
   void setCheckVersion(bool value);

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
   bool initialize();

   /*!
    * \brief Run after QApplication exits to clean up shit, close database, etc.
    */
   void cleanup();

   /*!
    * \brief If false, run Brewtarget in a way that requires no user interaction
    *
    * For example, if running a test case, ensure that no dialogs pop up that
    * prevent Brewtarget from starting
    */
   bool isInteractive();

   //! \brief Set the mode to an interactive or non-interactive state
   void setInteractive(bool val);

}


/*!
 * \mainpage Brewtarget Source Code Documentation
 *
 * \section secIntro Introduction
 *
 * Brewtarget is a cross-platform open source beer recipe software suite.
 */

#endif
