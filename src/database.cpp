/*
 * database.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - David Grundberg <individ@acc.umu.se>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Luke Vincent <luke.r.vincent@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
 * - Samuel Östling <MrOstling@gmail.com>
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

#include "database.h"

// Uncomment the following two includes to enable stacktraces with boost::stacktrace::stacktrace()
#include <boost/stacktrace.hpp>
#include <sstream>      // std::ostringstream

#include <QList>
#include <QDomDocument>
#include <QIODevice>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>
#include <QTextCodec>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlIndex>
#include <QSqlError>
#include <QSqlField>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>
#include <QInputDialog>
#include <QCryptographicHash>
#include <QPair>

#include "Algorithms.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Salt.h"
#include "model/Yeast.h"

#include "config.h"
#include "xml/BeerXml.h"
#include "brewtarget.h"
#include "QueuedMethod.h"
#include "DatabaseSchemaHelper.h"
#include "DatabaseSchema.h"
#include "TableSchema.h"
#include "TableSchemaConst.h"
#include "MashStepSchema.h"
#include "InstructionSchema.h"
#include "BrewnoteSchema.h"
#include "RecipeSchema.h"
#include "WaterSchema.h"
#include "SaltSchema.h"
#include "SettingsSchema.h"

// Static members.
Database* Database::dbInstance = nullptr;
DatabaseSchema* Database::dbDefn;
QString Database::dbHostname;
int Database::dbPortnum;
QString Database::dbUsername;
QString Database::dbPassword;
QString Database::dbName;
QString Database::dbSchema;

QFile Database::dbFile;
QString Database::dbFileName;
QFile Database::dataDbFile;
QString Database::dataDbFileName;
QString Database::dbConName;

QHash< QThread*, QString > Database::_threadToConnection;
QMutex Database::_threadToConnectionMutex;

Database::Database()
{
   //.setUndoLimit(100);
   // Lock this here until we actually construct the first database connection.
   _threadToConnectionMutex.lock();
   converted = false;
   dbDefn = new DatabaseSchema();
   m_beerxml = new BeerXML(dbDefn);

}

Database::~Database()
{

   // If we have not explicitly unloaded, do so now and discard changes.
   if( QSqlDatabase::database( dbConName, false ).isOpen() )
      unload();

   // Delete all the ingredients floating around.
   qDeleteAll(allBrewNotes);
   qDeleteAll(allEquipments);
   qDeleteAll(allFermentables);
   qDeleteAll(allHops);
   qDeleteAll(allInstructions);
   qDeleteAll(allMashSteps);
   qDeleteAll(allMashs);
   qDeleteAll(allMiscs);
   qDeleteAll(allStyles);
   qDeleteAll(allWaters);
   qDeleteAll(allSalts);
   qDeleteAll(allYeasts);
   qDeleteAll(allRecipes);

}

bool Database::loadSQLite()
{
   qDebug() << "Loading SQLITE...";
   bool dbIsOpen;
   QSqlDatabase sqldb;

   // Set file names.
   dbFileName = Brewtarget::getUserDataDir().filePath("database.sqlite");
   qDebug() << QString("%1 - dbFileName = \"%2\"").arg(Q_FUNC_INFO).arg(dbFileName);
   // Set the files.
   dbFile.setFileName(dbFileName);

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
      if( dataDbFile.exists() )
      {
         dataDbFile.copy(dbFileName);
         QFile::setPermissions( dbFileName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );
      }

      // Reset the last merge request.
      Brewtarget::lastDbMergeRequest = QDateTime::currentDateTime();
   }

   // Open SQLite db.
   QString conName = QString("0x%1").arg(reinterpret_cast<uintptr_t>(QThread::currentThread()), 0, 16);

   sqldb = QSqlDatabase::addDatabase("QSQLITE",conName);
   sqldb.setDatabaseName(dbFileName);
   dbIsOpen = sqldb.open();
   dbConName = sqldb.connectionName();

   if( ! dbIsOpen )
   {
      qCritical() << QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text());
      if (Brewtarget::isInteractive()) {
         QMessageBox::critical(
            nullptr,
            QObject::tr("Database Failure"),
            QString(QObject::tr("Failed to open the database '%1'.").arg(dbFileName))
         );
      }
   }
   else
   {
      // NOTE: synchronous=off reduces query time by an order of magnitude!
      QSqlQuery pragma(sqldb);
      try {
         if ( ! pragma.exec( "PRAGMA synchronous = off" ) )
            throw QString("could not disable synchronous writes");
         if ( ! pragma.exec( "PRAGMA foreign_keys = on"))
            throw QString("could not enable foreign keys");
         if ( ! pragma.exec( "PRAGMA locking_mode = EXCLUSIVE"))
            throw QString("could not enable exclusive locks");
         if ( ! pragma.exec("PRAGMA temp_store = MEMORY") )
            throw QString("could not enable temporary memory");

         // older sqlite databases may not have a settings table. I think I will
         // just check to see if anything is in there.
         createFromScratch = sqldb.tables().size() == 0;

         // Associate this db with the current thread.
         _threadToConnection.insert(QThread::currentThread(), dbConName);
      }
      catch(QString e) {
         qCritical() << QString("%1: %2 (%3)").arg(Q_FUNC_INFO).arg(e).arg(pragma.lastError().text());
         dbIsOpen = false;
      }
   }
   return dbIsOpen;
}

bool Database::loadPgSQL()
{
   bool dbIsOpen;
   QSqlDatabase sqldb;

   dbHostname = Brewtarget::option("dbHostname").toString();
   dbPortnum  = Brewtarget::option("dbPortnum").toInt();
   dbName     = Brewtarget::option("dbName").toString();
   dbSchema   = Brewtarget::option("dbSchema").toString();

   dbUsername = Brewtarget::option("dbUsername").toString();

   if ( Brewtarget::hasOption("dbPassword") ) {
      dbPassword = Brewtarget::option("dbPassword").toString();
   }
   else {
      bool isOk = false;

      // prompt for the password until we get it? I don't think this is a good
      // idea?
      while ( ! isOk ) {
         dbPassword = QInputDialog::getText(nullptr,tr("Database password"),
               tr("Password"), QLineEdit::Password,QString(),&isOk);
         if ( isOk ) {
            isOk = verifyDbConnection( Brewtarget::PGSQL, dbHostname, dbPortnum, dbSchema,
                                 dbName, dbUsername, dbPassword);
         }
      }
   }

   QString conName = QString("0x%1").arg(reinterpret_cast<uintptr_t>(QThread::currentThread()), 0, 16);

   sqldb = QSqlDatabase::addDatabase("QPSQL",conName);
   sqldb.setHostName( dbHostname );
   sqldb.setDatabaseName( dbName );
   sqldb.setUserName( dbUsername );
   sqldb.setPort( dbPortnum );
   sqldb.setPassword( dbPassword );

   dbIsOpen = sqldb.open();
   dbConName = sqldb.connectionName();

   if( ! dbIsOpen ) {
      qCritical() << QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text());
      QMessageBox::critical(nullptr,
                            QObject::tr("Database Failure"),
                            QString(QObject::tr("Failed to open the database '%1'.").arg(dbHostname))
                           );
   }
   else {
      // by the time we had pgsql support, there is a settings table
      createFromScratch = ! sqldb.tables().contains("settings");
      // Associate this db with the current thread.
      _threadToConnection.insert(QThread::currentThread(), sqldb.connectionName());
   }

   return dbIsOpen;
}

bool Database::load()
{
   bool dbIsOpen;
   QSqlDatabase sqldb;

   dataDbFileName = Brewtarget::getDataDir().filePath("default_db.sqlite");
   dataDbFile.setFileName(dataDbFileName);
   qDebug() << QString("%1 - dataDbFileName=\"%2\"").arg(Q_FUNC_INFO).arg(dataDbFileName);

   createFromScratch=false;
   schemaUpdated=false;
   loadWasSuccessful = false;

   if ( Brewtarget::dbType() == Brewtarget::PGSQL ) {
      dbIsOpen = loadPgSQL();
   }
   else {
      dbIsOpen = loadSQLite();
   }

   _threadToConnectionMutex.unlock();
   if ( ! dbIsOpen )
      return false;

   sqldb = sqlDatabase();

   // This should work regardless of the db being used.
   if( createFromScratch ) {
      bool success = DatabaseSchemaHelper::create(sqldb,dbDefn,Brewtarget::dbType());
      if( !success ) {
         qCritical() << "DatabaseSchemaHelper::create() failed";
         return success;
      }
   }

   // Update the database if need be. This has to happen before we do anything
   // else or we dump core
   bool schemaErr = false;
   schemaUpdated = updateSchema(&schemaErr);

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

   // See if there are new ingredients that we need to merge from the data-space db.
   // Don't do this if we JUST copied the dataspace database.
   if( dataDbFile.fileName() != dbFile.fileName()
      && ! Brewtarget::userDatabaseDidNotExist
      && QFileInfo(dataDbFile).lastModified() > Brewtarget::lastDbMergeRequest )
   {
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
         if ( Brewtarget::dbType() == Brewtarget::SQLITE ) {
            QString file_name = QString("%1.%2").arg("bt_update").arg(QFileInfo(dataDbFile).lastModified().toSecsSinceEpoch());
            QString backupDir = Brewtarget::option("directory", Brewtarget::getConfigDir().canonicalPath(),"backups").toString();
            backupToDir(backupDir, file_name);
         }

         updateDatabase(dataDbFile.fileName());
      }

      // Update this field.
      Brewtarget::lastDbMergeRequest = QDateTime::currentDateTime();
   }

   // Create and store all pointers.
   populateElements( allBrewNotes, Brewtarget::BREWNOTETABLE );
   populateElements( allEquipments, Brewtarget::EQUIPTABLE );
   populateElements( allFermentables, Brewtarget::FERMTABLE );
   populateElements( allHops, Brewtarget::HOPTABLE );
   populateElements( allInstructions, Brewtarget::INSTRUCTIONTABLE );
   populateElements( allMashs, Brewtarget::MASHTABLE );
   populateElements( allMashSteps, Brewtarget::MASHSTEPTABLE );
   populateElements( allMiscs, Brewtarget::MISCTABLE );
   populateElements( allStyles, Brewtarget::STYLETABLE );
   populateElements( allWaters, Brewtarget::WATERTABLE );
   populateElements( allSalts, Brewtarget::SALTTABLE );
   populateElements( allYeasts, Brewtarget::YEASTTABLE );

   populateElements( allRecipes, Brewtarget::RECTABLE );

   // Connect fermentable,hop changed signals to their parent recipe.
   QHash<int,Recipe*>::iterator i;
   QList<Fermentable*>::iterator j;
   QList<Hop*>::iterator k;
   QList<Yeast*>::iterator l;
   QList<Mash*>::iterator m;
   QList<MashStep*>::iterator n;


   for( i = allRecipes.begin(); i != allRecipes.end(); i++ )
   {
      Equipment* e = equipment(*i);
      if( e )
      {
         connect( e, &NamedEntity::changed, *i, &Recipe::acceptEquipChange );
         connect( e, &Equipment::changedBoilSize_l, *i, &Recipe::setBoilSize_l);
         connect( e, &Equipment::changedBoilTime_min, *i, &Recipe::setBoilTime_min);
      }

      QList<Fermentable*> tmpF = fermentables(*i);
      for( j = tmpF.begin(); j != tmpF.end(); ++j ) {
         connect( *j, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptFermChange(QMetaProperty,QVariant)) );
      }

      QList<Hop*> tmpH = hops(*i);
      for( k = tmpH.begin(); k != tmpH.end(); ++k ) {
         connect( *k, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptHopChange(QMetaProperty,QVariant)) );
      }

      QList<Yeast*> tmpY = yeasts(*i);
      for( l = tmpY.begin(); l != tmpY.end(); ++l ) {
         connect( *l, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptYeastChange(QMetaProperty,QVariant)) );
      }

      // a recipe may not have a mash. Can't connect what doesn't exist
      if ( mash(*i) != nullptr ) {
         connect( mash(*i), SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptMashChange(QMetaProperty,QVariant)) );
      }
   }

   QList<Mash*> tmpM = mashs();
   for( m = tmpM.begin(); m != tmpM.end(); ++m )
   {
      QList<MashStep*> tmpMS = mashSteps(*m);
      for( n=tmpMS.begin(); n != tmpMS.end(); ++n) {
         connect( *n, SIGNAL(changed(QMetaProperty,QVariant)), *m, SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );
      }
   }

   loadWasSuccessful = true;
   return loadWasSuccessful;
}

int Database::numberOfRecipes() const {
   return allRecipes.size();
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

      if ( dbDefn == nullptr ) {
         dbDefn = new DatabaseSchema();
      }
      DatabaseSchemaHelper::create(sqldb,dbDefn,Brewtarget::SQLITE);

      sqldb.close();
   } // sqldb gets destroyed as it goes out of scope before removeDatabase()

   QSqlDatabase::removeDatabase( "blank" );
   return true;
}

bool Database::loadSuccessful()
{
   return loadWasSuccessful;
}

void Database::convertFromXml()
{

   // We have two use cases to consider here. The first is a BT
   // 1.x user running BT 2 for the first time. The second is a BT 2 clean
   // install. I am also trying to protect the developers from double imports.
   // If the old "obsolete" directory exists, don't do anything other than
   // set the converted flag
   QDir dir(Brewtarget::getUserDataDir());

   // Checking for non-existence is redundant with the new "converted" setting,
   // but better safe than sorry.
   if( !dir.exists("obsolete") )
   {
      dir.mkdir("obsolete");
      dir.cd("obsolete");

      QStringList oldFiles = QStringList() << "database.xml" << "mashs.xml" << "recipes.xml";
      for ( int i = 0; i < oldFiles.size(); ++i )
      {
         QFile oldXmlFile(Brewtarget::getUserDataDir().filePath(oldFiles[i]));
         // If the old file exists, import.
         if( oldXmlFile.exists() )
         {
            QString errorMessage;
            QTextStream errorMessageAsStream{&errorMessage};
            if (!m_beerxml->importFromXML( oldXmlFile.fileName(), errorMessageAsStream )) {
               QString exceptionMessage = QString("Error importing old XML file: %1").arg(errorMessage);
               qCritical() << exceptionMessage;
               throw std::runtime_error(exceptionMessage.toLocal8Bit().constData());
            }

            // Move to obsolete/ directory.
            if( oldXmlFile.copy(dir.filePath(oldFiles[i])) )
               oldXmlFile.remove();

            // Let us know something was converted
            converted = true;
         }
      }
   }
   Brewtarget::setOption("converted", QDate().currentDate().toString());
}

bool Database::isConverted()
{
   return converted;
}

QSqlDatabase Database::sqlDatabase()
{
   // Need a unique database connection for each thread.
   //http://www.linuxjournal.com/article/9602

   QThread* t = QThread::currentThread();
   QSqlDatabase sqldb;

   _threadToConnectionMutex.lock();
   // If this thread already has a connection, return it.
   if( _threadToConnection.contains(t) )
   {
      QSqlDatabase ret = QSqlDatabase::database(_threadToConnection[t]);
      _threadToConnectionMutex.unlock();
      return ret;
   }
   // Create a unique connection name, just containing the addy of the thread.
   QString conName = QString("0x%1").arg(reinterpret_cast<uintptr_t>(t), 0, 16);

   // Create the new connection.
   try {
      if ( Brewtarget::dbType() == Brewtarget::PGSQL ) {
         sqldb = QSqlDatabase::addDatabase("QPSQL",conName);

         sqldb.setHostName( dbHostname );
         sqldb.setDatabaseName( dbName );
         sqldb.setUserName( dbUsername );
         sqldb.setPort( dbPortnum );
         sqldb.setPassword( dbPassword );

         if( ! sqldb.open() )
            throw QString("Could not open %1 for reading.\n%2")
            .arg(dbHostname).arg(sqldb.lastError().text());
      }
      else {
         sqldb = QSqlDatabase::addDatabase("QSQLITE",conName);
         sqldb.setDatabaseName(dbFileName);
         if( ! sqldb.open() )
            throw QString("Could not open %1 for reading.\n%2")
            .arg(dbFileName).arg(sqldb.lastError().text());
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      _threadToConnectionMutex.unlock();
      throw;
   }

   // Put new connection in the hash.
   _threadToConnection.insert(t,conName);
   _threadToConnectionMutex.unlock();
   return sqldb;
}


template <class T> void Database::populateElements( QHash<int,T*>& hash, Brewtarget::DBTable table )
{
   QSqlQuery q(sqlDatabase());
   TableSchema* tbl = dbDefn->table(table);
   q.setForwardOnly(true);
   QString queryString = QString("SELECT * FROM %1").arg(tbl->tableName());
   q.prepare( queryString );

   try {
      if ( ! q.exec() )
         throw QString("%1 %2").arg(q.lastQuery()).arg(q.lastError().text());
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      q.finish();
      throw;
   }

   while( q.next() ) {
      int key = q.record().value(tbl->keyName(Brewtarget::dbType())).toInt();

      // if the thing is already in the hash, there's no point making a new
      // one
      if( ! hash.contains(key) ) {
         T* e = new T(tbl, q.record());
         hash.insert(key, e);
      }
   }

   q.finish();
}


template <class T> bool Database::getElements(QList<T*>& list,
                                              QString filter,
                                              Brewtarget::DBTable table,
                                              QHash<int,T*> allElements,
                                              QString id)
{
   QSqlQuery q(sqlDatabase());
   TableSchema* tbl = dbDefn->table( table );
   q.setForwardOnly(true);
   QString queryString;

   if ( id.isEmpty() ) {
      id = tbl->keyName(Brewtarget::dbType());
   }

   if( !filter.isEmpty() ) {
      queryString = QString("SELECT %1 as id FROM %2 WHERE %3").arg(id).arg(tbl->tableName()).arg(filter);
   }
   else {
      queryString = QString("SELECT %1 as id FROM %2").arg(id).arg(tbl->tableName());
   }

   qDebug() << QString("%1 SQL: %2").arg(Q_FUNC_INFO).arg(queryString);

   try {
      if ( ! q.exec(queryString) )
         throw QString("could not execute query: %2 : %3").arg(queryString).arg(q.lastError().text());
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      q.finish();
      throw;
   }

   while( q.next() )
   {
      int key = q.record().value("id").toInt();
      if( allElements.contains(key) )
         list.append( allElements[key] );
   }

   q.finish();
   return true;
}


void Database::unload()
{
   // selectSome saves context. If we close the database before we tear that
   // context down, core gets dumped
   selectSome.clear();

   // so far, it seems we only create one connection to the db. This is
   // likely overkill
   foreach( QString conName, _threadToConnection.values() ) {
      QSqlDatabase::database( conName, false ).close();
      QSqlDatabase::removeDatabase( conName );
   }

   if (loadWasSuccessful && Brewtarget::dbType() == Brewtarget::SQLITE )
   {
      dbFile.close();
      automaticBackup();
   }
}

void Database::automaticBackup()
{
   int count = Brewtarget::option("count",0,"backups").toInt() + 1;
   int frequency = Brewtarget::option("frequency",4,"backups").toInt();
   int maxBackups = Brewtarget::option("maximum",10,"backups").toInt();

   // The most common case is update the counter and nothing else
   // A frequency of 1 means backup every time. Which this statisfies
   if ( count % frequency != 0 ) {
      Brewtarget::setOption( "count", count, "backups");
      return;
   }

   // If the user has selected 0 max backups, we just return. There's a weird
   // case where they have a frequency of 1 and a maxBackup of 0. In that
   // case, maxBackup wins
   if ( maxBackups == 0 ) {
      return;
   }

   QString backupDir = Brewtarget::option("directory", Brewtarget::getConfigDir().canonicalPath(),"backups").toString();
   QString listOfFiles = Brewtarget::option("files",QVariant(),"backups").toString();
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QStringList fileNames = listOfFiles.split(",", QString::SkipEmptyParts);
#else
   QStringList fileNames = listOfFiles.split(",", Qt::SkipEmptyParts);
#endif

   QString halfName = QString("%1.%2").arg("bt_database").arg(QDate::currentDate().toString("yyyyMMdd"));
   QString newName = halfName;
   // Unique filenames are a pain in the ass. In the case you open brewtarget
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
      Brewtarget::removeOption("files","backups");
      return;
   }

   fileNames.append(newName);

   // If we have too many backups. This is in a while loop because we need to
   // handle the case where a user decides they only want 4 backups, not 10.
   // The while loop will clean that up properly.
   while ( fileNames.size() > maxBackups ) {
      // takeFirst() removes the file from the list, which is important
      QString victim = backupDir + "/" + fileNames.takeFirst();
      QFileInfo *fileThing = new QFileInfo(victim);

      // Make sure it exists, and make sure it is a file before we
      // try remove it
      if ( fileThing->exists() && fileThing->isFile() ) {
         // If we can't remove it, give a warning.
         QFile *file = new QFile(victim);
         if (! file->remove() ) {
            qWarning() << QString("%1 : could not remove %2 (%3).").arg(Q_FUNC_INFO).arg(victim).arg(file->error());
         }
      }
   }

   // re-encode the list
   listOfFiles = fileNames.join(",");

   // finally, reset the counter and save the new list of files
   Brewtarget::setOption( "count", 0, "backups");
   Brewtarget::setOption( "files", listOfFiles, "backups");
}

Database& Database::instance()
{

   // Not thread-safe
   //static Database dbSingleton;
   //return dbSingleton;

   // This is not safe either. This is the double-check pattern that
   // avoids acquiring the lock unless we need to make a new instance.
   // The problem is that it's not safe. Should replace this lazy
   // initialization with eager initialization, or just do a single
   // check lock.
   // http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
   static QMutex mutex;
   if( ! dbInstance )
   {
      mutex.lock();

      if( ! dbInstance ) {
         dbInstance = new Database();
         dbInstance->load();
      }

      mutex.unlock();
   }

   return *dbInstance;
}

void Database::dropInstance()
{
   static QMutex mutex;

   mutex.lock();
   dbInstance->unload();
   delete dbInstance;
   dbInstance=nullptr;
   mutex.unlock();

}

char const * Database::getDefaultBackupFileName()
{
    return "database.sqlite";
}

bool Database::backupToFile(QString newDbFileName)
{
   // Make sure the singleton exists - otherwise there's nothing to backup.
   instance();

   bool success = true;

   // Remove the files if they already exist so that
   // the copy() operation will succeed.
   QFile::remove(newDbFileName);

   success = dbFile.copy( newDbFileName );

   qDebug() << QString("Database backup to \"%1\" %2").arg(newDbFileName, success ? "succeeded" : "failed");

   return success;
}

bool Database::backupToDir(QString dir,QString filename)
{
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
   if( !newDbFile.exists() )
      return false;

   success &= newDbFile.copy(QString("%1.new").arg(dbFile.fileName()));
   QFile::setPermissions( newDbFile.fileName(), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );

   return success;
}

int Database::getParentNamedEntityKey(NamedEntity const & ingredient) {
   int parentKey = 0;

   const QMetaObject* meta = ingredient.metaObject();

   Brewtarget::DBTable parentToChildTableId =
      this->dbDefn->table(
         this->dbDefn->classNameToTable(meta->className())
      )->childTable();

   // Don't do this if no child table is defined (like instructions)
   if (parentToChildTableId != Brewtarget::NOTABLE) {
      TableSchema * parentToChildTable = this->dbDefn->table(parentToChildTableId);

      QString findParentNamedEntity =
         QString("SELECT %1 FROM %2 WHERE %3=%4").arg(parentToChildTable->parentIndexName())
                                                 .arg(parentToChildTable->tableName())
                                                 .arg(parentToChildTable->childIndexName())
                                                 .arg(ingredient.key());
      qDebug() << Q_FUNC_INFO << "Find Parent NamedEntity SQL: " << findParentNamedEntity;

      QSqlQuery query(this->sqlDatabase());
      if (!query.exec(findParentNamedEntity)) {
         throw QString("Database error trying to find parent ingredient.");
      }

      if (query.next()) {
         parentKey = query.record().value(parentToChildTable->parentIndexName()).toInt();
         qDebug() << QString("Found Parent with Key: %1").arg(parentKey);
      }
   }
   return parentKey;
}


bool Database::isStored(NamedEntity const & ingredient) {
   // Valid database keys are all positive
   if (ingredient.key() <= 0) {
      return false;
   }

   const QMetaObject* meta = ingredient.metaObject();

   TableSchema * table = this->dbDefn->table(this->dbDefn->classNameToTable(meta->className()));

   QString idColumnName = table->keyName(Brewtarget::dbType());

   QString queryString = QString("SELECT %1 AS id FROM %2 WHERE %3=%4").arg(idColumnName)
                                                                       .arg(table->tableName())
                                                                       .arg(idColumnName)
                                                                       .arg(ingredient.key());

   qDebug() << QString("%1 SQL: %2").arg(Q_FUNC_INFO).arg(queryString);

   QSqlQuery query(this->sqlDatabase());

   try {
      if ( ! query.exec(queryString) )
         throw QString("could not execute query: %2 : %3").arg(queryString).arg(query.lastError().text());
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      query.finish();
      throw;
   }

   bool foundInDatabase = query.next();

   query.finish();
   return foundInDatabase;
}


// removeFromRecipe ===========================================================
NamedEntity * Database::removeNamedEntityFromRecipe( Recipe* rec, NamedEntity* ing )
{
   NamedEntity * parentNamedEntity = ing->getParent();

   const QMetaObject* meta = ing->metaObject();
   TableSchema *table;
   TableSchema *child;
   TableSchema *inrec;

   int ndx = meta->indexOfClassInfo("signal");
   QString propName;

   // Oh wow. If we are doing versioning, removing a named entity really means
   // "make a new copy with everything but this". I think I am about to run
   // headlong into the Undo. I might address that by making the undo test its
   // returns?
   if ( wantsVersion(rec) ) {
      copyRecipeExcept(rec,ing);
      return ing;
   }

   sqlDatabase().transaction();
   QSqlQuery q(sqlDatabase());

   qDebug() << QString("%1 Deleting NamedEntity %2 #%3").arg(Q_FUNC_INFO).arg(meta->className()).arg(ing->key());

   try {
      if ( ndx != -1 ) {
         propName  = meta->classInfo(ndx).value();
      }
      else {
         throw QString("could not locate classInfo for signal on %2").arg(meta->className());
      }

      table = dbDefn->table( dbDefn->classNameToTable(meta->className()) );
      child = dbDefn->table( table->childTable() );
      inrec = dbDefn->table( table->inRecTable() );
      // We need to do many things -- remove the link in *in_recipe,
      // remove the entry from *_children
      // and DELETE THE COPY
      // delete from misc_in_recipe where misc_id = [misc key] and recipe_id = [rec key]
      QString deleteFromInRecipe = QString("DELETE FROM %1 WHERE %2=%3 AND %4=%5")
                                 .arg(inrec->tableName() )
                                 .arg(inrec->inRecIndexName())
                                 .arg(ing->key())
                                 .arg(inrec->recipeIndexName())
                                 .arg(rec->key());
      qDebug() << QString("Delete From In Recipe SQL: %1").arg(deleteFromInRecipe);

      // delete from misc where id = [misc key]
      QString deleteNamedEntity = QString("DELETE FROM %1 where %2=%3")
                                 .arg(table->tableName())
                                 .arg(table->keyName())
                                 .arg(ing->key());
      qDebug() << QString("Delete NamedEntity SQL: %1").arg(deleteNamedEntity);

      q.setForwardOnly(true);

      if (parentNamedEntity) {

         // delete from misc_child where child_id = [misc key]
         QString deleteFromChildren = QString("DELETE FROM %1 WHERE %2=%3")
                                    .arg(child->tableName())
                                    .arg( child->childIndexName() )
                                    .arg(ing->key());
         qDebug() << QString("Delete From Children SQL: %1").arg(deleteFromChildren);
         if ( ! q.exec( deleteFromChildren ) ) {
            qInfo() << Q_FUNC_INFO << q.lastQuery() << q.lastError().text();
            throw QString("failed to delete children.");
         }
      }

      if ( ! q.exec(deleteFromInRecipe) ) {
         qInfo() << Q_FUNC_INFO << q.lastQuery() << q.lastError().text();
         throw QString("failed to delete in_recipe.");
      }

      if ( ! q.exec( deleteNamedEntity ) ) {
         qInfo() << Q_FUNC_INFO << q.lastQuery() << q.lastError().text();
         throw QString("failed to delete ingredient.");
      }

   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text());
      sqlDatabase().rollback();
      q.finish();
      abort();
   }

   rec->recalcAll();
   sqlDatabase().commit();

   q.finish();
   emit rec->changed( rec->metaProperty(propName), QVariant() );

   return ing;
}

void Database::removeFromRecipe( Recipe* rec, Instruction* ins )
{
   qDebug() << QString("%1").arg(Q_FUNC_INFO);

   try {
      removeNamedEntityFromRecipe( rec, ins);
   }
   catch (QString e) {
      throw; //up the stack!
   }

   allInstructions.remove(ins->key());
   emit changed( metaProperty("instructions"), QVariant() );
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Database::removeFrom( Mash* mash, MashStep* step )
{
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   // Just mark the step as deleted.
   try {
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
               QString("%1 = %2").arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted)).arg(Brewtarget::dbTrue()),
               QString("%1 = %2").arg(tbl->keyName()).arg(step->key()));
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

   emit mash->mashStepsChanged();
}


// equipment, style and mashes don't have in_recipe tables. This does those.
QString Database::findRecipeFromForeignKey(TableSchema* tbl, NamedEntity const *obj)
{
   TableSchema* recTable = this->dbDefn->table( Brewtarget::RECTABLE );
   QString fKey;

   if ( obj->table() == Brewtarget::EQUIPTABLE ) fKey = kpropEquipmentId;
   if ( obj->table() == Brewtarget::MASHTABLE )  fKey = kpropMashId;
   if ( obj->table() == Brewtarget::STYLETABLE)  fKey = kpropStyleId;

   if ( ! fKey.isEmpty() ) {
      return QString("SELECT %1 AS id FROM %2 WHERE %3 = %4")
                        .arg(recTable->keyName())
                        .arg(recTable->tableName())
                        .arg(recTable->foreignKeyToColumn(fKey))
                        .arg(obj->key());
   }
   else {
      qInfo() <<
         Q_FUNC_INFO << "couldn't find a key for" << obj->metaObject()->className() << ":" << obj->name() <<
         "(which has table" << obj->table() << ")";
   }
   return QString();
}

QString Database::findRecipeFromInRec(TableSchema* tbl, TableSchema* inrec, NamedEntity const *obj)
{
   return QString("SELECT %1 AS id FROM %2 WHERE %3=%4")
             .arg(inrec->recipeIndexName())
             .arg(inrec->tableName())
             .arg(inrec->inRecIndexName())
             .arg(obj->key());
}

// this handles all things with in_recipe tables (fermentables, hops, miscs, waters and yeasts)
Recipe* Database::getParentRecipe(NamedEntity const * ing)
{
   qDebug() <<
      Q_FUNC_INFO << ing->metaObject()->className() << "has table #" << ing->table() << "(" <<
      this->dbDefn->tableName(ing->table()) << ")";
   TableSchema* table = this->dbDefn->table( ing->table() );
   TableSchema* inrec = this->dbDefn->table( table->inRecTable() );
   QString select;

   if ( inrec == nullptr ) {
      select = findRecipeFromForeignKey(table, ing);
   }
   else {
      select = findRecipeFromInRec(table, inrec, ing);
   }

   Recipe * parent = nullptr;

   QSqlQuery q(sqlDatabase());
   qDebug() << Q_FUNC_INFO << "NamedEntity in recipe search:" << select;

   if (! q.exec(select) ) {
      throw QString("Couldn't execute ingredient in recipe search: Query: %1 error: %2")
               .arg(q.lastQuery())
               .arg(q.lastError().text());
   }
   if ( q.next() ) {
      int key = q.record().value("id").toInt();
      parent = this->allRecipes[key];
   }

   return parent;
}

// Finally, brewnotes are always weird.
Recipe* Database::getParentRecipe( BrewNote const* note )
{
   int key;
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);
   // SELECT recipe_id FROM brewnote WHERE id = [key]
   QString query = QString("SELECT %1 FROM %2 WHERE %3 = %4")
           .arg( tbl->recipeIndexName())
           .arg( tbl->tableName() )
           .arg( tbl->keyName() )
           .arg( note->key() );

   QSqlQuery q(sqlDatabase());

   try {
      if ( ! q.exec(query) )
         throw QString("could not find recipe id");
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text());
      q.finish();
      throw;
   }

   q.next();
   key = q.record().value(tbl->recipeIndexName()).toInt();
   q.finish();

   return allRecipes[key];
}

// Mashsteps need some special handling. Oddly, we don't need to invoke the
// mash table -- just the recipe and the mashstep.
Recipe* Database::getParentRecipe( MashStep const* step )
{
   int key;
   TableSchema* rec = dbDefn->table(Brewtarget::RECTABLE);
   TableSchema* ms  = dbDefn->table(Brewtarget::MASHSTEPTABLE);

   // SELECT recipe.id AS id FROM recipe, mashstep where recipe.mash_id = mashstep.mash_id and mashstep.id = [step->key[
   QString query = QString("SELECT %1.%2 AS id FROM %1,%3 WHERE %1.%4 = %3.%5 and %3.%6 = %7")
                      .arg(rec->tableName())  //recipe
                      .arg(rec->keyName())    //recipe.id
                      .arg(ms->tableName())   //mashstep
                      .arg(rec->foreignKeyToColumn(kpropMashId)) // recipe.mash_id
                      .arg(ms->foreignKeyToColumn(kpropMashId))  // mashstep.mash_id
                      .arg(ms->keyName())    // mashstep.id
                      .arg(step->key());

   QSqlQuery q(sqlDatabase());

   try {
      if ( ! q.exec(query) )
         throw QString("could not find recipe id");
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text());
      q.finish();
      abort();
   }

   q.next();
   key = q.record().value("id").toInt();
   q.finish();

   return allRecipes[key];
}

Recipe*      Database::recipe(int key)      { return allRecipes[key]; }
Equipment*   Database::equipment(int key)   { return allEquipments[key]; }
Fermentable* Database::fermentable(int key) { return allFermentables[key]; }
Hop*         Database::hop(int key)         { return allHops[key]; }
Misc*        Database::misc(int key)        { return allMiscs[key]; }
Style*       Database::style(int key)       { return allStyles[key]; }
Yeast*       Database::yeast(int key)       { return allYeasts[key]; }
Salt*        Database::salt(int key)        { return allSalts[key]; }
Water*       Database::water(int key)       { return allWaters[key]; }

void Database::swapMashStepOrder(MashStep* m1, MashStep* m2)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   // maybe this wasn't such a good idear?
   // UPDATE mashstep SET step_number = CASE id
   //    WHEN [m1->key] then [m2->stepnumber]
   //    WHEN [m2->key] then [m1->stepnumber]
   // END
   // WHERE id IN ([m1->key],[m2->key])
   QString update = QString("UPDATE %1 SET %2 = CASE %3 "
                               "WHEN %4 then %5 "
                               "WHEN %6 then %7 "
                            "END "
                            "WHERE %3 IN (%4,%6)")
                .arg(tbl->tableName() )
                .arg(tbl->propertyToColumn(PropertyNames::MashStep::stepNumber))
                .arg(tbl->keyName())
                .arg(m1->key())
                .arg(m2->stepNumber())
                .arg(m2->key())
                .arg(m1->stepNumber());

   QSqlQuery q(sqlDatabase() );

   try {
      if ( !q.exec(update) )
         throw QString("failed to swap steps");
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text());
      q.finish();
      throw;
   }

   q.finish();

   emit m1->changed( m1->metaProperty(PropertyNames::MashStep::stepNumber) );
   emit m2->changed( m2->metaProperty(PropertyNames::MashStep::stepNumber) );
}

void Database::swapInstructionOrder(Instruction* in1, Instruction* in2)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::INSTINRECTABLE);

   // UPDATE instruction_in_recipe SET instruction_number = CASE instruction_id
   //    WHEN [in1->key] THEN [in2->instructionNumber]
   //    WHEN [in2->key] THEN [in1->instructionNumber]
   // END
   // WHERE instruction_id IN ([in1->key],[in2->key])
   QString update =
      QString( "UPDATE %1 SET %2 = CASE %3 "
                  "WHEN %4 THEN %5 "
                  "WHEN %6 THEN %7 "
               "END "
               "WHERE %3 IN (%4,%6)")
      .arg(tbl->tableName())
      .arg(tbl->propertyToColumn(kpropInstructionNumber))
      .arg(tbl->inRecIndexName())
      .arg(in1->key())
      .arg(in2->instructionNumber())
      .arg(in2->key())
      .arg(in1->instructionNumber());

   QSqlQuery q( sqlDatabase());

   try {
      if ( !q.exec(update) )
         throw QString("failed to swap steps");
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text());
      q.finish();
      throw;
   }

   q.finish();

   emit in1->changed( in1->metaProperty("instructionNumber") );
   emit in2->changed( in2->metaProperty("instructionNumber") );
}

void Database::insertInstruction(Instruction* in, int pos)
{
   int parentRecipeKey;
   TableSchema* tbl = dbDefn->table( Brewtarget::INSTINRECTABLE );
   // SELECT recipe_id FROM instruction_in_recipe WHERE instruction_id=[key]
   QString query = QString("SELECT %1 FROM %2 WHERE %3=%4")
                   .arg( tbl->recipeIndexName())
                   .arg( tbl->tableName() )
                   .arg( tbl->inRecIndexName() )
                   .arg(in->key());
   QString update;

   sqlDatabase().transaction();

   QSqlQuery q(sqlDatabase());

   try {
      if ( !q.exec(query) )
         throw QString("failed to find recipe");

      q.next();
      parentRecipeKey = q.record().value(tbl->recipeIndexName()).toInt();
      q.finish();

      // this happens in three steps --
      // 1. Bump the instruction number for any instruction >= where we need to insert the new one
      // 2. Generate all the signals for what we just did
      // 3. Insert the instruction into the desired slot.

      // Increment all instruction positions greater or equal to pos.
      // update instruction_in_recipe set instruction_number = instruction_number + 1 where recipe_id = [key] and instruction_number > [pos]
      update = QString( "UPDATE %1 SET %2=%2+1 WHERE %3=%4 AND %2>=%5")
         .arg( tbl->tableName() )
         .arg( tbl->propertyToColumn(kpropInstructionNumber) )
         .arg( tbl->recipeIndexName() )
         .arg(parentRecipeKey)
         .arg(pos);
      qDebug() << Q_FUNC_INFO << "Update 1 SQL:" << update;

      if ( !q.exec(update) )
         throw QString("failed to renumber instructions recipe");

      // Emit the signals for everything we just changed.
      // SELECT instruction_id, instruction_number FROM instruction_in_recipe WHERE recipe_id=[key] and instruction_number>[pos]")
      query = QString("SELECT %1, %2 FROM %3 WHERE %4=%5 and %2>%6")
         .arg( tbl->inRecIndexName() )
         .arg( tbl->propertyToColumn(kpropInstructionNumber) )
         .arg( tbl->tableName() )
         .arg( tbl->recipeIndexName())
         .arg(parentRecipeKey)
         .arg(pos);
      qDebug() << Q_FUNC_INFO << "Query SQL:" << query;

      if ( !q.exec(query) )
         throw QString("failed to find renumbered instructions");

      while( q.next() ) {
         Instruction* inst = allInstructions[q.record().value(tbl->inRecIndexName()).toInt() ];
         int newPos = q.record().value(tbl->propertyToColumn(kpropInstructionNumber)).toInt();

         emit inst->changed( inst->metaProperty("instructionNumber"),newPos );
      }

      // Insert the instruction into the new place
      // UPDATE instruction_in_recipe set instruction_number = [pos] where instruction_id=[key]
      update = QString( "UPDATE %1 SET %2=%3 WHERE %4=%5")
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropInstructionNumber))
         .arg(pos)
         .arg( tbl->inRecIndexName() )
         .arg(in->key());
      qDebug() << Q_FUNC_INFO << "Update 2 SQL:" << update;

      if ( !q.exec(update) )
         throw QString("failed to insert new instruction recipe");
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text());
      q.finish();
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   q.finish();

   emit in->changed( in->metaProperty("instructionNumber"), pos );
}

QList<int> Database::ancestoralIds(Recipe const* descendant)
{
   QList<int> ret;
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   // Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn
   // The first select initializes the chain, so we get the id and ancestor_id
   // of the current recipe. The UNION ALL does the recursive trick, where
   // find all the ancestors. The query stops when recipe.id = ancestor_id.
   //
   // WITH RECURSIVE ancestor(id,ancestor_id) AS
   //  (SELECT id,ancestor_id FROM recipe r WHERE r.id = [key]
   //   UNION ALL
   //   SELECT r.id, r.ancestor_id FROM ancestor a, recipe r
   //     WHERE r.id = a.ancestor_id AND r.ancestor_id != a.id )
   // SELECT id FROM ancestor
   QString recursiveQuery =
      QString("WITH RECURSIVE ancestor(%1,%2) AS "
               "(SELECT %1,%2 FROM %3 r WHERE r.%1 = %4 "
                "UNION ALL "
                "SELECT r.%1, r.%2 FROM ancestor a, recipe r "
                "WHERE r.%1 = a.%2 AND r.%2 != a.%1 ) "
              "SELECT %1 from ancestor")
      .arg( tbl->keyName() )
      .arg( tbl->foreignKeyToColumn(PropertyNames::Recipe::ancestorId) )
      .arg( tbl->tableName() )
      .arg( descendant->key() );

   QSqlQuery q( sqlDatabase() );

   try {
      if ( ! q.exec(recursiveQuery) ) {
         throw QString("Could not find ancestoral recipes");
      }
      while ( q.next() ) {
         ret.append( q.record().value( tbl->keyName() ).toInt() );
      }
   }
   catch( QString e ) {
      qCritical() << Q_FUNC_INFO << e << q.lastError().text();
      abort();
   }

   return ret;
}

QList<BrewNote*> Database::brewNotes(Recipe const* parent, bool recurse)
{
   QList<int> ancestors;
   QString inList;

   QList<BrewNote*> ret;
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);

   if ( recurse ) {
      ancestors = ancestoralIds(parent);
      inList = QString("%1").arg(ancestors.takeFirst());
      foreach(int key, ancestors) {
         inList.append(QString(",%1").arg(key));
      }
   }
   else {
      inList = QString("%1").arg(parent->key());
   }

   // this may not be the most efficient way to query. It compresses the code
   // nicely, but maybe that's the wrong optimization
   //  deleted = false and recipe_id in (LIST)
   QString filterString = QString("%1 = %2 AND %3 in (%4)")
         .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted))
         .arg(Brewtarget::dbFalse())
         .arg(tbl->recipeIndexName())
         .arg(inList);

   getElements(ret, filterString, Brewtarget::BREWNOTETABLE, allBrewNotes);

   return ret;
}

QList<Fermentable*> Database::fermentables(Recipe const* parent)
{
   QList<Fermentable*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::FERMINRECTABLE);
   // recipe_id = [parent->key]
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->key());

   getElements(ret,filter, Brewtarget::FERMINRECTABLE, allFermentables, inrec->inRecIndexName());

   return ret;
}

QList<Hop*> Database::hops(Recipe const* parent)
{
   QList<Hop*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::HOPINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->key());

   getElements(ret,filter, Brewtarget::HOPINRECTABLE, allHops, inrec->inRecIndexName());

   return ret;
}

QList<Misc*> Database::miscs(Recipe const* parent)
{
   QList<Misc*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::MISCINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->key());

   getElements(ret,filter, Brewtarget::MISCINRECTABLE, allMiscs, inrec->inRecIndexName());

   return ret;
}

Equipment* Database::equipment(Recipe const* parent)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);
   int id = get( tbl, parent->key(), tbl->foreignKeyToColumn(kpropEquipmentId)).toInt();

   if( allEquipments.contains(id) )
      return allEquipments[id];
   else
      return nullptr;
}

Style* Database::style(Recipe const* parent)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);
   int id = get( tbl, parent->key(), tbl->foreignKeyToColumn(kpropStyleId)).toInt();

   if( allStyles.contains(id) )
      return allStyles[id];
   else
      return nullptr;
}

Style* Database::styleById(int styleId )
{
   if( allStyles.contains(styleId) )
      return allStyles[styleId];
   else
      return nullptr;
}

Mash* Database::mash( Recipe const* parent )
{
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);
   int mashId = get( tbl, parent->key(), tbl->foreignKeyToColumn(kpropMashId)).toInt();

   if( allMashs.contains(mashId) )
      return allMashs[mashId];
   else
      return nullptr;
}

QList<MashStep*> Database::mashSteps(Mash const* parent)
{
   QList<MashStep*> ret;
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);

   // mash_id = [parent->key] AND deleted = false order by step_number ASC
   QString filterString = QString("%1 = %2 AND %3 = %4 order by %5 ASC")
         .arg(tbl->foreignKeyToColumn())
         .arg(parent->key())
         .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted))
         .arg(Brewtarget::dbFalse())
         .arg(tbl->propertyToColumn(PropertyNames::MashStep::stepNumber));

   getElements(ret, filterString, Brewtarget::MASHSTEPTABLE, allMashSteps);

   return ret;
}

QList<Instruction*> Database::instructions( Recipe const* parent )
{
   QList<Instruction*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::INSTINRECTABLE);
   // recipe_id = [parent->key] ORDER BY instruction_number ASC
   QString filter = QString("%1 = %2 ORDER BY %3 ASC")
         .arg( inrec->recipeIndexName())
         .arg(parent->key())
         .arg( inrec->propertyToColumn(kpropInstructionNumber));

   getElements(ret,filter,Brewtarget::INSTINRECTABLE,allInstructions,inrec->inRecIndexName());

   return ret;
}

QList<Water*> Database::waters(Recipe const* parent)
{
   QList<Water*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::WATERINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->key());

   getElements(ret,filter,Brewtarget::WATERINRECTABLE,allWaters,inrec->inRecIndexName());

   return ret;
}

QList<Salt*> Database::salts(Recipe const* parent)
{
   QList<Salt*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::SALTINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->key());

   getElements(ret,filter,Brewtarget::SALTINRECTABLE,allSalts,inrec->inRecIndexName());

   return ret;
}

QList<Yeast*> Database::yeasts(Recipe const* parent)
{
   QList<Yeast*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::YEASTINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->key());

   getElements(ret,filter,Brewtarget::YEASTINRECTABLE,allYeasts,inrec->inRecIndexName());

   return ret;
}

// Named constructors =========================================================

BrewNote* Database::newBrewNote(BrewNote* other, bool signal)
{
   BrewNote* tmp = copy<BrewNote>(other, &allBrewNotes);

   if ( tmp && signal ) {
      emit changed( metaProperty("brewNotes"), QVariant() );
      emit createdSignal(tmp);
   }

   return tmp;
}

BrewNote* Database::newBrewNote(Recipe* parent, bool signal)
{
   BrewNote* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newNamedEntity(&allBrewNotes);
      TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);

      sqlUpdate( Brewtarget::BREWNOTETABLE,
               QString("%1=%2").arg(tbl->recipeIndexName()).arg(parent->key()),
               QString("%1=%2").arg(tbl->keyName()).arg(tmp->key()) );

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   tmp->setDisplay(true);
   if ( signal )
   {
      emit changed( metaProperty("brewNotes"), QVariant() );
      emit createdSignal(tmp);
   }

   return tmp;
}

Equipment* Database::newEquipment(Equipment* other)
{
   Equipment* tmp;

   if (other)
      tmp = copy(other, &allEquipments);
   else
      tmp = newNamedEntity(&allEquipments);

   if ( tmp ) {
      emit changed( metaProperty("equipments"), QVariant() );
      emit createdSignal(tmp);
   }
   else {
      QString msg = QString("%1 couldn't create newEquipment %2")
                     .arg(Q_FUNC_INFO)
                     .arg( other != nullptr ? other->name() : "");
      qCritical() << msg;
   }

   return tmp;
}

Fermentable* Database::newFermentable(Fermentable* other, bool add_inventory)
{
   Fermentable* tmp;
   add_inventory = add_inventory || other == nullptr;

   sqlDatabase().transaction();
   try {
      if (other != nullptr) {
         tmp = copy(other, &allFermentables);
      }
      else {
         tmp = newNamedEntity(&allFermentables);
      }

      if ( add_inventory ) {
         int invkey = newInventory( dbDefn->table(Brewtarget::FERMTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      abort();
   }

   sqlDatabase().commit();

   if ( tmp ) {
      emit changed( metaProperty("fermentables"), QVariant() );
      emit createdSignal(tmp);
   }
   else {
      qCritical() << QString("%1 couldn't copy %2").arg(Q_FUNC_INFO).arg(other->name());
   }

   return tmp;
}

Hop* Database::newHop(Hop* other, bool add_inventory )
{
   Hop* tmp;
   add_inventory = add_inventory || other == nullptr;

   sqlDatabase().transaction();
   try {
      if ( other != nullptr )
         tmp = copy(other, &allHops);
      else
         tmp = newNamedEntity(&allHops);

      if ( add_inventory ) {
         int invkey = newInventory( dbDefn->table(Brewtarget::HOPTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      abort();
   }

   sqlDatabase().commit();

   if ( tmp ) {
      emit changed( metaProperty("hops"), QVariant() );
      emit createdSignal(tmp);
   }
   else {
      qCritical() << QString("%1 could not %2 hop")
            .arg(Q_FUNC_INFO)
            .arg( other ? "copy" : "create");
   }

   return tmp;
}

Instruction* Database::newInstruction(Recipe* rec)
{
   Instruction* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newNamedEntity(&allInstructions);
      tmp->setRecipe(rec);

      // Add without copying to "instruction_in_recipe". We already have a
      // transaction open, so tell addIng to not worry about it
      tmp = addNamedEntityToRecipe<Instruction>(rec,tmp,true,nullptr,false,false);
   }
   catch ( QString e ) {
      qCritical() << QString("%1 %2").arg( Q_FUNC_INFO ).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   // Database's instructions have changed.
   sqlDatabase().commit();
   emit changed( metaProperty("instructions"), QVariant() );

   return tmp;
}

// needs fixed
int Database::instructionNumber(Instruction const* in)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::INSTINRECTABLE);
   QString colName = tbl->propertyToColumn(kpropInstructionNumber);
   // SELECT instruction_number FROM instruction_in_recipe WHERE instruction_id=[in->key]
   QString query = QString("SELECT %1 FROM %2 WHERE %3=%4")
         .arg(colName)
         .arg(tbl->tableName())
         .arg(tbl->inRecIndexName())
         .arg(in->key());

   QSqlQuery q(query,sqlDatabase());

   if( q.next() )
      return q.record().value(colName).toInt();
   else
      return 0;
}

Mash* Database::newMash(Mash* other, bool displace)
{
   Mash* tmp;

   try {
      if ( other ) {
         sqlDatabase().transaction();
         tmp = copy<Mash>(other, &allMashs);
      }
      else {
         tmp = newNamedEntity(&allMashs);
      }

      if ( other ) {
         // Just copying the Mash isn't enough. We need to copy the mashsteps too
         duplicateMashSteps(other,tmp);

         // Connect tmp to parent, removing any existing mash in parent.
         // This doesn't really work. It simply orphans the old mash and its
         // steps.
         if( displace ) {
            TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);
            sqlUpdate( Brewtarget::RECTABLE,
                       QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId)).arg(tmp->key()),
                       QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId)).arg(other->key()));
         }
      }
   }
   catch (QString e) {
      if ( other )
         sqlDatabase().rollback();
      throw;
   }

   if ( other ) {
      sqlDatabase().commit();
   }

   emit changed( metaProperty("mashs"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Mash* Database::newMash(Recipe* parent, bool transact)
{
   Mash* tmp;

   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      tmp = newNamedEntity(&allMashs);

      // Connect tmp to parent, removing any existing mash in parent.
      TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);
      sqlUpdate( Brewtarget::RECTABLE,
                 QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId)).arg(tmp->key()),
                 QString("%1=%2").arg(tbl->keyName()).arg(parent->key()));
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }

   emit changed( metaProperty("mashs"), QVariant() );
   emit createdSignal(tmp);

   connect( tmp, SIGNAL(changed(QMetaProperty,QVariant)), parent, SLOT(acceptMashChange(QMetaProperty,QVariant)) );
   return tmp;
}

// If we are doing triggers for instructions, why aren't we doing triggers for
// mash steps?
MashStep* Database::newMashStep(Mash* mash, bool connected)
{
   // NOTE: we have unique(mash_id,step_number) constraints on this table,
   // so may have to pay special attention when creating the new record.
   MashStep* tmp;
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   // step_number = (SELECT COALESCE(MAX(step_number)+1,0) FROM mashstep
   // WHERE deleted=false AND mash_id=[mash->key] )
   QString coalesce = QString( "%1 = (SELECT COALESCE(MAX(%1)+1,0) FROM %2 "
                                      "WHERE %3=%4 AND %5=%6 )")
                        .arg(tbl->propertyToColumn(PropertyNames::MashStep::stepNumber))
                        .arg(tbl->tableName())
                        .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted))
                        .arg(Brewtarget::dbFalse())
                        .arg(tbl->foreignKeyToColumn())
                        .arg(mash->key());

   sqlDatabase().transaction();

   QSqlQuery q(sqlDatabase());
   q.setForwardOnly(true);

   // mashsteps are weird, because we have to do the linking between step and
   // mash
   try {
      tmp = newNamedEntity(&allMashSteps);

      // we need to set the mash_id first
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 QString("%1=%2 ").arg(tbl->foreignKeyToColumn()).arg(mash->key()),
                 QString("%1=%2").arg(tbl->keyName()).arg(tmp->key())
               );

      // Just sets the step number within the mash to the next available number.
      // we need coalesce here instead of isnull. coalesce is SQL standard, so
      // should be more widely supported than isnull
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 coalesce,
                 QString("%1=%2").arg(tbl->keyName()).arg(tmp->key()));
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   if ( connected )
      connect( tmp, SIGNAL(changed(QMetaProperty,QVariant)), mash, SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );

   emit changed( metaProperty("mashs"), QVariant() );
   emit mash->mashStepsChanged();
   return tmp;
}

Misc* Database::newMisc(Misc* other, bool add_inventory)
{
   Misc* tmp;
   add_inventory = add_inventory || other == nullptr;

   sqlDatabase().transaction();
   try {
      if ( other != nullptr ) {
        tmp = copy(other, &allMiscs);
      }
      else {
         tmp = newNamedEntity(&allMiscs);
      }

      if ( add_inventory ) {
         int invkey = newInventory( dbDefn->table(Brewtarget::MISCTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      abort();
   }

   sqlDatabase().commit();

   if ( tmp ) {
      emit changed( metaProperty("miscs"), QVariant() );
      emit createdSignal(tmp);
   }
   else {
      qCritical() << QString("%1 could not %2 misc")
            .arg(Q_FUNC_INFO)
            .arg( other ? "copy" : "create");
   }

   return tmp;
}

Recipe* Database::newRecipe(QString name)
{
   Recipe* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newNamedEntity(&allRecipes);

      newMash(tmp,false);
   }
   catch (QString e ) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   try {
      // setting it in the DB doesn't set it in the cache. This makes sure the
      // name is in the cache before we throw the signal
      tmp->setName(name,true);
      tmp->setDisplay(true);
      tmp->setDeleted(false);
   }
   catch (QString e ) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

   sqlDatabase().commit();
   emit changed( metaProperty("recipes"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Recipe* Database::copyRecipeExcept(Recipe *other, NamedEntity *except)
{
   Recipe *tmp;
   sqlDatabase().transaction();

   try {
      tmp = copy<Recipe>(other, &allRecipes, true);

      // the cast will return nullptr if the NamedEntity cannot be cast. The
      // addToRecipe will do the right thing and simply copy it all.
      addToRecipe( tmp, other->fermentables(), qobject_cast<Fermentable*>(except), false);
      addToRecipe( tmp, other->hops(),         qobject_cast<Hop*>(except),         false);
      addToRecipe( tmp, other->miscs(),        qobject_cast<Misc*>(except),        false);
      addToRecipe( tmp, other->yeasts(),       qobject_cast<Yeast*>(except),       false);

      // if the exception cannot be cast to a equipment/mash/style, then copy
      // the equipment/mash/style to the clone
      if ( qobject_cast<Equipment*>(except) == nullptr ) {
         addToRecipe(tmp, other->equipment(), false, false);
      }
      if ( qobject_cast<Mash*>(except) == nullptr ) {
         addToRecipe(tmp, other->mash(), false, false);
      }
      if ( qobject_cast<Style*>(except) == nullptr ) {
         addToRecipe(tmp, other->style(), false, false);
      }

      setAncestor( tmp, other, false );
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO << e;
      sqlDatabase().rollback();
      abort();
   }

   sqlDatabase().commit();
   return tmp;
}

bool Database::wantsVersion(Recipe* rec)
{
   bool ret = false;

   // if the user has said they don't want versioning, just return false
   if ( ! Brewtarget::option("versioning", false).toBool() ) {
      return ret;
   }

   QSqlQuery q(sqlDatabase());
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);

   // select id from brewnote where recipe_id = [key]
   QString queryExistence = QString("SELECT %1 FROM %2 WHERE %3=%4")
                              .arg( tbl->keyName() )
                              .arg( tbl->tableName() )
                              .arg( tbl->foreignKeyToColumn(kpropRecipeId) )
                              .arg( rec->key() );
   try {
      qDebug() << Q_FUNC_INFO << queryExistence;
      if ( ! q.exec(queryExistence) ) {
         throw QString("Could not query existence. Seek theological help");
      }
      ret =  q.next();
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO << e << q.lastError().text();
   }

   q.finish();
   return ret;
}

// nice way to handle defining an ancestor. This does all the work -- setting
// ancestor_id, locking the ancestor and making the ancestor not display
void Database::setAncestor(Recipe* descendant, Recipe* ancestor, bool transact)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      QSqlQuery q(sqlDatabase());

      // update recipe set ancestor_id = [ancestor->key] where id = [descendant->key]
      QString set_ancestor = QString("UPDATE %1 SET %2 = %3 where %4 = %5")
                                .arg(tbl->tableName())
                                .arg(tbl->foreignKeyToColumn(kpropAncestorId))
                                .arg(ancestor->key())
                                .arg(tbl->keyName())
                                .arg(descendant->key());
      // update recipe set display = false where id = [ancestor_id]
      QString set_display = QString("UPDATE %1 SET %2 = %3 where %4 = %5")
                               .arg(tbl->tableName())
                               .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::display))
                               .arg( ancestor == descendant ? Brewtarget::dbTrue() : Brewtarget::dbFalse())
                               .arg(tbl->keyName())
                               .arg(ancestor->key());
      QString set_locked = QString("UPDATE %1 SET %2 = %3 where %4 = %5")
                              .arg(tbl->tableName())
                              .arg(tbl->propertyToColumn(PropertyNames::Recipe::locked))
                              .arg( ancestor == descendant ? Brewtarget::dbTrue() : Brewtarget::dbFalse())
                              .arg(tbl->keyName())
                              .arg(ancestor->key());

      if ( ! q.exec( set_ancestor ) ) {
         throw QString("Could not define ancestor. %1 : %2")
                  .arg(q.lastQuery())
                  .arg(q.lastError().text());
      }

      if ( ! q.exec( set_display ) ) {
         throw QString("Could not set ancestor display. %1 : %2")
                  .arg(q.lastQuery())
                  .arg(q.lastError().text());
      }

      if ( ! q.exec( set_locked ) ) {
         throw QString("Could not lock ancestor. %1 : %2")
                  .arg(q.lastQuery())
                  .arg(q.lastError().text());
      }
      q.finish();
   }
   catch( QString e ) {
      if ( transact )
         sqlDatabase().rollback();
      qCritical() << Q_FUNC_INFO << e;
      abort();
   }
   sqlDatabase().commit();

   // some housekeeping to keep my caches synced
   ancestor->setCacheOnly(true);
   ancestor->setDisplay( ancestor == descendant );
   ancestor->setLocked( ancestor != descendant );
   ancestor->setCacheOnly(false);
}

Recipe* Database::newRecipe(Recipe* other, bool ancestor)
{
   Recipe* tmp;

   sqlDatabase().transaction();
   try {
      tmp = copy<Recipe>(other, &allRecipes, true);

      // Copy fermentables, hops, miscs and yeasts. We've the convenience
      // methods, so use them? And now I have to instruct all of them to not do
      // transactions either. -4 SAN points!
      addToRecipe( tmp, other->fermentables(), false);
      addToRecipe( tmp, other->hops(), false);
      addToRecipe( tmp, other->miscs(), false);
      addToRecipe( tmp, other->yeasts(), false);

      // Copy style/mash/equipment
      // Style or equipment might be non-existent but these methods handle that.
      addToRecipe( tmp, other->equipment(), false, false);
      addToRecipe( tmp, other->mash(), false, false);
      addToRecipe( tmp, other->style(), false, false);

      // if other is an ancestor, we need to set display false on other and
      // link the two.
      if ( ancestor ) {
         setAncestor(tmp,other,false);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   emit changed( metaProperty("recipes"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Style* Database::newStyle(Style* other)
{
   Style* tmp;

   try {
      tmp = copy(other, &allStyles);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("styles"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Style* Database::newStyle(QString name)
{
   Style* tmp;

   try {
      tmp = newNamedEntity(&allStyles);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   try {
      // setting it in the DB doesn't set it in the cache. This makes sure the
      // name is in the cache before we throw the signal
      tmp->setName(name,true);
      tmp->setDisplay(true);
      tmp->setDeleted(false);
   }

   catch (QString e ) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

   emit changed( metaProperty("styles"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Water* Database::newWater(Water* other)
{
   Water* tmp;

   try {
      if ( other )
         tmp = copy(other,&allWaters);
      else
         tmp = newNamedEntity(&allWaters);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("waters"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Salt* Database::newSalt(Salt* other)
{
   Salt* tmp;

   try {
      if ( other )
         tmp = copy(other,&allSalts);
      else
         tmp = newNamedEntity(&allSalts);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("salts"), QVariant() );
   emit createdSignal(tmp);

   return tmp;
}

Yeast* Database::newYeast(Yeast* other, bool add_inventory)
{
   Yeast* tmp;
   add_inventory = add_inventory || other == nullptr;

   sqlDatabase().transaction();
   try {
      if (other != nullptr) {
         tmp = copy(other, &allYeasts);
      }
      else {
         tmp = newNamedEntity(&allYeasts);
      }

      if (add_inventory) {
         int invkey = newInventory( dbDefn->table(Brewtarget::YEASTTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   if ( tmp ) {
      emit changed( metaProperty("yeasts"), QVariant() );
      emit createdSignal(tmp);
   }
   else {
      qCritical() << QString("%1 could not %2 yeast")
            .arg(Q_FUNC_INFO)
            .arg( other ? "copy" : "create");
   }

   return tmp;
}

NamedEntity* Database::clone( Recipe *rec, NamedEntity *donor, QString propName, QVariant value )
{
   if ( qobject_cast<Equipment*>(donor) != nullptr ) {
      Equipment* tmp = replicant(donor, &allEquipments, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   if ( qobject_cast<Fermentable*>(donor) != nullptr ) {
      Fermentable* tmp = replicant(donor, &allFermentables, propName, value);
      addToRecipe( rec, tmp, true );
      return tmp;
   }

   if ( qobject_cast<Hop*>(donor) != nullptr ) {
      Hop* tmp = replicant(donor, &allHops, propName, value);
      addToRecipe( rec, tmp, true );
      return tmp;
   }

   if ( qobject_cast<Mash*>(donor) != nullptr ) {
      Mash* tmp = replicant(donor, &allMashs, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   // What does this actually do? It gets the last mashstep from the recipe?
   // Huh?
   if ( qobject_cast<MashStep*>(donor) != nullptr ) {
      MashStep* whiskey = qobject_cast<MashStep*>(donor);
      MashStep* tmp = rec->mash()->mashSteps().at( whiskey->stepNumber() - 1);
      return tmp;
   }

   if ( qobject_cast<Misc*>(donor) != nullptr ) {
      Misc* tmp = replicant(donor, &allMiscs, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   if ( qobject_cast<Salt*>(donor) != nullptr ) {
      Salt* tmp = replicant(donor, &allSalts, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   if ( qobject_cast<Style*>(donor) != nullptr ) {
      Style* tmp = replicant(donor, &allStyles, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   if ( qobject_cast<Water*>(donor) != nullptr ) {
      Water* tmp = replicant(donor, &allWaters, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   if ( qobject_cast<Yeast*>(donor) != nullptr ) {
      Yeast* tmp = replicant(donor, &allYeasts, propName, value);
      addToRecipe(rec, tmp, true);
      return tmp;
   }

   return nullptr;
}

// newWhatever handles the allWhatever hash, so I can copy the ingredient and
// insert into the hash in a single step. I don't think these can be templated
// because we change the names
NamedEntity* Database::clone( NamedEntity* donor, Recipe* rec )
{
   abort();

}

int Database::insertElement(NamedEntity * ins)
{
   // Check whether this ingredient is already in the DB.  If so, bail here.
   if (this->isStored(*ins)) {
      qDebug() << Q_FUNC_INFO << "Already stored";
      return ins->key();
   }

   int key;
   QSqlQuery q( sqlDatabase() );

   TableSchema* schema = dbDefn->table(ins->table());
   QString insertQ = schema->generateInsertProperties(Brewtarget::dbType());
   QStringList allProps = schema->allProperties();

   qDebug() << Q_FUNC_INFO << "SQL:" << insertQ;
   q.prepare(insertQ);

   QString sqlParameters;
   QTextStream sqlParametersConcat(&sqlParameters);
   foreach (QString prop, allProps) {
      QString pname = schema->propertyName(prop);
      QVariant val_to_ins = ins->property(pname.toUtf8().data());

      if ( ins->table() == Brewtarget::BREWNOTETABLE && prop == PropertyNames::BrewNote::brewDate ) {
         val_to_ins = val_to_ins.toString();
      }
      // I've arranged it such that the bindings are on the property names. It simplifies a lot
      q.bindValue( QString(":%1").arg(prop), val_to_ins);
      sqlParametersConcat << QString(":%1").arg(prop) << " = " << val_to_ins.toString() << "\n\t";
   }
   qDebug() << Q_FUNC_INFO << "SQL Parameters: " << *sqlParametersConcat.string();

   try {
      if ( ! q.exec() ) {
         throw QString("could not insert a record into %1: %2")
               .arg(schema->tableName())
               .arg(insertQ);
      }

      qDebug() << Q_FUNC_INFO << "Query succeeded";
      key = q.lastInsertId().toInt();
      q.finish();
   }
   catch (QString e) {
      sqlDatabase().rollback();
      qCritical() << QString("%1 %2 %3").arg(Q_FUNC_INFO).arg(e).arg( q.lastError().text());
      abort();
   }
   ins->m_key = key;

   return key;
}


// I need to break each of these out because of our signals. I will someday
// find a way to determine which signals are sent, when, from what and then
// there will come a Purge
int Database::insertStyle(Style* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allStyles.insert(key,ins);

   emit changed( metaProperty("styles"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertEquipment(Equipment* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allEquipments.insert(key,ins);
   emit changed( metaProperty("equipments"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertFermentable(Fermentable* ins)
{
   int key;
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);
      // I think this must go here -- we need the inventory id value written
      // to the db, and we don't have the fermentable id until now
      int invKey = newInventory(dbDefn->table(Brewtarget::FERMTABLE));
      ins->setInventoryId(invKey);
   }
   catch( QString e ) {
      qCritical() << e;
      throw;
   }

   sqlDatabase().commit();
   allFermentables.insert(key,ins);
   emit changed( metaProperty("fermentables"), QVariant() );
   emit createdSignal(ins);
   return key;
}

int Database::insertHop(Hop* ins)
{
   int key;
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);
      int invKey = newInventory(dbDefn->table(Brewtarget::HOPTABLE));
      ins->setInventoryId(invKey);
   }
   catch( QString e ) {
      qCritical() << e;
      throw;
   }

   sqlDatabase().commit();
   allHops.insert(key,ins);
   emit changed( metaProperty("hops"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertInstruction(Instruction* ins, Recipe* parent)
{
   int key;
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);

      ins = addNamedEntityToRecipe<Instruction>(parent,ins,true,nullptr,false,false);

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   allInstructions.insert(key,ins);
   emit changed( metaProperty("instructions"), QVariant() );

   return key;
}

int Database::insertMash(Mash* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allMashs.insert(key,ins);
   emit changed( metaProperty("mashs"), QVariant() );
   emit createdSignal(ins);

   return key;
}

// this one will be harder, because we have to link the mashstep to the parent
// mash
int Database::insertMashStep(MashStep* ins, Mash* parent)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   // step_number = (SELECT COALESCE(MAX(step_number)+1,0) FROM mashstep WHERE deleted=false AND mash_id=[key] )
   QString coalesce = QString( "%1 = (SELECT COALESCE(MAX(%1)+1,0) FROM %2 WHERE %3=%4 AND %5=%6 )")
                        .arg(tbl->propertyToColumn(PropertyNames::MashStep::stepNumber))
                        .arg(tbl->tableName())
                        .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted))
                        .arg(Brewtarget::dbFalse())
                        .arg(tbl->foreignKeyToColumn())
                        .arg(parent->key());
   int key;

   sqlDatabase().transaction();
   try {
      // we need to insert the mashstep into the db first to get the key
      key = insertElement(ins);
      ins->setCacheOnly(false);

      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 QString("%1=%2 ").arg(tbl->foreignKeyToColumn()).arg(parent->key()),
                 QString("%1=%2").arg(tbl->keyName()).arg(ins->key())
               );

      // Just sets the step number within the mash to the next available number.
      // we need coalesce here instead of isnull. coalesce is SQL standard, so
      // should be more widely supported than isnull
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 coalesce,
                 QString("%1=%2").arg(tbl->keyName()).arg(ins->key()));
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   allMashSteps.insert(key,ins);
   connect( ins, SIGNAL(changed(QMetaProperty,QVariant)), parent,
                 SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );

   emit changed( metaProperty("mashs"), QVariant() );
   emit parent->mashStepsChanged();

   return key;
}

int Database::insertMisc(Misc* ins)
{
   int key;
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);

      int invKey = newInventory(dbDefn->table(Brewtarget::MISCTABLE));
      ins->setInventoryId(invKey);
   }
   catch( QString e ) {
      qCritical() << e;
      throw;
   }

   sqlDatabase().commit();
   allMiscs.insert(key,ins);
   emit changed( metaProperty("miscs"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertRecipe(Recipe* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allRecipes.insert(key,ins);
   emit changed( metaProperty("recipes"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertYeast(Yeast* ins)
{
   int key;
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);
      int invKey = newInventory(dbDefn->table(Brewtarget::YEASTTABLE));
      ins->setInventoryId(invKey);
   }
   catch( QString e ) {
      qCritical() << e;
      throw;
   }

   sqlDatabase().commit();
   allYeasts.insert(key,ins);
   emit changed( metaProperty("yeasts"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertWater(Water* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allWaters.insert(key,ins);
   emit changed( metaProperty("waters"), QVariant() );
   emit createdSignal(ins);

   return key;
}

int Database::insertSalt(Salt* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allSalts.insert(key,ins);
   emit changed( metaProperty("salts"), QVariant() );
   emit createdSignal(ins);

   return key;
}

// This is more similar to a mashstep in that we need to link the brewnote to
// the parent recipe.
int Database::insertBrewNote(BrewNote* ins, Recipe* parent) {
   // It's a coding error to try to insert a BrewNote without a Recipe
   Q_ASSERT(nullptr != parent);
   // It's a coding error to try to insert a null BrewNote!
   Q_ASSERT(nullptr != ins);

   int key;
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);

      QString const setClause = QString("%1=%2").arg(tbl->foreignKeyToColumn()).arg(parent->key());
      QString const whereClause = QString("%1=%2").arg(tbl->keyName()).arg(key);

      sqlUpdate(Brewtarget::BREWNOTETABLE, setClause, whereClause);

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   qDebug() << Q_FUNC_INFO << "DB update succeeded; key =" << key;
   sqlDatabase().commit();

   this->allBrewNotes.insert(key,ins);
   emit changed( metaProperty("brewNotes"), QVariant() );
   emit createdSignal(ins);

   return key;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


QMetaProperty Database::metaProperty(const char* name)
{
   return metaObject()->property(metaObject()->indexOfProperty(name));
}


void Database::deleteRecord( NamedEntity* object )
{
   try {
      updateEntry( object, PropertyNames::NamedEntity::deleted, Brewtarget::dbTrue(), true);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

}


template<class T> T* Database::addNamedEntityToRecipe(
   Recipe* rec,
   NamedEntity* ing,
   bool noCopy,
   QHash<int,T*>* keyHash,
   bool doNotDisplay,
   bool transact
)
{
   qDebug() <<
     Q_FUNC_INFO << "noCopy:" << (noCopy ? "true" : "false") << ", doNotDisplay:" <<
     (doNotDisplay ? "true" : "false") << ", transact" << (transact ? "true" : "false");

   T* newIng = nullptr;
   QString propName, relTableName, ingKeyName, childTableName;
   TableSchema* table;
   TableSchema* child;
   TableSchema* inrec;
   const QMetaObject* meta = ing->metaObject();
   int ndx = meta->indexOfClassInfo("signal");

   if( rec == nullptr || ing == nullptr )
      return nullptr;

   if ( rec->locked() )
      return nullptr;


   // TRANSACTION BEGIN, but only if requested. Yeah. Had to go there.
   if ( transact ) {
      sqlDatabase().transaction();
   }

   // Queries have to be created inside transactional boundaries
   QSqlQuery q(sqlDatabase());
   try {
      if ( ndx != -1 ) {
         propName  = meta->classInfo(ndx).value();
      }
      else {
         throw QString("could not locate classInfo for signal on %2").arg(meta->className());
      }

      table = dbDefn->table( dbDefn->classNameToTable(meta->className()) );
      child = dbDefn->table( table->childTable() );
      inrec = dbDefn->table( table->inRecTable() );
      // Ensure this ingredient is not already in the recipe.
      QString select = QString("SELECT %5 from %1 WHERE %2=%3 AND %5=%4")
                           .arg(inrec->tableName())
                           .arg(inrec->inRecIndexName())
                           .arg(ing->key())
                           .arg(reinterpret_cast<NamedEntity*>(rec)->key())
                           .arg(inrec->recipeIndexName());
      qDebug() << Q_FUNC_INFO << "NamedEntity in recipe search:" << select;
      if (! q.exec(select) ) {
         throw QString("Couldn't execute ingredient in recipe search: Query: %1 error: %2")
            .arg(q.lastQuery()).arg(q.lastError().text());
      }

      // this probably should just be a warning, not a throw?
      if ( q.next() ) {
         throw QString("NamedEntity already exists in recipe." );
      }

      q.finish();

      if ( noCopy ) {
         newIng = qobject_cast<T*>(ing);
         // Any ingredient part of a recipe shouldn't be visible, unless otherwise requested.
         // Not sure I like this. It's a long call stack just to end up back
         // here
         ing->setDisplay(! doNotDisplay );
         // Ensure the ingredient exists in the DB - eg if this is something that was previously deleted and we are
         // adding it back via Undo.  NB: The reinsertion will change the ingredient's key.
         ing->insertInDatabase();
      }
      else
      {
         newIng = copy<T>(ing, keyHash, false);
         if ( newIng == nullptr ) {
            throw QString("error copying ingredient");
         }
         qDebug() << QString("%1 Copy %2 #%3 to %2 #%4").arg(Q_FUNC_INFO).arg(meta->className()).arg(ing->key()).arg(newIng->key());
         newIng->setParent(*ing);
      }

      // Put this (ing,rec) pair in the <ing_type>_in_recipe table.
      q.setForwardOnly(true);

      // Link the ingredient to the recipe in the DB
      // Eg, for a fermentable, this is INSERT INTO fermentable_in_recipe (fermentable_id, recipe_id) VALUES (:ingredient, :recipe)
      QString insert = QString("INSERT INTO %1 (%2, %3) VALUES (:ingredient, :recipe)")
               .arg(inrec->tableName())
               .arg(inrec->inRecIndexName(Brewtarget::dbType()))
               .arg(inrec->recipeIndexName());

      q.prepare(insert);
      q.bindValue(":ingredient", newIng->key());
      q.bindValue(":recipe", rec->key());

      qDebug() << QString("%1 Link ingredient to recipe: %2 with args %3, %4").arg(Q_FUNC_INFO).arg(insert).arg(newIng->key()).arg(rec->key());

      if ( ! q.exec() ) {
         throw QString("%2 : %1.").arg(q.lastQuery()).arg(q.lastError().text());
      }

      emit rec->changed( rec->metaProperty(propName), QVariant() );

      q.finish();

      //Put this in the <ing_type>_children table.
      // instructions and salts have no children.
      if( inrec->dbTable() != Brewtarget::INSTINRECTABLE && inrec->dbTable() != Brewtarget::SALTINRECTABLE ) {
         /*
          * The parent to link to depends on where the ingredient is copied from:
          * - A fermentable from the fermentable table -> the ID of the fermentable.
          * - An ingredient from another recipe -> the ID of the ingredient's parent.
          *
          * This is required:
          * - When deleting the ingredient from the original recipe no longer fails.
          *   Else if fails due to a foreign key constrain.
          */
         int key = ing->key();
         QString parentChildSql = QString("SELECT %1 FROM %2 WHERE %3=%4")
               .arg(child->parentIndexName())
               .arg(child->tableName())
               .arg(child->childIndexName())
               .arg(key);
         q.prepare(parentChildSql);
         qDebug() << QString("%1 Parent-Child find: %2").arg(Q_FUNC_INFO).arg(parentChildSql);
         if (q.exec() && q.next()) {
            key = q.record().value(child->parentIndexName()).toInt();
         }
         q.finish();

         insert = QString("INSERT INTO %1 (%2, %3) VALUES (:parent, :child)")
               .arg(child->tableName())
               .arg(child->parentIndexName())
               .arg(child->childIndexName());

         q.prepare(insert);
         q.bindValue(":parent", key);
         q.bindValue(":child", newIng->key());

         qDebug() <<
            Q_FUNC_INFO << "Parent-Child Insert:" << insert << "with args" << key << "," << newIng->key();

         if ( ! q.exec() ) {
            throw QString("%1 %2.").arg(q.lastQuery()).arg(q.lastError().text());
         }

         emit rec->changed( rec->metaProperty(propName), QVariant() );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(QString("Q_FUNC_INFO")).arg(e);
      q.finish();
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }
   q.finish();
   if ( transact )
      sqlDatabase().commit();

   return newIng;
}



