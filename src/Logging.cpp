/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Logging.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "Logging.h"

#include <sstream>      // For std::ostringstream

#include <boost/stacktrace.hpp>

#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>
#include <QTime>

#include "config.h"
#include "PersistentSettings.h"

// Qt has changed how you do endl in writing to a QTextStream
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
#define END_OF_LINE endl
#else
#define END_OF_LINE Qt::endl
#endif

//
// Anonymous namespace for constants, global variables and functions used only in this file
//
namespace {

   Logging::Level currentLoggingLevel = Logging::LogLevel_INFO;

   // We decompose the log filename into its body and suffix for log rotation
   // The _current_ log file is always "[applicaiton name].log"
   static QString const logFilename = QString{CONFIG_APPLICATION_NAME_LC};
   static QString const logFilenameExtension{"log"};

   // Stores the path to the log files
   QDir logDirectory;

   // Time format to use in log messages
   QString const timeFormat{"hh:mm:ss.zzz"};

   QFile logFile;
   QMutex mutex;

   // This global flag controls whether, in general, we are logging to stderr or not.  Usually it's turned off for at
   // least the part of automated testing where we're generating lots of test logging.
   bool isLoggingToStderr{true};

   QTextStream errStream{stderr};
   QTextStream * stream;

   //
   // It's useful to include the thread ID in log messages.  We don't care what the actual ID is, we just need to be
   // able to differentiate between log messages from different threads.  Qt gives us thread ID as void *, which we
   // turn into a base-36 string.  (This is the most concise encoding that we can trivially get from QString.)
   //
   // We only need to create the string representation of a thread ID once per thread.  Since C++11, we can use
   // thread_local to define thread-specific variables that are initialized "before first use"
   //
   thread_local QString const threadId{QString{"%1"}.arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 36)};

   //
   // Although we can turn off logging to stderr (eg for running test cases), we want to force it to be enabled for
   // logging errors about logging.  The combination of this thread-local variable and little RAII class enable you to
   // create an object that will force stderr logging to be enabled for the current thread for the duration of the
   // object's life (typically the duration of a function).
   //
   thread_local bool forceStderrLogging{false};
   class TemporarilyForceStderrLogging {
   public:
      TemporarilyForceStderrLogging()  {
         this->savedState = forceStderrLogging;
         forceStderrLogging = true;
         return;
      }
      ~TemporarilyForceStderrLogging() {
         forceStderrLogging = this->savedState;
         return;
      }
   private:
      // We need to save state because one function that has created one of these objects might call another that does
      // the same
      bool savedState;
   };

   //
   // We use the Qt functions (qDebug(), qInfo(), etc) do our logging but we need to convert from QtMsgType to our own
   // logging level for two reasons:
   //  * we don't differentiate between QtCriticalMsg and QtFatalMsg (both count as LogLevel_ERROR for us)
   //  * QtMsgType ends up putting things in a funny order (because they added QtInfoMsg later than the other levels)
   //
   Logging::Level levelFromQtMsgType(QtMsgType const qtMsgType) {
      switch (qtMsgType) {
         case QtDebugMsg    : return Logging::LogLevel_DEBUG;
         case QtInfoMsg     : return Logging::LogLevel_INFO;
         case QtWarningMsg  : return Logging::LogLevel_WARNING;
         case QtCriticalMsg : return Logging::LogLevel_ERROR;
         case QtFatalMsg    : return Logging::LogLevel_ERROR;
         default: Q_ASSERT(false && "Unknown QtMsgType"); return Logging::LogLevel_ERROR;
      }
   }

   //
   // This is what actually outputs a message to the log file and/or std::cerr
   //
   void doLog(const Logging::Level level, const QString message) {
      QString logEntry = QString{"[%1] (%2) %3 : %4"}.arg(QTime::currentTime().toString(timeFormat))
                                                     .arg(threadId)
                                                     .arg(Logging::getStringFromLogLevel(level))
                                                     .arg(message);
      QMutexLocker locker(&mutex);
      if (isLoggingToStderr ||
          forceStderrLogging) { errStream << logEntry << END_OF_LINE; }
      if (stream)             {   *stream << logEntry << END_OF_LINE; }
      return;
   }

   /**
    * \brief Generates a log file name
    */
   QString logFileFullName() {
      return QString("%1.%2").arg(logFilename).arg(logFilenameExtension);
   }

   /**
    * \brief Closes the log file stream and the file handle.
    */
   void closeLogFile() {
      // Close and reset the stream if it is set.
      if (stream) {
         delete stream;
         stream = nullptr;
      }

      // Close the file if it's open.
      if (logFile.isOpen()) {
         logFile.close();
      }
      return;
   }

   /**
    * \brief If a log file is too big or otherwise in the way†, we want to rename it in some way that's likely to be
    *        unique.  Adding a fine-grained timestamp seems to fit the bill.
    *
    *        † Specifically, the "otherwise in the way" case is when we are changing logging directories and we have a
    *        brewtarget.log file in both the old and the new directories.  We want to move the log file from the old to
    *        the new directory, but we don't want to blat the file in the new directory.
    *
    *        NB it is the caller's responsibility to ensure files are closed, mutex held, etc.
    *
    * \param dir The directory in which to do the renaming - usually the current or about-to-be-set logging directory
    */
   bool renameLogFileWithTimestamp(QDir & dir) {
      //Generate a new filename for the logfile adding timestamp to it and then rename the file.
      QString newlogFilename = QString("%1_%2_%3.%4")
         .arg(logFilename)
         .arg(QDate::currentDate().toString("yyyy_MM_dd"))
         .arg(QTime::currentTime().toString("hh_mm_ss_zzz"))
         .arg(logFilenameExtension);
      return dir.rename(logFileFullName(), newlogFilename);
   }

   /**
    * \brief initializes the log file and opens the stream for writing.
    *        This was moved to its own function as this has to be called every time logs are being pruned.
    */
   bool openLogFile() {
      // We _really_ need to see problems with opening the log file on stderr!
      TemporarilyForceStderrLogging temporarilyForceStderrLogging;

      // First check if it's time to rotate the log file
      if (logFile.size() > Logging::logFileSize) {
         // Acquire lock due to the file mangling below.  NB: This means we do not want to use Qt logging in this
         // block, as we'd end up attempting to acquire the same mutex in the doLog() function above!  So, any errors
         // between here and the closing brace need to go to stderr
         QMutexLocker locker(&mutex);
         // Double check that the stream is not initiated, if so, kill it.
         closeLogFile();
         if (!renameLogFileWithTimestamp(logDirectory)) {
            errStream <<
               "Could not rename the log file " << logFileFullName() << " in directory " <<
               logDirectory.absolutePath() << END_OF_LINE;
         }
      }

      // Recreate/reopen the log file
      // Test default location
      logFile.setFileName(logDirectory.filePath(logFileFullName()));
      if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
         stream = new QTextStream(&logFile);
         qInfo() << Q_FUNC_INFO << "Logging to file" << QFileInfo(logFile).canonicalFilePath();
         return true;
      }

      qInfo() <<
         Q_FUNC_INFO << "Could not open log file" << QFileInfo(logFile).canonicalFilePath() <<
         "for writing.  Will try using temporary directory";

      // Defaults to temporary
      logFile.setFileName(QDir::temp().filePath(logFileFullName()));
      if (logFile.open(QFile::WriteOnly | QFile::Truncate)) {
         logFile.setPermissions(QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser);
         stream = new QTextStream(&logFile);
         qWarning() <<
            Q_FUNC_INFO << "Log file is in a temporary directory: " << QFileInfo(logFile).canonicalFilePath();
         return true;
      }

      qCritical() << Q_FUNC_INFO << "Unable to open" << QFileInfo(logFile).canonicalFilePath();

      return false;
   }

   /**
    * \brief Prunes old log files from the directory, keeping only the specified number of files in logFileCount,
    *        Purpose is to keep log files to a mininum while keeping the logs up-to-date and also not require manual
    *        pruning of files.
    */
   void pruneLogFiles() {
      QMutexLocker locker(&mutex);
      // Need to close and reset the stream before deleting any files.
      closeLogFile();

      // If the logfile is closed and we're in testing mode where stderr is disabled, we need to enable it temporarily.
      // saving old value to reset to after pruning.
      TemporarilyForceStderrLogging temporarilyForceStderrLogging;

      //Get the list of log files.
      QFileInfoList fileList = Logging::getLogFileList();
      if (fileList.size() > Logging::logFileCount)
      {
         for (int i = 0; i < (fileList.size() - Logging::logFileCount); i++)
         {
            QFile f(QString(fileList.at(i).canonicalFilePath()));
            f.remove();
         }
      }
      return;
   }

   /**
    * \brief Handles all log messages, which should be logged using the standard Qt functions, eg:
    *        qDebug() << "message" << some_variable; //for a debug message!
    */
   void logMessageHandler(QtMsgType qtMsgType, QMessageLogContext const & context, QString const & message) {
      Logging::Level logLevelOfMessage = levelFromQtMsgType(qtMsgType);
      //
      // First things first!  What logging level has the user chosen.
      // then, if the file-stream is open and the log file size is to big, we need to prune the old logs and initiate a new logfile.
      // after that we're all set, Log away!
      //

      // Check that we're set to log this level, this is set by the user options.
      if (logLevelOfMessage < currentLoggingLevel) {
         return;
      }

      // Check if there is a file actually set yet.  In a rare case if the logfile was not created at initialization,
      // then we won't be logging to a file, the location may not yet have been loaded from the settings, thus only logging to the stderr.
      // In this case we cannot do any of the pruning or filename generation.
      if (stream) {
         if (logFile.size() >= Logging::logFileSize) {
            pruneLogFiles();
            openLogFile();
         }
      }

      // Writing the actual log
      //
      // QMessageLogContext members are a bit hard to find in Qt documentation so noted here:
      //    category : const char *
      //    file : const char *      -- full path of the source file
      //    function : const char *  -- same as what gets written out by Q_FUNC_INFO
      //    line : int
      //    version : int
      //
      // We don't want to log the full path of the source file, because that might contain private info about the
      // directory structure on the machine on which the build was done.  We could just show the filename with:
      //    QString sourceFile = QFileInfo(context.file).fileName();
      // But we'd like to show the relative path under the src directory (eg database/Database.cpp rather than just
      // Database.cpp).  (The code here assumes there will not be any subdirectory of src that is also called src,
      // which seems pretty reasonable.)
      QString sourceFile = QString{context.file}.split("/src/").last();
      doLog(logLevelOfMessage, QString("%1  [%2:%3]").arg(message).arg(sourceFile).arg(context.line));
      return;
   }

}


