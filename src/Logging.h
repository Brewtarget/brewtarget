/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Logging.h is part of Brewtarget, and is copyright the following authors 2009-2021:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
#ifndef LOGGING_H
#define LOGGING_H
#pragma once

#include <optional>

#include <QDir>
#include <QFileInfoList>
#include <QString>
#include <QVector>

/*!
 * \brief Provides a proxy to an OS agnostic log file.
 */
namespace Logging {
   /**
    * \brief Defines the importance of an individual message and used to controls what type of messages to log
    *
    *        This is similar to QtMsgType except we need the numeric order of these levels to match the "logical
    *        order", which is not the case with QtMsgType (because QtInfoMsg was a later addition).  What we mean
    *        by the "logical order" is that the higher the level number, the more urgent/important the message is.
    *
    *        Thus, if logging level is set to LogLevel_WARNING then only messages of LogLevel_WARNING and
    *        LogLevel_ERROR will be logged.
    */
   enum Level {
      //! Message about the inner workings of the application.  Mainly used during development or debugging.  End users
      //  shouldn't normally need to see these messages.
      LogLevel_DEBUG,
      //! An FYI message that an end user can safely ignore but that might be useful to understand what the app has
      //  done or to diagnose a bug.  This is the default logging level.
      LogLevel_INFO,
      //! This is something that might be a problem and is almost certainly good to know when diagnosing problems.
      LogLevel_WARNING,
      //! Something that is definitely an error and that we always want to log
      LogLevel_ERROR
   };

   /**
    * \brief User-friendly info about logging levels.  Although we use an enum internally to identify a logging level,
    *        we also need:
    *           - A string name to record the level in the log messages themselves and to use in the config file
    *           - A description to show the user on the Options dialog
    */
   struct LevelDetail {
      Level level;
      char const * name;
      QString description;
   };
   extern QVector<LevelDetail> const levelDetails;

   /**
    * \brief Convert logging level to a string representation
    */
   extern QString getStringFromLogLevel(const Level type);

   /**
    * \brief Convert a string representation of a logging level to a logging level
    */
   extern Level getLogLevelFromString(QString const level = QString("INFO"));

   /**
    * \brief Get current logging level
    */
   extern Level getLogLevel();

   /**
    * \brief Set logging level
    */
   extern void setLogLevel(Level newLevel);

   /**
    * \return \b true if we are logging in the config dir (the default), \b false if we are logging in a directory
    *         configured via \c Logging::setDirectory()
    */
   extern bool getLogInConfigDir();

   extern int const logFileSize;
   extern int const logFileCount;

   /**
    * \brief See \c Logging::setDirectory()
    */
   enum PersistNewDirectory {
      NewDirectoryIsPermanent,
      NewDirectoryIsTemporary
   };

   /**
    * \brief Sets the directory in which log files are stored.  Note however that this setting, whilst remembered, is
    *        ignored if we are configured to log in the config directory.
    * \param newDirectory  If set, specifies where to write log files.  If not set, then use the default location,
    *                      which is the config directory.
    *
    *                      Although it's not strictly obvious that config (or rather persistent settings) and log files
    *                      go in the same directory, it is pragmatic for a couple of reasons:
    *                        - If we want end users to supply diagnostic info with bug reports, then it's easier for
    *                          them not to have too many different locations to go looking for that data.
    *                        - Qt provides canonical application-specific locations for user data and for config data,
    *                          but not for log files.  (This is perhaps because there is not always an obvious
    *                          "standard" location for application log files.)  Whilst we could put log files with user
    *                          data (eg in a "logging" sub-directory), that's also not ideal as we'd like to be able to
    *                          tell end users that the user data directory contains everything that they need to take
    *                          backups of.
    *
    * \param persistNewDirectory By default we the new logging directory in persistent settings so it will be remembered
    *                            on the next run of the program.  But this behaviour can be turned off for testing.
    *
    * \return true if succeeds, false otherwise
    */
   extern bool setDirectory(std::optional<QDir> newDirectory,
                            Logging::PersistNewDirectory const persistNewDirectory = Logging::NewDirectoryIsPermanent);

   /**
    * \brief Gets the directory in which log files are stored
    * \return The directory
    */
   extern QDir getDirectory();

   /**
    * \brief  Initialize logging to utilize the built in logging functionality in Qt.
    *         This has to be called before any logging is done, but after PersistentSettings::initialise() is called.
    * \return
    */
   extern bool initializeLogging();

   /**
    * \brief By default, logging goes to stderr and to the logfile.  In certain very limited circumstances, you might
    *        want to disable logging to stderr.  Calling this function with \c false as the parameter will do that (with
    *        the exception of errors about logging itself, eg inability to rotate log files, which will still show up on
    *        stderr).
    *
    *        At time of writing, the only known reason for suspending logging to stderr is to run a test that generates
    *        a lot of dummy logs...
    */
   extern void setLoggingToStderr(bool const enabled);

   /**
    * \brief  Get the list of Logfiles present in the directory currently logging in
    */
   extern QFileInfoList getLogFileList();

   /**
    * \brief Terminate logging
    */
   extern void terminateLogging();

   /**
    * \brief Gets a stack trace, which should be logged with noquote()
    */
   extern QString getStackTrace();
}

#endif