// NOTE: This really should be in a transaction, but I am going to leave that
// as the responsibility of the calling method. I am not comfortable with this
// idea.
void Database::duplicateMashSteps(Mash *oldMash, Mash *newMash)
{
   QList<MashStep*> tmpMS = mashSteps(oldMash);
   QList<MashStep*>::iterator ms;

   try {
      for( ms=tmpMS.begin(); ms != tmpMS.end(); ++ms)
      {
         // Copy the old mash step.
         MashStep* newStep = copy<MashStep>(*ms,&allMashSteps);
         TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);

         // Put it in the new mash.
         sqlUpdate( Brewtarget::MASHSTEPTABLE,
                      QString("%1=%2").arg(tbl->foreignKeyToColumn()).arg(newMash->key()),
                      QString("%1=%2").arg(tbl->keyName()).arg(newStep->key())
                  );
         // Make the new mash pay attention to the new step.
         connect( newStep, &NamedEntity::changed,
                  newMash, &Mash::acceptMashStepChange );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

   emit changed( metaProperty("mashs"), QVariant() );
   emit newMash->mashStepsChanged();

}

QString Database::getDbFileName()
{
   // Ensure instance exists.
   instance();

   return dbFileName;
}

int Database::getInventoryId(TableSchema* tbl, int key )
{
   QString query = QString("SELECT %1 from %2 where %3 = %4")
         .arg(tbl->foreignKeyToColumn())
         .arg(tbl->tableName())
         .arg(tbl->keyName())
         .arg(key);

   QSqlQuery q( query, sqlDatabase());
   q.first();

   return q.record().value(tbl->foreignKeyToColumn()).toInt();
}

// this may be bad form. After a lot of refactoring, setInventory is the only method
// that needs to update something other than the NamedEntity's table. To simplify
// other things, I merged the nastier updateEntry into here and removed that method
//
// I need one of two things here for caching to work -- either every child of a inventory capable
// thing (eg, hops) listens for the parent to signal an inventory change, or this code has to
// reach into every child and update the inventory. I am leaning towards the first.
// Turns out, both are required in some order. Still thinking signal/slot
//
void Database::setInventory(NamedEntity* ins, QVariant value, int invKey, bool notify )
{
   TableSchema* tbl = dbDefn->table(ins->table());
   TableSchema* inv = dbDefn->table(tbl->invTable());

   QString invProp = inv->propertyName(kpropInventory);

   int ndx = ins->metaObject()->indexOfProperty(invProp.toUtf8().data());
   // I would like to get rid of this, but I need it to properly signal
   if ( invKey == 0 ) {
      // Uncomment this block if the message below is firing, as it will usually help find the bug quickly
      std::ostringstream stacktrace;
      stacktrace << boost::stacktrace::stacktrace();
      qDebug().noquote() << Q_FUNC_INFO << "bad inventory call. find it an kill it.  Stack:\n" << QString::fromStdString(stacktrace.str());
   }

   if ( ! value.isValid() || value.isNull() ) {
      value = 0.0;
   }

   try {
      QSqlQuery update( sqlDatabase() );
      // update hop_in_inventory set amount = [value] where hop_in_inventory.id = [invKey]
      QString command = QString("UPDATE %1 set %2=%3 where %4=%5")
                           .arg(inv->tableName())
                           .arg(inv->propertyToColumn(kpropInventory))
                           .arg(value.toString())
                           .arg(inv->keyName())
                           .arg(invKey);


      if ( ! update.exec(command) )
         throw QString("Could not update %1.%2 to %3: %4 %5")
                  .arg(inv->tableName())
                  .arg(inv->propertyToColumn(kpropInventory))
                  .arg( value.toString() )
                  .arg( update.lastQuery() )
                  .arg( update.lastError().text() );

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

   if ( notify ) {
      emit ins->changed(ins->metaObject()->property(ndx),value);
      emit changedInventory(tbl->dbTable(),invKey, value);
   }
}

// Versioning when modifying something in a recipe is *hard*. If we copy the
// recipe, there is no easy way to say "this ingredient in the old recipe is
// that ingredient in the new". The best I can think of is to use the delete
// idea -- copy everything but what's being modified, clone what's being
// modified and add the clone to the copy.  This is named to echo
// updateEntry()
bool Database::modifyEntry(NamedEntity* object, QString propName, QVariant value, bool notify )
{
   // Yog-Sothoth is the gate. Yog-Sothoth is the key and guardian of the
   // gate. Past, present, future, all are one in Yog-Sothoth
   Recipe *owner, *spawn;
   NamedEntity* neClone;
   bool noclone = true;
   qDebug() <<
      Q_FUNC_INFO << "Modifying: " << object->metaObject()->className() << " property " << propName << "to value" <<
      value;

   //
   // We have to be careful here as there are several overloaded versions of getParentRecipe(), one for NamedEntity,
   // one for BrewNote and one for MashStep.  You might think that if object is actually pointing to a BrewNote or a
   // MashStep then the right version of getParentRecipe() would get called but, in C++, that's not the case.  In C++,
   // dynamic dispatch only happens on virtual member functions.
   //
   if (MashStep * mashStep = dynamic_cast<MashStep *>(object)) {
      owner = getParentRecipe(mashStep);
   } else if (BrewNote * brewNote = dynamic_cast<BrewNote *>(object)) {
      owner = getParentRecipe(brewNote);
   } else {
      owner = getParentRecipe(object);
   }

   // if the ingredient is in a recipe and that recipe needs a version
   if ( owner && wantsVersion(owner) ) {
      // create the copy of the recipe, excluding the thing
      spawn = copyRecipeExcept(owner, object);
      // Copy the ingredient we want to change and change it. This is REALLY
      // important magic.
      neClone = clone(spawn,object,propName,value);
      noclone = false;

      emit createdSignal(spawn);
      emit spawned(owner,spawn);
   }
   else {
      // we don't want a version, or the ingredient isn't in a recipe
      neClone = object;
   }

   updateEntry( neClone, propName, value, false );

   return noclone;
}

void Database::updateEntry( NamedEntity* object, QString propName, QVariant value, bool notify, bool transact )
{
   TableSchema* schema =dbDefn->table( object->table() );
   int idx = object->metaObject()->indexOfProperty(propName.toUtf8().data());
   QMetaProperty mProp = object->metaObject()->property(idx);
   QString colName = schema->propertyToColumn(propName);

   if ( colName.isEmpty() ) {
      colName = schema->foreignKeyToColumn(propName);
   }

   if ( colName.isEmpty() ) {
      qCritical() << Q_FUNC_INFO << "Could not translate " << propName << " to a column name";
      throw  QString("Could not translate %1 to a column name").arg(propName);
   }
   if ( transact )
      sqlDatabase().transaction();

   try {
      QSqlQuery update( sqlDatabase() );
      QString command = QString("UPDATE %1 set %2=:value where id=%3")
                           .arg(schema->tableName())
                           .arg(colName)
                           .arg(object->key());

      update.prepare( command );
      update.bindValue(":value", value);

      if ( ! update.exec() )
         throw QString("Could not update %1.%2 to %3: %4 %5")
                  .arg( schema->tableName() )
                  .arg( colName )
                  .arg( value.toString() )
                  .arg( update.lastQuery() )
                  .arg( update.lastError().text() );

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact )
         sqlDatabase().rollback();
      abort();
   }

   if ( transact )
      sqlDatabase().commit();

   if ( notify )
      emit object->changed(mProp,value);

}


QVariant Database::get( Brewtarget::DBTable table, int key, QString col_name )
{
   QSqlQuery q;
   TableSchema* tbl = dbDefn->table(table);

   QString index = QString("%1_%2").arg(tbl->tableName()).arg(col_name);

   if ( ! selectSome.contains(index) ) {
      QString query = QString("SELECT %1 from %2 WHERE %3=:id")
                        .arg(col_name)
                        .arg(tbl->tableName())
                        .arg(tbl->keyName());
      q = QSqlQuery( sqlDatabase() );
      q.prepare(query);
      selectSome.insert(index,q);
   }

   q = selectSome.value(index);
   q.bindValue(":id", key);

   q.exec();
   if( !q.next() ) {
      q.finish();
      return QVariant();
   }

   QVariant ret( q.record().value(col_name) );
   q.finish();
   return ret;
}


QVariant Database::get( TableSchema* tbl, int key, QString col_name )
{
   return get( tbl->dbTable(), key, col_name.toUtf8().data());
}


// Inventory functions ========================================================

//This links ingredients with the same name.
//The first displayed ingredient in the database is assumed to be the parent.
void Database::populateChildTablesByName(Brewtarget::DBTable table)
{
   TableSchema* tbl = dbDefn->table(table);
   TableSchema* cld = dbDefn->childTable( table );
   qInfo() << QString("Populating Children NamedEntity Links (%1)").arg(tbl->tableName());

   try {
      // "SELECT DISTINCT name FROM [tablename]"
      QString queryString = QString("SELECT DISTINCT %1 FROM %2")
            .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::name)).arg(tbl->tableName());

      qDebug() << Q_FUNC_INFO << "DISTINCT:" << queryString;
      QSqlQuery nameq( queryString, sqlDatabase() );

      if ( ! nameq.exec() ) {
         throw QString("%1 %2").arg(nameq.lastQuery()).arg(nameq.lastError().text());
      }

      while (nameq.next()) {
         QString name = nameq.record().value(0).toString();
         // select id from [tablename] where ( name = :name and display = :boolean ) order by id asc
         queryString = QString( "SELECT %1 FROM %2 WHERE ( %3=:name AND %4=:boolean ) ORDER BY %1")
                     .arg(tbl->keyName())
                     .arg(tbl->tableName())
                     .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::name))
                     .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::display));
         QSqlQuery query( sqlDatabase() );
         qDebug() << Q_FUNC_INFO << "FIND:" << queryString;

         // find the first element with display set true (assumed parent)
         query.prepare(queryString);
         query.bindValue(":name", name);
         query.bindValue(":boolean",Brewtarget::dbTrue());

         if ( !query.exec() ) {
            throw QString("%1 %2").arg(query.lastQuery()).arg(query.lastError().text());
         }

         query.first();
         QString parentID = query.record().value(tbl->keyName()).toString();

         // find the every element with display set false (assumed children)
         query.bindValue(":name", name);
         query.bindValue(":boolean", Brewtarget::dbFalse());

         if ( !query.exec() ) {
            throw QString("%1 %2").arg(query.lastQuery()).arg(query.lastError().text());
         }

         // Postgres uses a more verbose upsert syntax. I don't like this, but
         // I'm not seeing a better way yet.
         while (query.next()) {
            QString childID = query.record().value(tbl->keyName()).toString();
            switch( Brewtarget::dbType() ) {
               case Brewtarget::PGSQL:
                //  INSERT INTO [child table] (parent_id, child_id) VALUES (:parentid, child_id) ON CONFLICT(child_id) DO UPDATE set parent_id = EXCLUDED.parent_id
                  queryString = QString("INSERT INTO %1 (%2, %3) VALUES (%4, %5) ON CONFLICT(%3) DO UPDATE set %2 = EXCLUDED.%2")
                        .arg(dbDefn->childTableName((table)))
                        .arg(cld->parentIndexName())
                        .arg(cld->childIndexName())
                        .arg(parentID)
                        .arg(childID);
                  break;
               default:
                  // insert or replace into [child table] (parent_id, child_id) values (:parentid,:childid)
                  queryString = QString("INSERT OR REPLACE INTO %1 (%2, %3) VALUES (%4, %5)")
                              .arg(dbDefn->childTableName(table))
                              .arg(cld->parentIndexName())
                              .arg(cld->childIndexName())
                              .arg(parentID)
                              .arg(childID);
            }
            qDebug() << Q_FUNC_INFO << "UPSERT:" << queryString;
            QSqlQuery insertq( queryString, sqlDatabase() );
            if ( !insertq.exec() ) {
               throw QString("%1 %2").arg(insertq.lastQuery()).arg(insertq.lastError().text());
            }
         }
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      abort();
   }
}