QVector<Logging::LevelDetail> const Logging::levelDetails{
   { Logging::LogLevel_DEBUG,   "DEBUG",   QObject::tr("Detailed (for debugging)")},
   { Logging::LogLevel_INFO,    "INFO",    QObject::tr("Normal")},
   { Logging::LogLevel_WARNING, "WARNING", QObject::tr("Warnings and Errors only")},
   { Logging::LogLevel_ERROR,   "ERROR",   QObject::tr("Errors only")}
};

QString Logging::getStringFromLogLevel(const Logging::Level level) {
   auto match = std::find_if(Logging::levelDetails.begin(),
                              Logging::levelDetails.end(),
                              [level](LevelDetail ld) {return ld.level == level;});
   // It's a coding error if we couldn't find the level
   Q_ASSERT(match != Logging::levelDetails.end());

   return QString(match->name);
}

Logging::Level Logging::getLogLevelFromString(QString const name) {
   auto match = std::find_if(Logging::levelDetails.begin(),
                              Logging::levelDetails.end(),
                              [name](LevelDetail ld) {return ld.name == name;});
   // It's a coding error if we couldn't find the level
   Q_ASSERT(match != Logging::levelDetails.end());

   return match->level;
}

Logging::Level Logging::getLogLevel() {
   return currentLoggingLevel;
}

