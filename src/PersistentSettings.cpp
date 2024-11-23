/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * PersistentSettings.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Chris Pavetto <chrispavetto@gmail.com>
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • Daniel Moreno <danielm5@users.noreply.github.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Medic Momcilo <medicmomcilo@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Mikhail Gorbunov <mikhail@sirena2000.ru>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *   • Scott Peshak <scott@peshak.net>
 *   • Ted Wright <tedwright@users.sourceforge.net>
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
#include "PersistentSettings.h"

#include <memory>

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

#include "config.h"

//
// Anonymous namespace for constants, global variables and functions used only in this file
//
namespace {
   //
   // Most of the PersistentSettings functions should not be called until after PersistentSettings::initialise()
   //
   bool initialised = false;

   /**
    * \brief Generate a fully-qualified key based on key, section and extension
    */
   QString generateFqKey(QString const & key, QString const section, PersistentSettings::Extension extension) {

      // If section is specified, we prepend it to the key in front of a slash, otherwise we just use the key as is.
      //
      // Concatenating section + '/' + key makes key a subkey of section.  But, when using QSettings::IniFormat format,
      // this translates to grouping the entry for key under a heading of "[section]".
      QString fullyQualifiedKey{
         section.isNull() ? key : QString("%1/%2").arg(section).arg(key)
      };

      switch (extension) {
         case PersistentSettings::Extension::UNIT:
            fullyQualifiedKey += "_unit";
            break;
         case PersistentSettings::Extension::SCALE:
            fullyQualifiedKey += "_scale";
            break;
         case PersistentSettings::Extension::NONE:
         default:
            break;
      }

      return fullyQualifiedKey;
   }

   //
   // By default, QSettings stores things differently on different platforms (registry on Windows, CFPreferences API on
   // Mac, a text .conf file on Linux).  HOWEVER, we prefer to force use of a text file on all platforms as potentially
   // aids debugging (eg it's relatively then easy for an end user raising a bug report to upload a text config file,
   // but it might be somewhat harder for them to dig around in the Windows registry).
   //
   // For similar reasons, we would like to control the location and name of the file.
   //
   // (The text format used is the Windows Ini file format, as described in Qt documentation at
   // https://doc.qt.io/qt-5/qsettings.html#Format-enum.)
   //
   // This is set by PersistentSettings::initialise()
   //
   std::unique_ptr<QSettings> qSettings{nullptr};

   // These also get set by PersistentSettings::initialise()
   QDir configDir{""};
   QDir userDataDir{""};

}

void PersistentSettings::initialise(QString customUserDataDir) {
   // It's a programming error if this is called more than once.
   Q_ASSERT(!initialised);

   //
   // First work out which directory we want to store config files in.  Since Qt 5.5, this is trivial as the framework
   // can do all the work for us.
   //
   // Qt documentation says QStandardPaths::AppConfigLocation is "a directory location where user-specific
   // configuration files should be written. This is an application-specific directory, and the returned path is never
   // empty".
   //
   // Pretty sure this needs to be done after calls to QApplication::setOrganizationName() etc in main(), which is why
   // we don't just use static initialisation.
   //
   // Yes, we are knowingly logging before Logging::initializeLogging() has been called (as the latter needs
   // PersistentSettings::initialise() to be called first).  This is OK as it just means the log message will go to the
   // default Qt-0determined location (eg stderr on Linux).
   //
   // The value in this logging is that we have seen instances where the software is unable to determine a value for
   // configDir, and we would like to diagnose why (eg whether it is some permissions problem).
   //
   qInfo() <<
      Q_FUNC_INFO << "Qt-proposed directories for user-specific configuration files are:" <<
      QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
   configDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
   qInfo() <<
      Q_FUNC_INFO << "Preferred writeable directory for user-specific configuration files is:" <<
      configDir.absolutePath();

   // Older versions of the software had a different, less descriptive, name for the .conf file.  If the old file is
   // present but the new one is not, we rename it.
   QString const oldConfigFileName = QString("%1.conf").arg(CONFIG_APPLICATION_NAME_LC);
   QString const newConfigFileName = QString("%1PersistentSettings.conf").arg(CONFIG_APPLICATION_NAME_LC);
   if (configDir.exists(oldConfigFileName) && !configDir.exists(newConfigFileName)) {
      qInfo() << Q_FUNC_INFO << "Renaming" << oldConfigFileName << "to" << newConfigFileName << "in" << configDir;
      if (!configDir.rename(oldConfigFileName, newConfigFileName)) {
         // QDir::rename will tell you if it failed, but not why, so we just log the failure
         qWarning() <<
            Q_FUNC_INFO << "Error renaming" << oldConfigFileName << "to" << newConfigFileName << "in" << configDir;
      }
   }

   // Now we can set the full path of the persistent settings file
   qSettings = std::make_unique<QSettings>(configDir.absoluteFilePath(newConfigFileName), QSettings::IniFormat);
   qInfo() << Q_FUNC_INFO << "Persistent settings file:" << qSettings->fileName();

   // We've done enough now for calls to contains()/insert()/value() etc to work.  Mark that we're initialised so we
   // can (potentially) use one of those calls to initialise the user data directory.
   initialised = true;

   qInfo() << Q_FUNC_INFO << "Config dir:" << configDir.canonicalPath() << "(" << configDir.absolutePath() << ")";

   // For dev and testing purposes, the user data directory can be overridden via a command-line option, hence the
   // parameter to this function
   if (!customUserDataDir.isEmpty()) {
      userDataDir.setPath(customUserDataDir);
   } else {
      userDataDir.setPath(
         // Note that Brewtarget differs from Brewken in its default location for the database.  Specifically,
         // Brewtarget puts it in the same directory as the logging and conf files (~/.config/brewtarget/ on Linux)
         // Brewken puts it QStandardPaths::AppDataLocation (which, for Brewtarget, would be ~/.local/share/brewtarget/
         // on Linux).  This is a slightly more logical location, but we don't want to change directories for existing
         // Brewtarget users.
         PersistentSettings::value(PersistentSettings::Names::UserDataDirectory,
                                   QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).toString()
      );
   }

   return;
}