// populate ingredient tables
void Database::populateChildTablesByName()
{

   try {
      // I really dislike this. It counts as spooky action at a distance, but
      // the populateChildTablesByName methods need these hashes populated
      // early and there is no easy way to untangle them. Yes, this results in
      // the work being done twice. Such is life.
      populateChildTablesByName(Brewtarget::FERMTABLE);
      populateChildTablesByName(Brewtarget::HOPTABLE);
      populateChildTablesByName(Brewtarget::MISCTABLE);
      populateChildTablesByName(Brewtarget::YEASTTABLE);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }
}

QVariant Database::getInventoryAmt(Brewtarget::DBTable table, int key)
{
   QVariant val = QVariant(0.0);
   TableSchema* tbl = dbDefn->table(table);
   TableSchema* inv = dbDefn->table(tbl->invTable());
   QString amount_col = inv->propertyToColumn(kpropInventory);

   // select hop_in_inventory.amount from hop_in_inventory,hop where hop.id = key and hop_in_inventory.id = hop.inventory_id
   QString query = QString("select %1.%2 from %1,%3 where %3.%4 = %5 and %1.%6 = %3.%7")
         .arg(inv->tableName())
         .arg(amount_col)
         .arg(tbl->tableName())
         .arg(tbl->keyName())
         .arg(key)
         .arg(inv->keyName())
         .arg(tbl->foreignKeyToColumn(kpropInventoryId));


   QSqlQuery q( query, sqlDatabase() );

   if ( q.first() ) {
      val = q.record().value(amount_col);
   }
   return val;
}

