/**
 * PersistentSettings.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2021:
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
#include "PersistentSettings.h"

#include <QSettings>
#include <QStandardPaths>

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
         case PersistentSettings::UNIT:
            fullyQualifiedKey += "_unit";
            break;
         case PersistentSettings::SCALE:
            fullyQualifiedKey += "_scale";
            break;
         case PersistentSettings::NONE:
         default:
            break;
      }

      return fullyQualifiedKey;
   }

   //
   // By default, QSettings stores things differently on different platforms (registry on Windows, CFPreferences API on
   // Mac, a text .conf file on Linux).  We prefer to force use of a text file on all platforms as potentially aids
   // debugging (eg it's relatively then easy for an end user raising a bug report to upload a text config file, but it
   // might be somewhat harder for them to dig around in the Windows registry).
   //
   // For similar reasons, we would like to control the location and name of the file.
   //
   // (The text format used is the Windows Ini file format, as described in Qt documentation at
   // https://doc.qt.io/qt-5/qsettings.html#Format-enum.)
   //
   // This is set by PersistentSettings::initialise()
   //
   QSettings * qSettings = nullptr;

   // These also get set by PersistentSettings::initialise()
   QDir configDir;
   QDir userDataDir;

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
   configDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

   // Now we can set the full path of the persistent settings file
   qSettings = new QSettings{configDir.absoluteFilePath("brewtarget.conf"),
                             QSettings::IniFormat};

   // We've done enough now for calls to contains()/insert()/value() etc to work.  Mark that we're initialised so we
   // can (potentially) use one of those calls to initialise the user data directory.
   initialised = true;

   // For dev and testing purposes, the user data directory can be overridden via a command-line option, hence the
   // parameter to this function
   if (!customUserDataDir.isEmpty()) {
      userDataDir.setPath(customUserDataDir);
   } else {
      userDataDir.setPath(
         PersistentSettings::value(PersistentSettings::Names::UserDataDirectory,
                                   QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString()
      );
   }

   return;
}

QDir PersistentSettings::getConfigDir() {
   Q_ASSERT(initialised);
   return configDir;
}

QDir PersistentSettings::getUserDataDir() {
   Q_ASSERT(initialised);
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
