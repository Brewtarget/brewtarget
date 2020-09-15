/*
 * Log.cpp is part of Brewtarget, and is Copyright the following
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

#include "Log.h"

#include <QDir>
#include <QTime>
#include <QString>

const QString Log::filename = "brewtarget_log.txt";
const QString Log::timeFormat = "hh:mm:ss.zzz";
const QString Log::tmpl = "[%1] %2 : %3";

Log::Log(bool isLoggingToStderr)
   : errStream(stderr),
     file(),
     isLoggingToStderr(isLoggingToStderr),
     stream(nullptr) {
}

Log::~Log() {
   delete stream;
   stream = nullptr;
   if( file.isOpen() )
      file.close();
}

void Log::changeDirectory(const QDir defaultDir) {
   if (stream) {
      doLog(LogType_ERROR, "Cannot change logging directory after it is initialized.");
      return;
   }
   // Test default location
   file.setFileName(defaultDir.filePath(filename));
   if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) ) {
      stream = new QTextStream(&file);
      return;
   }

   // Defaults to temporary
   file.setFileName(QDir::temp().filePath(filename));
   if( file.open(QFile::WriteOnly | QFile::Truncate) ) {
      stream = new QTextStream(&file);
      warn(QString("Log is in a temporary directory: %1").arg(file.fileName()));
      return;
   }

   warn(QString("Could not create a log file."));
}

void Log::changeDirectory() {
   //If it's the same, just return, no need to do anything.
   if ( LogFilePath.filePath(filename) == file.fileName() ) {
      return;
   }

   //If the file is already initialize, it needs to be closed and redefined.
   if (stream) {
      delete stream;
      stream = nullptr;
      if( file.isOpen() )
         file.close();
      //Preserving the old logfile
      if ( ! file.copy(LogFilePath.filePath(filename)) ) {
         error("Error while copying to the new file location\nReverting settings");
         LogFilePath = QDir(QFileInfo(file).filePath());
      }
   }

   // Test default location
   file.setFileName(LogFilePath.filePath(filename));
   if( file.open(QIODevice::Append) ) {
      stream = new QTextStream(&file);
      return;
   }

   // Defaults to temporary
   file.setFileName(QDir::temp().filePath(filename));
   if( file.open(QFile::WriteOnly | QFile::Truncate) ) {
      stream = new QTextStream(&file);
      warn(QString("Log is in a temporary directory: %1").arg(file.fileName()));
      return;
   }

   warn(QString("Could not create a log file."));
}

void Log::debug(const QString message) {
   doLog(LogType_DEBUG, message);
}

void Log::info(const QString message) {
   doLog(LogType_INFO, message);
}

void Log::warn(const QString message) {
   doLog(LogType_WARNING, message);
}

void Log::error(const QString message) {
   doLog(LogType_ERROR, message);
}

void Log::doLog(const LogType lt, const QString message) {
   QString logEntry = tmpl
         .arg(QTime::currentTime().toString(timeFormat))
         .arg(getTypeName(lt))
         .arg(message);

   mutex.lock();
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   if (isLoggingToStderr)
      errStream << logEntry << endl;
   if (stream)
      *stream << logEntry << endl;
#else
   if (isLoggingToStderr)
      errStream << logEntry << Qt::endl;
   if (stream)
      *stream << logEntry << Qt::endl;
#endif
   mutex.unlock();

   emit wroteEntry(logEntry);
}

QString Log::getTypeName(const LogType type) const {
   switch(type) {
      case LogType_DEBUG: return tr("DEBUG");
      case LogType_INFO: return tr("INFO");
      case LogType_WARNING: return tr("WARNING");
      case LogType_ERROR: return tr("ERROR");
   }
   return tr("ERROR");
}

Log::LogType Log::getLogTypeFromString(QString type)
{
   if (type == "INFO") return LogType_INFO;
   else if (type == "WARNING") return LogType_WARNING;
   else if (type == "ERROR") return LogType_ERROR;
   else if (type == "DEBUG") return LogType_DEBUG;
   else return LogType_INFO;
}

QString Log::getOptionStringFromLogType(const LogType type)
{
   switch(type) {
      case LogType_DEBUG: return QString("DEBUG");
      case LogType_INFO: return QString("INFO");
      case LogType_WARNING: return QString("WARNING");
      case LogType_ERROR: return QString("ERROR");
   }
   return QString("INFO");
}