//create a new inventory row
int Database::newInventory(TableSchema* schema) {
   TableSchema* inv = dbDefn->table(schema->invTable());
   int newKey;

   // not sure why we were doing an upsert earlier. We already know there is no
   // inventory row for this element. So doesn't this just need an insert?
   // insert into hop_in_inventory DEFAULT VALUES
   QString queryString = QString("INSERT INTO %1 DEFAULT VALUES").arg(inv->tableName());
   QSqlQuery q( queryString, sqlDatabase() );
   newKey = q.lastInsertId().toInt();

   return newKey;
}

QMap<int, double> Database::getInventory(const Brewtarget::DBTable table) const
{
   QMap<int, double> result;
   TableSchema* tbl = dbDefn->table(table);
   TableSchema* inv = dbDefn->invTable(table);

   // select fermentable.id as id,fermentable_in_inventory.amount as amount from
   //   fermentable_in_inventory where amount > 0 and fermentable.inventory_id = fermentable_in_inventory.id
   QString query = QString("SELECT %1.%2 as id,%3.%4 as amount FROM %1,%3 WHERE %3.%4 > 0 and %1.%5=%3.%6 and %1.%7=%8 and %1.%9=%10")
         .arg(tbl->tableName())
         .arg(tbl->keyName())
         .arg(inv->tableName())
         .arg(inv->propertyToColumn(kpropInventory))
         .arg(tbl->foreignKeyToColumn())
         .arg(inv->keyName())
         .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::display))
         .arg(Brewtarget::dbTrue())
         .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted))
         .arg(Brewtarget::dbFalse());

   QSqlQuery sql(query, sqlDatabase());
   if (! sql.isActive()) {
      throw QString("Failed to get the inventory.\nQuery:\n%1\nError:\n%2")
            .arg(sql.lastQuery())
            .arg(sql.lastError().text());
   }

   while (sql.next()) {
      result[sql.value("id").toInt()] = sql.value("amount").toDouble();
   }

   return result;
}