void Logging::setLogLevel(Level newLevel) {
   currentLoggingLevel = newLevel;
   PersistentSettings::insert(PersistentSettings::Names::LoggingLevel, Logging::getStringFromLogLevel(currentLoggingLevel));
   return;
}

bool Logging::getLogInConfigDir() {
   return PersistentSettings::getConfigDir().canonicalPath() == logDirectory.canonicalPath();
}

namespace Logging {

   // .:TODO:. Make these configurable by the end user in OptionDialog
   // Set the log file size for the rotation.
   int const logFileSize = 500 * 1024;
   // set the number of files to keep when rotating.
   int const logFileCount = 5;

}

bool Logging::initializeLogging() {
   // We _really_ need to see problems with opening the log file on stderr!
   TemporarilyForceStderrLogging temporarilyForceStderrLogging;

   currentLoggingLevel = Logging::getLogLevelFromString(PersistentSettings::value(PersistentSettings::Names::LoggingLevel, "INFO").toString());
   Logging::setDirectory(
      PersistentSettings::contains(PersistentSettings::Names::LogDirectory) ?
         std::optional<QDir>(PersistentSettings::value(PersistentSettings::Names::LogDirectory).toString()) : std::optional<QDir>(std::nullopt)
   );

   qInstallMessageHandler(logMessageHandler);
   qDebug() << Q_FUNC_INFO << "Logging initialized.  Logs will be written to" << logDirectory.absolutePath();

   // It's quite useful on debug builds to check that stack trace logging is working, rather than to find out it's not
   // when you need the info to fix another bug.
   qDebug().noquote() << Q_FUNC_INFO << "Check that stacktraces are working:" << Logging::getStackTrace();

   return true;
}

