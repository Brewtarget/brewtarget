/*
 * database/Database.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2021:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Chris Pavetto <chrispavetto@gmail.com>
 *   • Chris Speck <cgspeck@gmail.com>
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • David Grundberg <individ@acc.umu.se>
 *   • Greg Greenaae <ggreenaae@gmail.com>
 *   • Jamie Daws <jdelectronics1@gmail.com>
 *   • Jean-Baptiste Wons <wonsjb@gmail.com>
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Luke Vincent <luke.r.vincent@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#include "database/Database.h"

#include <mutex>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QString>
#include <QThread>

#include "brewtarget.h"
#include "config.h"
#include "database/DatabaseSchemaHelper.h"
#include "PersistentSettings.h"


namespace {

   //
   // Constants for DB native type names etc
   //
   struct DbNativeVariants {
      char const * const sqliteName;
      char const * const postgresqlName;
      // GCC will let you get away without it, but some C++ compilers are more strict about the need for a non-default
      // constructor when you have const members in a struct
      DbNativeVariants(char const * const sqliteName = nullptr, char const * const postgresqlName = nullptr) :
         sqliteName{sqliteName},
         postgresqlName{postgresqlName} {
         return;
      }
   };

   //
   // SQLite actually lets you store any type in any column, and only offers five "affinities" for "the recommended
   // type for data stored in a column".   There are no special types for boolean or date.  However, it also allows you
   // to use traditional SQL type names in create table statements etc (and does the mapping down to the affinities
   // under the hood) and retains those names when you're browsing the database   We therefore use those traditional
   // SQL typenames here as it makes the intent clearer for anyone looking directly at the database.
   //
   // PostgreSQL is the other extreme and has all sorts of specialised types (including for networking addresses,
   // geometric shapes and XML).  We need only a small subset of these.
   //
   template<typename T> DbNativeVariants const nativeTypeNames;
   //                                                                SQLite    PostgreSQL
   template<> DbNativeVariants const nativeTypeNames<bool>         {"BOOLEAN", "BOOLEAN"         };
   template<> DbNativeVariants const nativeTypeNames<int>          {"INTEGER", "INTEGER"         };
   template<> DbNativeVariants const nativeTypeNames<unsigned int> {"INTEGER", "INTEGER"         };
   template<> DbNativeVariants const nativeTypeNames<double>       {"DOUBLE",  "DOUBLE PRECISION"};
   template<> DbNativeVariants const nativeTypeNames<QString>      {"TEXT",    "TEXT"            };
   template<> DbNativeVariants const nativeTypeNames<QDate>        {"DATE",    "DATE"            };

   // Note that, per https://www.sqlite.org/autoinc.html, SQLite explicitly recommends against using AUTOINCREMENT for
   // integer primary keys, as specifying "PRIMARY KEY" alone will result in automatic generation of primary keys with
   // less overhead.
   DbNativeVariants const nativeIntPrimaryKeyModifier {
      "PRIMARY KEY",        // SQLite
      "SERIAL PRIMARY KEY"  // PostgreSQL
   };

   DbNativeVariants const sqlToAddColumnAsForeignKey {
      "ALTER TABLE %1 ADD COLUMN %2 INTEGER REFERENCES %3(%4);", // SQLite
      "ALTER TABLE %1 ADD COLUMN %2 INTEGER REFERENCES %3(%4);"  // PostgreSQL
      // MySQL would be ALTER TABLE %1 ADD COLUMN %2 int, FOREIGN KEY (%2) REFERENCES %3(%4)
   };

   char const * getDbNativeName(DbNativeVariants const & dbNativeVariants, Database::DbType dbType) {
      switch (dbType) {
         case Database::SQLITE: return dbNativeVariants.sqliteName;
         case Database::PGSQL:  return dbNativeVariants.postgresqlName;
         default:
            // It's a coding error if we get here
            qCritical() << Q_FUNC_INFO << "Unrecognised DB type:" << dbType;
            Q_ASSERT(false);
            break;
      }
      return "NotSupported";
   }

   //
   // Variables
   //

   // These are for SQLite databases
   QFile dbFile;
   QString dbFileName;
   QFile dataDbFile;
   QString dataDbFileName;

   // And these are for Postgres databases -- are these really required? Are
   // the sqlite ones really required?
   QString dbHostname;
   int dbPortnum;
   QString dbName;
   QString dbSchema;
   QString dbUsername;
   QString dbPassword;

   //
   // Each thread has its own connection to the database, and each connection has to have a unique name (otherwise,
   // calling QSqlDatabase::addDatabase() with the same name as an existing connection will replace that existing
   // connection with the new one created by that function).  We just create a unique connection name from the thread
   // ID in the same way that we do in the Logging module.
   //
   // We only need to store the name of the connection here.  (See header file comment for Database::sqlDatabase() for
   // more details of why it would be unhelpful to store a QSqlDatabase object in thread-local storage.)
   //
   // Since C++11, we can use thread_local to define thread-specific variables that are initialized "before first use"
   //
   thread_local QString const dbConnectionNameForThisThread {
      QString{"%1"}.arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 36)
   };

   // May St. Stevens intercede on my behalf.
   //
   //! \brief opens an SQLite db for transfer
   QSqlDatabase openSQLite() {
      QString filePath = PersistentSettings::getUserDataDir().filePath("database.sqlite");
      QSqlDatabase newConnection = QSqlDatabase::addDatabase("QSQLITE", "altdb");

      try {
         dbFile.setFileName(dbFileName);

         if ( filePath.isEmpty() )
            throw QString("Could not read the database file(%1)").arg(filePath);

         newConnection.setDatabaseName(filePath);

         if (!  newConnection.open() )
            throw QString("Could not open %1 : %2").arg(filePath).arg(newConnection.lastError().text());
      }
      catch (QString e) {
         qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
         throw;
      }

      return newConnection;
   }

   //! \brief opens a PostgreSQL db for transfer. I need
   QSqlDatabase openPostgres(QString const& Hostname, QString const& DbName,
                             QString const& Username, QString const& Password,
                             int Portnum) {
      QSqlDatabase newConnection = QSqlDatabase::addDatabase("QPSQL", "altdb");

      try {
         newConnection.setHostName(Hostname);
         newConnection.setDatabaseName(DbName);
         newConnection.setUserName(Username);
         newConnection.setPort(Portnum);
         newConnection.setPassword(Password);

         if ( ! newConnection.open() )
            throw QString("Could not open %1 : %2").arg(Hostname).arg(newConnection.lastError().text());
      }
      catch (QString e) {
         qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
         throw;
      }
      return newConnection;
   }

}

//
// This private implementation class holds all private non-virtual members of Database
//
class Database::impl {
public:

   /**
    * Constructor
    */
   impl(Database::DbType dbType) : dbType{dbType},
                                   dbConName{},
                                   loaded{false},
                                   loadWasSuccessful{false},
                                   mutex{} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // Don't know where to put this, so it goes here for right now
   bool loadSQLite(Database & database) {
      qDebug() << "Loading SQLITE...";

      // Set file names.
      dbFileName = PersistentSettings::getUserDataDir().filePath("database.sqlite");
      dataDbFileName = Brewtarget::getResourceDir().filePath("default_db.sqlite");
      qDebug() << Q_FUNC_INFO << QString("dbFileName = \"%1\"\nDatabase::loadSQLite() - dataDbFileName=\"%2\"").arg(dbFileName).arg(dataDbFileName);
      // Set the files.
      dbFile.setFileName(dbFileName);
      dataDbFile.setFileName(dataDbFileName);

      // If user restored the database from a backup, make the backup into the primary.
      {
         QFile newdb(QString("%1.new").arg(dbFileName));
         if( newdb.exists() )
         {
            dbFile.remove();
            newdb.copy(dbFileName);
            QFile::setPermissions( dbFileName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );
            newdb.remove();
         }
      }

      // If there's no dbFile, try to copy from dataDbFile.
      if( !dbFile.exists() )
      {
         Brewtarget::userDatabaseDidNotExist = true;

         // Have to wait until db is open before creating from scratch.
         if (dataDbFile.exists()) {
            dataDbFile.copy(dbFileName);
            QFile::setPermissions( dbFileName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );
         }

         // Reset the last merge request.
         Database::lastDbMergeRequest = QDateTime::currentDateTime();
      }

      // Open SQLite DB
      // It's a coding error if we didn't already establish that SQLite is the type of DB we're talking to, so assert
      // that and then call the generic code to get a connection
      Q_ASSERT(this->dbType == Database::SQLITE);
      QSqlDatabase connection = database.sqlDatabase();

      this->dbConName = connection.connectionName();
      qDebug() << Q_FUNC_INFO << "dbConName=" << this->dbConName;

      //
      // It's quite useful to record the DB version in the logs
      //
      QSqlQuery sqlQuery(connection);
      QString queryString{"SELECT sqlite_version() AS version;"};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec() || !sqlQuery.next()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return false;
      }
      QVariant fieldValue = sqlQuery.value("version");
      qInfo() << Q_FUNC_INFO << "SQLite version" << fieldValue;

      // NOTE: synchronous=off reduces query time by an order of magnitude!
      QSqlQuery pragma(connection);
      if ( ! pragma.exec( "PRAGMA synchronous = off" ) ) {
         qCritical() << Q_FUNC_INFO << "Could not disable synchronous writes: " << pragma.lastError().text();
         return false;
      }
      if ( ! pragma.exec( "PRAGMA foreign_keys = on")) {
         qCritical() << Q_FUNC_INFO << "Could not enable foreign keys: " << pragma.lastError().text();
         return false;
      }
      if ( ! pragma.exec( "PRAGMA locking_mode = EXCLUSIVE")) {
         qCritical() << Q_FUNC_INFO << "Could not enable exclusive locks: " << pragma.lastError().text();
         return false;
      }
      if ( ! pragma.exec("PRAGMA temp_store = MEMORY") ) {
         qCritical() << Q_FUNC_INFO << "Could not enable temporary memory: " << pragma.lastError().text();
         return false;
      }

      // older sqlite databases may not have a settings table. I think I will
      // just check to see if anything is in there.
      this->createFromScratch = connection.tables().size() == 0;

      return true;
   }

   bool loadPgSQL(Database & database) {

      dbHostname = PersistentSettings::value("dbHostname").toString();
      dbPortnum  = PersistentSettings::value("dbPortnum").toInt();
      dbName     = PersistentSettings::value("dbName").toString();
      dbSchema   = PersistentSettings::value("dbSchema").toString();

      dbUsername = PersistentSettings::value("dbUsername").toString();

      if ( PersistentSettings::contains("dbPassword") ) {
         dbPassword = PersistentSettings::value("dbPassword").toString();
      }
      else {
         bool isOk = false;

         // prompt for the password until we get it? I don't think this is a good
         // idea?
         while ( ! isOk ) {
            dbPassword = QInputDialog::getText(nullptr,tr("Database password"),
                  tr("Password"), QLineEdit::Password,QString(),&isOk);
            if ( isOk ) {
               isOk = verifyDbConnection( Database::PGSQL, dbHostname, dbPortnum, dbSchema,
                                    dbName, dbUsername, dbPassword);
            }
         }
      }

      // It's a coding error if we didn't already establish that PostgreSQL is the type of DB we're talking to, so assert
      // that and then call the generic code to get a connection
      Q_ASSERT(this->dbType == Database::PGSQL);
      QSqlDatabase connection = database.sqlDatabase();

      this->dbConName = connection.connectionName();
      qDebug() << Q_FUNC_INFO << "dbConName=" << this->dbConName;

      //
      // It's quite useful to record the DB version in the logs
      //
      QSqlQuery sqlQuery(connection);
      QString queryString{"SELECT version() AS version;"};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec() || !sqlQuery.next()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return false;
      }
      QVariant fieldValue = sqlQuery.value("version");
      qInfo() << Q_FUNC_INFO << "PostgreSQL version" << fieldValue;

      // by the time we had pgsql support, there is a settings table
      this->createFromScratch = ! connection.tables().contains("settings");

      return true;
   }

   // Returns true if the schema gets updated, false otherwise.
   // If err != 0, set it to true if an error occurs, false otherwise.
   bool updateSchema(Database & database, bool* err = nullptr) {
      int currentVersion = DatabaseSchemaHelper::currentVersion( database.sqlDatabase() );
      int newVersion = DatabaseSchemaHelper::dbVersion;
      bool doUpdate = currentVersion < newVersion;

      if (doUpdate) {
         bool success = DatabaseSchemaHelper::migrate(database, currentVersion, newVersion, database.sqlDatabase() );
         if (!success) {
            qCritical() << Q_FUNC_INFO << QString("Database migration %1->%2 failed").arg(currentVersion).arg(newVersion);
            if (err) {
               *err = true;
            }
            return false;
         }
      }

      return doUpdate;
   }


   void automaticBackup() {
      int count = PersistentSettings::value("count",0,"backups").toInt() + 1;
      int frequency = PersistentSettings::value("frequency",4,"backups").toInt();
      int maxBackups = PersistentSettings::value("maximum",10,"backups").toInt();

      // The most common case is update the counter and nothing else
      // A frequency of 1 means backup every time. Which this statisfies
      if ( count % frequency != 0 ) {
         PersistentSettings::insert( "count", count, "backups");
         return;
      }

      // If the user has selected 0 max backups, we just return. There's a weird
      // case where they have a frequency of 1 and a maxBackup of 0. In that
      // case, maxBackup wins
      if ( maxBackups == 0 ) {
         return;
      }

      QString backupDir = PersistentSettings::value("directory", PersistentSettings::getUserDataDir().canonicalPath(), "backups").toString();
      QString listOfFiles = PersistentSettings::value("files", QVariant(), "backups").toString();
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
      QStringList fileNames = listOfFiles.split(",", QString::SkipEmptyParts);
#else
      QStringList fileNames = listOfFiles.split(",", Qt::SkipEmptyParts);
#endif

      QString halfName = QString("%1.%2").arg("databaseBackup").arg(QDate::currentDate().toString("yyyyMMdd"));
      QString newName = halfName;
      // Unique filenames are a pain in the ass. In the case you open Brewtarget
      // twice in a day, this loop makes sure we don't over write (or delete) the
      // wrong thing
      int foobar = 0;
      while ( foobar < 10000 && QFile::exists( backupDir + "/" + newName ) ) {
         foobar++;
         newName = QString("%1_%2").arg(halfName).arg(foobar,4,10,QChar('0'));
         if ( foobar > 9999 ) {
            qWarning() << QString("%1 : could not find a unique name in 10000 tries. Overwriting %2").arg(Q_FUNC_INFO).arg(halfName);
            newName = halfName;
         }
      }
      // backup the file first
      backupToDir(backupDir,newName);

      // If we have maxBackups == -1, it means never clean. It also means we
      // don't track the filenames.
      if ( maxBackups == -1 )  {
         PersistentSettings::remove("files", "backups");
         return;
      }

      fileNames.append(newName);

      // If we have too many backups. This is in a while loop because we need to
      // handle the case where a user decides they only want 4 backups, not 10.
      // The while loop will clean that up properly.
      while ( fileNames.size() > maxBackups ) {
         // takeFirst() removes the file from the list, which is important
         QString victim = backupDir + "/" + fileNames.takeFirst();
         QFile *file = new QFile(victim);
         QFileInfo *fileThing = new QFileInfo(victim);

         // Make sure it exists, and make sure it is a file before we
         // try remove it
         if ( fileThing->exists() && fileThing->isFile() ) {
            qInfo() <<
               Q_FUNC_INFO << "Removing oldest database backup file," << victim << "as more than" << maxBackups <<
               "files in" << backupDir;
            // If we can't remove it, give a warning.
            if (! file->remove() ) {
               qWarning() <<
                  Q_FUNC_INFO << "Could not remove old database backup file " << victim << ".  Error:" << file->error();
            }
         }
      }

      // re-encode the list
      listOfFiles = fileNames.join(",");

      // finally, reset the counter and save the new list of files
      PersistentSettings::insert("count", 0, "backups");
      PersistentSettings::insert("files", listOfFiles, "backups");
   }

   Database::DbType dbType;
   QString dbConName;

   bool loaded;

   // Instance variables.
   bool loadWasSuccessful;
   bool createFromScratch;
   bool schemaUpdated;

   // Used for locking member functions that must be single-threaded
   QMutex mutex;
};