[[nodiscard]] bool PersistentSettings::isInitialised() {
   return initialised;
}

QDir PersistentSettings::getConfigDir() {
   Q_ASSERT(initialised);
   // Note that it can be valid for canonicalPath() to return empty string -- if config dir is current dir
   Q_ASSERT(!configDir.absolutePath().isEmpty());
   return configDir;
}

QDir PersistentSettings::getUserDataDir() {
   Q_ASSERT(initialised);
   // Note that it can be valid for canonicalPath() to return empty string -- if user data dir is current dir
   Q_ASSERT(!userDataDir.absolutePath().isEmpty());
   return userDataDir;
}

void PersistentSettings::setUserDataDir(QDir newDirectory) {
   Q_ASSERT(initialised);
   userDataDir = newDirectory;
   PersistentSettings::insert(PersistentSettings::Names::UserDataDirectory, userDataDir.absolutePath());
   return;
}

bool PersistentSettings::contains(QString const & key,
                                  QString const section,
                                  PersistentSettings::Extension extension) {
   Q_ASSERT(initialised);
   return qSettings->contains(generateFqKey(key, section, extension));
}

bool PersistentSettings::contains(BtStringConst const & constKey,
                                  QString const section,
                                  PersistentSettings::Extension extension) {
   Q_ASSERT(!constKey.isNull());
   QString key{*constKey};
   return PersistentSettings::contains(key, section, extension);
}

bool PersistentSettings::contains(BtStringConst const & constKey,
                                  BtStringConst const & constSection,
                                  PersistentSettings::Extension extension) {
   Q_ASSERT(!constSection.isNull());
   QString section{*constSection};
   return PersistentSettings::contains(constKey, section, extension);
}

void PersistentSettings::insert(QString const & key,
                                QVariant value,
                                QString const section,
                                PersistentSettings::Extension extension) {
   Q_ASSERT(initialised);
   // QSettings is a bit inconsistent here in using setValue() when QMap, QHash etc use insert() for the equivalent
   // functionality
   qSettings->setValue(generateFqKey(key, section, extension), value);
   return;
}

void PersistentSettings::insert(BtStringConst const & constKey,
                                QVariant value,
                                QString const section,
                                PersistentSettings::Extension extension) {
   Q_ASSERT(!constKey.isNull());
   QString key{*constKey};
   PersistentSettings::insert(key, value, section, extension);
   return;
}

void PersistentSettings::insert(BtStringConst const & constKey,
                                QVariant value,
                                BtStringConst const & constSection,
                                Extension extension) {
   Q_ASSERT(!constSection.isNull());
   QString section{*constSection};
   PersistentSettings::insert(constKey, value, section, extension);
   return;
}

QVariant PersistentSettings::value(QString const & key,
                                   QVariant const defaultValue,
                                   QString const section,
                                   PersistentSettings::Extension extension) {
   Q_ASSERT(initialised);
   return qSettings->value(generateFqKey(key, section, extension), defaultValue);
}

QVariant PersistentSettings::value(BtStringConst const & constKey,
                                   QVariant const defaultValue,
                                   QString const section,
                                   PersistentSettings::Extension extension) {
   Q_ASSERT(!constKey.isNull());
   QString key{*constKey};
   return PersistentSettings::value(key, defaultValue, section, extension);
}

QVariant PersistentSettings::value(BtStringConst const & constKey,
                                   QVariant const defaultValue,
                                   BtStringConst const & constSection,
                                   PersistentSettings::Extension extension) {
   Q_ASSERT(!constSection.isNull());
   QString section{*constSection};
   return PersistentSettings::value(constKey, defaultValue, section, extension);
}

void PersistentSettings::remove(QString const & key,
                                QString const section,
                                PersistentSettings::Extension extension) {
   Q_ASSERT(initialised);
   QString fqKey{generateFqKey(key, section, extension)};

   // Not entirely clear from Qt docs whether we need to bother checking contains() before calling remove(), but it
   // doesn't hurt any.
   if (PersistentSettings::contains(fqKey)) {
      qSettings->remove(fqKey);
   }
   return;
}

void PersistentSettings::remove(BtStringConst const & constKey,
                                QString const section,
                                PersistentSettings::Extension extension) {
   Q_ASSERT(!constKey.isNull());
   QString key{*constKey};
   PersistentSettings::remove(key, section, extension);
   return;
}

void PersistentSettings::remove(BtStringConst const & constKey,
                                    BtStringConst const & constSection,
                                    PersistentSettings::Extension extension) {
   Q_ASSERT(!constSection.isNull());
   QString section{*constSection};
   PersistentSettings::remove(constKey, section, extension);
   return;
}