Recipe* Database::breed(Recipe* parent)
{
   // only breed when the parent is unlocked and the recipe wants to be
   // versioned
   if ( !parent->locked() && wantsVersion(parent) ) {
      return newRecipe(parent,true);
   }
   else {
      return parent;
   }
}

// Add to recipe ==============================================================
Equipment* Database::addToRecipe( Recipe* rec, Equipment* e, bool noCopy, bool transact )
{
   Equipment* newEquip = e;
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   if( e == nullptr )
      return nullptr;

   if ( rec->locked() )
      return nullptr;

   if ( transact )
      sqlDatabase().transaction();

   try {
      // Make a copy of equipment.
      if ( ! noCopy ) {
         newEquip = copy<Equipment>(e, &allEquipments, false);
      }

      // Update equipment_id
      sqlUpdate(Brewtarget::RECTABLE,
                QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropEquipmentId)).arg(newEquip->key()),
                QString("%1=%2").arg(tbl->keyName()).arg(rec->key()));

   }
   catch (QString e ) {
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   // This is likely illadvised. But if you are telling me to not transact it,
   // it is up to you to commit the changes
   if ( transact ) {
      sqlDatabase().commit();
   }
   // NOTE: need to disconnect the recipe's old equipment?
   connect( newEquip, &NamedEntity::changed, rec, &Recipe::acceptEquipChange );
   // NOTE: If we don't reconnect these signals, bad things happen when
   // changing boil times on the mainwindow
   connect( newEquip, &Equipment::changedBoilSize_l, rec, &Recipe::setBoilSize_l);
   connect( newEquip, &Equipment::changedBoilTime_min, rec, &Recipe::setBoilTime_min);

   // Emit a changed signal.
   emit rec->changed( rec->metaProperty("equipment"), NamedEntity::qVariantFromPtr(newEquip) );

   // If we are already wrapped in a transaction boundary, do not call
   // recaclAll(). Weirdness ensues. But I want this after all the signals are
   // attached, etc.
   if ( transact )
      rec->recalcAll();

   return newEquip;

}

