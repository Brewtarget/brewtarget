/*
 * database.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - David Grundberg <individ@acc.umu.se>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Luke Vincent <luke.r.vincent@gmail.com>
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
#include "brewnote.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "instruction.h"
#include "mash.h"
#include "mashstep.h"
#include "misc.h"
#include "recipe.h"
#include "style.h"
#include "water.h"
#include "salt.h"
#include "yeast.h"

#include "config.h"
#include "beerxml.h"
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
   Brewtarget::logD("Loading SQLITE...");
   bool dbIsOpen;
   QSqlDatabase sqldb;

   // Set file names.
   dbFileName = Brewtarget::getUserDataDir().filePath("database.sqlite");
   dataDbFileName = Brewtarget::getDataDir().filePath("default_db.sqlite");
   Brewtarget::logD(QString("Database::loadSQLite() - dbFileName = \"%1\"\nDatabase::loadSQLite() - dataDbFileName=\"%2\"").arg(dbFileName).arg(dataDbFileName));
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
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text()));
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
         Brewtarget::logE( QString("%1: %2 (%3)").arg(Q_FUNC_INFO).arg(e).arg(pragma.lastError().text()));
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
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text()));
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
         Brewtarget::logE("DatabaseSchemaHelper::create() failed");
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
         connect( e, &Ingredient::changed, *i, &Recipe::acceptEquipChange );
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

bool Database::createBlank(QString const& filename)
{
   {
      QSqlDatabase sqldb = QSqlDatabase::addDatabase("QSQLITE", "blank");
      sqldb.setDatabaseName(filename);
      bool dbIsOpen = sqldb.open();
      if( ! dbIsOpen )
      {
         Brewtarget::logW(QString("Database::createBlank(): could not open '%1'").arg(filename));
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
            importFromXML( oldXmlFile.fileName() );

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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      _threadToConnectionMutex.unlock();
      throw;
   }

   // Put new connection in the hash.
   _threadToConnection.insert(t,conName);
   _threadToConnectionMutex.unlock();
   return sqldb;
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
         Brewtarget::logW( QString("%1 : could not find a unique name in 10000 tries. Overwriting %2").arg(Q_FUNC_INFO).arg(halfName));
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
      QFile *file = new QFile(victim);
      QFileInfo *fileThing = new QFileInfo(victim);

      // Make sure it exists, and make sure it is a file before we
      // try remove it
      if ( fileThing->exists() && fileThing->isFile() ) {
         // If we can't remove it, give a warning.
         if (! file->remove() ) {
            Brewtarget::logW( QString("%1 : could not remove %2 (%3).").arg(Q_FUNC_INFO).arg(victim).arg(file->error()));
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

   Brewtarget::logD( QString("Database backup to \"%1\" %2").arg(newDbFileName, success ? "succeeded" : "failed") );

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

// removeFromRecipe ===========================================================
void Database::removeIngredientFromRecipe( Recipe* rec, Ingredient* ing )
{
   const QMetaObject* meta = ing->metaObject();
   TableSchema *table;
   TableSchema *child;
   TableSchema *inrec;

   int ndx = meta->indexOfClassInfo("signal");
   QString propName;

   sqlDatabase().transaction();
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
      // We need to do many things -- remove the link in *in_recipe,
      // remove the entry from *_children
      // and DELETE THE COPY
      // delete from misc_in_recipe where misc_id = [misc key] and recipe_id = [rec key]
      QString deleteFromInRecipe = QString("DELETE FROM %1 WHERE %2=%3 AND %4=%5")
                                 .arg(inrec->tableName() )
                                 .arg(inrec->inRecIndexName())
                                 .arg(ing->_key)
                                 .arg(inrec->recipeIndexName())
                                 .arg(rec->_key);

      // delete from misc where id = [misc key]
      QString deleteIngredient = QString("DELETE FROM %1 where %2=%3")
                                 .arg(table->tableName())
                                 .arg(table->keyName())
                                 .arg(ing->_key);
      q.setForwardOnly(true);

      // Don't do this if no child table is defined (like instructions)
      if ( table->childTable() != Brewtarget::NOTABLE ) {
         // delete from misc_child where child_id = [misc key]
         QString deleteFromChildren = QString("DELETE FROM %1 WHERE %2=%3")
                                    .arg(child->tableName())
                                    .arg( child->childIndexName() )
                                    .arg(ing->_key);
         if ( ! q.exec( deleteFromChildren ) ) {
            throw QString("failed to delete children.");
         }
      }

      if ( ! q.exec(deleteFromInRecipe) )
         throw QString("failed to delete in_recipe.");

      if ( ! q.exec( deleteIngredient ) )
         throw QString("failed to delete ingredient.");

   }
   catch ( QString e ) {
      Brewtarget::logE(QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text()));
      sqlDatabase().rollback();
      q.finish();
      abort();
   }

   rec->recalcAll();
   sqlDatabase().commit();

   q.finish();
   emit rec->changed( rec->metaProperty(propName), QVariant() );
}

void Database::removeFromRecipe( Recipe* rec, Instruction* ins )
{
   try {
      removeIngredientFromRecipe( rec, ins);
   }
   catch (QString e) {
      throw; //up the stack!
   }

   allInstructions.remove(ins->_key);
   emit changed( metaProperty("instructions"), QVariant() );
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Database::removeFrom( Mash* mash, MashStep* step )
{
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   // Just mark the step as deleted.
   try {
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
               QString("%1 = %2").arg(tbl->propertyToColumn(kpropDeleted)).arg(Brewtarget::dbTrue()),
               QString("%1 = %2").arg(tbl->keyName()).arg(step->_key));
   }
   catch ( QString e ) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }

   emit mash->mashStepsChanged();
}

Recipe* Database::getParentRecipe( BrewNote const* note )
{
   int key;
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);
   // SELECT recipe_id FROM brewnote WHERE id = [key]
   QString query = QString("SELECT %1 FROM %2 WHERE %3 = %4")
           .arg( tbl->recipeIndexName())
           .arg( tbl->tableName() )
           .arg( tbl->keyName() )
           .arg(note->_key);

   QSqlQuery q(sqlDatabase());

   try {
      if ( ! q.exec(query) )
         throw QString("could not find recipe id");
   }
   catch ( QString e ) {
      Brewtarget::logE(QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text()));
      q.finish();
      throw;
   }

   q.next();
   key = q.record().value(tbl->recipeIndexName()).toInt();
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
                .arg(tbl->propertyToColumn(kpropStepNumber))
                .arg(tbl->keyName())
                .arg(m1->_key)
                .arg(m2->stepNumber())
                .arg(m2->_key)
                .arg(m1->stepNumber());

   QSqlQuery q(sqlDatabase() );

   try {
      if ( !q.exec(update) )
         throw QString("failed to swap steps");
   }
   catch ( QString e ) {
      Brewtarget::logE(QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text()));
      q.finish();
      throw;
   }

   q.finish();

   emit m1->changed( m1->metaProperty("stepNumber") );
   emit m2->changed( m2->metaProperty("stepNumber") );
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
      .arg(in1->_key)
      .arg(in2->instructionNumber())
      .arg(in2->_key)
      .arg(in1->instructionNumber());

   QSqlQuery q( sqlDatabase());

   try {
      if ( !q.exec(update) )
         throw QString("failed to swap steps");
   }
   catch ( QString e ) {
      Brewtarget::logE(QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text()));
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
                   .arg(in->_key);
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
         .arg(in->_key);

      if ( !q.exec(update) )
         throw QString("failed to insert new instruction recipe");
   }
   catch ( QString e ) {
      Brewtarget::logE(QString("%1 %2 %3 %4")
                           .arg(Q_FUNC_INFO)
                           .arg(e)
                           .arg(q.lastQuery())
                           .arg(q.lastError().text()));
      q.finish();
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   q.finish();

   emit in->changed( in->metaProperty("instructionNumber"), pos );
}

QList<BrewNote*> Database::brewNotes(Recipe const* parent)
{
   QList<BrewNote*> ret;
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);

   //  recipe_id = [parent->key] AND deleted = false
   QString filterString = QString("%1 = %2 AND %3 = %4")
           .arg( tbl->recipeIndexName() )
           .arg(parent->_key)
           .arg(tbl->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());

   getElements(ret, filterString, Brewtarget::BREWNOTETABLE, allBrewNotes);

   return ret;
}

QList<Fermentable*> Database::fermentables(Recipe const* parent)
{
   QList<Fermentable*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::FERMINRECTABLE);
   // recipe_id = [parent->key]
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->_key);

   getElements(ret,filter, Brewtarget::FERMINRECTABLE, allFermentables, inrec->inRecIndexName());

   return ret;
}

QList<Hop*> Database::hops(Recipe const* parent)
{
   QList<Hop*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::HOPINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->_key);

   getElements(ret,filter, Brewtarget::HOPINRECTABLE, allHops, inrec->inRecIndexName());

   return ret;
}

QList<Misc*> Database::miscs(Recipe const* parent)
{
   QList<Misc*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::MISCINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->_key);

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
         .arg(parent->_key)
         .arg(tbl->propertyToColumn(kpropDeleted))
         .arg(Brewtarget::dbFalse())
         .arg(tbl->propertyToColumn(kpropStepNumber));

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
         .arg(parent->_key)
         .arg( inrec->propertyToColumn(kpropInstructionNumber));

   getElements(ret,filter,Brewtarget::INSTINRECTABLE,allInstructions,inrec->inRecIndexName());

   return ret;
}

QList<Water*> Database::waters(Recipe const* parent)
{
   QList<Water*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::WATERINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->_key);

   getElements(ret,filter,Brewtarget::WATERINRECTABLE,allWaters,inrec->inRecIndexName());

   return ret;
}

QList<Salt*> Database::salts(Recipe const* parent)
{
   QList<Salt*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::SALTINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->_key);

   getElements(ret,filter,Brewtarget::SALTINRECTABLE,allSalts,inrec->inRecIndexName());

   return ret;
}

QList<Yeast*> Database::yeasts(Recipe const* parent)
{
   QList<Yeast*> ret;
   TableSchema* inrec = dbDefn->table(Brewtarget::YEASTINRECTABLE);
   QString filter = QString("%1 = %2").arg(inrec->recipeIndexName()).arg(parent->_key);

   getElements(ret,filter,Brewtarget::YEASTINRECTABLE,allYeasts,inrec->inRecIndexName());

   return ret;
}

// Named constructors =========================================================

BrewNote* Database::newBrewNote(BrewNote* other, bool signal)
{
   BrewNote* tmp = copy<BrewNote>(other, &allBrewNotes);

   if ( tmp && signal ) {
      emit changed( metaProperty("brewNotes"), QVariant() );
      emit newBrewNoteSignal(tmp);
   }

   return tmp;
}

BrewNote* Database::newBrewNote(Recipe* parent, bool signal)
{
   BrewNote* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newIngredient(&allBrewNotes);
      TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);

      sqlUpdate( Brewtarget::BREWNOTETABLE,
               QString("%1=%2").arg(tbl->recipeIndexName()).arg(parent->_key),
               QString("%1=%2").arg(tbl->keyName()).arg(tmp->_key) );

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   tmp->setDisplay(true);
   if ( signal )
   {
      emit changed( metaProperty("brewNotes"), QVariant() );
      emit newBrewNoteSignal(tmp);
   }

   return tmp;
}

Equipment* Database::newEquipment(Equipment* other)
{
   Equipment* tmp;

   if (other)
      tmp = copy(other, &allEquipments);
   else
      tmp = newIngredient(&allEquipments);

   if ( tmp ) {
      emit changed( metaProperty("equipments"), QVariant() );
      emit newEquipmentSignal(tmp);
   }
   else {
      Brewtarget::logE( QString("%1 couldn't copy %2").arg(Q_FUNC_INFO).arg(other->name()));
   }

   return tmp;
}

Fermentable* Database::newFermentable(Fermentable* other)
{
   Fermentable* tmp;
   bool transact = false;

   try {
      // copies automatically get their inventory_id properly set
      if (other) {
         tmp = copy(other, &allFermentables);
      }
      else {
         // new ingredients don't. this gets ugly fast, because we are now
         // writing to two tables and need some transactional protection
         sqlDatabase().transaction();
         transact = true;
         tmp = newIngredient(&allFermentables);
         int invkey = newInventory( dbDefn->table(Brewtarget::FERMTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact ) sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }
   if ( tmp ) {
      emit changed( metaProperty("fermentables"), QVariant() );
      emit newFermentableSignal(tmp);
   }
   else {
      Brewtarget::logE( QString("%1 couldn't copy %2").arg(Q_FUNC_INFO).arg(other->name()));
   }

   return tmp;
}

Hop* Database::newHop(Hop* other)
{
   Hop* tmp;
   bool transact = false;

   try {
      if ( other ) {
         tmp = copy(other, &allHops);
      }
      else {
         sqlDatabase().transaction();
         transact = true;
         tmp = newIngredient(&allHops);
         int invkey = newInventory( dbDefn->table(Brewtarget::HOPTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact ) sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }

   if ( tmp ) {
      emit changed( metaProperty("hops"), QVariant() );
      emit newHopSignal(tmp);
   }
   else {
      Brewtarget::logE( QString("%1 could not %2 hop")
            .arg(Q_FUNC_INFO)
            .arg( other ? "copy" : "create"));
   }

   return tmp;
}

Instruction* Database::newInstruction(Recipe* rec)
{
   Instruction* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newIngredient(&allInstructions);

      // Add without copying to "instruction_in_recipe". We already have a
      // transaction open, so tell addIng to not worry about it
      tmp = addIngredientToRecipe<Instruction>(rec,tmp,true,nullptr,false,false);
   }
   catch ( QString e ) {
      Brewtarget::logE( QString("%1 %2").arg( Q_FUNC_INFO ).arg(e));
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
         tmp = newIngredient(&allMashs);
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
                       QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId)).arg(tmp->_key),
                       QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId)).arg(other->_key));
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
   emit newMashSignal(tmp);

   return tmp;
}

Mash* Database::newMash(Recipe* parent, bool transact)
{
   Mash* tmp;

   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      tmp = newIngredient(&allMashs);

      // Connect tmp to parent, removing any existing mash in parent.
      TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);
      sqlUpdate( Brewtarget::RECTABLE,
                 QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropMashId)).arg(tmp->_key),
                 QString("%1=%2").arg(tbl->keyName()).arg(parent->_key));
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }

   emit changed( metaProperty("mashs"), QVariant() );
   emit newMashSignal(tmp);

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
                        .arg(tbl->propertyToColumn(kpropStepNumber))
                        .arg(tbl->tableName())
                        .arg(tbl->propertyToColumn(kpropDeleted))
                        .arg(Brewtarget::dbFalse())
                        .arg(tbl->foreignKeyToColumn())
                        .arg(mash->_key);

   sqlDatabase().transaction();

   QSqlQuery q(sqlDatabase());
   q.setForwardOnly(true);

   // mashsteps are weird, because we have to do the linking between step and
   // mash
   try {
      tmp = newIngredient(&allMashSteps);

      // we need to set the mash_id first
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 QString("%1=%2 ").arg(tbl->foreignKeyToColumn()).arg(mash->_key),
                 QString("%1=%2").arg(tbl->keyName()).arg(tmp->_key)
               );

      // Just sets the step number within the mash to the next available number.
      // we need coalesce here instead of isnull. coalesce is SQL standard, so
      // should be more widely supported than isnull
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 coalesce,
                 QString("%1=%2").arg(tbl->keyName()).arg(tmp->_key));
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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

Misc* Database::newMisc(Misc* other)
{
   Misc* tmp;
   bool transact = false;

   try {
      if ( other ) {
        tmp = copy(other, &allMiscs);
      }
      else {
         sqlDatabase().transaction();
         transact = true;
         tmp = newIngredient(&allMiscs);
         int invkey = newInventory( dbDefn->table(Brewtarget::MISCTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact ) sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }

   if ( tmp ) {
      emit changed( metaProperty("miscs"), QVariant() );
      emit newMiscSignal(tmp);
   }
   else {
      Brewtarget::logE( QString("%1 could not %2 misc")
            .arg(Q_FUNC_INFO)
            .arg( other ? "copy" : "create"));
   }

   return tmp;
}

Recipe* Database::newRecipe(QString name)
{
   Recipe* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newIngredient(name,&allRecipes);

      newMash(tmp,false);
   }
   catch (QString e ) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }

   sqlDatabase().commit();
   emit changed( metaProperty("recipes"), QVariant() );
   emit newRecipeSignal(tmp);

   return tmp;
}

Recipe* Database::newRecipe(Recipe* other)
{
   Recipe* tmp;

   sqlDatabase().transaction();
   try {
      tmp = copy<Recipe>(other, &allRecipes);

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
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   emit changed( metaProperty("recipes"), QVariant() );
   emit newRecipeSignal(tmp);

   return tmp;
}

Style* Database::newStyle(Style* other)
{
   Style* tmp;

   try {
      tmp = copy(other, &allStyles);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("styles"), QVariant() );
   emit newStyleSignal(tmp);

   return tmp;
}

Style* Database::newStyle(QString name)
{
   Style* tmp;

   try {
      tmp = newIngredient(name, &allStyles);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }

   emit changed( metaProperty("styles"), QVariant() );
   emit newStyleSignal(tmp);

   return tmp;
}

Water* Database::newWater(Water* other)
{
   Water* tmp;

   try {
      if ( other )
         tmp = copy(other,&allWaters);
      else
         tmp = newIngredient(&allWaters);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("waters"), QVariant() );
   emit newWaterSignal(tmp);

   return tmp;
}

Salt* Database::newSalt(Salt* other)
{
   Salt* tmp;

   try {
      if ( other )
         tmp = copy(other,&allSalts);
      else
         tmp = newIngredient(&allSalts);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("salts"), QVariant() );
   emit newSaltSignal(tmp);

   return tmp;
}

Yeast* Database::newYeast(Yeast* other)
{
   Yeast* tmp;
   bool transact = false;

   try {
      if (other) {
         tmp = copy(other, &allYeasts);
      }
      else {
         sqlDatabase().transaction();
         transact = true;
         tmp = newIngredient(&allYeasts);
         int invkey = newInventory( dbDefn->table(Brewtarget::YEASTTABLE));
         tmp->setInventoryId(invkey);
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }
   emit changed( metaProperty("yeasts"), QVariant() );
   emit newYeastSignal(tmp);

   return tmp;
}

int Database::insertElement(Ingredient* ins)
{
   int key;
   QSqlQuery q( sqlDatabase() );

   TableSchema* schema = dbDefn->table(ins->table());
   QString insertQ = schema->generateInsertProperties(Brewtarget::dbType());
   QStringList allProps = schema->allPropertyNames(Brewtarget::dbType());

   q.prepare(insertQ);

   foreach (QString prop, allProps) {
      QVariant val_to_ins = ins->property(prop.toUtf8().data());
      if ( ins->table() == Brewtarget::BREWNOTETABLE && prop == kpropBrewDate ) {
         val_to_ins = val_to_ins.toString();
      }
      // I've arranged it such that the bindings are on the property names. It simplifies a lot
      q.bindValue( QString(":%1").arg(prop), val_to_ins);
   }

   try {
      if ( ! q.exec() ) {
         throw QString("could not insert a record into %1: %2")
               .arg(schema->tableName())
               .arg(insertQ);
      }

      key = q.lastInsertId().toInt();
      q.finish();
   }
   catch (QString e) {
      sqlDatabase().rollback();
      Brewtarget::logE(QString("%1 %2 %3").arg(Q_FUNC_INFO).arg(e).arg( q.lastError().text()));
      abort();
   }
   ins->_key = key;

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
   emit newStyleSignal(ins);

   return key;
}

int Database::insertEquipment(Equipment* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allEquipments.insert(key,ins);
   emit changed( metaProperty("equipments"), QVariant() );
   emit newEquipmentSignal(ins);

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
      Brewtarget::logE(e);
      throw;
   }

   sqlDatabase().commit();
   allFermentables.insert(key,ins);
   emit changed( metaProperty("fermentables"), QVariant() );
   emit newFermentableSignal(ins);
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
      Brewtarget::logE(e);
      throw;
   }

   sqlDatabase().commit();
   allHops.insert(key,ins);
   emit changed( metaProperty("hops"), QVariant() );
   emit newHopSignal(ins);

   return key;
}

int Database::insertInstruction(Instruction* ins, Recipe* parent)
{
   int key;
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);

      ins = addIngredientToRecipe<Instruction>(parent,ins,true,nullptr,false,false);

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
   emit newMashSignal(ins);

   return key;
}

// this one will be harder, because we have to link the mashstep to the parent
// mash
int Database::insertMashStep(MashStep* ins, Mash* parent)
{
   TableSchema* tbl = dbDefn->table(Brewtarget::MASHSTEPTABLE);
   // step_number = (SELECT COALESCE(MAX(step_number)+1,0) FROM mashstep WHERE deleted=false AND mash_id=[key] )
   QString coalesce = QString( "%1 = (SELECT COALESCE(MAX(%1)+1,0) FROM %2 WHERE %3=%4 AND %5=%6 )")
                        .arg(tbl->propertyToColumn(kpropStepNumber))
                        .arg(tbl->tableName())
                        .arg(tbl->propertyToColumn(kpropDeleted))
                        .arg(Brewtarget::dbFalse())
                        .arg(tbl->foreignKeyToColumn())
                        .arg(parent->_key);
   int key;

   sqlDatabase().transaction();
   try {
      // we need to insert the mashstep into the db first to get the key
      key = insertElement(ins);
      ins->setCacheOnly(false);

      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 QString("%1=%2 ").arg(tbl->foreignKeyToColumn()).arg(parent->_key),
                 QString("%1=%2").arg(tbl->keyName()).arg(ins->_key)
               );

      // Just sets the step number within the mash to the next available number.
      // we need coalesce here instead of isnull. coalesce is SQL standard, so
      // should be more widely supported than isnull
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 coalesce,
                 QString("%1=%2").arg(tbl->keyName()).arg(ins->_key));
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
      Brewtarget::logE(e);
      throw;
   }

   sqlDatabase().commit();
   allMiscs.insert(key,ins);
   emit changed( metaProperty("miscs"), QVariant() );
   emit newMiscSignal(ins);

   return key;
}

int Database::insertRecipe(Recipe* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allRecipes.insert(key,ins);
   emit changed( metaProperty("recipes"), QVariant() );
   emit newRecipeSignal(ins);

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
      Brewtarget::logE(e);
      throw;
   }

   sqlDatabase().commit();
   allYeasts.insert(key,ins);
   emit changed( metaProperty("yeasts"), QVariant() );
   emit newYeastSignal(ins);

   return key;
}

int Database::insertWater(Water* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allWaters.insert(key,ins);
   emit changed( metaProperty("waters"), QVariant() );
   emit newWaterSignal(ins);

   return key;
}

int Database::insertSalt(Salt* ins)
{
   int key = insertElement(ins);
   ins->setCacheOnly(false);

   allSalts.insert(key,ins);
   emit changed( metaProperty("salts"), QVariant() );
   emit newSaltSignal(ins);

   return key;
}
// This is more similar to a mashstep in that we need to link the brewnote to
// the parent recipe.
int Database::insertBrewnote(BrewNote* ins, Recipe* parent)
{
   int key;
   TableSchema* tbl = dbDefn->table(Brewtarget::BREWNOTETABLE);
   sqlDatabase().transaction();

   try {
      key = insertElement(ins);
      ins->setCacheOnly(false);

      sqlUpdate( Brewtarget::BREWNOTETABLE,
               QString("%1=%2").arg(tbl->foreignKeyToColumn()).arg(parent->_key),
               QString("%1=%2").arg(tbl->keyName()).arg(key) );

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   allBrewNotes.insert(key,ins);
   emit changed( metaProperty("brewNotes"), QVariant() );
   emit newBrewNoteSignal(ins);

   return key;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Database::deleteRecord( Ingredient* object )
{
   try {
      updateEntry( object, kpropDeleted, Brewtarget::dbTrue(), true);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      throw;
   }

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
         connect( newStep, &Ingredient::changed,
                  newMash, &Mash::acceptMashStepChange );
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
// that needs to update something other than the Ingredient's table. To simplify
// other things, I merged the nastier updateEntry into here and removed that method
//
// I need one of two things here for caching to work -- either every child of a inventory capable
// thing (eg, hops) listens for the parent to signal an inventory change, or this code has to
// reach into every child and update the inventory. I am leaning towards the first.
// Turns out, both are required in some order. Still thinking signal/slot
//
void Database::setInventory(Ingredient* ins, QVariant value, int invKey, bool notify )
{
   TableSchema* tbl = dbDefn->table(ins->table());
   TableSchema* inv = dbDefn->table(tbl->invTable());

   QString invProp = inv->propertyName(kpropInventory);

   int ndx = ins->metaObject()->indexOfProperty(invProp.toUtf8().data());
   // I would like to get rid of this, but I need it to properly signal
   if ( invKey == 0 ) {
      qDebug() << "bad inventory call. find it an kill it";
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      throw;
   }

   if ( notify ) {
      emit ins->changed(ins->metaObject()->property(ndx),value);
      emit changedInventory(tbl->dbTable(),invKey, value);
   }
}

void Database::updateEntry( Ingredient* object, QString propName, QVariant value, bool notify, bool transact )
{
   TableSchema* schema =dbDefn->table( object->table() );
   int idx = object->metaObject()->indexOfProperty(propName.toUtf8().data());
   QMetaProperty mProp = object->metaObject()->property(idx);
   QString colName = schema->propertyToColumn(propName);

   if ( colName.isEmpty() ) {
      colName = schema->foreignKeyToColumn(propName);
   }

   if ( colName.isEmpty() ) {
      Brewtarget::logE(QString("Could not translate %1 to a column name").arg(propName));
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact )
      sqlDatabase().commit();

   if ( notify )
      emit object->changed(mProp,value);

}

// Inventory functions ========================================================

//This links ingredients with the same name.
//The first displayed ingredient in the database is assumed to be the parent.
void Database::populateChildTablesByName(Brewtarget::DBTable table)
{
   TableSchema* tbl = dbDefn->table(table);
   TableSchema* cld = dbDefn->childTable( table );
   Brewtarget::logI( QString("Populating Children Ingredient Links (%1)").arg(tbl->tableName()));

   try {
      // "SELECT DISTINCT name FROM [tablename]"
      QString queryString = QString("SELECT DISTINCT %1 FROM %2")
            .arg(tbl->propertyToColumn(kpropName)).arg(tbl->tableName());

      QSqlQuery nameq( queryString, sqlDatabase() );

      if ( ! nameq.isActive() )
         throw QString("%1 %2").arg(nameq.lastQuery()).arg(nameq.lastError().text());

      while (nameq.next()) {
         QString name = nameq.record().value(0).toString();
         queryString = QString( "SELECT %1 FROM %2 WHERE ( %3=:name AND %4=:boolean ) ORDER BY %1 ASC LIMIT 1")
                     .arg(tbl->keyName())
                     .arg(tbl->tableName())
                     .arg(tbl->propertyToColumn(kpropName))
                     .arg(tbl->propertyToColumn(kpropDisplay));
         QSqlQuery query( sqlDatabase() );

         query.prepare(queryString);
         query.bindValue(":name", name);
         query.bindValue(":boolean",Brewtarget::dbTrue());
         query.exec();

         if ( !query.isActive() )
            throw QString("%1 %2").arg(query.lastQuery()).arg(query.lastError().text());

         query.first();
         QString parentID = query.record().value(tbl->keyName()).toString();

         query.bindValue(":name", name);
         query.bindValue(":boolean", Brewtarget::dbFalse());
         query.exec();

         if ( !query.isActive() )
            throw QString("%1 %2").arg(query.lastQuery()).arg(query.lastError().text());
         // Postgres uses a more verbose upsert syntax. I don't like this, but
         // I'm not seeing a better way yet.
         while (query.next()) {
            QString childID = query.record().value(tbl->keyName()).toString();
            switch( Brewtarget::dbType() ) {
               case Brewtarget::PGSQL:
                //  INSERT INTO %1 (parent_id, child_id) VALUES (%2, %3) ON CONFLICT(child_id) DO UPDATE set parent_id = EXCLUDED.parent_id
                  queryString = QString("INSERT INTO %1 (%2, %3) VALUES (%4, %5) ON CONFLICT(%3) DO UPDATE set %2 = EXCLUDED.%2")
                        .arg(dbDefn->childTableName((table)))
                        .arg(cld->parentIndexName())
                        .arg(cld->childIndexName())
                        .arg(parentID)
                        .arg(childID);
                  break;
               default:
                  queryString = QString("INSERT OR REPLACE INTO %1 (%2, %3) VALUES (%4, %5)")
                              .arg(dbDefn->childTableName(table))
                              .arg(cld->parentIndexName())
                              .arg(cld->childIndexName())
                              .arg(parentID)
                              .arg(childID);
            }
            QSqlQuery insertq( queryString, sqlDatabase() );
            if ( !insertq.isActive() )
               throw QString("%1 %2").arg(insertq.lastQuery()).arg(insertq.lastError().text());
         }
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }
}

QVariant Database::getInventoryAmt(QString col_name, Brewtarget::DBTable table, int key)
{
   QVariant val = QVariant(0.0);
   TableSchema* tbl = dbDefn->table(table);
   TableSchema* inv = dbDefn->table(tbl->invTable());

   // select hop_in_inventory.amount from hop_in_inventory,hop where hop.id = key and hop_in_inventory.id = hop.inventory_id
   QString query = QString("select %1.%2 from %1,%3 where %3.%4 = %5 and %1.%6 = %3.%7")
         .arg(inv->tableName())
         .arg(inv->propertyToColumn(kpropInventory))
         .arg(tbl->tableName())
         .arg(tbl->keyName())
         .arg(key)
         .arg(inv->keyName())
         .arg(tbl->foreignKeyToColumn(kpropInventoryId));


   QSqlQuery q( query, sqlDatabase() );

   if ( q.first() ) {
      val = q.record().value(inv->propertyToColumn(col_name));
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
         .arg(tbl->propertyToColumn(kpropDisplay))
         .arg(Brewtarget::dbTrue())
         .arg(tbl->propertyToColumn(kpropDeleted))
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

// Add to recipe ==============================================================
void Database::addToRecipe( Recipe* rec, Equipment* e, bool noCopy, bool transact )
{
   Equipment* newEquip = e;
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   if( e == nullptr )
      return;

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
                QString("%1=%2").arg(tbl->keyName()).arg(rec->_key));

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
   connect( newEquip, &Ingredient::changed, rec, &Recipe::acceptEquipChange );
   // NOTE: If we don't reconnect these signals, bad things happen when
   // changing boil times on the mainwindow
   connect( newEquip, &Equipment::changedBoilSize_l, rec, &Recipe::setBoilSize_l);
   connect( newEquip, &Equipment::changedBoilTime_min, rec, &Recipe::setBoilTime_min);

   // Emit a changed signal.
   emit rec->changed( rec->metaProperty("equipment"), Ingredient::qVariantFromPtr(newEquip) );

   // If we are already wrapped in a transaction boundary, do not call
   // recaclAll(). Weirdness ensues. But I want this after all the signals are
   // attached, etc.
   if ( transact )
      rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy, bool transact )
{
   if ( ferm == nullptr )
      return;

   try {
      Fermentable* newFerm = addIngredientToRecipe<Fermentable>(rec,ferm,noCopy,&allFermentables,true,transact );
      connect( newFerm, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptFermChange(QMetaProperty,QVariant)) );
   }
   catch (QString e) {
      throw;
   }

   // If somebody upstream is doing the transaction, let them call recalcAll
   if ( transact && ! noCopy )
      rec->recalcAll();

}

void Database::addToRecipe( Recipe* rec, QList<Fermentable*>ferms, bool transact )
{
   if ( ferms.size() == 0 )
      return;

   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      foreach (Fermentable* ferm, ferms )
      {
         Fermentable* newFerm = addIngredientToRecipe<Fermentable>(rec,ferm,false,&allFermentables,true,false);
         connect( newFerm, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptFermChange(QMetaProperty,QVariant)) );
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
}

void Database::addToRecipe( Recipe* rec, Hop* hop, bool noCopy, bool transact )
{
   try {
      Hop* newHop = addIngredientToRecipe<Hop>( rec, hop, noCopy, &allHops, true, transact );
      // it's slightly dirty pool to put this all in the try block. Sue me.
      connect( newHop, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptHopChange(QMetaProperty,QVariant)));
      if ( transact ) {
         rec->recalcIBU();
      }
   }
   catch (QString e) {
      throw;
   }
}

void Database::addToRecipe( Recipe* rec, QList<Hop*>hops, bool transact )
{
   if ( hops.size() == 0 )
      return;

   if ( transact ) {
      sqlDatabase().transaction();
   }

   try {
      foreach (Hop* hop, hops ) {
         Hop* newHop = addIngredientToRecipe<Hop>( rec, hop, false, &allHops, true, false );
         connect( newHop, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptHopChange(QMetaProperty,QVariant)));
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact ) {
         sqlDatabase().rollback();
      }
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
      rec->recalcIBU();
   }
}

void Database::addToRecipe( Recipe* rec, Mash* m, bool noCopy, bool transact )
{
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
               QString("%1=%2").arg(tbl->keyName()).arg(rec->_key));
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }
   connect( newMash, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptMashChange(QMetaProperty,QVariant)));
   emit rec->changed( rec->metaProperty("mash"), Ingredient::qVariantFromPtr(newMash) );
   // And let the recipe recalc all?
   if ( !noCopy && transact )
      rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Misc* m, bool noCopy, bool transact )
{
   try {
      addIngredientToRecipe( rec, m, noCopy, &allMiscs, true, transact );
   }
   catch (QString e) {
      throw;
   }

   if ( transact && ! noCopy )
      rec->recalcAll();

}

void Database::addToRecipe( Recipe* rec, QList<Misc*>miscs, bool transact )
{
   if ( miscs.size() == 0 )
      return;

   if ( transact )
      sqlDatabase().transaction();

   try {
      foreach (Misc* misc, miscs ) {
         addIngredientToRecipe( rec, misc, false, &allMiscs,true,false );
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact ) {
         sqlDatabase().rollback();
      }
      throw;
   }
   if ( transact ) {
      sqlDatabase().commit();
      rec->recalcAll();
   }
}

void Database::addToRecipe( Recipe* rec, Water* w, bool noCopy, bool transact )
{

   try {
      addIngredientToRecipe( rec, w, noCopy, &allWaters,true,transact );
   }
   catch (QString e) {
      throw;
   }
}

void Database::addToRecipe( Recipe* rec, Salt* s, bool noCopy, bool transact )
{

   try {
      addIngredientToRecipe( rec, s, noCopy, &allSalts,true,transact );
   }
   catch (QString e) {
      throw;
   }
}

void Database::addToRecipe( Recipe* rec, Style* s, bool noCopy, bool transact )
{
   Style* newStyle = s;
   TableSchema* tbl = dbDefn->table(Brewtarget::RECTABLE);

   if ( s == nullptr )
      return;

   if ( transact )
      sqlDatabase().transaction();

   try {
      if ( ! noCopy )
         newStyle = copy<Style>(s, &allStyles, false);

      sqlUpdate(Brewtarget::RECTABLE,
                QString("%1=%2").arg(tbl->foreignKeyToColumn(kpropStyleId)).arg(newStyle->key()),
                QString("%1=%2").arg(tbl->keyName()).arg(rec->_key));
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
   }
   // Emit a changed signal.
   rec->m_style_id = newStyle->key();
   emit rec->changed( rec->metaProperty("style"), Ingredient::qVariantFromPtr(newStyle) );
}

void Database::addToRecipe( Recipe* rec, Yeast* y, bool noCopy, bool transact )
{
   try {
      Yeast* newYeast = addIngredientToRecipe<Yeast>( rec, y, noCopy, &allYeasts, true, transact );
      connect( newYeast, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptYeastChange(QMetaProperty,QVariant)));
      if ( transact && ! noCopy )
      {
         rec->recalcOgFg();
         rec->recalcABV_pct();
      }
   }
   catch (QString e) {
      throw;
   }
}

void Database::addToRecipe( Recipe* rec, QList<Yeast*>yeasts, bool transact )
{
   if ( yeasts.size() == 0 )
      return;

   if ( transact )
      sqlDatabase().transaction();

   try {
      foreach (Yeast* yeast, yeasts )
      {
         Yeast* newYeast = addIngredientToRecipe( rec, yeast, false, &allYeasts,true,false );
         connect( newYeast, SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptYeastChange(QMetaProperty,QVariant)));
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact ) {
      sqlDatabase().commit();
      rec->recalcOgFg();
      rec->recalcABV_pct();
   }
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
           .arg(dbDefn->table(Brewtarget::EQUIPTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::EQUIPTABLE, allEquipments);
   return tmp;
}

QList<Fermentable*> Database::fermentables()
{
   QList<Fermentable*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::FERMTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::FERMTABLE, allFermentables);
   return tmp;
}

QList<Hop*> Database::hops()
{
   QList<Hop*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::HOPTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::HOPTABLE, allHops);
   return tmp;
}

QList<Mash*> Database::mashs()
{
   QList<Mash*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::MASHTABLE)->propertyToColumn(kpropDeleted))
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
           .arg(tbl->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse())
           .arg(tbl->propertyToColumn(kpropStepNumber));
   getElements( tmp, query, Brewtarget::MASHSTEPTABLE, allMashSteps);
   return tmp;
}

QList<Misc*> Database::miscs()
{
   QList<Misc*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::MISCTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::MISCTABLE, allMiscs );
   return tmp;
}

QList<Recipe*> Database::recipes()
{
   QList<Recipe*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::RECTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   // This is gonna kill me.
   getElements( tmp, query, Brewtarget::RECTABLE, allRecipes );
   return tmp;
}

QList<Style*> Database::styles()
{
   QList<Style*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::STYLETABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::STYLETABLE, allStyles );
   return tmp;
}

QList<Water*> Database::waters()
{
   QList<Water*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::WATERTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::WATERTABLE, allWaters );
   return tmp;
}

QList<Salt*> Database::salts()
{
   QList<Salt*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::SALTTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::SALTTABLE, allSalts );
   return tmp;
}

QList<Yeast*> Database::yeasts()
{
   QList<Yeast*> tmp;
   QString query = QString("%1=%2")
           .arg(dbDefn->table(Brewtarget::YEASTTABLE)->propertyToColumn(kpropDeleted))
           .arg(Brewtarget::dbFalse());
   getElements( tmp, query, Brewtarget::YEASTTABLE, allYeasts );
   return tmp;
}

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
         Brewtarget::logE(QString("Database migration %1->%2 failed").arg(currentVersion).arg(newVersion));
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
         populateChildTablesByName();

         QSqlQuery popchildq( "UPDATE settings SET repopulateChildrenOnNextStart = 0", sqlDatabase() );
         if ( ! popchildq.isActive() )
            throw QString("Could not modify settings table: %1 %2").arg(popchildq.lastQuery()).arg(popchildq.lastError().text());
      }
   }
   catch (QString e ) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();

   return doUpdate;
}

bool Database::importFromXML(const QString& filename)
{
   int count;
   int line, col;
   QDomDocument xmlDoc;
   QDomElement root;
   QDomNodeList list;
   QString err;
   QFile inFile;
   QStringList tags = QStringList() << "EQUIPMENT" << "FERMENTABLE" << "HOP" << "MISC" << "STYLE" << "YEAST" << "WATER" << "MASHS";
   inFile.setFileName(filename);
   bool ret = true;

   if( ! inFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::logW(QString("Database::importFromXML: Could not open %1 for reading.").arg(filename));
      return false;
   }

   if( ! xmlDoc.setContent(&inFile, false, &err, &line, &col) )
      Brewtarget::logW(QString("Database::importFromXML: Bad document formatting in %1 %2:%3. %4").arg(filename).arg(line).arg(col).arg(err) );

   list = xmlDoc.elementsByTagName("RECIPE");
   if ( list.count() )
   {
      for(int i = 0; i < list.count(); ++i )
      {
         Recipe* temp = m_beerxml->recipeFromXml( list.at(i) );
         if ( ! temp || ! temp->isValid() )
            ret = false;
      }
   }
   else
   {
      foreach (QString tag, tags)
      {
         list = xmlDoc.elementsByTagName(tag);
         count = list.size();

         if ( count > 0 )
         {
            // Tell how many there were in the status bar.
            //statusBar()->showMessage( tr("Found %1 %2.").arg(count).arg(tag.toLower()), 5000 );

            if (tag == "RECIPE")
            {
            }
            else if ( tag == "EQUIPMENT" )
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Equipment* temp = m_beerxml->equipmentFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "FERMENTABLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Fermentable* temp = m_beerxml->fermentableFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }

            }
            else if (tag == "HOP")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Hop* temp = m_beerxml->hopFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "MISC")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Misc* temp = m_beerxml->miscFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "STYLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Style* temp = m_beerxml->styleFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "YEAST")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Yeast* temp = m_beerxml->yeastFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "WATER" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Water* temp = m_beerxml->waterFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "MASHS" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Mash* temp = m_beerxml->mashFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
         }
      }
   }
   return ret;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QMap<QString, std::function<Ingredient*(QString name)> > Database::makeTableParams()
{
   QMap<QString, std::function<Ingredient*(QString name)> > tmp;
   //=============================Equipment====================================

   tmp.insert(ktableEquipment,   [&](QString name) { return this->newEquipment(); } );
   tmp.insert(ktableFermentable, [&](QString name) { return this->newFermentable(); } );
   tmp.insert(ktableHop,         [&](QString name) { return this->newHop(); } );
   tmp.insert(ktableMisc,        [&](QString name) { return this->newMisc(); } );
   tmp.insert(ktableStyle,       [&](QString name) { return this->newStyle(name); } );
   tmp.insert(ktableYeast,       [&](QString name) { return this->newYeast(); } );
   tmp.insert(ktableWater,       [&](QString name) { return this->newWater(); } );
   tmp.insert(ktableSalt,        [&](QString name) { return this->newSalt(); } );

   return tmp;
}

void Database::updateDatabase(QString const& filename)
{
   // In the naming here "old" means our local database, and
   // "new" means the database coming from 'filename'.

   QVariant btid, newid, oldid;
   QMap<QString, std::function<Ingredient*(QString name)> >  makeObject = makeTableParams();

   try {
      QString newCon("newSqldbCon");
      QSqlDatabase newSqldb = QSqlDatabase::addDatabase("QSQLITE", newCon);
      newSqldb.setDatabaseName(filename);
      if( ! newSqldb.open() ) {
         QMessageBox::critical(nullptr,
                              QObject::tr("Database Failure"),
                              QString(QObject::tr("Failed to open the database '%1'.").arg(filename)));
         throw QString("Could not open %1 for reading.\n%2").arg(filename).arg(newSqldb.lastError().text());
      }

      // This is the basic gist...
      // For each (id, hop_id) in newSqldb.bt_hop...

      // Call this newRecord
      // SELECT * FROM newSqldb.hop WHERE id=<hop_id>

      // UPDATE hop SET name=:name, alpha=:alpha,... WHERE id=(SELECT hop_id FROM bt_hop WHERE id=:bt_id)

      // Bind :bt_id from <id>
      // Bind :name, :alpha, ..., from newRecord.

      // Execute.

      foreach( TableSchema* tbl, dbDefn->baseTables() )
      {
         TableSchema* btTbl = dbDefn->btTable(tbl->dbTable());
         // not all tables have bt* tables
         if ( btTbl == nullptr ) {
            continue;
         }
         QSqlQuery qNewBtIng( QString("SELECT * FROM %1").arg(btTbl->tableName()), newSqldb );

         QSqlQuery qNewIng( newSqldb );
         qNewIng.prepare(QString("SELECT * FROM %1 WHERE %2=:id").arg(tbl->tableName()).arg(tbl->keyName()));

         // Construct the big update query.
         QSqlQuery qUpdateOldIng( sqlDatabase() );
         QString updateString = tbl->generateUpdateRow();
         qUpdateOldIng.prepare(updateString);

         QSqlQuery qOldBtIng( sqlDatabase() );
         qOldBtIng.prepare( QString("SELECT * FROM %1 WHERE %2=:btid").arg(btTbl->tableName()).arg(btTbl->keyName()) );

         QSqlQuery qOldBtIngInsert( sqlDatabase() );
         qOldBtIngInsert.prepare( QString("INSERT INTO %1 (%2,%3) values (:id,:%3)")
                                  .arg(btTbl->tableName())
                                  .arg(btTbl->keyName())
                                  .arg(btTbl->childIndexName()));

         while( qNewBtIng.next() ) {
            btid = qNewBtIng.record().value(btTbl->keyName());
            newid = qNewBtIng.record().value(btTbl->childIndexName());

            qNewIng.bindValue(":id", newid);
            // if we can't run the query
            if ( ! qNewIng.exec() )
               throw QString("Could not retrieve new ingredient: %1 %2").arg(qNewIng.lastQuery()).arg(qNewIng.lastError().text());

            // if we can't get the result from the query
            if( !qNewIng.next() )
               throw QString("Could not advance query: %1 %2").arg(qNewIng.lastQuery()).arg(qNewIng.lastError().text());

            foreach( QString pn, tbl->allColumnNames()) {
               // Bind the old values to the new unless it is deleted, which we always set to false
               if ( pn == kcolDeleted ) {
                  qUpdateOldIng.bindValue( QString(":%1").arg(pn), Brewtarget::dbFalse());
               }
               qUpdateOldIng.bindValue( QString(":%1").arg(pn), qNewIng.record().value(pn));
            }

            // Done retrieving new ingredient data.
            qNewIng.finish();

            // Find the bt_<ingredient> record in the local table.
            qOldBtIng.bindValue( ":btid", btid );
            if ( ! qOldBtIng.exec() ) {
               throw QString("Could not find btID (%1): %2 %3")
                        .arg(btid.toInt())
                        .arg(qOldBtIng.lastQuery())
                        .arg(qOldBtIng.lastError().text());
            }

            // If the btid exists in the old bt_hop table, do an update.
            if( qOldBtIng.next() ) {
               oldid = qOldBtIng.record().value( btTbl->keyName() );
               qOldBtIng.finish();

               qUpdateOldIng.bindValue( ":id", oldid );

               if ( ! qUpdateOldIng.exec() )
                  throw QString("Could not update old btID (%1): %2 %3")
                           .arg(oldid.toInt())
                           .arg(qUpdateOldIng.lastQuery())
                           .arg(qUpdateOldIng.lastError().text());

            }
            // If the btid doesn't exist in the old bt_ table, do an insert into
            // the new table, then into the new bt_ table.
            else {
               // Create a new ingredient.
               oldid = makeObject.value(tbl->tableName())(qNewBtIng.record().value("name").toString())->_key;

               // Copy in the new data.
               qUpdateOldIng.bindValue( ":id", oldid );

               if ( ! qUpdateOldIng.exec() )
                  throw QString("Could not insert new btID (%1): %2 %3")
                           .arg(oldid.toInt())
                           .arg(qUpdateOldIng.lastQuery())
                           .arg(qUpdateOldIng.lastError().text());


               // Insert an entry into our bt_<ingredient> table.
               qOldBtIngInsert.bindValue( ":id", btid );
               qOldBtIngInsert.bindValue( QString(":%1").arg(btTbl->childIndexName()), oldid );

               if ( !  qOldBtIngInsert.exec() )
                  throw QString("Could not insert btID (%1): %2 %3")
                           .arg(btid.toInt())
                           .arg(qOldBtIngInsert.lastQuery())
                           .arg(qOldBtIngInsert.lastError().text());
            }
         }
      }
      // If we, by some miracle, get here, commit
      sqlDatabase().commit();
      // I think
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      abort();
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
         Brewtarget::logW( QString("It appears the database is already configured."));
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
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
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
   else if ( field.name() == "fermentDate" && field.value().toString() == "CURRENT_DATETIME" ) {
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

      QString findAllQuery = QString("SELECT * FROM %1 order by %2 asc").arg(tname).arg(table->keyName(oldType));
      try {
         if (! readOld.exec(findAllQuery) )
            throw QString("Could not execute %1 : %2")
               .arg(readOld.lastQuery())
               .arg(readOld.lastError().text());

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

            // All that's left is to bind
            for(int i = 0; i < here.count(); ++i) {
               if ( table->dbTable() == Brewtarget::BREWNOTETABLE
                    && here.fieldName(i) == kpropBrewDate ) {
                  QVariant helpme(here.field(i).value().toString());
                  upsertNew.bindValue(":brewdate",helpme);
               }
               else {
                  upsertNew.bindValue(QString(":%1").arg(here.fieldName(i)),
                                      convertValue(newType, here.field(i)));
               }
            }
            // and execute
            if ( ! upsertNew.exec() )
               throw QString("Could not insert new row %1 : %2")
                  .arg(upsertNew.lastQuery())
                  .arg(upsertNew.lastError().text());
         }
         // We need to create the increment and decrement things for the
         // instructions_in_recipe table. This seems a little weird to do this
         // here, but it makes sense to wait until after we've inserted all
         // the data. The increment trigger happens on insert, and I suspect
         // bad things would happen if it were in place before we inserted all our data.
         if ( table->dbTable() == Brewtarget::INSTINRECTABLE ) {
            QString trigger = table->generateIncrementTrigger(newType);
            if ( trigger.isEmpty() ) {
               Brewtarget::logE(QString("No increment triggers found for %1").arg(table->tableName()));
            }
            else {
               upsertNew.exec(trigger);
               trigger =  table->generateDecrementTrigger(newType);
               if ( trigger.isEmpty() ) {
                  Brewtarget::logE(QString("No decrement triggers found for %1").arg(table->tableName()));
               }
               else {
                  if ( ! upsertNew.exec(trigger) ) {
                     throw QString("Could not insert new row %1 : %2")
                        .arg(upsertNew.lastQuery())
                        .arg(upsertNew.lastError().text());
                  }
               }
            }
         }
         // We need to manually reset the sequences
         if ( newType == Brewtarget::PGSQL ) {
             // this probably should be fixed somewhere, but this is enough for now?
            //
            QString seq = QString("SELECT setval('%1_%2_seq',(SELECT MAX(id) FROM %1))").arg(table->tableName()).arg(table->keyName());

            if ( ! upsertNew.exec(seq) )
               throw QString("Could not reset the sequences: %1 %2")
                  .arg(seq).arg(upsertNew.lastError().text());
         }
      }
      catch (QString e) {
         Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
         newDb.rollback();
         throw;
      }

      newDb.commit();
   }
}