void Logging::setLoggingToStderr(bool const enabled) {
   if (enabled != isLoggingToStderr) {
      qInfo() << Q_FUNC_INFO << (enabled ? "Enabling" : "Suspending") << "logging to stderr";
      isLoggingToStderr = enabled;
   }
   return;
}

bool Logging::setDirectory(std::optional<QDir> newDirectory, Logging::PersistNewDirectory const persistNewDirectory) {
   qDebug() << Q_FUNC_INFO;

   QDir oldDirectory = logDirectory;

   // Supplying no directory in the parameter means use the default location, ie the config directory
   if (newDirectory.has_value()) {
      logDirectory = *newDirectory;
      qDebug() << Q_FUNC_INFO << "Logging to specified directory: " << logDirectory.absolutePath();
   } else {
      logDirectory = PersistentSettings::getConfigDir();
      qDebug() << Q_FUNC_INFO << "Logging to configuration directory: " << logDirectory.absolutePath();
   }

   // We assert that, one way or another, we have above set the log directory to something.
   // Note that we must use absolutePath here.  It can be valid for canonicalPath() to return empty string -- if log dir
   // is current dir.
   Q_ASSERT(!logDirectory.absolutePath().isEmpty());

   // Check if the new directory exists, if not create it.
   QString errorReason;
   if (!logDirectory.exists()) {
      qDebug() << Q_FUNC_INFO << logDirectory.absolutePath() << "does not exist, creating";
      if (!logDirectory.mkpath(logDirectory.absolutePath())) {
         errorReason = QObject::tr("Could not create new log file directory");
      }
   }

   // Check the new directory is usable
   if (errorReason.isEmpty()) {
      if (!logDirectory.isReadable()) {
         errorReason = QObject::tr("Could not read new log file directory");
      } else if (!logDirectory.isReadable()) {
         errorReason = QObject::tr("Could not write to new log file directory");
      }
   }

   if (!errorReason.isEmpty()) {
      qCritical() <<
         errorReason << logDirectory.absolutePath() << QObject::tr(" reverting to ") <<
         oldDirectory.absolutePath();
      logDirectory = oldDirectory;
      return false;
   }

   // At this point, enough has succeeded that we're OK to commit to using the new directory
   if (persistNewDirectory == Logging::NewDirectoryIsPermanent) {
      PersistentSettings::insert(PersistentSettings::Names::LogDirectory, logDirectory.absolutePath());
   }

   //
   // If we are already writing to a log file in the old directory, it needs to be closed and moved to the new one
   //
   // NB: This only moves the current Logfile, the older ones will be left behind.
   //
   if (stream && logDirectory.canonicalPath() != oldDirectory.canonicalPath()) {

      // NB Don't try to log inside this if statement.  We are moving the log file!  Errors need to go to stderr.
      QMutexLocker locker(&mutex);

      // Close the file if open and reset the stream.
      closeLogFile();

      //
      // Attempt to move existing log file to the new directory, making some attempt to avoid overwriting any existing
      // file of the same name (by moving/renaming it to have a .bak extension).
      //
      // Note however that some of this file moving/renaming could still fail for a couple of reasons:
      //    - If we try to move/rename a file to overwrite a file that already exists (eg if the .bak file also already
      //      exists) then, on some operating systems (eg Windows), the move will fail and, on others (eg Linux), it
      //      will succeed (with the clashing file getting overwritten).
      //    - On some operating systems, you can't move from one file system to another (eg on Windows from C: drive to
      //      D: drive)
      //
      // If things go wrong we can't really write a message to the log file(!) but we can emit something to stderr
      //
      // The first check is whether there's anything to move!
      //
      QString fileName = logFileFullName();
      if (oldDirectory.exists(fileName)) {
         //
         // Make a reasonable effort to move out the way anything we might otherwise be about to stomp on
         //
         if (logDirectory.exists(fileName)) {
            if (!renameLogFileWithTimestamp(logDirectory)) {
               errStream <<
                  Q_FUNC_INFO << "Unable to rename " << fileName << " in directory " << logDirectory.absolutePath() <<
                  END_OF_LINE;
               return false;
            }
         }
         if (!logFile.rename(logDirectory.filePath(fileName))) {
            errStream <<
               Q_FUNC_INFO << "Unable to move " << fileName << " from " << oldDirectory.absolutePath() << " to " <<
               logDirectory.absolutePath() << END_OF_LINE;
            return false;
         }
      }
   }

   // Now make sure the log file in the new directory is open for writing
   if (!openLogFile()) {
      qWarning() << Q_FUNC_INFO << QString("Could not open/create a log file");
      return false;
   }

   return true;
}

QDir Logging::getDirectory() {
   return logDirectory;
}


QFileInfoList Logging::getLogFileList() {
   QStringList filters;
   filters << QString("%1*.%2").arg(logFilename).arg(logFilenameExtension);

   //configuring the file filters to only remove the log files as the directory also contains the database.
   QDir dir;
   dir.setSorting(QDir::Reversed | QDir::Time);
   dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
   dir.setPath(logDirectory.canonicalPath());
   dir.setNameFilters(filters);
   return dir.entryInfoList();
}


void Logging::terminateLogging() {
   QMutexLocker locker(&mutex);
   closeLogFile();
   return;
}

QString Logging::getStackTrace() {
   //
   // TBD: Once all our compilers have full C++23 support, we should look at switching to <stacktrace> in the standard
   //      library.
   //
   std::ostringstream stacktrace;
   stacktrace << boost::stacktrace::stacktrace();
   QString returnValue;
   QTextStream returnValueAsStream(&returnValue);
   returnValueAsStream << "\nStacktrace:\n" << QString::fromStdString(stacktrace.str());
   return returnValue;
}