Fermentable * Database::addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy, bool transact )
{
   if ( ferm == nullptr )
      return nullptr;

   if ( rec->locked() )
      return nullptr;

   try {
      Fermentable* newFerm = addNamedEntityToRecipe<Fermentable>(rec,ferm,noCopy,&allFermentables,true,transact );
      connect( newFerm, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptFermChange(QMetaProperty,QVariant)) );

      // If somebody upstream is doing the transaction, let them call recalcAll
      if ( transact && ! noCopy )
         rec->recalcAll();

      return newFerm;
   }
   catch (QString e) {
      throw;
   }
}

QList<Fermentable*> Database::addToRecipe( Recipe* rec, QList<Fermentable*>ferms, bool transact )
{
   QList<Fermentable*> rets;

   if ( ferms.size() == 0 )
      return rets;

   if ( rec->locked() )
      return rets;

   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      foreach (Fermentable* ferm, ferms )
      {
         Fermentable* newFerm = addNamedEntityToRecipe<Fermentable>(rec,ferm,false,&allFermentables,true,false);
         connect( newFerm, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptFermChange(QMetaProperty,QVariant)) );
         rets.append(newFerm);
      }
   }
   catch ( QString e  ) {
      if ( transact ) {
         sqlDatabase().rollback();
      }
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
      rec->recalcAll();
   }
   return rets;
}

QList<Fermentable*> Database::addToRecipe( Recipe* rec, QList<Fermentable*>ferms, Fermentable* exclude, bool transact )
{
   QList<Fermentable*> nothing;
   if ( ferms.size() == 0 )
      return nothing;

   if ( rec->locked() )
      return nothing;

   int r_ndx = ferms.indexOf(exclude);

   if ( r_ndx != -1 ) {
      ferms.removeAt(r_ndx);
   }
   return addToRecipe( rec, ferms, transact );
}

Hop * Database::addToRecipe( Recipe* rec, Hop* hop, bool noCopy, bool transact )
{
   if ( rec->locked() )
      return nullptr;

   Recipe* spawn = breed(rec);
   try {
      Hop* newHop = addNamedEntityToRecipe<Hop>( spawn, hop, noCopy, &allHops, true, transact );
      // it's slightly dirty pool to put this all in the try block. Sue me.
      connect( newHop, SIGNAL(changed(QMetaProperty,QVariant)), spawn, SLOT(acceptHopChange(QMetaProperty,QVariant)));
      if ( transact ) {
         spawn->recalcIBU();
      }
      return newHop;
   }
   catch (QString e) {
      throw;
   }

   if ( spawn != rec ) {
      emit spawned(rec,spawn);
   }
}

QList<Hop*> Database::addToRecipe( Recipe* rec, QList<Hop*>hops, bool transact )
{
   QList<Hop*> rets;

   if ( hops.size() == 0 )
      return rets;

   if ( rec->locked() )
      return rets;

   Recipe *spawn = breed(rec);
   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      foreach (Hop* hop, hops ) {
         Hop* newHop = addNamedEntityToRecipe<Hop>( spawn, hop, false, &allHops, true, false );
         connect( newHop, SIGNAL(changed(QMetaProperty,QVariant)), spawn, SLOT(acceptHopChange(QMetaProperty,QVariant)));
         rets.append(newHop);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact ) {
         sqlDatabase().rollback();
      }
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
      rec->recalcIBU();
   }
   return rets;
}

QList<Hop*> Database::addToRecipe( Recipe* rec, QList<Hop*>hops, Hop* exclude, bool transact )
{
   QList<Hop*> nothing;

   if ( hops.size() == 0 )
      return nothing;

   if ( rec->locked() )
      return nothing;

   int r_ndx = hops.indexOf(exclude);

   if ( r_ndx != -1 ) {
      hops.removeAt(r_ndx);
   }
   return addToRecipe( rec, hops, transact );
}

Mash * Database::addToRecipe( Recipe* rec, Mash* m, bool noCopy, bool transact )
{
   if ( m == nullptr )
      return nullptr;

   if ( rec->locked() )
      return nullptr;

   Mash* newMash = m;
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   if ( transact )
      sqlDatabase().transaction();
   // Make a copy of mash.
   // Making a copy of the mash isn't enough. We need a copy of the mashsteps
   // too.
   try {
      if ( ! noCopy ) {
         newMash = copy<Mash>(m, &allMashs, false);
         duplicateMashSteps(m,newMash);
      }

      // Update mash_id
      sqlUpdate(Brewtarget::RECTABLE,
               QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId) ).arg(newMash->key()),
               QString("%1=%2").arg(tbl->keyName()).arg(rec->key()));
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }
   connect( newMash, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptMashChange(QMetaProperty,QVariant)));
   emit rec->changed( rec->metaProperty("mash"), NamedEntity::qVariantFromPtr(newMash) );
   // And let the recipe recalc all?
   if ( !noCopy && transact )
      rec->recalcAll();

   return newMash;
}

