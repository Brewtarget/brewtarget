/*
 * Log.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mattias Måhl <mattias@kejsarsten.com>
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

#ifndef _LOG_H
#define _LOG_H

#include <QDir>
#include <QObject>
#include <QMutex>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>
#include <QTime>
#include <QApplication>
#include <QMutexLocker>

/*!
 * \NameSpace Log
 *
 * \brief Provides a proxy to an OS agnostic Log file.
 */
namespace Log
{
   //! \brief The log level of a message.
   enum LogType {
      //! Meant to express big actions one would want to find in the log file.
      LogType_INFO,
      //! Just a warning.
      LogType_WARNING,
      //! Full-blown error.
      LogType_ERROR,
      //! Meant for debugging only. If we see this in prod, we can safely remove.
      LogType_DEBUG
   };

   extern QTextStream errStream;
   extern QFile logFile;
   extern bool isLoggingToStderr;
   extern QTextStream* stream;
   extern QMutex mutex;
   extern char const* logLevelNames[];
   extern char const* qtLogLevelTranslate[];
   extern LogType const qtLogLevelTranslateEnum[];

   // options set by the end user.
   extern bool loggingEnabled;
   extern LogType logLevel;
   extern QDir logFilePath;
   extern bool logUseConfigDir;
   extern int const logFileSize;
   extern int const logFileCount;
   extern QString logFileName;
   extern QString logFileNameSuffix;
   extern QString timeFormat;
   extern QString tmpl;

   //! \brief Sets the default directory of the log file
   //! \param defaultDir The directory which will host the log file.
   extern void changeDirectory(const QDir defaultDir);
   extern void changeDirectory();
   extern void doLog(const LogType lt, const QString message);
   extern QString getTypeName(const LogType type);
   extern LogType getLogTypeFromString(QString type = QString("INFO"));

   /*
   * Generates a Logfilename based on the settings above.
   * prepared for user settings if need be.
   */
   extern QString logFileFullName();

   /* initLogFileName initializes the log file and opens the stream for writing.
    * This was moved to its own function as this has to be called everytime logs are being pruned.
    */
   extern bool initLogFileName();

   /* logMessageHandler is going to handle all the logmessages.
    * the old way of calling the Brewtarget::LogX functions will persist, but will use this way of writing logs as well.
    * Usage after this version will be:
    * qDebug << "message" << some_variable; //for a debug message!
    * There is also, qWarning, qCritical, qFatal, qInfo that could be used.
    */
   extern void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);

   /* Initialize logging to utilize the built in logging funxtionality in QT5
    * this has to be called before any logging is done. should be self contained and not depend on anything being loaded.
    * although user settings may alter location of files, this module will always start logging at the default application data path.
    * i.e. on linux: ~/.config/brewtarget or on Windows %APPDATA% path.
    */
   extern bool initializeLog();

   /* Prunes old log files from the directory, keeping only the specified number of files in logFileCount,
    * purpose is to keep log files to a mininum while keeping the logs up-to-date and also not require manual pruning of files.
    */
   extern void pruneLogFiles();

   /*
   * \brief Closes the log file stream and the file handle.
   */
   extern void closeLogFile();

   /* Get the list of Logfiles present in the directory currently logging in and returns a FileInfoList containing the files.*/
   extern QFileInfoList getLogFileList();
}

#endif /* _LOG_H */
