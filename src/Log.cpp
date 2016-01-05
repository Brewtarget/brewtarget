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

const QString Log::filename = "brewtarget_log.txt";
const QString Log::timeFormat = "hh:mm:ss.zzz";
const QString Log::tmpl = "[%1] %2 : %3";

Log::Log(bool isLoggingToStderr)
   : errStream(stderr),
     file(),
     isLoggingToStderr(isLoggingToStderr),
     stream(NULL) {
}

Log::~Log() {
   delete stream;
   stream = NULL;
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
   if (isLoggingToStderr)
      errStream << logEntry << endl;
   if (stream)
      *stream << logEntry << endl;
   mutex.unlock();

   emit wroteEntry(logEntry);
}

QString Log::getTypeName(const LogType type) const {
   switch(type) {
      case LogType_DEBUG: return "DEBUG";
      case LogType_INFO: return "INFO";
      case LogType_WARNING: return "WARNING";
      case LogType_ERROR: return "ERROR";
      default: return "";
   }
}