Misc * Database::addToRecipe( Recipe* rec, Misc* m, bool noCopy, bool transact )
{
   if ( rec->locked() )
      return nullptr;

   Recipe* spawn = breed(rec);
   try {
      Misc * newMisc = addNamedEntityToRecipe( spawn, m, noCopy, &allMiscs, true, transact );
      if ( transact && ! noCopy )
         spawn->recalcAll();
      return newMisc;
   }
   catch (QString e) {
      abort();
   }
}

QList<Misc*> Database::addToRecipe( Recipe* rec, QList<Misc*>miscs, bool transact )
{

   QList<Misc*> rets;

   if ( miscs.size() == 0 )
      return rets;

   if ( rec->locked() )
      return rets;

   if ( transact )
      sqlDatabase().transaction();

   Recipe* spawn = breed(rec);
   try {
      foreach (Misc* misc, miscs ) {
         rets.append( addNamedEntityToRecipe( spawn, misc, false, &allMiscs,true,false ) );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact ) {
         sqlDatabase().rollback();
      }
      abort();
   }
   if ( transact ) {
      sqlDatabase().commit();
      spawn->recalcAll();
   }
   return rets;
}

QList<Misc*> Database::addToRecipe( Recipe* rec, QList<Misc*>miscs, Misc* exclude, bool transact )
{
   QList<Misc*> rets;
   if ( miscs.size() == 0 )
      return rets;

   if ( rec->locked() )
      return rets;

   int r_ndx = miscs.indexOf(exclude);

   if ( r_ndx != -1 ) {
      miscs.removeAt(r_ndx);
   }
   return addToRecipe( rec, miscs, transact );
}

Water * Database::addToRecipe( Recipe* rec, Water* w, bool noCopy, bool transact )
{
   if ( rec->locked() )
      return nullptr;

   Recipe* spawn = breed(rec);
   try {
      return addNamedEntityToRecipe( spawn, w, noCopy, &allWaters,true,transact );
   }
   catch (QString e) {
      abort();
   }
}

Salt * Database::addToRecipe( Recipe* rec, Salt* s, bool noCopy, bool transact )
{
   if ( rec->locked() )
      return nullptr;

   Recipe* spawn = breed(rec);
   try {
      return addNamedEntityToRecipe( spawn, s, noCopy, &allSalts,true,transact );
   }
   catch (QString e) {
      abort();
   }
}

Style * Database::addToRecipe( Recipe* rec, Style* s, bool noCopy, bool transact )
{
   Style* newStyle = s;
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   if ( s == nullptr )
      return nullptr;

   if ( rec->locked() )
      return nullptr;

   if ( transact )
      sqlDatabase().transaction();

   Recipe* spawn = breed(rec);
   try {
      if ( ! noCopy )
         newStyle = copy<Style>(s, &allStyles, false);

      sqlUpdate(Brewtarget::RECTABLE,
                QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropStyleId)).arg(newStyle->key()),
                QString("%1=%2").arg(tbl->keyName()).arg(spawn->key()));
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact )
         sqlDatabase().rollback();
      abort();
   }

   if ( transact ) {
      sqlDatabase().commit();
   }
   // Emit a changed signal.
   spawn->m_style_id = newStyle->key();
   emit spawn->changed( spawn->metaProperty("style"), NamedEntity::qVariantFromPtr(newStyle) );
   return newStyle;
}

Yeast * Database::addToRecipe( Recipe* rec, Yeast* y, bool noCopy, bool transact )
{
   if ( rec->locked() )
      return nullptr;

   Recipe* spawn = breed(rec);
   try {
      Yeast* newYeast = addNamedEntityToRecipe<Yeast>( spawn, y, noCopy, &allYeasts, true, transact );
      connect( newYeast, SIGNAL(changed(QMetaProperty,QVariant)), spawn, SLOT(acceptYeastChange(QMetaProperty,QVariant)));
      if ( transact && ! noCopy )
      {
         spawn->recalcOgFg();
         spawn->recalcABV_pct();
      }
      return newYeast;
   }
   catch (QString e) {
      abort();
   }
}

QList<Yeast*> Database::addToRecipe( Recipe* rec, QList<Yeast*>yeasts, bool transact )
{
   QList<Yeast*> rets;

   if ( yeasts.size() == 0 )
      return rets;

   if ( rec->locked() )
      return rets;

   if ( transact )
      sqlDatabase().transaction();

   Recipe* spawn = breed(rec);
   try {
      foreach (Yeast* yeast, yeasts )
      {
         Yeast* newYeast = addNamedEntityToRecipe( spawn, yeast, false, &allYeasts,true,false );
         connect( newYeast, SIGNAL(changed(QMetaProperty,QVariant)), spawn, SLOT(acceptYeastChange(QMetaProperty,QVariant)));
         rets.append(newYeast);
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( transact )
         sqlDatabase().rollback();
      abort();
   }

   if ( transact ) {
      sqlDatabase().commit();
      spawn->recalcOgFg();
      spawn->recalcABV_pct();
   }

   return rets;
}

QList<Yeast*> Database::addToRecipe( Recipe* rec, QList<Yeast*>yeasts, Yeast* exclude, bool transact )
{
   QList<Yeast*> nothing;

   if ( yeasts.size() == 0 )
      return nothing;

   if ( rec->locked() )
      return nothing;

   int r_ndx = yeasts.indexOf(exclude);

   if ( r_ndx != -1 ) {
      yeasts.removeAt(r_ndx);
   }

   return addToRecipe( rec, yeasts, transact );
}

QSqlRecord Database::fetchOne(TableSchema* tbl, int key)
{
   QSqlQuery q(sqlDatabase());
   QString select = QString("SELECT * FROM %1 WHERE %2 = %3")
                        .arg(tbl->tableName())
                        .arg(tbl->keyName())
                        .arg(key);

   qDebug() << Q_FUNC_INFO << "SELECT SQL:" << select;

   if( !q.exec(select) ) {
      throw QString("%1 %2")
               .arg(q.lastQuery())
               .arg(q.lastError().text());
   }

   qDebug() << Q_FUNC_INFO << "Returned " << q.size() << " rows";

   q.next();

   QSqlRecord record = q.record();
   q.finish();
   return record;
}

template<class T> T* Database::replicant(NamedEntity const* object, QHash<int,T*>* keyHash, QString propName, QVariant value)
{
   int newKey;
   T* newOne;

   Brewtarget::DBTable t = dbDefn->classNameToTable(object->metaObject()->className());
   TableSchema* tbl = dbDefn->table(t);

   QSqlQuery q(sqlDatabase());

   try {
      QSqlRecord oldRecord = this->fetchOne(tbl,object->key());

      QString insert_string = tbl->generateInsertProperties();
      QSqlQuery insert = QSqlQuery( sqlDatabase() );
      insert.prepare(insert_string);

      foreach (QString prop, tbl->allProperties() ) {
         QVariant val = oldRecord.value(tbl->propertyToColumn(prop));

         // What has risen may sink, and what has sunk may rise.
         if ( prop == propName ) {
            val = value;
         }
         insert.bindValue(QString(":%1").arg(prop), val);
      }

      // For debugging, it's useful to know what the SQL parameters were
      auto boundValues = insert.boundValues();
      for (auto ii : boundValues.keys()) {
         qDebug() << Q_FUNC_INFO << ii << "=" << boundValues.value(ii);
      }

      if (! insert.exec() )
         throw QString("could not execute %1 : %2").arg(insert.lastQuery()).arg(insert.lastError().text());

      newKey = insert.lastInsertId().toInt();
      // I was never a fan of using the old record. So get the record we just
      // created and don't be clever
      QSqlRecord newRecord = this->fetchOne(tbl,newKey);

      newOne = new T(tbl, newRecord, newKey);
      keyHash->insert( newKey, newOne );
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      q.finish();
      abort();
   }
   q.finish();

   return newOne;
}

template<class T> T* Database::copy( NamedEntity const* object, QHash<int,T*>* keyHash, bool displayed )
{
   int newKey;
   T* newOne;

   Brewtarget::DBTable t = dbDefn->classNameToTable(object->metaObject()->className());
   TableSchema* tbl = dbDefn->table(t);

   QSqlQuery q(sqlDatabase());

   try {
      QSqlRecord oldRecord = this->fetchOne(tbl,object->key());

      QString insert_string = tbl->generateInsertProperties();
      QSqlQuery insert = QSqlQuery( sqlDatabase() );
      insert.prepare(insert_string);

      foreach (QString prop, tbl->allProperties() ) {
         QVariant val = oldRecord.value(tbl->propertyToColumn(prop));

         // we allow the caller to over ride the display() value
         if ( prop == PropertyNames::NamedEntity::display) {
            qDebug() << Q_FUNC_INFO << Brewtarget::dbBoolean(displayed);
            val = Brewtarget::dbBoolean(displayed);
         }
         insert.bindValue(QString(":%1").arg(prop), val);
      }

      // For debugging, it's useful to know what the SQL parameters were
      auto boundValues = insert.boundValues();
      for (auto ii : boundValues.keys()) {
         qDebug() << Q_FUNC_INFO << ii << "=" << boundValues.value(ii);
      }

      if (! insert.exec() )
         throw QString("could not execute %1 : %2").arg(insert.lastQuery()).arg(insert.lastError().text());

      newKey = insert.lastInsertId().toInt();
      // I was never a fan of using the old record. So get the record we just
      // created and don't be clever
      QSqlRecord newRecord = this->fetchOne(tbl,newKey);

      newOne = new T(tbl, newRecord, newKey);
      keyHash->insert( newKey, newOne );
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      q.finish();
      abort();
   }
   q.finish();

   return newOne;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::sqlUpdate( Brewtarget::DBTable table, QString const& setClause, QString const& whereClause )
{
   QString update = QString("UPDATE %1 SET %2 WHERE %3")
                .arg(dbDefn->tableName(table))
                .arg(setClause)
                .arg(whereClause);

   QSqlQuery q(sqlDatabase());
   try {
      if ( ! q.exec(update) )
         throw QString("Could not execute update %1 : %2").arg(update).arg(q.lastError().text());
   }
   catch (QString e) {
      q.finish();
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
   }

   q.finish();
}

void Database::sqlDelete( Brewtarget::DBTable table, QString const& whereClause )
{
   QString del = QString("DELETE FROM %1 WHERE %2")
                .arg(dbDefn->tableName(table))
                .arg(whereClause);

   QSqlQuery q(sqlDatabase());
   try {
      if ( ! q.exec(del) )
         throw QString("Could not delete %1 : %2").arg(del).arg(q.lastError().text());
   }
   catch (QString e) {
      q.finish();
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
   }

   q.finish();
}

QList<BrewNote*> Database::brewNotes()
{
   QList<BrewNote*> tmp;

   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::BREWNOTETABLE, allBrewNotes );
   return tmp;
}

QList<Equipment*> Database::equipments()
{
   QList<Equipment*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::EQUIPTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::EQUIPTABLE, allEquipments);
   return tmp;
}

QList<Fermentable*> Database::fermentables()
{
   QList<Fermentable*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::FERMTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::FERMTABLE, allFermentables);
   return tmp;
}

QList<Hop*> Database::hops()
{
   QList<Hop*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::HOPTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::HOPTABLE, allHops);
   return tmp;
}

QList<Mash*> Database::mashs()
{
   QList<Mash*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::MASHTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   //! Mashs and mashsteps are the odd balls.
   getElements( tmp, query, Brewtarget::MASHTABLE, allMashs);
   return tmp;
}

QList<MashStep*> Database::mashSteps()
{
   QList<MashStep*> tmp;
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   QString query = QString("%1=%2 order by %3")
           .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse())
           .arg(tbl->propertyToColumn(PropertyNames::MashStep::stepNumber));
   getElements( tmp, query, Brewtarget::MASHSTEPTABLE, allMashSteps);
   return tmp;
}

QList<Misc*> Database::miscs()
{
   QList<Misc*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::MISCTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::MISCTABLE, allMiscs );
   return tmp;
}

QList<Recipe*> Database::recipes()
{
   QList<Recipe*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::RECTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());

   // Filters will handle the display flag upstream from here. I can't believe
   // I ever made this work
   getElements( tmp, query, Brewtarget::RECTABLE, allRecipes );
   return tmp;
}

QList<Style*> Database::styles()
{
   QList<Style*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::STYLETABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::STYLETABLE, allStyles );
   return tmp;
}

QList<Water*> Database::waters()
{
   QList<Water*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::WATERTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::WATERTABLE, allWaters );
   return tmp;
}

QList<Salt*> Database::salts()
{
   QList<Salt*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::SALTTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::SALTTABLE, allSalts );
   return tmp;
}

QList<Yeast*> Database::yeasts()
{
   QList<Yeast*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::YEASTTABLE)->propertyToColumn(PropertyNames::NamedEntity::deleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::YEASTTABLE, allYeasts );
   return tmp;
}


//
// These templated wrappers of the member functions make it easier for callers to use templates to do generic
// processing of NamedEntity derivatives (ie Hop, Yeast, Equipment, etc objects).
//
template<> QList<BrewNote*>    Database::getAll<BrewNote>()    { return this->brewNotes();    }
template<> QList<Equipment*>   Database::getAll<Equipment>()   { return this->equipments();   }
template<> QList<Fermentable*> Database::getAll<Fermentable>() { return this->fermentables(); }
template<> QList<Hop*>         Database::getAll<Hop>()         { return this->hops();         }
template<> QList<Mash*>        Database::getAll<Mash>()        { return this->mashs();        }
template<> QList<MashStep*>    Database::getAll<MashStep>()    { return this->mashSteps();    }
template<> QList<Misc*>        Database::getAll<Misc>()        { return this->miscs();        }
template<> QList<Recipe*>      Database::getAll<Recipe>()      { return this->recipes();      }
template<> QList<Style*>       Database::getAll<Style>()       { return this->styles();       }
template<> QList<Water*>       Database::getAll<Water>()       { return this->waters();       }
template<> QList<Salt*>        Database::getAll<Salt>()        { return this->salts();        }
template<> QList<Yeast*>       Database::getAll<Yeast>()       { return this->yeasts();       }


bool Database::updateSchema(bool* err)
{
   int currentVersion = DatabaseSchemaHelper::currentVersion( sqlDatabase() );
   int newVersion = DatabaseSchemaHelper::dbVersion;
   bool doUpdate = currentVersion < newVersion;

   if( doUpdate )
   {
      bool success = DatabaseSchemaHelper::migrate( currentVersion, newVersion, sqlDatabase() );
      if( !success )
      {
         qCritical() << QString("Database migration %1->%2 failed").arg(currentVersion).arg(newVersion);
         if( err )
            *err = true;
         return false;
      }
   }

   sqlDatabase().transaction();

   try {
      //populate ingredient links
      int repopChild = 0;
      QSqlQuery popchildq( "SELECT repopulateChildrenOnNextStart FROM settings WHERE id=1", sqlDatabase() );

      if( popchildq.next() )
         repopChild = popchildq.record().value("repopulateChildrenOnNextStart").toInt();
      else
         throw QString("%1 %2").arg(popchildq.lastQuery()).arg(popchildq.lastError().text());

      if(repopChild == 1) {
         qDebug() << Q_FUNC_INFO << "calling populateChildTablesByName()";
         populateChildTablesByName();

         QSqlQuery popchildq( "UPDATE settings SET repopulateChildrenOnNextStart = 0", sqlDatabase() );
         if ( ! popchildq.isActive() )
            throw QString("Could not modify settings table: %1 %2").arg(popchildq.lastQuery()).arg(popchildq.lastError().text());
      }
   }
   catch (QString e ) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   return doUpdate;
}

/*******
 *
 * I will be using hop as my example, because it is easy to type.  You should
 * be able to substitue any of the base tables and it will work the same.
 *
 * We maintain a table named bt_hop. The bt_hop table has two columns: id and
 * hop_id. id is the standard autosequence we use. hop_id is the id of a row
 * in the hop table for a hop that we shipped. In the default database, the
 * two values will almost always be equal. In all databases, hop_id will point
 * to a parent hop.
 *
 * When a new hop is added to the default-db.sqlite, a new row has to be
 * inserted into bt_hop pointing to the new hop.
 *
 * When the user gets the dialog saying "There are new ingredients, would you
 * like to merge?", updateDatabase() is called and it works like this:
 *     1. We get all the rows from bt_hop from default_db.sqlite
 *     2. We seach for each bt.id in the user's database.
 *     3. If we do not find the bt.id, it means the hop is new to the user and
 *        we need to add it to their database.
 *     4. We do the necessary binding and inserting to add the new hop to the
 *        user's database
 *     5. We put a new entry in the user's bt_hop table, pointing to the
 *        record we just added.
 *     6. Repeat steps 3 - 5 until we run out of rows.
 *
 * It is really important that we DO NOTHING if the user already has the hop.
 * We should NEVER over write user data without explicit permission. I have no
 * interest in working up a diff mechanism, a display mechanism, etc. to show
 * the user what would be done. For now, then, we don't over write any
 * existing records.
 *
 * A few other notes. Any use of TableSchema on the default_db.sqlite must
 * specify the database type as SQLite. We cannot be sure the user's database
 * is SQLite. There's no real difference yet, but I am considering tackling
 * mysql again.
 */
