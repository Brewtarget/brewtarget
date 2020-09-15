/*
 * Log.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Maxime Lavigne <duguigne@gmail.com>
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

/*!
 * \class Log
 *
 * \brief Provides a proxy to an OS agnostic Log file.
 */
class Log : public QObject
{
   Q_OBJECT
   friend class Brewtarget;
   friend class OptionDialog;
public:
   ~Log();

   //! \brief Logs a debug message. (safe to remove on prod)
   void debug(const QString message);
   //! \brief Logs a informational message.
   void info(const QString message);
   //! \brief Logs a warning message.
   void warn(const QString message);
   //! \brief Logs a severe error. This is a potential fatal error.
   void error(const QString message);

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

signals:
   void wroteEntry(const QString entry);

private:
   QTextStream errStream;
   QFile file;
   bool isLoggingToStderr;
   QTextStream* stream;
   QMutex mutex;

   // options set by the end user.
   bool LoggingEnabled = false;
   Log::LogType LoggingLevel = LogType_INFO;
   QDir LogFilePath;
   bool LoggingUseConfigDir = true;

   static const QString filename;
   static const QString timeFormat;
   static const QString tmpl;
   Log(bool isLoggingToStderr);

   //! \brief Sets the default directory of the log file
   //! \param defaultDir The directory which will host the log file.
   void changeDirectory(const QDir defaultDir);
   void changeDirectory();
   void doLog(const LogType lt, const QString message);
   QString getTypeName(const LogType type) const;
   LogType getLogTypeFromString(QString type = QString("INFO"));
   QString getOptionStringFromLogType(const LogType type = LogType_INFO);
};

#endif /* _LOG_H */