Database::Database(Database::DbType dbType) : pimpl{ new impl{dbType} } {
   return;
}

Database::~Database() {
   // Don't try and log in this function as it's called pretty close to the program exiting, at the end of main(), at
   // which point the objects used by the logging module may be in a weird state.

   // If we have not explicitly unloaded, do so now and discard changes.
   if (this->pimpl->loaded) {
      this->unload();
   }

   return;
}


QSqlDatabase Database::sqlDatabase() const {
   // Need a unique database connection for each thread.
   //http://www.linuxjournal.com/article/9602

   //
   // If we already created a valid DB connection for this thread, this call will get it, and we can just return it to
   // the caller.  Otherwise, we'll just get an invalid connection.
   //
   Q_ASSERT(!dbConnectionNameForThisThread.isEmpty());
   QSqlDatabase connection = QSqlDatabase::database(dbConnectionNameForThisThread);
   if (connection.isValid()) {
      qDebug() << Q_FUNC_INFO << "Returning connection " << dbConnectionNameForThisThread;
      return connection;
   }

   //
   // Create a new connection in Qt's register of connections.  (NB: The call to QSqlDatabase::addDatabase() is thread-
   // safe, so we don't need to worry about mutexes here.)
   //
   QString driverType{this->pimpl->dbType == Database::PGSQL ? "QPSQL" : "QSQLITE"};
   qDebug() <<
      Q_FUNC_INFO << "Creating connection " << dbConnectionNameForThisThread << " with " << driverType << " driver";
   connection = QSqlDatabase::addDatabase(driverType, dbConnectionNameForThisThread);
   if (!connection.isValid()) {
      //
      // If the connection is not valid, it means the specified driver type is not available or could not be loaded
      // Log an error here in the knowledge that we'll also throw an exception below
      //
      qCritical() << Q_FUNC_INFO << "Unable to load " << driverType << " database driver";
   }

   qDebug() << Q_FUNC_INFO << "Created connection of type" << connection.driver()->handle().typeName();

   //
   // Initialisation parameters depend on the DB type
   //
   if (this->pimpl->dbType == Database::PGSQL) {
      connection.setHostName(dbHostname);
      connection.setDatabaseName(dbName);
      connection.setUserName(dbUsername);
      connection.setPort(dbPortnum);
      connection.setPassword(dbPassword);
   } else {
      connection.setDatabaseName(dbFileName);
   }

   //
   // The moment of truth is when we try to open the new connection
   //
   if (!connection.open()) {
      QString errorMessage;
      if (this->pimpl->dbType == Database::PGSQL) {
         errorMessage = QString{
            QObject::tr("Could not open PostgreSQL DB connection to %1.\n%2")
         }.arg(dbHostname).arg(connection.lastError().text());
      } else {
         errorMessage = QString{
            QObject::tr("Could not open SQLite DB file %1.\n%2")
         }.arg(dbFileName).arg(connection.lastError().text());
      }
      qCritical() << Q_FUNC_INFO << errorMessage;

      if (Brewtarget::isInteractive()) {
         QMessageBox::critical(nullptr,
                               QObject::tr("Database Failure"),
                               errorMessage);
      }

      // If we can't talk to the DB, there's not much we can do to recover
      throw errorMessage;
   }

   return connection;
}


