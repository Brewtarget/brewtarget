/**
 * PersistentSettings.h is part of Brewtarget, and is copyright the following
 * authors 2009-2021:
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Scott Peshak <scott@peshak.net>
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
#ifndef PERSISTENTSETTINGS_H
#define PERSISTENTSETTINGS_H
#pragma once

#include <QDir>
#include <QString>
#include <QVariant>

// .:TODO:. Most of the code still uses hard-coded strings for settings names.  We should go through and convert these
//          all to compile-time-checkable constants in the PersistentSettings::Names namespace (ie defined here).
//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddSettingName(setting) namespace PersistentSettings::Names {static char const * const setting = #setting; }
AddSettingName(dbType)
AddSettingName(last_db_merge_req)
#undef AddSettingName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \brief Functions that manage remembering settings across sessions.
 *        Most of the heavy lifting is done by Qt's QSettings class.  We just add some minor extensions and make the
 *        interface more consistent with QHash, QMap, etc.
 *
 *        Users should bear in mind the following guidelines, based on QSettings documentation:
 *          - Always refer to the same key using the same case. For example, if you refer to a key as "text fonts" in
 *            one place in your code, don't refer to it as "Text Fonts" somewhere else.
 *          - Avoid key names that are identical except for the case. For example, if you have a key called
 *            "MainWindow", don't try to save another key as "mainwindow".
 *          - Do not use slashes or backslashes in section or key names
 *          - Avoid key names that are the same as section names
 *        For the same reason we use PropertyNames::NamedEntity::name and similar constants, we have
 *        PersistentSettings::Names::foobar etc above
 */
namespace PersistentSettings {

   /**
    * \brief Sets up where persistent settings are stored.  This needs to be called \b after calls to
    *        \c QApplication::setOrganizationName() etc
    *
    * \param customUserDataDir Used only for development/testing (via a command-line option) to override where user
    *                          data is stored
    */
   void initialise(QString customUserDataDir = "");

   /**
    * \return the config directory
    */
   QDir getConfigDir();

   /**
    * \return the directory where all user data (apart from logs & settings) should be stored
    */
   QDir getUserDataDir();

   /**
    * \brief Set a custom location for the directory where all user data (apart from logs & settings) should be stored.
    *
    *        NB: It is the caller's responsibility to move any files from the old directory and either reset the DB
    *            connections to SQLite or restart the application.
    */
   void setUserDataDir(QDir newDirectory);

   /**
    * \brief What extension (if any) to add to the key when accessing a stored setting.  This is useful for associating
    *        a stored value with its scale and/or unit
    */
   enum Extension {
      NONE,
      SCALE,
      UNIT
   };

   /**
    * \brief Returns true if the persistent settings storage contains an item with "fully-qualified" key (generated
    *        from key, section and Extension); otherwise returns false.
    */
   bool contains(QString const & key, QString const section = QString(), Extension extension = NONE);

   /**
    * \brief Inserts a new item with the "fully-qualified" key (generated from key, section and Extension) and a value
    *        of value.
    *        If there is already an item with a corresponding key, that item's value is replaced with value.
    */
   void insert(QString const & key, QVariant value, QString const section = QString(), Extension extension = NONE);

   /**
    * \brief Returns the value associated with the "fully-qualified" key (generated from key, section and Extension).
    *        If the persistent settings storage contains no item with a corresponding key, the function returns
    *        defaultValue.
    *        If no defaultValue is specified, the function returns a default-constructed value.
    */
   QVariant value(QString const & key,
                  QVariant const defaultValue = QVariant(),
                  QString const section = QString(),
                  Extension = NONE);

   /**
    * \brief Removes the item matching key (unless key is the name of a section, in which case it removes all keys in
    *        that section -- hence one reason you don't want keys and sections to share names).
    */
   void remove(QString const & key, QString const section = QString(), Extension extension = NONE);

}
#endif
