/*
 * Log.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mattias M�hl <mattias@kejsarsten.com>
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
namespace Log
{
   QTextStream errStream(stderr);
   QFile logFile;
   bool isLoggingToStderr(true);
   QTextStream* stream;
   QMutex mutex;
   char const* logLevelNames[] =
   {
      "INFO",
      "WARNING",
      "ERROR",
      "DEBUG"
   };

   char const* qtTypeTranslate[] =
   {
      "DEBUG",   /* QtDebugMsg    */
      "WARNING", /* QtWarningMsg  */
      "CRITICAL",   /* QtCriticalMsg */
      "FATAL",   /* QtFatalMsg    */
      "INFO"     /* QtInfoMsg     */
   };

   LogType const qtLogLevelTranslateEnum[] =
   {
      LogType_DEBUG,   /* QtDebugMsg = 0    */
      LogType_WARNING, /* QtWarningMsg = 1  */
      LogType_ERROR,   /* QtCriticalMsg = 2 */
      LogType_ERROR,   /* QtFatalMsg = 3    */
      LogType_INFO     /* QtInfoMsg = 4     */
   };

   // options set by the end user.
   // Default logging set to enable to get the first logs out.
   bool loggingEnabled(true);
   // Default logging level set to INFO, to get only status logs.
   LogType logLevel = LogType_INFO;
   // Stores the path to the logfiles
   QDir logFilePath;
   // Set to true if you want brewtarget to figure out where to put the logs.
   bool logUseConfigDir = true;
   // Set the log file size for the rotation.
   int const logFileSize = 500 * 1024;
   // set the number of files to keep when rotating.
   int const logFileCount = 5;
   // \brief this is the file we're always logging to.
   QString logFileName("brewtarget_log");
   // \brief what file ending to use for the file.
   QString logFileNameSuffix("txt");
   QString timeFormat;
   QString tmpl;

   QString logFileFullName() {
      return QString("%1.%2")
         .arg(logFileName)
         .arg(logFileNameSuffix);
   }

   bool initializeLog()
   {
      timeFormat = "hh:mm:ss.zzz";
      tmpl = "[%1] %2 : %3";
      //need to test if we're running in a test and ajust the location for the logfiles accordingly, this to not clutter the application path with dummy data.
      if (QCoreApplication::applicationName() == "brewtarget-test")
      {
         logFilePath.setPath(QDir::tempPath());
      }
      qInstallMessageHandler(Log::logMessageHandler);
      qDebug() << Q_FUNC_INFO << "Logging initialized ";
      return true;
   }

   void changeDirectory(const QDir defaultDir) {
      if (stream) {
         doLog(LogType_ERROR, "Cannot change logging directory after it is initialized.");
         return;
      }
      // default location
      logFile.setFileName(defaultDir.filePath(logFileName));
      if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
         stream = new QTextStream(&logFile);
         return;
      }

      // Defaults to temporary
      logFile.setFileName(QDir::temp().filePath(logFileName));
      if (logFile.open(QFile::WriteOnly | QFile::Truncate)) {
         stream = new QTextStream(&logFile);
         qWarning() << QString("Log is in a temporary directory: %1").arg(logFile.fileName());
         return;
      }

      qWarning() << "Could not create a log file.";
   }

   void changeDirectory() {
      qDebug() << Q_FUNC_INFO;
      //If it's the same, just return, no need to do anything.
      if (logFilePath.filePath(logFileFullName()) == logFile.fileName()) {
         return;
      }

      //Check if the new directory exists, if not create it.
      if (!logFilePath.exists())
      {
         qDebug() << Q_FUNC_INFO << logFilePath.canonicalPath() << "does not exist, creating";
         if (!logFilePath.mkpath(logFilePath.canonicalPath()))
         {
            qCritical() << QString("Could not create directory as location '%1'").arg(logFilePath.canonicalPath());
            logFilePath = QDir(QFileInfo(logFile).filePath());
            return;
         }
      }

      /*If the file is already initialize, it needs to be closed and redefined.
         * Note! This only coppies the current Logfile, the older ones will be left behind.
         * May be a later refraction/development though.
         */
      if (stream)
      {
         //Close the file if open and reset the stream.
         closeLogFile();
         //Preserving the old logfiles
         if (!logFile.copy(logFilePath.filePath(logFileFullName())))
         {
            qCritical() << "Error while copying to the new file location. Reverting settings";
            logFilePath = QDir(QFileInfo(logFile).filePath());
            return;
         }
      }

      // Generate a new filename to save the logs in
      if (initLogFileName())
      {
         return;
      }

      qWarning() << QString("Could not create a log file.");
   }

   void doLog(const LogType lt, const QString message) {
      QMutexLocker locker(&mutex);
      QString logEntry = tmpl
         .arg(QTime::currentTime().toString(timeFormat))
         .arg(getTypeName(lt))
         .arg(message);

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

      //emit wroteEntry(logEntry);
   }

   QString getTypeName(const LogType type) {
      return QString(logLevelNames[type]);
   }

   LogType getLogTypeFromString(QString type)
   {
      if (type == "INFO") return LogType_INFO;
      else if (type == "WARNING") return LogType_WARNING;
      else if (type == "ERROR") return LogType_ERROR;
      else if (type == "DEBUG") return LogType_DEBUG;
      else return LogType_INFO;
   }

   bool initLogFileName()
   {
      //Accuire lock due to the file mangling below.
      QMutexLocker locker(&mutex);
      //first check if it's time to rotate the log file
      if (logFile.size() > logFileSize)
      {
         //Doublecheck that the stream is not initiated, if so, kill it.
         closeLogFile();
         //Generate a new filename for the logfile adding timestamp to it and then rename the file.
         QString newlogFileName = QString("%1_%2_%3.%4")
            .arg(logFileName)
            .arg(QDate::currentDate().toString("yyyy_MM_dd"))
            .arg(QTime::currentTime().toString("hh_mm_ss_zzz"))
            .arg(logFileNameSuffix);

         if ( ! logFile.rename(logFilePath.filePath(logFileFullName()), logFilePath.filePath(newlogFileName)) )
         {
            qCritical() << "Could not rename the log file";
         }
      }
      //Recreating the log file again with the name specified in logFileName variable.
      // Test default location
      logFile.setFileName(logFilePath.filePath(logFileFullName()));
      if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
         stream = new QTextStream(&logFile);
         return true;
      }

      // Defaults to temporary
      logFile.setFileName(QDir::temp().filePath(logFileFullName()));
      if (logFile.open(QFile::WriteOnly | QFile::Truncate)) {
         logFile.setPermissions(QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser);
         stream = new QTextStream(&logFile);
         qWarning() << QString("Log is in a temporary directory: %1").arg(logFile.fileName());
         return true;
      }
      return false;
   }

   void closeLogFile()
   {
      //Close and reset the stream if it is set.
      if (stream)
      {
         delete stream;
         stream = nullptr;
      }
      //Close the file if it's open.
      if (logFile.isOpen())
         logFile.close();
   }

   void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
   {
      /*
      * First things first! do the user want to save logs, and if so, to what level.
      * then, if the file-stream is open and the log file size is to big, we need to prune the old logs and initiate a new logfile.
      * after that we're all set, Log away!
      */

      //Check if we're actually want to log and that we're set to log this level, this is set by the user options.
      if( ! loggingEnabled || ! (qtLogLevelTranslateEnum[type] <= logLevel) )
         return;

      /* Check if there is a file actually set yet, in a rare case if the logfile was not created at initialization.
      * then we won't be logging to a file, the location may not yet have been loaded from the settings, thus only logging to the stderr.
      * I this case we cannot do any of the pruning or filename generation.
      */
      if (stream)
      {
         if (logFile.size() >= logFileSize)
         {
            pruneLogFiles();
            initLogFileName();
         }
      }

      //Writing the actual log.
      doLog(qtLogLevelTranslateEnum[type], QString("%1, in %2").arg(message).arg(context.line));
   } // End logMessageHandler

   void pruneLogFiles()
   {
      QMutexLocker locker(&mutex);
      //Need to close and reset the stream before deleting any files.
      closeLogFile();

      //if the logfile is closed and we're in testing mode where stderr is disabled, we need to enable is temporarily.
      //saving old value to reset to after pruning.
      bool old_isLoggingToStderr = isLoggingToStderr;
      isLoggingToStderr = true;

      //Get the list of log files.
      QFileInfoList fileList = getLogFileList();
      if (fileList.size() > logFileCount)
      {
         for (int i = 0; i < (fileList.size() - logFileCount); i++)
         {
            QFile f(QString(fileList.at(i).canonicalFilePath()));
            f.remove();
         }
      }
      isLoggingToStderr = old_isLoggingToStderr;
   } // function pruneLogFiles

   QFileInfoList getLogFileList()
   {
      //testing the number of logfiles that exist under the logging directory, and also checking that neither is bigger than the specified size restriction.
      QDir dir;
      QStringList filters;
      filters << QString("%1*.%2").arg(logFileName).arg(logFileNameSuffix);

      //configuring the file filters to only remove the log files as the directory also contains the database.
      dir.setSorting(QDir::Reversed | QDir::Time);
      dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
      dir.setPath(Log::logFilePath.canonicalPath());
      dir.setNameFilters(filters);
      return dir.entryInfoList();
   } // getLogFileList
}