bool Database::load() {
   bool dbIsOpen;

   this->pimpl->createFromScratch=false;
   this->pimpl->schemaUpdated=false;
   this->pimpl->loadWasSuccessful = false;

   if (this->dbType() == Database::PGSQL ) {
      dbIsOpen = this->pimpl->loadPgSQL(*this);
   }
   else {
      dbIsOpen = this->pimpl->loadSQLite(*this);
   }

   if ( ! dbIsOpen ) {
      return false;
   }

   this->pimpl->loaded = true;

   QSqlDatabase sqldb = this->sqlDatabase();

   // This should work regardless of the db being used.
   if( this->pimpl->createFromScratch ) {
      bool success = DatabaseSchemaHelper::create(*this, sqldb);
      if( !success ) {
         qCritical() << "DatabaseSchemaHelper::create() failed";
         return false;
      }
   }

   // Update the database if need be. This has to happen before we do anything
   // else or we dump core
   bool schemaErr = false;
   this->pimpl->schemaUpdated = this->pimpl->updateSchema(*this, &schemaErr);

   if( schemaErr ) {
      if (Brewtarget::isInteractive()) {
         QMessageBox::critical(
            nullptr,
            QObject::tr("Database Failure"),
            QObject::tr("Failed to update the database")
         );
      }
      return false;
   }

   this->pimpl->loadWasSuccessful = true;
   return this->pimpl->loadWasSuccessful;
}