void Database::updateDatabase(QString const& filename)
{
   // In the naming here "old" means the user's database, and
   // "new" means the database coming from 'filename'.

   QVariant btid, newid, oldid;

   try {
      // connect to the new database
      QString newCon("newSqldbCon");
      QSqlDatabase newSqldb = QSqlDatabase::addDatabase("QSQLITE", newCon);
      newSqldb.setDatabaseName(filename);
      if( ! newSqldb.open() ) {
         QMessageBox::critical(nullptr,
                              QObject::tr("Database Failure"),
                              QString(QObject::tr("Failed to open the database '%1'.").arg(filename)));
         throw QString("Could not open %1 for reading.\n%2").arg(filename).arg(newSqldb.lastError().text());
      }

      // For each (id, hop_id) in newSqldb.bt_hop...

      // SELECT * FROM newSqldb.hop WHERE id=<hop_id>

      // INSERT INTO hop SET name=:name, alpha=:alpha,... WHERE id=(SELECT hop_id FROM bt_hop WHERE id=:bt_id)

      // Bind :bt_id from <id>
      // Bind :name, :alpha, ..., from newRecord.

      // Execute.

      foreach ( TableSchema* tbl, dbDefn->baseTables() )
      {
         TableSchema* btTbl = dbDefn->btTable(tbl->dbTable());
         // skip any table that doesn't have a bt_ table
         if ( btTbl == nullptr ) {
            continue;
         }

         // build and prepare all the queries once per table.

         // get the new hop referenced by bt_hop.hop_id
         QSqlQuery qNewIng(newSqldb);
         QString   newIngString = QString("SELECT * FROM %1 WHERE %2=:id")
                                    .arg(tbl->tableName())
                                    .arg(tbl->keyName(Brewtarget::SQLITE));
         qNewIng.prepare(newIngString);
         qDebug() << Q_FUNC_INFO << newIngString;

         // get the same row from the old bt_hop.
         QSqlQuery qOldBtIng(sqlDatabase());
         QString   oldBtIngString = QString("SELECT * FROM %1 WHERE %2=:btid")
                                    .arg(btTbl->tableName())
                                    .arg(btTbl->keyName());
         qOldBtIng.prepare(oldBtIngString);
         qDebug() << Q_FUNC_INFO << oldBtIngString;

         // insert the new bt_hop row into the old database.
         QSqlQuery qOldBtIngInsert(sqlDatabase());
         QString   oldBtIngInsert = QString("INSERT INTO %1 (%2,%3) values (:id,:%3)")
                                          .arg(btTbl->tableName())
                                          .arg(btTbl->keyName())
                                          .arg(btTbl->childIndexName());
         qOldBtIngInsert.prepare(oldBtIngInsert);
         qDebug() << Q_FUNC_INFO << oldBtIngInsert;

         // Create in insert statement for new records. We will bind this
         // later
         QSqlQuery qInsertOldIng(sqlDatabase());
         QString   insertString = tbl->generateInsertProperties();
         qInsertOldIng.prepare(insertString);
         qDebug() << Q_FUNC_INFO << insertString;

         // get the bt_hop rows from the new database
         QSqlQuery qNewBtIng(newSqldb);
         QString   newBtIngString = QString("SELECT * FROM %1").arg(btTbl->tableName());
         qDebug() << Q_FUNC_INFO << newBtIngString;

         if ( ! qNewBtIng.exec(newBtIngString) ) {
            throw QString("Could not find btID (%1): %2 %3")
                     .arg(btid.toInt())
                     .arg(qNewBtIng.lastQuery())
                     .arg(qNewBtIng.lastError().text());
         }

         // start processing the ingredients from the new db
         while ( qNewBtIng.next() ) {

            // get the bt.id and bt.hop_id. Note we specify the db type here
            btid  = qNewBtIng.record().value(btTbl->keyName(Brewtarget::SQLITE));
            newid = qNewBtIng.record().value(btTbl->childIndexName(Brewtarget::SQLITE));

            // bind the id to find the hop in the new db
            qNewIng.bindValue(":id", newid);

            // if we can't execute the search
            if ( ! qNewIng.exec() ) {
               throw QString("Could not retrieve new ingredient: %1 %2")
                        .arg(qNewIng.lastQuery())
                        .arg(qNewIng.lastError().text());
            }

            // if we can't read/find the hop
            if ( ! qNewIng.next() ) {
               throw QString("Could not advance query: %1 %2")
                        .arg(qNewIng.lastQuery())
                        .arg(qNewIng.lastError().text());
            }

            // Find the bt_hop record in the old database.
            qOldBtIng.bindValue( ":btid", btid );
            if ( ! qOldBtIng.exec() ) {
               throw QString("Could not find btID (%1): %2 %3")
                        .arg(btid.toInt())
                        .arg(qOldBtIng.lastQuery())
                        .arg(qOldBtIng.lastError().text());
            }

            // If the new bt_hop.id isn't in the old bt_hop
            if( ! qOldBtIng.next() ) {
               // bind the values from the new hop to the insert query
               bindForUpdateDatabase(tbl,qInsertOldIng,qNewIng.record());
               // we need a transaction here, as we are updating two tables
               sqlDatabase().transaction();

               // execute the insert
               if ( ! qInsertOldIng.exec() ) {
                  throw QString("Could not insert new btID (%1): %2 %3")
                           .arg(oldid.toInt())
                           .arg(qInsertOldIng.lastQuery())
                           .arg(qInsertOldIng.lastError().text());
               }

               // get the id from the last insert
               oldid = qInsertOldIng.lastInsertId().toInt();

               // Insert an entry into the old bt_hop table.
               qOldBtIngInsert.bindValue( ":id", btid);
               qOldBtIngInsert.bindValue( QString(":%1").arg(btTbl->childIndexName()), oldid);

               if ( ! qOldBtIngInsert.exec() ) {
                  throw QString("Could not insert btID (%1): %2 %3")
                           .arg(btid.toInt())
                           .arg(qOldBtIngInsert.lastQuery())
                           .arg(qOldBtIngInsert.lastError().text());
               }

               // finally, commit the transaction
               sqlDatabase().commit();
            }
         }
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      sqlDatabase().rollback();
      abort();
   }
}

// updateDatabase is ugly enough. This takes 20-ish lines out of it that do
// not really enhance understanding
void Database::bindForUpdateDatabase(TableSchema* tbl, QSqlQuery qry, QSqlRecord rec)
{
   foreach( QString prop, tbl->allProperties() ) {
      // we need to specify the database here. The default database might be
      // postgres, but the new ingredients are always shipped in sqlite
      QString col = tbl->propertyToColumn(prop, Brewtarget::SQLITE);
      QVariant bindVal;

      // deleted is always false, but spell 'false' properly for
      // the database
      if ( prop == PropertyNames::NamedEntity::deleted ) {
         bindVal = Brewtarget::dbFalse();
      }
      // boolean values suck, so make sure we spell them properly
      else if ( tbl->propertyColumnType(prop) == "boolean" ) {
         // makes the lines short enough
         bool intermediate = rec.value(col).toBool();
         bindVal = intermediate ? Brewtarget::dbTrue() : Brewtarget::dbFalse();
      }
      // otherwise, just grab the value
      else {
         bindVal = rec.value(col);
      }
      // and bind it.
      qry.bindValue(QString(":%1").arg(prop), bindVal);
   }
}

bool Database::verifyDbConnection(Brewtarget::DBTypes testDb, QString const& hostname, int portnum, QString const& schema,
                              QString const& database, QString const& username, QString const& password)
{
   QString driverName;
   QSqlDatabase connDb;
   bool results;

   switch( testDb )
   {
      case Brewtarget::PGSQL:
         driverName = "QPSQL";
         break;
      default:
         driverName = "QSQLITE";
   }
   connDb = QSqlDatabase::addDatabase(driverName,"testConnDb");

   switch( testDb )
   {
      case Brewtarget::PGSQL:
         connDb.setHostName(hostname);
         connDb.setPort(portnum);
         connDb.setDatabaseName(database);
         connDb.setUserName(username);
         connDb.setPassword(password);
         break;
      default:
         connDb.setDatabaseName(hostname);
   }

   results = connDb.open();

   if ( results )
      connDb.close();
   else
      QMessageBox::critical(nullptr, tr("Connection failed"),
               QString(tr("Could not connect to %1 : %2")).arg(hostname).arg(connDb.lastError().text())
            );
   return results;

}

QSqlDatabase Database::openSQLite()
{
   QString filePath = Brewtarget::getUserDataDir().filePath("database.sqlite");
   QSqlDatabase newDb = QSqlDatabase::addDatabase("QSQLITE", "altdb");

   try {
      dbFile.setFileName(dbFileName);

      if ( filePath.isEmpty() )
         throw QString("Could not read the database file(%1)").arg(filePath);

      newDb.setDatabaseName(filePath);

      if (!  newDb.open() )
         throw QString("Could not open %1 : %2").arg(filePath).arg(newDb.lastError().text());
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }

   return newDb;
}

QSqlDatabase Database::openPostgres(QString const& Hostname, QString const& DbName,
                                    QString const& Username, QString const& Password,
                                    int Portnum)
{
   QSqlDatabase newDb = QSqlDatabase::addDatabase("QPSQL","altdb");

   try {
      newDb.setHostName(Hostname);
      newDb.setDatabaseName(DbName);
      newDb.setUserName(Username);
      newDb.setPort(Portnum);
      newDb.setPassword(Password);

      if ( ! newDb.open() )
         throw QString("Could not open %1 : %2").arg(Hostname).arg(newDb.lastError().text());
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }
   return newDb;
}

void Database::convertDatabase(QString const& Hostname, QString const& DbName,
                               QString const& Username, QString const& Password,
                               int Portnum, Brewtarget::DBTypes newType)
{
   QSqlDatabase newDb;

   Brewtarget::DBTypes oldType = static_cast<Brewtarget::DBTypes>(Brewtarget::option("dbType",Brewtarget::SQLITE).toInt());

   try {
      if ( newType == Brewtarget::NODB ) {
         throw QString("No type found for the new database.");
      }

      if ( oldType == Brewtarget::NODB ) {
         throw QString("No type found for the old database.");
      }

      switch( newType ) {
         case Brewtarget::PGSQL:
            newDb = openPostgres(Hostname, DbName,Username, Password, Portnum);
            break;
         default:
            newDb = openSQLite();
      }

      if ( ! newDb.isOpen() ) {
         throw QString("Could not open new database: %1").arg(newDb.lastError().text());
      }

      // this is to prevent us from over-writing or doing heavens knows what to an existing db
      if( newDb.tables().contains(QLatin1String("settings")) ) {
         qWarning() << QString("It appears the database is already configured.");
         return;
      }

      newDb.transaction();

      // make sure we get the inventory tables first
      foreach( TableSchema* table, dbDefn->allTables(true) ) {
         QString createTable = table->generateCreateTable(newType);
         QSqlQuery results( newDb );
         if ( ! results.exec(createTable) ) {
            throw QString("Could not create %1 : %2").arg(table->tableName()).arg(results.lastError().text());
         }
      }
      newDb.commit();

      copyDatabase(oldType,newType,newDb);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }
}

QVariant Database::convertValue(Brewtarget::DBTypes newType, QSqlField field)
{
   QVariant retVar = field.value();
   if ( field.type() == QVariant::Bool ) {
      switch(newType) {
         case Brewtarget::PGSQL:
            retVar = field.value().toBool();
            break;
         default:
            retVar = field.value().toInt();
            break;
      }
   }
   else if ( field.name() == PropertyNames::BrewNote::fermentDate && field.value().toString() == "CURRENT_DATETIME" ) {
      retVar = "'now()'";
   }
   return retVar;
}

void Database::copyDatabase( Brewtarget::DBTypes oldType, Brewtarget::DBTypes newType, QSqlDatabase newDb)
{
   QSqlDatabase oldDb = sqlDatabase();
   QSqlQuery readOld(oldDb);

   // There are a lot of tables to process, and we need to make
   // sure the inventory tables go first
   foreach( TableSchema* table, dbDefn->allTables(true) ) {
      QString tname = table->tableName();
      QSqlField field;
      bool mustPrepare = true;
      int maxid = -1;

      // select * from [table] order by id asc
      QString findAllQuery = QString("SELECT * FROM %1 order by %2 asc")
                                 .arg(tname)
                                 .arg(table->keyName(oldType)); // make sure we specify the right db type
      qDebug() << Q_FUNC_INFO << "FIND ALL:" << findAllQuery;
      try {
         if (! readOld.exec(findAllQuery) ) {
            throw QString("Could not execute %1 : %2")
               .arg(readOld.lastQuery())
               .arg(readOld.lastError().text());
         }

         newDb.transaction();

         QSqlQuery upsertNew(newDb); // we will prepare this in a bit

         // Start reading the records from the old db
         while(readOld.next()) {
            int idx;
            QSqlRecord here = readOld.record();
            QString upsertQuery;

            idx = here.indexOf(table->keyName(oldType));

            // We are going to need this for resetting the indexes later. We only
            // need it for copying to postgresql, but .. meh, not worth the extra
            // work
            if ( idx != -1 && here.value(idx).toInt() > maxid ) {
               maxid = here.value(idx).toInt();
            }

            // Prepare the insert for this table if required
            if ( mustPrepare ) {
               upsertQuery = table->generateInsertRow(newType);
               upsertNew.prepare(upsertQuery);
               // but do it only once for this table
               mustPrepare = false;
            }

            qDebug() << Q_FUNC_INFO << "INSERT:" << upsertQuery;
            // All that's left is to bind
            for(int i = 0; i < here.count(); ++i) {
               if ( table->dbTable() == Brewtarget::BREWNOTETABLE
                    && here.fieldName(i) == PropertyNames::BrewNote::brewDate ) {
                  QVariant helpme(here.field(i).value().toString());
                  upsertNew.bindValue(":brewdate",helpme);
               }
               else {
                  upsertNew.bindValue(QString(":%1").arg(here.fieldName(i)),
                                      convertValue(newType, here.field(i)));
               }
            }
            // and execute
            if ( ! upsertNew.exec() ) {
               throw QString("Could not insert new row %1 : %2")
                  .arg(upsertNew.lastQuery())
                  .arg(upsertNew.lastError().text());
            }
         }
         // We need to create the increment and decrement things for the
         // instructions_in_recipe table. This seems a little weird to do this
         // here, but it makes sense to wait until after we've inserted all
         // the data. The increment trigger happens on insert, and I suspect
         // bad things would happen if it were in place before we inserted all our data.
         if ( table->dbTable() == Brewtarget::INSTINRECTABLE ) {
            QString trigger = table->generateIncrementTrigger(newType);
            if ( trigger.isEmpty() ) {
               qCritical() << QString("No increment triggers found for %1").arg(table->tableName());
            }
            else {
               qDebug() << "INC TRIGGER:" << trigger;
               upsertNew.exec(trigger);
               trigger =  table->generateDecrementTrigger(newType);
               if ( trigger.isEmpty() ) {
                  qCritical() << QString("No decrement triggers found for %1").arg(table->tableName());
               }
               else {
                  qDebug() << "DEC TRIGGER:" << trigger;
                  if ( ! upsertNew.exec(trigger) ) {
                     throw QString("Could not insert new row %1 : %2")
                        .arg(upsertNew.lastQuery())
                        .arg(upsertNew.lastError().text());
                  }
               }
            }
         }
         // We need to manually reset the sequences in postgresql
         if ( newType == Brewtarget::PGSQL ) {
             // this probably should be fixed somewhere, but this is enough for now?
             // SELECT setval(hop_id_seq,(SELECT MAX(id) from hop))
            QString seq = QString("SELECT setval('%1_%2_seq',(SELECT MAX(%2) FROM %1))")
                                          .arg(table->tableName())
                                          .arg(table->keyName());
            qDebug() << "SEQ reset: " << seq;
            if ( ! upsertNew.exec(seq) )
               throw QString("Could not reset the sequences: %1 %2")
                  .arg(seq).arg(upsertNew.lastError().text());
         }
      }
      catch (QString e) {
         qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
         newDb.rollback();
         abort();
      }

      newDb.commit();
   }
}