void Database::checkForNewDefaultData() {
   // See if there are new ingredients that we need to merge from the data-space db.
   // Don't do this if we JUST copied the dataspace database.
   if (dataDbFile.fileName() != dbFile.fileName() &&
       !Brewtarget::userDatabaseDidNotExist &&
       QFileInfo(dataDbFile).lastModified() > Database::lastDbMergeRequest) {
      if( Brewtarget::isInteractive() &&
         QMessageBox::question(
            nullptr,
            tr("Merge Database"),
            tr("There may be new ingredients and recipes available. Would you like to add these to your database?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
         )
         == QMessageBox::Yes
      ) {
         QString userMessage;
         QTextStream userMessageAsStream{&userMessage};

         bool succeeded = DatabaseSchemaHelper::updateDatabase(userMessageAsStream);

         QString messageBoxTitle{succeeded ? tr("Success!") : tr("ERROR")};
         QString messageBoxText;
         if (succeeded) {
            // The userMessage parameter will tell how many files were imported/exported and/or skipped (as duplicates)
            // Do separate messages for import and export as it makes translations easier
            messageBoxText = QString(
               tr("Successfully read new default data\n\n%1").arg(userMessage)
            );
         } else {
            messageBoxText = QString(
               tr("Unable to import new default data\n\n"
                  "%1\n\n"
                  "Log file may contain more details.").arg(userMessage)
            );
            qCritical() << Q_FUNC_INFO << userMessage;
         }
         qDebug() << Q_FUNC_INFO << "Message box text : " << messageBoxText;
         QMessageBox msgBox{succeeded ? QMessageBox::Information : QMessageBox::Critical,
                            messageBoxTitle,
                            messageBoxText};
         msgBox.exec();
      }

      // Update this field.
      Database::lastDbMergeRequest = QDateTime::currentDateTime();
   }
   return;
}


bool Database::createBlank(QString const& filename)
{
   {
      QSqlDatabase sqldb = QSqlDatabase::addDatabase("QSQLITE", "blank");
      sqldb.setDatabaseName(filename);
      bool dbIsOpen = sqldb.open();
      if( ! dbIsOpen )
      {
         qWarning() << QString("Database::createBlank(): could not open '%1'").arg(filename);
         return false;
      }

      DatabaseSchemaHelper::create(Database::instance(Database::SQLITE), sqldb);

      sqldb.close();
   } // sqldb gets destroyed as it goes out of scope before removeDatabase()

   QSqlDatabase::removeDatabase( "blank" );
   return true;
}

bool Database::loadSuccessful()
{
   return this->pimpl->loadWasSuccessful;
}


void Database::unload() {

   // This RAII wrapper does all the hard work on mutex.lock() and mutex.unlock() in an exception-safe way
   QMutexLocker locker(&this->pimpl->mutex);

   // So far, it seems we only create one connection to the db. This is
   // likely overkill
   QStringList allConnectionNames{QSqlDatabase::connectionNames()};
   for (QString conName : allConnectionNames) {
      qDebug() << Q_FUNC_INFO << "Closing connection " << conName;
      {
         //
         // Extra braces here are to ensure that this QSqlDatabase object is out of scope before the call to
         // QSqlDatabase::removeDatabase() below
         //
         QSqlDatabase connectionToClose = QSqlDatabase::database(conName, false);
         if (connectionToClose.isOpen()) {
            connectionToClose.rollback();
            connectionToClose.close();
         }
      }
      QSqlDatabase::removeDatabase(conName);
   }

   qDebug() << Q_FUNC_INFO << "DB connections all closed";

   if (this->pimpl->loadWasSuccessful && this->dbType() == Database::SQLITE ) {
      dbFile.close();
      this->pimpl->automaticBackup();
   }

   this->pimpl->loaded = false;
   this->pimpl->loadWasSuccessful = false;

   qDebug() << Q_FUNC_INFO << "Drop Instance done";

   return;
}


Database& Database::instance(Database::DbType dbType) {
   //
   // For the moment, with only two types of database supported, we don't do anything too sophisticated here, but we
   // should probably change that if we end up supporting more.
   //
   if (dbType == Database::NODB) {
      dbType = static_cast<Database::DbType>(PersistentSettings::value(PersistentSettings::Names::dbType, Database::SQLITE).toInt());
   }

   //
   // As of C++11, simple "Meyers singleton" is now thread-safe -- see
   // https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton#h3-guarantees-of-the-c-runtime
   //
   static Database dbSingleton_SQLite{Database::SQLITE}, dbSingleton_PostgresSQL{Database::PGSQL};

   //
   // And C++11 also provides a thread-safe way to ensure a function is called exactly once
   //
   // (See http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf for why user-implemented efforts to do this via
   // double-checked locking often come unstuck in the face of compiler optimisations, especially on multi-processor
   // platforms, back in the days when the C++ language had "no notion of threading (or any other form of
   // concurrency)".
   //
   static std::once_flag initFlag_SQLite, initFlag_PostgresSQL;

   if (dbType == Database::SQLITE) {
      std::call_once(initFlag_SQLite, &Database::load, &dbSingleton_SQLite);
      return dbSingleton_SQLite;
   }

   std::call_once(initFlag_PostgresSQL, &Database::load, &dbSingleton_PostgresSQL);
   return dbSingleton_PostgresSQL;
}

char const * Database::getDefaultBackupFileName() {
    return "database.sqlite";
}

bool Database::backupToFile(QString newDbFileName) {
   // Make sure the singleton exists - otherwise there's nothing to backup.
   instance();

   // Remove the files if they already exist so that
   // the copy() operation will succeed.
   QFile::remove(newDbFileName);

   bool success = dbFile.copy( newDbFileName );

   qDebug() << QString("Database backup to \"%1\" %2").arg(newDbFileName, success ? "succeeded" : "failed");

   return success;
}

bool Database::backupToDir(QString dir, QString filename) {
   bool success = true;
   QString prefix = dir + "/";
   QString newDbFileName = prefix + getDefaultBackupFileName();

   if ( !filename.isEmpty() ) {
      newDbFileName = prefix + filename;
   }

   success = backupToFile( newDbFileName );

   return success;
}

bool Database::restoreFromFile(QString newDbFileStr)
{
   bool success = true;

   QFile newDbFile(newDbFileStr);
   // Fail if we can't find file.
   if( !newDbFile.exists() ) {
      return false;
   }
   success &= newDbFile.copy(QString("%1.new").arg(dbFile.fileName()));
   QFile::setPermissions( newDbFile.fileName(), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );

   return success;
}

bool Database::verifyDbConnection(Database::DbType testDb, QString const& hostname, int portnum, QString const& schema,
                                  QString const& database, QString const& username, QString const& password) {
   QString driverName;

   switch (testDb) {
      case Database::PGSQL:
         driverName = "QPSQL";
         break;
      default:
         driverName = "QSQLITE";
   }

   QSqlDatabase connDb = QSqlDatabase::addDatabase(driverName,"testConnDb");

   switch (testDb) {
      case Database::PGSQL:
         connDb.setHostName(hostname);
         connDb.setPort(portnum);
         connDb.setDatabaseName(database);
         connDb.setUserName(username);
         connDb.setPassword(password);
         break;
      default:
         connDb.setDatabaseName(hostname);
   }

   bool results = connDb.open();

   if ( results ) {
      connDb.close();
   } else {
      QMessageBox::critical(
         nullptr,
         tr("Connection failed"),
         QString(tr("Could not connect to %1 : %2")).arg(hostname).arg(connDb.lastError().text())
      );
   }
   return results;

}



void Database::convertDatabase(QString const& Hostname, QString const& DbName,
                               QString const& Username, QString const& Password,
                               int Portnum, Database::DbType newType)
{
   QSqlDatabase connectionNew;

   Database::DbType oldType = static_cast<Database::DbType>(PersistentSettings::value(PersistentSettings::Names::dbType, Database::SQLITE).toInt());

   try {
      if ( newType == Database::NODB ) {
         throw QString("No type found for the new database.");
      }

      if ( oldType == Database::NODB ) {
         throw QString("No type found for the old database.");
      }

      switch( newType ) {
         case Database::PGSQL:
            connectionNew = openPostgres(Hostname, DbName,Username, Password, Portnum);
            break;
         default:
            connectionNew = openSQLite();
      }

      if ( ! connectionNew.isOpen() ) {
         throw QString("Could not open new database: %1").arg(connectionNew.lastError().text());
      }

      DatabaseSchemaHelper::copyDatabase(oldType, newType, connectionNew);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }
}

Database::DbType Database::dbType() {
   return this->pimpl->dbType;
}

void Database::setForeignKeysEnabled(bool enabled, QSqlDatabase connection, Database::DbType type) {
   if (type == Database::NODB) {
      type = this->dbType();
   }

   switch( type ) {
      case SQLITE:
         if (enabled) {
            connection.exec("PRAGMA foreign_keys=on");
         } else {
            connection.exec("PRAGMA foreign_keys=off");
         }
         break;
      case PGSQL:
         // This is a bit of a hack, and needs you to be connected as super user, but seems more robust than
         // "SET CONSTRAINTS ALL DEFERRED" which requires foreign keys to have been set up in a particular way in the
         // first place (see https://www.postgresql.org/docs/13/sql-set-constraints.html).
         if (enabled) {
            connection.exec("SET session_replication_role TO 'origin'");
         } else {
            connection.exec("SET session_replication_role TO 'replica'");
         }
         break;
      default:
         // It's a coding error (somewhere) if we get here!
         Q_ASSERT(false);
   }

   return;
}


QString Database::dbBoolean(bool flag, Database::DbType type) {
   QString retval;

   if (type == Database::NODB) {
      type = static_cast<Database::DbType>(PersistentSettings::value(PersistentSettings::Names::dbType, Database::SQLITE).toInt());
   }

   switch( type ) {
      case SQLITE:
         retval = flag ? QString("1") : QString("0");
         break;
      case PGSQL:
         retval = flag ? QString("true") : QString("false");
         break;
      default:
         retval = "notwhiskeytangofoxtrot";
   }
   return retval;
}

template<typename T> char const * Database::getDbNativeTypeName(Database::DbType type) const {
   return getDbNativeName(nativeTypeNames<T>, this->pimpl->dbType);
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template char const * Database::getDbNativeTypeName<bool>(Database::DbType type) const;
template char const * Database::getDbNativeTypeName<int>(Database::DbType type) const;
template char const * Database::getDbNativeTypeName<unsigned int>(Database::DbType type) const;
template char const * Database::getDbNativeTypeName<double>(Database::DbType type) const;
template char const * Database::getDbNativeTypeName<QString>(Database::DbType type) const;
template char const * Database::getDbNativeTypeName<QDate>(Database::DbType type) const;

char const * Database::getDbNativeIntPrimaryKeyModifier(Database::DbType type) const {
   return getDbNativeName(nativeIntPrimaryKeyModifier, this->pimpl->dbType);
}

char const * Database::getSqlToAddColumnAsForeignKey(Database::DbType type) const {
   return getDbNativeName(sqlToAddColumnAsForeignKey, this->pimpl->dbType);
}

QDateTime Database::lastDbMergeRequest = QDateTime::fromString("1986-02-24T06:00:00", Qt::ISODate);
