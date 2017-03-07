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
#include "yeast.h"

#include "config.h"
#include "brewtarget.h"
#include "QueuedMethod.h"
#include "DatabaseSchemaHelper.h"

// Static members.
Database* Database::dbInstance = 0;
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
QHash<Brewtarget::DBTable,QString> Database::tableNames;
QHash<QString,Brewtarget::DBTable> Database::classNameToTable;
QHash<Brewtarget::DBTable,Brewtarget::DBTable> Database::tableToChildTable = Database::tableToChildTableHash();
QHash<Brewtarget::DBTable,Brewtarget::DBTable> Database::tableToInventoryTable = Database::tableToInventoryTableHash();

QHash< QThread*, QString > Database::_threadToConnection;
QMutex Database::_threadToConnectionMutex;

Database::Database()
{
   //.setUndoLimit(100);
   // Lock this here until we actually construct the first database connection.
   _threadToConnectionMutex.lock();
   converted = false;
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
   qDeleteAll(allYeasts);
   qDeleteAll(allRecipes);

}

bool Database::loadSQLite()
{
   bool dbIsOpen;
   QSqlDatabase sqldb;

   // Set file names.
   dbFileName = Brewtarget::getUserDataDir().filePath("database.sqlite");
   dataDbFileName = Brewtarget::getDataDir().filePath("default_db.sqlite");

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
   sqldb = QSqlDatabase::addDatabase("QSQLITE");
   sqldb.setDatabaseName(dbFileName);
   dbIsOpen = sqldb.open();
   dbConName = sqldb.connectionName();

   if( ! dbIsOpen )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text()));
      if (Brewtarget::isInteractive()) {
         QMessageBox::critical(
            0,
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
         _threadToConnection.insert(QThread::currentThread(), sqldb.connectionName());
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
         dbPassword = QInputDialog::getText(0,tr("Database password"),
               tr("Password"), QLineEdit::Password,QString(),&isOk);
         if ( isOk ) {
            isOk = verifyDbConnection( Brewtarget::PGSQL, dbHostname, dbPortnum, dbSchema,
                                 dbName, dbUsername, dbPassword);
         }
      }
   }

   sqldb = QSqlDatabase::addDatabase("QPSQL","brewtarget");
   sqldb.setHostName( dbHostname );
   sqldb.setDatabaseName( dbName );
   sqldb.setUserName( dbUsername );
   sqldb.setPort( dbPortnum );
   sqldb.setPassword( dbPassword );

   dbIsOpen = sqldb.open();
   dbConName = sqldb.connectionName();

   if( ! dbIsOpen ) {
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text()));
      QMessageBox::critical(0,
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

   if ( Brewtarget::dbType() == Brewtarget::PGSQL )
   {
      dbIsOpen = loadPgSQL();
   }
   else
   {
      dbIsOpen = loadSQLite();
   }

   _threadToConnectionMutex.unlock();
   if ( ! dbIsOpen )
      return false;

   sqldb = sqlDatabase();

   // This should work regardless of the db being used.
   if( createFromScratch )
   {
         bool success = DatabaseSchemaHelper::create(sqldb);
         if( !success )
         {
            Brewtarget::logE("DatabaseSchemaHelper::create() failed");
            return success;
         }
   }

   // Update the database if need be. This has to happen before we do anything
   // else or we dump core
   bool schemaErr = false;
   schemaUpdated = updateSchema(&schemaErr);

   // Since updateSchema could add new tables, we have to wait until this
   // point to populate the tables.
   Database::tableNames = tableNamesHash();
   Database::classNameToTable = classNameToTableHash();

   if( schemaErr )
   {
      if (Brewtarget::isInteractive()) {
         QMessageBox::critical(
            0,
            QObject::tr("Database Failure"),
            QObject::tr("Failed to update the database")
         );
      }
      return false;
   }

   // Initialize the SELECT * query hashes.
   // selectAll = Database::selectAllHash();
   // See if there are new ingredients that we need to merge from the data-space db.
   if( dataDbFile.fileName() != dbFile.fileName()
      && ! Brewtarget::userDatabaseDidNotExist // Don't do this if we JUST copied the dataspace database.
      && QFileInfo(dataDbFile).lastModified() > Brewtarget::lastDbMergeRequest )
   {

      if(
         Brewtarget::isInteractive() &&
         QMessageBox::question(
            0,
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
         connect( e, &BeerXMLElement::changed, *i, &Recipe::acceptEquipChange );
         connect( e, &Equipment::changedBoilSize_l, *i, &Recipe::setBoilSize_l);
         connect( e, &Equipment::changedBoilTime_min, *i, &Recipe::setBoilTime_min);
      }

      QList<Fermentable*> tmpF = fermentables(*i);
      for( j = tmpF.begin(); j != tmpF.end(); ++j )
         connect( *j, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptFermChange(QMetaProperty,QVariant)) );

      QList<Hop*> tmpH = hops(*i);
      for( k = tmpH.begin(); k != tmpH.end(); ++k )
         connect( *k, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptHopChange(QMetaProperty,QVariant)) );

      QList<Yeast*> tmpY = yeasts(*i);
      for( l = tmpY.begin(); l != tmpY.end(); ++l )
         connect( *l, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptYeastChange(QMetaProperty,QVariant)) );

      connect( mash(*i), SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptMashChange(QMetaProperty,QVariant)) );
   }

   QList<Mash*> tmpM = mashs();
   for( m = tmpM.begin(); m != tmpM.end(); ++m )
   {
      QList<MashStep*> tmpMS = mashSteps(*m);
      for( n=tmpMS.begin(); n != tmpMS.end(); ++n)
         connect( *n, SIGNAL(changed(QMetaProperty,QVariant)), *m, SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );
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

      DatabaseSchemaHelper::create(sqldb);

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
   QSqlDatabase::database( dbConName, false ).close();
   QSqlDatabase::removeDatabase( dbConName );

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
   QStringList fileNames = listOfFiles.split(",", QString::SkipEmptyParts);

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
   dbInstance=0;
   mutex.unlock();

}

bool Database::backupToDir(QString dir,QString filename)
{
   // Make sure the singleton exists.
   instance();

   bool success = true;
   QString prefix = dir + "/";
   QString newDbFileName = prefix + "database.sqlite";

   if ( filename.isEmpty() ) {
      newDbFileName = prefix + "database.sqlite";
   }
   else {
      newDbFileName = prefix + filename;
   }

   // Remove the files if they already exist so that
   // the copy() operation will succeed.
   QFile::remove(newDbFileName);

   success = dbFile.copy( newDbFileName );

   return success;
}

bool Database::restoreFromFile(QString newDbFileStr)
{
   bool success = true;
   /*
   QString prefix = dirStr + "/";
   QString newDbFileName = prefix + "database.sqlite";
   QFile newDbFile(newDbFileName);
   */

   QFile newDbFile(newDbFileStr);
   // Fail if we can't find file.
   if( !newDbFile.exists() )
      return false;

   success &= newDbFile.copy(QString("%1.new").arg(dbFile.fileName()));
   QFile::setPermissions( newDbFile.fileName(), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );

   return success;
}

// removeFromRecipe ===========================================================
void Database::removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing )
{
   QString tableName = tableNames[classNameToTable[ing->metaObject()->className()]];
   const QMetaObject* meta = ing->metaObject();

   int ndx = meta->indexOfClassInfo("signal");
   QString propName, relTableName, ingKeyName, childTableName;

   sqlDatabase().transaction();
   QSqlQuery q(sqlDatabase());

   try {
      if ( ndx != -1 ) {
         QString prefix = meta->classInfo( meta->indexOfClassInfo("prefix")).value();
         propName  = meta->classInfo(ndx).value();
         relTableName = QString("%1_in_recipe").arg(prefix);
         ingKeyName = QString("%1_id").arg(prefix);
         childTableName = QString("%1_children").arg(prefix);
      }
      else
         throw QString("could not locate classInfo for signal on %2").arg(meta->className());


      // We need to do many things -- remove the link in *in_recipe,
      // remove the entry from *_children
      // and DELETE THE COPY
      QString deleteFromInRecipe = QString("DELETE FROM %1 WHERE %2=%3 AND recipe_id=%4")
                                 .arg(relTableName)
                                 .arg(ingKeyName)
                                 .arg(ing->_key)
                                 .arg(rec->_key);
      QString deleteFromChildren = QString("DELETE FROM %1 WHERE child_id=%2")
                                 .arg(childTableName)
                                 .arg(ing->_key);
      QString deleteIngredient = QString("DELETE FROM %1 where id=%2")
                                 .arg(tableName)
                                 .arg(ing->_key);
      q.setForwardOnly(true);

      if ( ! q.exec(deleteFromInRecipe) )
         throw QString("failed to delete in_recipe.");

      // I don't really like this, but I can't think of a better solution. Of
      // all the ingredients, instructions don't have a _children table. Given
      // that it is only one table, I will try the easy way first
      if ( tableName != "instruction" && ! q.exec( deleteFromChildren ) )
         throw QString("failed to delete children.");

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
      throw QString("%1 %2 %3 %4").arg(Q_FUNC_INFO).arg(e).arg(q.lastQuery()).arg(q.lastError().text());

   }

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
   // Just mark the step as deleted.
   try {
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
               QString("deleted = %1").arg(Brewtarget::dbTrue()),
               QString("id=%1").arg(step->_key) );
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
   QString query = QString("SELECT recipe_id FROM brewnote WHERE id = %1").arg(note->_key);

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
   key = q.record().value("recipe_id").toInt();
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

void Database::swapMashStepOrder(MashStep* m1, MashStep* m2)
{
   QString update = QString("UPDATE mashstep SET step_number = CASE id WHEN %1 then %2 when %3 then %4 END WHERE id IN (%1,%3)")
                .arg(m1->_key).arg(m2->stepNumber()).arg(m2->_key).arg(m1->stepNumber());

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
   QString update =
      QString(
         "UPDATE instruction_in_recipe "
         "SET instruction_number = "
         "  CASE instruction_id "
         "    WHEN %1 THEN %3 "
         "    WHEN %2 THEN %4 "
         "  END "
         "WHERE instruction_id IN (%1,%2)")
      .arg(in1->_key).arg(in2->_key).arg(in2->instructionNumber()).arg(in1->instructionNumber());
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
      throw;;
   }

   q.finish();

   emit in1->changed( in1->metaProperty("instructionNumber") );
   emit in2->changed( in2->metaProperty("instructionNumber") );
}

void Database::insertInstruction(Instruction* in, int pos)
{
   int parentRecipeKey;
   QString query = QString("SELECT recipe_id FROM instruction_in_recipe WHERE instruction_id=%2")
                   .arg(in->_key);
   QString update;

   sqlDatabase().transaction();

   QSqlQuery q(sqlDatabase());

   try {
      if ( !q.exec(query) )
         throw QString("failed to find recipe");

      q.next();
      parentRecipeKey = q.record().value("recipe_id").toInt();
      q.finish();

      // Increment all instruction positions greater or equal to pos.
      update = QString(
            "UPDATE instruction_in_recipe "
            "SET instruction_number=instruction_number+1 "
            "WHERE recipe_id=%1 AND instruction_number>=%2")
         .arg(parentRecipeKey).arg(pos);

      if ( !q.exec(update) )
         throw QString("failed to renumber instructions recipe");

      // This is sort of spooky action at a distance -- the emit should really be
      // happening with the update. So it goes.
      query = QString("SELECT instruction_id as id, instruction_number as pos FROM instruction_in_recipe WHERE recipe_id=%1 and instruction_number>%2")
         .arg(parentRecipeKey).arg(pos);

      if ( !q.exec(query) )
         throw QString("failed to find renumbered instructions");

      while( q.next() ) {
         Instruction* inst = allInstructions[ q.record().value("id").toInt() ];
         int newPos = q.record().value("pos").toInt();

         emit inst->changed( inst->metaProperty("instructionNumber"),newPos );
      }

      // Change in's position to pos.
      update = QString(
            "UPDATE instruction_in_recipe "
            "SET instruction_number=%1 "
            "WHERE instruction_id=%2"
         ).arg(pos).arg(in->_key);

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
   QString filterString = QString("recipe_id = %1 AND deleted = %2").arg(parent->_key).arg(Brewtarget::dbFalse());

   getElements(ret, filterString, Brewtarget::BREWNOTETABLE, allBrewNotes);

   return ret;
}

QList<Fermentable*> Database::fermentables(Recipe const* parent)
{
   QList<Fermentable*> ret;
   QString filter = QString("recipe_id = %1").arg(parent->_key);

   getElements(ret,filter, Brewtarget::FERMINRECTABLE, allFermentables, "fermentable_id");

   return ret;
}

QList<Hop*> Database::hops(Recipe const* parent)
{
   QList<Hop*> ret;
   QString filter = QString("recipe_id = %1").arg(parent->_key);

   getElements(ret,filter, Brewtarget::HOPINRECTABLE, allHops, "hop_id");

   return ret;
}

QList<Misc*> Database::miscs(Recipe const* parent)
{
   QList<Misc*> ret;
   QString filter = QString("recipe_id = %1").arg(parent->_key);


   getElements(ret,filter, Brewtarget::MISCINRECTABLE, allMiscs, "misc_id");

   return ret;
}

Equipment* Database::equipment(Recipe const* parent)
{
   int id = get( Brewtarget::RECTABLE, parent->key(), "equipment_id" ).toInt();

   if( allEquipments.contains(id) )
      return allEquipments[id];
   else
      return 0;
}

Style* Database::style(Recipe const* parent)
{
   int id = get( Brewtarget::RECTABLE, parent->key(), "style_id").toInt();

   if( allStyles.contains(id) )
      return allStyles[id];
   else
      return 0;
}

Mash* Database::mash( Recipe const* parent )
{
   int mashId = get( Brewtarget::RECTABLE, parent->key(), "mash_id" ).toInt();

   if( allMashs.contains(mashId) )
      return allMashs[mashId];
   else
      return 0;
}

QList<MashStep*> Database::mashSteps(Mash const* parent)
{
   QList<MashStep*> ret;
   QString filterString = QString("mash_id = %1 AND deleted = %2 order by step_number").arg(parent->_key).arg(Brewtarget::dbFalse());

   getElements(ret, filterString, Brewtarget::MASHSTEPTABLE, allMashSteps);

   return ret;
}

QList<Instruction*> Database::instructions( Recipe const* parent )
{
   QList<Instruction*> ret;
   QString filter = QString("recipe_id = %1 ORDER BY instruction_number ASC").arg(parent->_key);

   getElements(ret,filter,Brewtarget::INSTINRECTABLE,allInstructions,"instruction_id");

   return ret;
}

QList<Water*> Database::waters(Recipe const* parent)
{
   QList<Water*> ret;
   QString filter = QString("recipe_id = %1").arg(parent->_key);

   getElements(ret,filter,Brewtarget::WATERINRECTABLE,allWaters,"water_id");

   return ret;
}

QList<Yeast*> Database::yeasts(Recipe const* parent)
{
   QList<Yeast*> ret;
   QString filter = QString("recipe_id = %1").arg(parent->_key);

   getElements(ret,filter,Brewtarget::YEASTINRECTABLE,allYeasts,"yeast_id");

   return ret;
}

// Named constructors =========================================================

BrewNote* Database::newBrewNote(BrewNote* other, bool signal)
{
   BrewNote* tmp = copy<BrewNote>(other, &allBrewNotes);

   if ( tmp ) {
      if ( signal )
      {
         emit changed( metaProperty("brewNotes"), QVariant() );
         emit newBrewNoteSignal(tmp);
      }

   }
   return tmp;
}

BrewNote* Database::newBrewNote(Recipe* parent, bool signal)
{
   BrewNote* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newIngredient(&allBrewNotes);

      sqlUpdate( Brewtarget::BREWNOTETABLE,
               QString("recipe_id=%1").arg(parent->_key),
               QString("id=%2").arg(tmp->_key) );

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
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

   if (other)
      tmp = copy(other, &allFermentables);
   else
      tmp = newIngredient(&allFermentables);

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

   if ( other )
      tmp = copy(other, &allHops);
   else
      tmp = newIngredient(&allHops);

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
   // TODO: encapsulate in QUndoCommand.
   Instruction* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newIngredient(&allInstructions);

      // Add without copying to "instruction_in_recipe". We already have a
      // transaction open, so tell addIng to not worry about it
      tmp = addIngredientToRecipe<Instruction>(rec,tmp,true,0,false,false);
   }
   catch ( QString e ) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   // Database's instructions have changed.
   sqlDatabase().commit();
   emit changed( metaProperty("instructions"), QVariant() );

   return tmp;
}

int Database::instructionNumber(Instruction const* in)
{
   QSqlQuery q(
      QString(
         "SELECT instruction_number FROM instruction_in_recipe WHERE instruction_id=%1"
      ).arg(in->_key),
      sqlDatabase()
   );

   if( q.next() )
      return q.record().value("instruction_number").toInt();
   else
      return 0;
}

Mash* Database::newMash(Mash* other, bool displace)
{
   Mash* tmp;

   if ( other ) {
      sqlDatabase().transaction();
   }

   try {
      if ( other )
         tmp = copy<Mash>(other, &allMashs);
      else
         tmp = newIngredient(&allMashs);

      if ( other ) {
         // Just copying the Mash isn't enough. We need to copy the mashsteps too
         duplicateMashSteps(other,tmp);

         // Connect tmp to parent, removing any existing mash in parent.
         // This doesn't really work. It simply orphans the old mash and its
         // steps.
         if( displace )
         {
            sqlUpdate( Brewtarget::RECTABLE,
                       QString("mash_id=%1").arg(tmp->_key),
                       QString("mash_id=%1").arg(other->_key) );
         }
      }
   }
   catch (QString e) {
      if ( other )
         sqlDatabase().rollback();
      throw;
   }

   if ( other )
      sqlDatabase().commit();
   emit changed( metaProperty("mashs"), QVariant() );
   emit newMashSignal(tmp);

   return tmp;
}

Mash* Database::newMash(Recipe* parent, bool transact)
{
   Mash* tmp;

   if ( transact )
      sqlDatabase().transaction();

   try {
      tmp = newIngredient(&allMashs);

      // Connect tmp to parent, removing any existing mash in parent.
      sqlUpdate( Brewtarget::RECTABLE,
                 QString("mash_id=%1").arg(tmp->_key),
                 QString("id=%1").arg(parent->_key) );
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( transact )
         sqlDatabase().rollback();
      throw;
   }

   if ( transact )
      sqlDatabase().commit();

   emit changed( metaProperty("mashs"), QVariant() );
   emit newMashSignal(tmp);

   connect( tmp, SIGNAL(changed(QMetaProperty,QVariant)), parent, SLOT(acceptMashChange(QMetaProperty,QVariant)) );
   return tmp;
}

// If we are doing triggers for instructions, why aren't we doing triggers for
// mash steps?
MashStep* Database::newMashStep(Mash* mash, bool connected)
{
   // TODO: encapsulate in QUndoCommand.
   // NOTE: we have unique(mash_id,step_number) constraints on this table,
   // so may have to pay special attention when creating the new record.
   MashStep* tmp;
   QString coalesce = QString( "step_number = (SELECT COALESCE(MAX(step_number)+1,0) FROM %1 WHERE deleted=%2 AND mash_id=%3 )")
                        .arg(tableNames[Brewtarget::MASHSTEPTABLE])
                        .arg(Brewtarget::dbFalse())
                        .arg(mash->_key);

   sqlDatabase().transaction();

   QSqlQuery q(sqlDatabase());
   q.setForwardOnly(true);

   // mashsteps are weird, because we have to do the linking between step and
   // mash
   try {
      tmp = newIngredient(&allMashSteps);

      // I *think* we need to set the mash_id first
      sqlUpdate( Brewtarget::MASHSTEPTABLE,
                 QString("mash_id=%1 ").arg(mash->_key),
                 QString("id=%1").arg(tmp->_key)
               );

      // Just sets the step number within the mash to the next available number.
      // we need coalesce here instead of isnull. coalesce is SQL standard, so
      // should be more widely supported than isnull
      sqlUpdate( Brewtarget::MASHSTEPTABLE, coalesce, QString("id=%1").arg(tmp->_key) );
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

   if ( other )
     tmp = copy(other, &allMiscs);
   else
      tmp = newIngredient(&allMiscs);

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

Recipe* Database::newRecipe()
{
   Recipe* tmp;

   sqlDatabase().transaction();

   try {
      tmp = newIngredient(&allRecipes);

      newMash(tmp,false);
   }
   catch (QString e ) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   sqlDatabase().commit();
   emit changed( metaProperty("recipes"), QVariant() );
   emit newRecipeSignal(tmp);

   return tmp;
}

// TODO: Oh my. This the entire thing should be transacted. It took some work
// to get all the addToRecipe methods to play nice.
//
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
      if ( other )
         tmp = copy(other, &allStyles);
      else
         tmp = newIngredient(&allStyles);
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

Yeast* Database::newYeast(Yeast* other)
{
   Yeast* tmp;

   try {
      if (other)
         tmp = copy(other, &allYeasts);
      else
         tmp = newIngredient(&allYeasts);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      throw;
   }

   emit changed( metaProperty("yeasts"), QVariant() );
   emit newYeastSignal(tmp);

   return tmp;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Database::deleteRecord( Brewtarget::DBTable table, BeerXMLElement* object )
{
   int ndx = object->metaObject()->indexOfProperty("deleted");

   try {
      updateEntry( table, object->_key, "deleted", Brewtarget::dbTrue(),
                   object->metaObject()->property(ndx), object, true);
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

         // Put it in the new mash.
         sqlUpdate( Brewtarget::MASHSTEPTABLE,
                      QString("mash_id=%1").arg(newMash->key()),
                      QString("id=%1").arg(newStep->key())
                  );
         // Make the new mash pay attention to the new step.
         connect( newStep, &BeerXMLElement::changed,
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

// Cthulhu weeps (and we lose 2 SAN points)
void Database::updateEntry( Brewtarget::DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify, bool transact )
{
   // Assumes the table has a column called 'deleted'.
   QString tableName = tableNames[table];

   if ( transact )
      sqlDatabase().transaction();

   try {
      QSqlQuery update( sqlDatabase() );
      QString command = QString("UPDATE %1 set %2=:value where id=%3")
                           .arg(tableName)
                           .arg(col_name)
                           .arg(key);

      update.prepare( command );
      update.bindValue(":value", value);

      if ( ! update.exec() )
         throw QString("Could not update %1.%2 to %3: %4 %5")
                  .arg( tableName )
                  .arg( col_name )
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
      emit object->changed(prop,value);

}

void Database::updateColumns(Brewtarget::DBTable table, int key, const QVariantMap& colValMap)
{
   // Assumes the table has a column called 'deleted'.
   QString tableName = tableNames[table];
   try {

      static const QString kSetStr("%1=?, ");
      static const QString kSetStrLast("%1=?");
      QStringList cols = colValMap.keys();
      QString setValsStr;
      for(int i = 0; i < cols.length(); i++ )
      {
         if(i < cols.length() - 1)
            setValsStr += kSetStr.arg(cols[i]);
         else
            setValsStr += kSetStrLast.arg(cols[i]);
      }

      QString command = QString("UPDATE %1 set %2 where id=%3")
                           .arg(tableName)
                           .arg(setValsStr)
                           .arg(key);
      QSqlQuery query( sqlDatabase() );
      query.prepare( command );

      for(int i = 0; i < cols.length(); i++)
      {
         const QString key(cols[i]);
         query.addBindValue(colValMap[key]);
      }


      if ( ! query.exec() )
         throw QString("Could not update %1: %4 %5")
                  .arg( tableName )
                  .arg( query.lastQuery() )
                  .arg( query.lastError().text() );

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      throw;
   }

   /*if ( notify )
      emit object->changed(prop,value);*/
}

// Inventory functions ========================================================

//This links ingredients with the same name.
//The first displayed ingredient in the database is assumed to be the parent.
void Database::populateChildTablesByName(Brewtarget::DBTable table){
   Brewtarget::logW( "Populating Children Ingredient Links" );

   try {
      QString queryString = QString("SELECT DISTINCT name FROM %1").arg(tableNames[table]);

      QSqlQuery nameq( queryString, sqlDatabase() );

      if ( ! nameq.isActive() )
         throw QString("%1 %2").arg(nameq.lastQuery()).arg(nameq.lastError().text());

      while (nameq.next()) {
         QString name = nameq.record().value(0).toString();
         queryString = QString( "SELECT id FROM %1 WHERE ( name=:name AND display=%2 ) ORDER BY id ASC LIMIT 1")
                     .arg(tableNames[table])
                     .arg(Brewtarget::dbTrue());
         QSqlQuery parentq( sqlDatabase() );

         parentq.prepare(queryString);
         parentq.bindValue(":name", name);
         parentq.exec();

         if ( !parentq.isActive() )
            throw QString("%1 %2").arg(parentq.lastQuery()).arg(parentq.lastError().text());

         parentq.first();
         QString parentID = parentq.record().value("id").toString();

         queryString = QString( "SELECT id FROM %1 WHERE ( name=:name AND display=%2 ) ORDER BY id ASC")
                     .arg(tableNames[table])
                     .arg(Brewtarget::dbFalse());
         QSqlQuery childrenq( sqlDatabase() );
         childrenq.prepare(queryString);
         childrenq.bindValue(":name", name);
         childrenq.exec();

         if ( !childrenq.isActive() )
            throw QString("%1 %2").arg(childrenq.lastQuery()).arg(childrenq.lastError().text());
         // Postgres uses a more verbose upsert syntax. I don't like this, but
         // I'm not seeing a better way yet.
         while (childrenq.next()) {
            QString childID = childrenq.record().value("id").toString();
            switch( Brewtarget::dbType() ) {
               case Brewtarget::PGSQL:
                  queryString = QString("INSERT INTO %1 (parent_id, child_id) VALUES (%2, %3) ON CONFLICT(child_id) DO UPDATE set parent_id = EXCLUDED.parent_id")
                        .arg(tableNames[tableToChildTable[table]])
                        .arg(parentID)
                        .arg(childID);
                  break;
               default:
                  queryString = QString("INSERT OR REPLACE INTO %1 (parent_id, child_id) VALUES (%2, %3)")
                              .arg(tableNames[tableToChildTable[table]])
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
void Database::populateChildTablesByName(){

   try {
      // I really dislike this. It counts as spooky action at a distance, but
      // the populateChildTablesByName methods need these hashes populated
      // early and there is no easy way to untangle them. Yes, this results in
      // the work being done twice. Such is life.
      Database::tableNames = tableNamesHash();
      Database::classNameToTable = classNameToTableHash();

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

//Returns the key of the parent ingredient
int Database::getParentID(Brewtarget::DBTable table, int childKey){
   int ret;
   //child_id is expected to be unique in table
   QString queryString = QString(
      "SELECT parent_id FROM %1 WHERE child_id = %2 LIMIT 1"
   ).arg(tableNames[tableToChildTable[table]]).arg(childKey);

   QSqlQuery q( queryString, sqlDatabase() );
   q.first();
   ret = q.record().value("parent_id").toInt();
   if(ret==0){
      return childKey;
   }else{
      return ret;
   }
}
//Returns the key to the inventory table for a given ingredient
int Database::getInventoryID(Brewtarget::DBTable table, int key){
   int ret;
   QString queryString = QString(
      "SELECT id FROM %1 WHERE %2_id = %3 LIMIT 1"
   ).arg(tableNames[tableToInventoryTable[table]]).arg(tableNames[table]).arg(getParentID(table, key));
   QSqlQuery q( queryString, sqlDatabase() );
   q.first();
   ret = q.record().value("id").toInt();
   return ret;
}
//Returns the parent table number from the hash
Brewtarget::DBTable Database::getChildTable(Brewtarget::DBTable table){
   return tableToChildTable[table];
}
//Returns the inventory table number from the hash
Brewtarget::DBTable Database::getInventoryTable(Brewtarget::DBTable table) {
   return tableToInventoryTable[table];
}
//create a new inventory row
void Database::newInventory(Brewtarget::DBTable invForTable, int invForID) {
   QString invTable = tableNames[tableToInventoryTable[invForTable]];

   QString queryString;

   switch(Brewtarget::dbType())
   {
      case Brewtarget::PGSQL:
         queryString = QString("INSERT INTO %1 (%2_id) VALUES(%3) ON CONFLICT(%2_id) DO UPDATE set %2_id = EXCLUDED.%2_id")
                     .arg(invTable)
                     .arg(tableNames[invForTable])
                     .arg(getParentID(invForTable, invForID));
         break;
      default:
         queryString = QString("INSERT OR REPLACE INTO %1 (%2_id) VALUES (%3)")
                     .arg(invTable)
                     .arg(tableNames[invForTable])
                     .arg(getParentID(invForTable, invForID));
   }

   QSqlQuery q( queryString, sqlDatabase() );
}

QMap<int, double> Database::getInventory(const Brewtarget::DBTable table) const
{
   QMap<int, double> result;

   const QString id = tableNames[table] + "_id";
   const QString amount = table == Brewtarget::YEASTTABLE ? "quanta" : "amount";

   QString query = QString("SELECT %1,%2 FROM %3 WHERE %2 > 0")
                         .arg(id)
                         .arg(amount)
                         .arg(tableNames[tableToInventoryTable[table]]);

   QSqlQuery sql(query, sqlDatabase());
   if (!sql.isActive())
   {
      throw QString("Failed to get the inventory.\nQuery:\n%1\nError:\n%2")
            .arg(sql.lastQuery())
            .arg(sql.lastError().text());
   }

   while (sql.next())
   {
      result[sql.value(id).toInt()] = sql.value(amount).toDouble();
   }

   return result;
}

// Add to recipe ==============================================================
void Database::addToRecipe( Recipe* rec, Equipment* e, bool noCopy, bool transact )
{
   Equipment* newEquip = e;

   if( e == 0 )
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
                QString("equipment_id=%1").arg(newEquip->key()),
                QString("id=%1").arg(rec->_key));

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
   connect( newEquip, &BeerXMLElement::changed, rec, &Recipe::acceptEquipChange );
   // NOTE: If we don't reconnect these signals, bad things happen when
   // changing boil times on the mainwindow
   connect( newEquip, &Equipment::changedBoilSize_l, rec, &Recipe::setBoilSize_l);
   connect( newEquip, &Equipment::changedBoilTime_min, rec, &Recipe::setBoilTime_min);

   // Emit a changed signal.
   emit rec->changed( rec->metaProperty("equipment"), BeerXMLElement::qVariantFromPtr(newEquip) );

   // If we are already wrapped in a transaction boundary, do not call
   // recaclAll(). Weirdness ensues. But I want this after all the signals are
   // attached, etc.
   if ( transact )
      rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy, bool transact )
{
   if ( ferm == 0 )
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
   catch ( QString(e) ) {
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

   if ( transact )
      sqlDatabase().transaction();

   try {
      foreach (Hop* hop, hops )
      {
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

   if ( transact )
      sqlDatabase().transaction();
   // Make a copy of mash.
   // Making a copy of the mash isn't enough. We need a copy of the mashsteps
   // too.
   try {
      if ( ! noCopy )
      {
         newMash = copy<Mash>(m, &allMashs, false);
         duplicateMashSteps(m,newMash);
      }

      // Update mash_id
      sqlUpdate(Brewtarget::RECTABLE,
               QString("mash_id=%1").arg(newMash->key()),
               QString("id=%1").arg(rec->_key));
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
   emit rec->changed( rec->metaProperty("mash"), BeerXMLElement::qVariantFromPtr(newMash) );
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
      foreach (Misc* misc, miscs )
      {
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

   if ( transact  && ! noCopy )
      rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Style* s, bool noCopy, bool transact )
{

   Style* newStyle = s;

   if ( s == 0 )
      return;

   if ( transact )
      sqlDatabase().transaction();

   try {
      if ( ! noCopy )
         newStyle = copy<Style>(s, &allStyles, false);

      sqlUpdate(Brewtarget::RECTABLE,
                QString("style_id=%1").arg(newStyle->key()),
                QString("id=%1").arg(rec->_key));
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
   emit rec->changed( rec->metaProperty("style"), BeerXMLElement::qVariantFromPtr(newStyle) );
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
                .arg(tableNames[table])
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
                .arg(tableNames[table])
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

/*
QHash<Brewtarget::DBTable,QSqlQuery> Database::selectAllHash()
{
   QHash<Brewtarget::DBTable,QSqlQuery> ret;

   foreach( Brewtarget::DBTable table, tableNames.keys() )
   {
      QSqlQuery q(sqlDatabase());
      QString query = QString("SELECT * FROM %1 WHERE id=:id").arg(tableNames[table]);
      q.prepare( query );

      ret[table] = q;
   }

   return ret;
}
*/
// Now the payoff for a lot of hard work elsewhere
QHash<Brewtarget::DBTable,QString> Database::tableNamesHash()
{

   QHash<Brewtarget::DBTable,QString> tmp;
   QString query = QString("SELECT name,table_id from bt_alltables");
   QSqlQuery q(query,sqlDatabase());

   while( q.next() ) {
      tmp [ (Brewtarget::DBTable)q.value("table_id").toInt() ] = q.value("name").toString();
   }
   return tmp;

}

QHash<QString,Brewtarget::DBTable> Database::classNameToTableHash()
{
   QHash<QString,Brewtarget::DBTable> tmp;
   QString query = QString("SELECT class_name,table_id from bt_alltables where class_name != ''");
   QSqlQuery q(query,sqlDatabase());

   while( q.next() ) {
      tmp [ q.value("class_name").toString() ] = (Brewtarget::DBTable)q.value("table_id").toInt();
   }

   return tmp;
}

QHash<Brewtarget::DBTable,Brewtarget::DBTable> Database::tableToChildTableHash()
{
   QHash<Brewtarget::DBTable,Brewtarget::DBTable> tmp;

   tmp[Brewtarget::FERMTABLE] = Brewtarget::FERMCHILDTABLE;
   tmp[Brewtarget::HOPTABLE] = Brewtarget::HOPCHILDTABLE;
   tmp[Brewtarget::MISCTABLE] = Brewtarget::MISCCHILDTABLE;
   tmp[Brewtarget::YEASTTABLE] = Brewtarget::YEASTCHILDTABLE;

   return tmp;
}

QHash<Brewtarget::DBTable,Brewtarget::DBTable> Database::tableToInventoryTableHash()
{
   QHash<Brewtarget::DBTable,Brewtarget::DBTable> tmp;

   tmp[Brewtarget::FERMTABLE] = Brewtarget::FERMINVTABLE;
   tmp[Brewtarget::HOPTABLE] = Brewtarget::HOPINVTABLE;
   tmp[Brewtarget::MISCTABLE] = Brewtarget::MISCINVTABLE;
   tmp[Brewtarget::YEASTTABLE] = Brewtarget::YEASTINVTABLE;

   return tmp;
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
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::EQUIPTABLE, allEquipments);
   return tmp;
}

QList<Fermentable*> Database::fermentables()
{
   QList<Fermentable*> tmp;
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::FERMTABLE, allFermentables);
   return tmp;
}

QList<Hop*> Database::hops()
{
   QList<Hop*> tmp;
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::HOPTABLE, allHops);
   return tmp;
}

QList<Mash*> Database::mashs()
{
   QList<Mash*> tmp;
   //! Mashs and mashsteps are the odd balls.
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::MASHTABLE, allMashs);
   return tmp;
}

QList<MashStep*> Database::mashSteps()
{
   QList<MashStep*> tmp;
   getElements( tmp, QString("deleted=%1 order by step_number").arg(Brewtarget::dbFalse()), Brewtarget::MASHSTEPTABLE, allMashSteps);
   return tmp;
}

QList<Misc*> Database::miscs()
{
   QList<Misc*> tmp;
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::MISCTABLE, allMiscs );
   return tmp;
}

QList<Recipe*> Database::recipes()
{
   QList<Recipe*> tmp;
   // This is gonna kill me.
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::RECTABLE, allRecipes );
   return tmp;
}

QList<Style*> Database::styles()
{
   QList<Style*> tmp;
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::STYLETABLE, allStyles );
   return tmp;
}

QList<Water*> Database::waters()
{
   QList<Water*> tmp;
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::WATERTABLE, allWaters );
   return tmp;
}

QList<Yeast*> Database::yeasts()
{
   QList<Yeast*> tmp;
   getElements( tmp, QString("deleted=%1").arg(Brewtarget::dbFalse()), Brewtarget::YEASTTABLE, allYeasts );
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
   unsigned int count;
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
         Recipe* temp = recipeFromXml( list.at(i) );
         if ( ! temp->isValid() )
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
                  Equipment* temp = equipmentFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "FERMENTABLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Fermentable* temp = fermentableFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }

            }
            else if (tag == "HOP")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Hop* temp = hopFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "MISC")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Misc* temp = miscFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "STYLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Style* temp = styleFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "YEAST")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Yeast* temp = yeastFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "WATER" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Water* temp = waterFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "MASHS" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Mash* temp = mashFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
         }
      }
   }
   return ret;
}

void Database::toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent )
{
   // TODO: implement
   QDomElement bNode;
   QDomElement tmpElement;
   QDomText tmpText;

   bNode = doc.createElement("BREWNOTE");

   tmpElement = doc.createElement("BREWDATE");
   tmpText = doc.createTextNode(a->brewDate_str());
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("DATE_FERMENTED_OUT");
   tmpText = doc.createTextNode(a->fermentDate_str());
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("SG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->sg()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("VOLUME_INTO_BK");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->volumeIntoBK_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("STRIKE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->strikeTemp_c()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MASH_FINAL_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->mashFinTemp_c()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("OG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->og()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("POST_BOIL_VOLUME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->postBoilVolume_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("VOLUME_INTO_FERMENTER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->volumeIntoFerm_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PITCH_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->pitchTemp_c()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fg()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("EFF_INTO_BK");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->effIntoBK_pct()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PREDICTED_OG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->calculateOg()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("BREWHOUSE_EFF");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->brewhouseEff_pct()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PREDICTED_ABV");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->calculateABV_pct()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ACTUAL_ABV");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->abv()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_BOIL_GRAV");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projBoilGrav()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_STRIKE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projStrikeTemp_c()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_MASH_FIN_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projMashFinTemp_c()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_VOL_INTO_BK");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projVolIntoBK_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_OG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projOg()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_VOL_INTO_FERM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projVolIntoFerm_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_FG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projFg()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_EFF");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projEff_pct()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_ABV");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projABV_pct()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_POINTS");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projPoints()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PROJECTED_ATTEN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->projAtten()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("BOIL_OFF");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilOff_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FINAL_VOLUME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->finalVolume_l()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   tmpElement = doc.createElement("NOTES");
   tmpText = doc.createTextNode(QString("%1").arg(a->notes()));
   tmpElement.appendChild(tmpText);
   bNode.appendChild(tmpElement);

   parent.appendChild(bNode);
}

void Database::toXml( Equipment* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement equipNode;
   QDomElement tmpNode;
   QDomText tmpText;

   equipNode = doc.createElement("EQUIPMENT");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOIL_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilSize_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BATCH_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->batchSize_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TUN_VOLUME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunVolume_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TUN_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunWeight_kg()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TUN_SPECIFIC_HEAT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunSpecificHeat_calGC()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TOP_UP_WATER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->topUpWater_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TRUB_CHILLER_LOSS");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->trubChillerLoss_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("EVAP_RATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->evapRate_pctHr()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("REAL_EVAP_RATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->evapRate_lHr()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOIL_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilTime_min()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CALC_BOIL_VOLUME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->calcBoilVolume()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("LAUTER_DEADSPACE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->lauterDeadspace_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TOP_UP_KETTLE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->topUpKettle_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("HOP_UTILIZATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->hopUtilization_pct()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   // My extensions below
   tmpNode = doc.createElement("ABSORPTION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->grainAbsorption_LKg()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOILING_POINT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilingPoint_c()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   parent.appendChild(equipNode);
}

void Database::toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement fermNode;
   QDomElement tmpNode;
   QDomText tmpText;

   fermNode = doc.createElement("FERMENTABLE");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(Fermentable::types.at(a->type()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount_kg()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("YIELD");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->yield_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->color_srm()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ADD_AFTER_BOIL");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->addAfterBoil()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ORIGIN");
   tmpText = doc.createTextNode(a->origin());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SUPPLIER");
   tmpText = doc.createTextNode(a->supplier());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COARSE_FINE_DIFF");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->coarseFineDiff_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MOISTURE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->moisture_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("DIASTATIC_POWER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->diastaticPower_lintner()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PROTEIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->protein_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MAX_IN_BATCH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->maxInBatch_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("RECOMMEND_MASH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->recommendMash()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IS_MASHED");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->isMashed()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_GAL_PER_LB");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ibuGalPerLb()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);

   parent.appendChild(fermNode);
}

void Database::toXml( Hop* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement hopNode;
   QDomElement tmpNode;
   QDomText tmpText;

   hopNode = doc.createElement("HOP");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ALPHA");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->alpha_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount_kg()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("USE");
   tmpText = doc.createTextNode(a->useString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->time_min()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FORM");
   tmpText = doc.createTextNode(a->formString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BETA");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->beta_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("HSI");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->hsi_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ORIGIN");
   tmpText = doc.createTextNode(a->origin());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SUBSTITUTES");
   tmpText = doc.createTextNode(a->substitutes());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("HUMULENE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->humulene_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARYOPHYLLENE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->caryophyllene_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COHUMULONE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->cohumulone_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MYRCENE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->myrcene_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);

   parent.appendChild(hopNode);
}

void Database::toXml( Instruction* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement insNode;
   QDomElement tmpNode;
   QDomText tmpText;

   insNode = doc.createElement("INSTRUCTION");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("DIRECTIONS");
   tmpText = doc.createTextNode(a->directions());
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("HAS_TIMER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->hasTimer()));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TIMER_VALUE");
   tmpText = doc.createTextNode(a->timerValue());
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COMPLETED");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->completed()));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INTERVAL");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->interval()));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   parent.appendChild(insNode);
}

void Database::toXml( Mash* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement mashNode;
   QDomElement tmpNode;
   QDomText tmpText;

   int i, size;

   mashNode = doc.createElement("MASH");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("GRAIN_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->grainTemp_c()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MASH_STEPS");
   QList<MashStep*> mashSteps = a->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
      toXml( mashSteps[i], doc, tmpNode);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TUN_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunTemp_c()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SPARGE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->spargeTemp_c()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ph()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TUN_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunWeight_kg()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TUN_SPECIFIC_HEAT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunSpecificHeat_calGC()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   tmpNode = doc.createElement("EQUIP_ADJUST");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->equipAdjust()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);

   parent.appendChild(mashNode);
}

void Database::toXml( MashStep* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement mashStepNode;
   QDomElement tmpNode;
   QDomText tmpText;

   mashStepNode = doc.createElement("MASH_STEP");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   if ( (a->type() == MashStep::flySparge) || (a->type() == MashStep::batchSparge ) )
      tmpText = doc.createTextNode(  MashStep::types[0] );
   else
      tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INFUSE_AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->infuseAmount_l()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STEP_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->stepTemp_c()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STEP_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->stepTime_min()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("RAMP_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->rampTime_min()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("END_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->endTemp_c()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INFUSE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->infuseTemp_c()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   tmpNode = doc.createElement("DECOCTION_AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->decoctionAmount_l()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);

   parent.appendChild(mashStepNode);
}

void Database::toXml( Misc* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement miscNode;
   QDomElement tmpNode;
   QDomText tmpText;

   miscNode = doc.createElement("MISC");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("USE");
   tmpText = doc.createTextNode(a->useString());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->time()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT_IS_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amountIsWeight()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("USE_FOR");
   tmpText = doc.createTextNode(a->useFor());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);

   parent.appendChild(miscNode);
}

void Database::toXml( Recipe* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement recipeNode;
   QDomElement tmpNode;
   QDomText tmpText;

   int i;

   recipeNode = doc.createElement("RECIPE");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->type());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   Style* style = a->style();
   if( style != 0 )
      toXml( style, doc, recipeNode);

   tmpNode = doc.createElement("BREWER");
   tmpText = doc.createTextNode(a->brewer());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BATCH_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->batchSize_l()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOIL_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilSize_l()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOIL_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilTime_min()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("EFFICIENCY");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->efficiency_pct()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("HOPS");
   QList<Hop*> hops = a->hops();
   for( i = 0; i < hops.size(); ++i )
      toXml( hops[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FERMENTABLES");
   QList<Fermentable*> ferms = a->fermentables();
   for( i = 0; i < ferms.size(); ++i )
      toXml( ferms[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MISCS");
   QList<Misc*> miscs = a->miscs();
   for( i = 0; i < miscs.size(); ++i )
      toXml( miscs[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("YEASTS");
   QList<Yeast*> yeasts = a->yeasts();
   for( i = 0; i < yeasts.size(); ++i )
      toXml( yeasts[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("WATERS");
   QList<Water*> waters = a->waters();
   for( i = 0; i < waters.size(); ++i )
      toXml( waters[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   Mash* mash = a->mash();
   if( mash != 0 )
      toXml( mash, doc, recipeNode);

   tmpNode = doc.createElement("INSTRUCTIONS");
   QList<Instruction*> instructions = a->instructions();
   for( i = 0; i < instructions.size(); ++i )
      toXml( instructions[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BREWNOTES");
   QList<BrewNote*> brewNotes = a->brewNotes();
   for(i=0; i < brewNotes.size(); ++i)
      toXml(brewNotes[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ASST_BREWER");
   tmpText = doc.createTextNode(a->asstBrewer());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   Equipment* equip = a->equipment();
   if( equip )
      toXml( equip, doc, recipeNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TASTE_NOTES");
   tmpText = doc.createTextNode(a->tasteNotes());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TASTE_RATING");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tasteRating()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->og()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fg()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FERMENTATION_STAGES");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fermentationStages()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PRIMARY_AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->primaryAge_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PRIMARY_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->primaryTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SECONDARY_AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->secondaryAge_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SECONDARY_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->secondaryTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TERTIARY_AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tertiaryAge_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TERTIARY_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tertiaryTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->age_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AGE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ageTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("DATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->date()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARBONATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbonation_vols()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FORCED_CARBONATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->forcedCarbonation()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PRIMING_SUGAR_NAME");
   tmpText = doc.createTextNode(a->primingSugarName());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARBONATION_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbonationTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PRIMING_SUGAR_EQUIV");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->primingSugarEquiv()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("KEG_PRIMING_FACTOR");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->kegPrimingFactor()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);

   parent.appendChild(recipeNode);
}

void Database::toXml( Style* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement styleNode;
   QDomElement tmpNode;
   QDomText tmpText;

   styleNode = doc.createElement("STYLE");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CATEGORY");
   tmpText = doc.createTextNode(a->category());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CATEGORY_NUMBER");
   tmpText = doc.createTextNode(a->categoryNumber());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STYLE_LETTER");
   tmpText = doc.createTextNode(a->styleLetter());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STYLE_GUIDE");
   tmpText = doc.createTextNode(a->styleGuide());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ogMin()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ogMax()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fgMin()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fgMax()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ibuMin()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ibuMax()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->colorMin_srm()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->colorMax_srm()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ABV_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->abvMin_pct()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ABV_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->abvMax_pct()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARB_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbMin_vol()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARB_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbMax_vol()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PROFILE");
   tmpText = doc.createTextNode(a->profile());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INGREDIENTS");
   tmpText = doc.createTextNode(a->ingredients());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("EXAMPLES");
   tmpText = doc.createTextNode(a->examples());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   parent.appendChild(styleNode);
}

void Database::toXml( Water* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement waterNode;
   QDomElement tmpNode;
   QDomText tmpText;

   waterNode = doc.createElement("WATER");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount_l()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CALCIUM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->calcium_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BICARBONATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->bicarbonate_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SULFATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->sulfate_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CHLORIDE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->chloride_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SODIUM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->sodium_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MAGNESIUM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->magnesium_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ph()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   parent.appendChild(waterNode);
}

void Database::toXml( Yeast* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement yeastNode;
   QDomElement tmpElement;
   QDomText tmpText;

   yeastNode = doc.createElement("YEAST");

   tmpElement = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("TYPE");
   tmpText = doc.createTextNode(Yeast::types.at(a->type()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FORM");
   tmpText = doc.createTextNode(Yeast::forms.at(a->form()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("AMOUNT_IS_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amountIsWeight()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("LABORATORY");
   tmpText = doc.createTextNode(a->laboratory());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PRODUCT_ID");
   tmpText = doc.createTextNode(a->productID());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MIN_TEMPERATURE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->minTemperature_c()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MAX_TEMPERATURE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->maxTemperature_c()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FLOCCULATION");
   tmpText = doc.createTextNode(Yeast::flocculations.at(a->flocculation()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ATTENUATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->attenuation_pct()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("BEST_FOR");
   tmpText = doc.createTextNode(a->bestFor());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("TIMES_CULTURED");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->timesCultured()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MAX_REUSE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->maxReuse()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ADD_TO_SECONDARY");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->addToSecondary()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   parent.appendChild(yeastNode);
}

// fromXml ====================================================================
void Database::fromXml(BeerXMLElement* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::logW( QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();

      if( xmlTagsToProperties.contains(xmlTag) )
      {
         switch( element->metaProperty(xmlTagsToProperties[xmlTag]).type() )
         {
            case QVariant::Bool:
               boolVal = BeerXMLElement::getBool(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = BeerXMLElement::getDouble(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = BeerXMLElement::getInt(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = BeerXMLElement::getDateTime(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = BeerXMLElement::getDate(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateVal);
               break;
            // NOTE: I believe that enum types like Fermentable::Type will go
            // here since Q_ENUMS() converts enums to strings. So, need to make
            // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = BeerXMLElement::getString(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), stringVal);
               break;
            default:
               Brewtarget::logW("Database::fromXML: don't understand property type.");
               break;
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            Brewtarget::logE( QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag));
            return;
         }
      }
      else
      {
         //if( showWarnings )
         //  Brewtarget::logW(QString("Database::fromXML: Unsupported property: %1. Line %2").arg(xmlTag).arg(node.lineNumber()) );
      }
   }

}

// Brewnotes can never be created w/ a recipe, so we will always assume the
// calling method has the transactions
BrewNote* Database::brewNoteFromXml( QDomNode const& node, Recipe* parent )
{
   BrewNote* ret = newBrewNote(parent);

   try {
      if ( ! ret )
         throw QString("Could not create new brewnote");

      // Need to tell the brewnote not to perform the calculations
      ret->setLoading(true);
      fromXml( ret, BrewNote::tagToProp, node);
      if ( ! ret->isValid() )
         throw QString("Error loading brewnote from XML");

      ret->setLoading(false);
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }

   return ret;
}

Equipment* Database::equipmentFromXml( QDomNode const& node, Recipe* parent )
{
   // When loading from XML, we need to delay the signals until after
   // everything is done. This should significantly speed up the load times
   blockSignals(true);

   Equipment* ret;
   QList<Equipment*> matchingEquips;
   QDomNode n;
   bool createdNew = true;

   try {
      // If we are just importing an equip by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // No parent means we handle the transaction
         sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         n = node.firstChildElement("NAME");
         QString name = n.firstChild().toText().nodeValue();

         getElements<Equipment>( matchingEquips, QString("name='%1'").arg(name), Brewtarget::EQUIPTABLE, allEquipments );

         if( matchingEquips.length() > 0 )
         {
            createdNew = false;
            ret = matchingEquips.first();
         }
         else
            ret = newEquipment();
      }
      else
         ret = newEquipment();

      if ( ! ret )
         throw QString("Could not create new equipment profile");

      fromXml( ret, Equipment::tagToProp, node );
      if ( ! ret->isValid() )
         throw QString("There was an error loading equipment profile from XML");

      // If we are importing one of our beerXML files, the utilization is always
      // 0%. We need to fix that.
      if ( ret->hopUtilization_pct() == 0.0 )
         ret->setHopUtilization_pct(100.0);

      if( parent )
      {
         ret->setDisplay(false);
         addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      if ( ! parent )
         sqlDatabase().rollback();
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   if ( ! parent )
      sqlDatabase().commit();

   if( createdNew )
   {
      emit changed( metaProperty("equipments"), QVariant() );
      emit newEquipmentSignal(ret);
   }

   return ret;
}

Fermentable* Database::fermentableFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Fermentable* ret;
   bool createdNew = true;
   blockSignals(true);

   try {
      // If we are just importing a ferm by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // Check to see if there is a ferm already in the DB with the same name.
         sqlDatabase().transaction();
         n = node.firstChildElement("NAME");
         QString name = n.firstChild().toText().nodeValue();
         QList<Fermentable*> matchingFerms;
         getElements<Fermentable>( matchingFerms, QString("name='%1'").arg(name), Brewtarget::FERMTABLE, allFermentables );

         if( matchingFerms.length() > 0 )
         {
            createdNew = false;
            ret = matchingFerms.first();
         }
         else
            ret = newFermentable();
      }
      else
         ret = newFermentable();

      if ( ! ret )
         throw QString("Could not create new fermentable");

      fromXml( ret, Fermentable::tagToProp, node );
      if ( ! ret->isValid() )
         throw QString("Error reading fermentable from XML");


      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = Fermentable::types.indexOf( n.firstChild().toText().nodeValue());
         if ( ndx != -1 )
            ret->setType( static_cast<Fermentable::Type>(ndx));
         else
            ret->invalidate();
      }

      if ( ! ret->isValid() )
         throw QString("Could not change the type of the fermentable");

      if( parent )
         addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      if ( ! parent )
         sqlDatabase().rollback();
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }

   if ( ! parent )
      sqlDatabase().commit();

   blockSignals(false);
   if( createdNew )
   {
      emit changed( metaProperty("fermentables"), QVariant() );
      emit newFermentableSignal(ret);
   }

   return ret;
}

int Database::getQualifiedHopTypeIndex(QString type, Hop* hop)
{
  if ( Hop::types.indexOf(type) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT htype FROM hop WHERE name='%1' AND htype != ''").arg(hop->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString htype = q.record().value(0).toString();
      q.finish();
      if ( htype != "" )
      {
         if ( Hop::types.indexOf(htype) >= 0 )
         {
            return Hop::types.indexOf(htype);
         }
      }
    }
    // out of ideas at this point so default to Both
    return Hop::types.indexOf(QString("Both"));
  }
  else
  {
     return Hop::types.indexOf(type);
  }
}

int Database::getQualifiedHopUseIndex(QString use, Hop* hop)
{
  if ( Hop::uses.indexOf(use) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT use FROM hop WHERE name='%1' AND use != ''").arg(hop->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString hUse = q.record().value(0).toString();
      q.finish();
      if ( hUse != "" )
         if ( Hop::uses.indexOf(hUse) >= 0 )
            return Hop::uses.indexOf(hUse);
    }
    // out of ideas at this point so default to Flavor
    return Hop::uses.indexOf(QString("Flavor"));
  }
  else
  {
     return Hop::uses.indexOf(use);
  }
}

Hop* Database::hopFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Hop* ret;
   bool createdNew = true;
   blockSignals(true);

   try {
      // If we are just importing a hop by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // as always, start the transaction if no parent
         sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         n = node.firstChildElement("NAME");
         QString name = n.firstChild().toText().nodeValue();
         QList<Hop*> matchingHops;
         getElements<Hop>( matchingHops, QString("name='%1'").arg(name), Brewtarget::HOPTABLE, allHops );

         if( matchingHops.length() > 0 )
         {
            createdNew = false;
            ret = matchingHops.first();
         }
         else
            ret = newHop();
      }
      else
         ret = newHop();

      fromXml( ret, Hop::tagToProp, node );

      // Handle enums separately.
      n = node.firstChildElement("USE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = getQualifiedHopUseIndex(n.firstChild().toText().nodeValue(), ret);
         if ( ndx != -1 )
            ret->setUse( static_cast<Hop::Use>(ndx));
         else
            ret->invalidate();
      }

      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = getQualifiedHopTypeIndex(n.firstChild().toText().nodeValue(), ret);
         if ( ndx != -1 )
            ret->setType( static_cast<Hop::Type>(ndx) );
         else
            ret->invalidate();
      }

      n = node.firstChildElement("FORM");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = Hop::forms.indexOf(n.firstChild().toText().nodeValue());
         if ( ndx != -1 )
            ret->setForm( static_cast<Hop::Form>(ndx));
         else
            ret->invalidate();
      }

      if ( ! ret->isValid() )
      {
         QString name = ret->name();
         QList<Hop*> matching;
         getElements<Hop>( matching, QString("name like '%1'").arg(name), Brewtarget::HOPTABLE, allHops );

         if( matching.length() > 0 )
         {
            createdNew = false;
            Hop* temp = ret;
            ret = matching.first();
            ret->setAmount_kg(temp->amount_kg());
         }
      }

      if( parent )
         addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      blockSignals(false);
      if ( ! parent )
         sqlDatabase().rollback();

      throw;
   }

   if ( ! parent )
      sqlDatabase().commit();

   blockSignals(false);
   if( createdNew )
   {
      emit changed( metaProperty("hops"), QVariant() );
      emit newHopSignal(ret);
   }
   return ret;
}

// like brewnotes, we will assume here that the caller has the transaction
// block
Instruction* Database::instructionFromXml( QDomNode const& node, Recipe* parent )
{

   blockSignals(true);
   try {
      Instruction* ret = newInstruction(parent);

      fromXml( ret, Instruction::tagToProp, node );

      blockSignals(false);
      emit changed( metaProperty("instructions"), QVariant() );
      return ret;
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      blockSignals(false);
      throw;
   }
}

Mash* Database::mashFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;

   blockSignals(true);
   Mash* ret;

   // Mashes are weird. We need to know if this is a duplicate, but we need to
   // make a copy of it anyway.
   n = node.firstChildElement("NAME");
   QString name = n.firstChild().toText().nodeValue();

   try {
      if( parent )
         ret = newMash(parent);
      else {
         sqlDatabase().transaction();
         ret = newMash();
      }

      // If the mash has a name
      if ( ! name.isEmpty() )
      {
         QList<Mash*> matchingMash;
         getElements<Mash>( matchingMash, QString("name='%1'").arg(name), Brewtarget::MASHTABLE, allMashs );

         // If there are no other matches in the database
         if( matchingMash.isEmpty() )
            ret->setDisplay(true);
      }
      // First, get all the standard properties.
      fromXml( ret, Mash::tagToProp, node );

      // Now, get the individual mash steps.
      n = node.firstChildElement("MASH_STEPS");
      if( n.isNull() )
         return ret;

      // Iterate through all the mash steps.
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         MashStep* temp = mashStepFromXml( n, ret );
         if ( ! temp->isValid() )
            throw;
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      blockSignals(false);
      if ( ! parent )
         sqlDatabase().rollback();
      throw;
   }

   if ( ! parent )
      sqlDatabase().commit();

   blockSignals(false);

   emit changed( metaProperty("mashs"), QVariant() );
   emit newMashSignal(ret);
   emit ret->mashStepsChanged();

   return ret;
}

// mashsteps don't exist without a mash. It is up to mashFromXml or
// recipeFromXml to deal with the transaction
MashStep* Database::mashStepFromXml( QDomNode const& node, Mash* parent )
{
   QDomNode n;
   QString str;
   bool blocked = signalsBlocked();

   if (! blocked )
      blockSignals(true);

   try {
      MashStep* ret = newMashStep(parent);
      // I am only doing this on mashsteps because they cause all sorts of
      // expensive recalculations to happen
      ret->blockSignals(true);

      fromXml(ret,MashStep::tagToProp,node);

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         //Try to make sure incoming format matches
         //e.g. convert INFUSION to Infusion
         str = n.firstChild().toText().nodeValue();
         str = str.toLower();
         str[0] = str.at(0).toTitleCase();
         int ndx =  MashStep::types.indexOf(str);

         if ( ndx != -1 )
            ret->setType( static_cast<MashStep::Type>(ndx) );
         else
            ret->invalidate();
      }

      ret->blockSignals(false);
      if (! blocked )
      {
         blockSignals(false);
         emit changed( metaProperty("mashs"), QVariant() );
         emit parent->mashStepsChanged();
      }
      return ret;
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if (! blocked )
         blockSignals(false);
      throw;
   }

}

int Database::getQualifiedMiscTypeIndex(QString type, Misc* misc)
{
  if ( Misc::types.indexOf(type) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT mtype FROM misc WHERE name='%1' AND mtype != ''").arg(misc->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString mtype = q.record().value(0).toString();
      q.finish();
      if ( mtype != "" )
      {
         if ( Misc::types.indexOf(mtype) >= 0 )
         {
            return Misc::types.indexOf(mtype);
         }
      }
    }
    // out of ideas at this point so default to Flavor
    return Misc::types.indexOf(QString("Flavor"));
  }
  else
  {
     return Misc::types.indexOf(type);
  }
}

int Database::getQualifiedMiscUseIndex(QString use, Misc* misc)
{
  if ( Misc::uses.indexOf(use) < 0 )
  {
    // look for a valid misc type from our database to use
    QSqlQuery q(QString("SELECT use FROM misc WHERE name='%1' AND use != ''").arg(misc->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString mUse = q.record().value(0).toString();
      q.finish();
      if ( mUse != "" )
         if ( Misc::uses.indexOf(mUse) >= 0 )
            return Misc::uses.indexOf(mUse);
    }
    // out of ideas at this point so default to Flavor
    return Misc::uses.indexOf(QString("Flavor"));
  }
  else
  {
     return Misc::uses.indexOf(use);
  }
}

Misc* Database::miscFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Misc* ret;

   try {
      // If we are just importing a misc by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // Check to see if there is a hop already in the DB with the same name.
         sqlDatabase().transaction();

         n = node.firstChildElement("NAME");
         QString name = n.firstChild().toText().nodeValue();
         QList<Misc*> matchingMiscs;
         getElements<Misc>( matchingMiscs, QString("name='%1'").arg(name), Brewtarget::MISCTABLE, allMiscs );

         if( matchingMiscs.length() > 0 )
         {
            createdNew = false;
            ret = matchingMiscs.first();
         }
         else
            ret = newMisc();
      }
      else
         ret = newMisc();

      fromXml( ret, Misc::tagToProp, node );

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      // Assuming these return anything is a bad idea. So far, several other brewing programs are not generating
      // valid XML.
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else
         ret->setType( static_cast<Misc::Type>(getQualifiedMiscTypeIndex(n.firstChild().toText().nodeValue(), ret)));

      n = node.firstChildElement("USE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else
         ret->setUse(static_cast<Misc::Use>(getQualifiedMiscUseIndex(n.firstChild().toText().nodeValue(), ret)));

      if ( ! ret->isValid() )
      {
         QString name = ret->name();
         QList<Misc*> matching;
         getElements<Misc>( matching, QString("name like '%1'").arg(name), Brewtarget::MISCTABLE, allMiscs );

         if( matching.length() > 0 )
         {
            createdNew = false;
            Misc* temp = ret;
            ret = matching.first();
            ret->setAmount( temp->amount() );
         }
      }

      if( parent )
         addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   if( createdNew )
   {
      emit changed( metaProperty("miscs"), QVariant() );
      emit newMiscSignal(ret);
   }
   return ret;
}

Recipe* Database::recipeFromXml( QDomNode const& node )
{
   QDomNode n;

   try {
      // I was wrong. We need to block signals here. Weird things happen if you don't.
      blockSignals(true);

      // This is all one long, gnarly transaction.
      sqlDatabase().transaction();

      // This works, strangely enough.
      Recipe* ret = newIngredient(&allRecipes);

      // Get standard properties.
      fromXml( ret, Recipe::tagToProp, node);

      // Get style. Note: styleFromXml requires the entire node, not just the
      // firstchild of the node.
      n = node.firstChildElement("STYLE");
      styleFromXml(n, ret);
      if ( ! ret->style()->isValid())
         ret->invalidate();

      // Get equipment. equipmentFromXml requires the entire node, not just the
      // first child
      n = node.firstChildElement("EQUIPMENT");
      equipmentFromXml(n, ret);
      if ( !ret->equipment()->isValid() )
         ret->invalidate();

      // Get hops.
      n = node.firstChildElement("HOPS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Hop* temp = hopFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // Get ferms.
      n = node.firstChildElement("FERMENTABLES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Fermentable* temp = fermentableFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // get mashes. There is only one mash per recipe, so this needs the entire
      // node.
      n = node.firstChildElement("MASH");
      mashFromXml(n, ret);
      if ( ! ret->mash()->isValid() )
         ret->invalidate();

      // Get miscs.
      n = node.firstChildElement("MISCS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Misc* temp = miscFromXml(n, ret);
         if (! temp->isValid())
            ret->invalidate();
      }

      // Get yeasts.
      n = node.firstChildElement("YEASTS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Yeast* temp = yeastFromXml(n, ret);
         if ( !temp->isValid() )
            ret->invalidate();
      }

      // Get waters. Odd. Waters don't invalidate.
      n = node.firstChildElement("WATERS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         waterFromXml(n, ret);

      /* That ends the beerXML defined objects. I'm not going to do the
      * validation for these last two. We write em, and we had better be
      * writing them properly
      */
      // Get instructions.
      n = node.firstChildElement("INSTRUCTIONS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         instructionFromXml(n, ret);

      // Get brew notes
      n = node.firstChildElement("BREWNOTES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         brewNoteFromXml(n, ret);

      // If we get here, commit
      sqlDatabase().commit();

      // Recalc everything, just for grins and giggles.
      ret->recalcAll();
      blockSignals(false);

      emit newRecipeSignal(ret);

      return ret;
   }

   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }
}

Style* Database::styleFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Style* ret;
   QString name;
   QList<Style*> matching;

   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // Check to see if there is a hop already in the DB with the same name.
         sqlDatabase().transaction();
         n = node.firstChildElement("NAME");
         name = n.firstChild().toText().nodeValue();
         getElements<Style>( matching, QString("name='%1'").arg(name), Brewtarget::STYLETABLE, allStyles );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
         else
            ret = newStyle();
      }
      else
         ret = newStyle();

      fromXml( ret, Style::tagToProp, node );

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = Style::types.indexOf( n.firstChild().toText().nodeValue());
         if ( ndx != -1 )
            ret->setType(static_cast<Style::Type>(ndx));
         else
            ret->invalidate();
      }

      // let's see if I can be clever and find this style in our db
      if (! ret->isValid() )
      {
         name = ret->name();
         getElements<Style>( matching, QString("name='%1'").arg(name), Brewtarget::STYLETABLE ,allStyles);
         // If we find a match, discard what we just built and use what's in teh DB instead
         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }

      }
      if( parent )
         addToRecipe( parent, ret );
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   if( createdNew )
   {
      emit changed( metaProperty("styles"), QVariant() );
      emit newStyleSignal(ret);
   }

   return ret;
}

Water* Database::waterFromXml( QDomNode const& node, Recipe* parent )
{
   blockSignals(true);
   bool createdNew = true;
   Water* ret;
   QDomNode n;

   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // Check to see if there is a hop already in the DB with the same name.
         n = node.firstChildElement("NAME");
         QString name = n.firstChild().toText().nodeValue();
         QList<Water*> matching;
         getElements<Water>( matching, QString("name='%1'").arg(name), Brewtarget::WATERTABLE, allWaters );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
         else
            ret = newWater();
      }
      else
         ret = newWater();

      fromXml( ret, Water::tagToProp, node );
      if( parent )
         addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   if( createdNew )
   {
      emit changed( metaProperty("waters"), QVariant() );
      emit newWaterSignal(ret);
   }

   return ret;
}

Yeast* Database::yeastFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Yeast* ret;
   QString name;
   QList<Yeast*> matching;

   try {
      // If we are just importing a yeast by itself, need to do some dupe-checking.
      if( parent == 0 )
      {
         // Check to see if there is a hop already in the DB with the same name.
         sqlDatabase().transaction();
         n = node.firstChildElement("NAME");
         name = n.firstChild().toText().nodeValue();
         getElements<Yeast>( matching, QString("name='%1'").arg(name), Brewtarget::YEASTTABLE, allYeasts );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
         else
            ret = newYeast();
      }
      else
         ret = newYeast();

      fromXml( ret, Yeast::tagToProp, node );

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = Yeast::types.indexOf( n.firstChild().toText().nodeValue());
         if ( ndx != -1)
            ret->setType( static_cast<Yeast::Type>(ndx) );
         else
            ret->invalidate();
      }
      // Handle enums separately.
      n = node.firstChildElement("FORM");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = Yeast::forms.indexOf( n.firstChild().toText().nodeValue());
         if ( ndx != -1 )
            ret->setForm( static_cast<Yeast::Form>(ndx) );
         else
            ret->invalidate();
      }
      // Handle enums separately.
      n = node.firstChildElement("FLOCCULATION");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         int ndx = Yeast::flocculations.indexOf( n.firstChild().toText().nodeValue());
         if (ndx != -1)
            ret->setFlocculation( static_cast<Yeast::Flocculation>(ndx) );
         else
            ret->invalidate();
      }

      // If we cannot import the yeast, we find the nearest possible match in
      // the database and just use that? With absolutely no feed back to the
      // user saying "Hey! That didn't work!"? No sir, I don't like it.
      if ( ! ret->isValid() )
      {
         name = ret->name();
         getElements<Yeast>( matching, QString("name like '%1'").arg(name), Brewtarget::YEASTTABLE, allYeasts );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
      }

      if( parent )
         addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   if( createdNew )
   {
      emit changed( metaProperty("yeasts"), QVariant() );
      emit newYeastSignal(ret);
   }

   return ret;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<TableParams> Database::makeTableParams()
{
   QList<TableParams> ret;
   TableParams tmp;

   //=============================Equipment====================================

   tmp.tableName = "equipment";
   tmp.propName = QStringList() <<
      "name" << "boil_size" << "batch_size" << "tun_volume" << "tun_weight" <<
      "tun_specific_heat" << "top_up_water" << "trub_chiller_loss" <<
      "evap_rate" << "real_evap_rate" << "boil_time" << "calc_boil_volume" <<
      "lauter_deadspace" << "top_up_kettle" << "hop_utilization" <<
      "notes";
   tmp.newElement = [&]() { return this->newEquipment(); };

   ret.append(tmp);
   //==============================Fermentables================================

   tmp.tableName = "fermentable";
   tmp.propName = QStringList() <<
      "name" << "ftype" << "amount" << "yield" << "color" <<
      "add_after_boil" << "origin" << "supplier" << "notes" <<
      "coarse_fine_diff" << "moisture" << "diastatic_power" << "protein" <<
      "max_in_batch" << "recommend_mash" << "ibu_gal_per_lb";
   tmp.newElement = [&]() { return this->newFermentable(); };

   //==============================Hops=============================
   tmp.tableName = "hop";
   tmp.propName = QStringList() <<
      "name" << "alpha" << "amount" << "use" << "time" << "notes" << "htype" <<
      "form" << "beta" << "hsi" << "origin" << "substitutes" << "humulene" <<
      "caryophyllene" << "cohumulone" << "myrcene",
   // First cast specifies which newHop() I want, since it is overloaded.
   // Second cast is to force the conversion of the function pointer.
   tmp.newElement = [&]() { return this->newHop(); };

   ret.append(tmp);

   //==================================Miscs===================================

   tmp.tableName = "misc";
   tmp.propName = QStringList() <<
      "name" << "mtype" << "use" << "time" << "amount" << "amount_is_weight" <<
      "use_for" << "notes";
   tmp.newElement = [&]() { return this->newMisc(); };

   ret.append(tmp);
   //==================================Styles==================================

   tmp.tableName = "style";
   tmp.propName = QStringList() <<
      "name" << "s_type" << "category" << "category_number" <<
      "style_letter" << "style_guide" << "og_min" << "og_max" << "fg_min" <<
      "fg_max" << "ibu_min" << "ibu_max" << "color_min" << "color_max" <<
      "abv_min" << "abv_max" << "carb_min" << "carb_max" << "notes" <<
      "profile" << "ingredients" << "examples";
   tmp.newElement = [&]() { return this->newStyle(); };

   ret.append(tmp);

   //==================================Yeasts==================================

   tmp.tableName = "yeast";
   tmp.propName = QStringList() <<
      "name" << "ytype" << "form" << "amount" << "amount_is_weight" <<
      "laboratory" << "product_id" << "min_temperature" << "max_temperature" <<
      "flocculation" << "attenuation" << "notes" << "best_for" <<
      "times_cultured" << "max_reuse" << "add_to_secondary";
   tmp.newElement = [&]() { return this->newYeast(); };

   ret.append(tmp);

   //===================================Waters=================================

   tmp.tableName = "water";
   tmp.propName = QStringList() <<
      "name" << "amount" << "calcium" << "bicarbonate" << "sulfate" <<
      "chloride" << "sodium" << "magnesium" << "ph" << "notes";
   tmp.newElement = [&]() { return this->newWater(); };

   ret.append(tmp);

   return ret;
}

void Database::updateDatabase(QString const& filename)
{
   // In the naming here "old" means our local database, and
   // "new" means the database coming from 'filename'.

   QVariant btid, newid, oldid;
   QVariant zero(0);
   QList<TableParams> tableParams = makeTableParams();

   QList<QVariant> propVal;
   QStringList varAndHolder;

   try {
      QString newCon("newSqldbCon");
      QSqlDatabase newSqldb = QSqlDatabase::addDatabase("QSQLITE", newCon);
      newSqldb.setDatabaseName(filename);
      if( ! newSqldb.open() )
      {
         QMessageBox::critical(0,
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

      foreach( TableParams tp, tableParams)
      {
         QSqlQuery qNewBtIng(
            QString("SELECT * FROM bt_%1").arg(tp.tableName),
            newSqldb );

         QSqlQuery qNewIng( newSqldb );
         qNewIng.prepare(QString("SELECT * FROM %1 WHERE id=:id").arg(tp.tableName));

         // Construct the big update query.
         QSqlQuery qUpdateOldIng( sqlDatabase() );
         QString updateString = QString("UPDATE %1 SET ").arg(tp.tableName);
         varAndHolder.clear();
         foreach( QString pn, tp.propName)
            varAndHolder.append(QString("%1=:%2").arg(pn).arg(pn));
         updateString.append(varAndHolder.join(", "));

         // Un-delete it if it is somehow deleted.
         updateString.append(", deleted=:zero WHERE id=:id");
         qUpdateOldIng.prepare(updateString);
         qUpdateOldIng.bindValue( ":zero", Brewtarget::dbFalse() );

         QSqlQuery qOldBtIng( sqlDatabase() );
         qOldBtIng.prepare( QString("SELECT * FROM bt_%1 WHERE id=:btid").arg(tp.tableName) );

         QSqlQuery qOldBtIngInsert( sqlDatabase() );
         qOldBtIngInsert.prepare(
            QString("INSERT INTO bt_%1 (id,%1_id) values (:id,:%1_id)").arg(tp.tableName) );

         // Resize propVal appropriately for current table.
         propVal.clear();
         foreach( QString pn, tp.propName )
            propVal.append(QVariant());

         while( qNewBtIng.next() )
         {
            btid = qNewBtIng.record().value("id");
            newid = qNewBtIng.record().value(QString("%1_id").arg(tp.tableName));

            qNewIng.bindValue(":id", newid);
            if ( ! qNewIng.exec() )
               throw QString("Could not retrieve new ingredient: %1 %2").arg(qNewIng.lastQuery()).arg(qNewIng.lastError().text());
            if( !qNewIng.next() )
               throw QString("Could not advance query: %1 %2").arg(qNewIng.lastQuery()).arg(qNewIng.lastError().text());

            QList<QVariant>::iterator it = propVal.begin();
            foreach( QString pn, tp.propName )
            {
               // Get new value.
               *it = qNewIng.record().value(pn);
               // Bind it to the old ingredient.
               qUpdateOldIng.bindValue(
                  QString(":%1").arg(pn),
                  *it );
               ++it;
            }

            // Done retrieving new ingredient data.
            qNewIng.finish();

            // Find the bt_<ingredient> record in the local table.
            qOldBtIng.bindValue( ":btid", btid );
            if ( ! qOldBtIng.exec() )
               throw QString("Could not find btID (%1): %2 %3")
                        .arg(btid.toInt())
                        .arg(qOldBtIng.lastQuery())
                        .arg(qOldBtIng.lastError().text());

            // If the btid exists in the old bt_hop table, do an update.
            if( qOldBtIng.next() )
            {
               oldid = qOldBtIng.record().value( QString("%1_id").arg(tp.tableName) );
               qOldBtIng.finish();

               qUpdateOldIng.bindValue( ":id", oldid );

               if ( ! qUpdateOldIng.exec() )
                  throw QString("Could not update old btID (%1): %2 %3")
                           .arg(oldid.toInt())
                           .arg(qUpdateOldIng.lastQuery())
                           .arg(qUpdateOldIng.lastError().text());

            }
            // If the btid doesn't exist in the old bt_hop table, do an insert into
            // the old hop table, then into the old bt_hop table.
            else
            {
               // Create a new ingredient.
               oldid = tp.newElement()->_key;

               // Copy in the new data.
               qUpdateOldIng.bindValue( ":id", oldid );

               if ( ! qUpdateOldIng.exec() )
                  throw QString("Could not insert new btID (%1): %2 %3")
                           .arg(oldid.toInt())
                           .arg(qUpdateOldIng.lastQuery())
                           .arg(qUpdateOldIng.lastError().text());


               // Insert an entry into our bt_<ingredient> table.
               qOldBtIngInsert.bindValue( ":id", btid );
               qOldBtIngInsert.bindValue( QString(":%1_id").arg(tp.tableName), oldid );

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
      blockSignals(false);
      throw;
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
      QMessageBox::critical(0, tr("Connection failed"),
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

   Brewtarget::DBTypes oldType = (Brewtarget::DBTypes)Brewtarget::option("dbType",Brewtarget::SQLITE).toInt();

   try {
      if ( newType == Brewtarget::NODB )
         throw QString("No type found for the new database.");

      if ( oldType == Brewtarget::NODB )
         throw QString("No type found for the old database.");

      switch( newType ) {
         case Brewtarget::PGSQL:
            newDb = openPostgres(Hostname, DbName,Username, Password, Portnum);
            break;
         default:
            newDb = openSQLite();
      }

      if ( ! newDb.isOpen() )
         throw QString("Could not open new database: %1").arg(newDb.lastError().text());

      if( newDb.tables().contains(QLatin1String("settings")) )
         return;

      if ( ! DatabaseSchemaHelper::create(newDb,newType) )
         throw QString("DatabaseSchemaHelper::create() failed");

      copyDatabase(oldType,newType,newDb);
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      throw;
   }
}

QString Database::makeInsertString( QSqlRecord here, QString realName )
{
   QString columns,qmarks;

   // Yes. I went from named to positional place holders. Such is life
   for(int i=0; i < here.count(); ++i) {
      if ( ! columns.isEmpty() ) {
         columns += QString(",%1").arg( here.fieldName(i));
         qmarks  += ",?";
      }
      else {
         columns = here.fieldName(i);
         qmarks = "?";
      }
   }
   return QString("INSERT INTO %1 (%2) VALUES(%3)").arg(realName).arg(columns).arg(qmarks);
}

QString Database::makeUpdateString( QSqlRecord here, QString realName, int key )
{
   QString columns;

   // Yes. I am still using positional place holders, and you are still dealing
   // with it
   for(int i=0; i < here.count(); ++i) {
      if ( ! columns.isEmpty() ) {
         columns += QString(",%1=?").arg( here.fieldName(i) );
      }
      else {
         columns = QString("%1=?").arg( here.fieldName(i) );
      }
   }
   return QString("UPDATE %1 SET %2 where id=%3").arg(realName).arg(columns).arg(key);
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

QStringList Database::allTablesInOrder(QSqlQuery q)
{
   QString query = "SELECT name FROM bt_alltables ORDER BY table_id";
   QStringList tmp;

   q.exec(query);
   while ( q.next() ) {
      tmp.append( q.value("name").toString());
   }
   return tmp;
}

void Database::copyDatabase( Brewtarget::DBTypes oldType, Brewtarget::DBTypes newType, QSqlDatabase newDb)
{
   QSqlDatabase oldDb = sqlDatabase();
   QSqlQuery readOld(oldDb);

   QStringList tables = allTablesInOrder(readOld);

   // There are a lot of tables to process
   foreach( QString table, tables ) {
      QSqlField field;
      bool mustPrepare = true;
      int maxid = -1;

      // bt_alltables don't get copied; metatables are created
      // when the database is. I used to say this about settings. I was wrong
      // about settings
      if ( table == "bt_alltables" )
         continue;

      QString findAllQuery = QString("SELECT * FROM %1").arg(table);
      try {

         if (! readOld.exec(findAllQuery) )
            throw QString("Could not execute %1 : %2").arg(readOld.lastQuery()).arg(readOld.lastError().text());

         newDb.transaction();

         QSqlQuery upsertNew(newDb); // we will prepare this in a bit

         // Start reading the records from the old db
         while(readOld.next()) {
            int idx;
            QSqlRecord here = readOld.record();
            QString upsertQuery;

            idx = here.indexOf("id");

            // We are going to need this for resetting the indexes later. We only
            // need it for copying to postgresql, but .. meh, not worth the extra
            // work
            if ( idx != -1 && here.value(idx).toInt() > maxid ) {
               maxid = here.value(idx).toInt();
            }

            // Prepare the insert for this table if required
            if ( mustPrepare ) {
               if ( table == QStringLiteral("settings") ) {
                  upsertQuery = makeUpdateString(here,table,here.value(idx).toInt());
               }
               else {
                  upsertQuery = makeInsertString(here,table);
               }
               upsertNew.prepare(upsertQuery);
               // but do it only once for this table
               mustPrepare = false;
            }
            // All that's left is to bind
            for(int i=0; i < here.count(); ++i) {
               upsertNew.bindValue(i,
                        convertValue(newType, here.field(i)),
                        QSql::In
               );
            }

            // and execute
            if ( ! upsertNew.exec() )
               throw QString("Could not insert new row %1 : %2").arg(upsertNew.lastQuery()).arg(upsertNew.lastError().text());
         }
         // We need to manually reset the sequences
         if ( newType == Brewtarget::PGSQL && maxid > 0 ) {
            QString seq = QString("SELECT setval('%1_id_seq',%2)").arg(table).arg(maxid);
            QSqlQuery updateSeq(seq, newDb);

            if ( ! updateSeq.exec(seq) )
               throw QString("Could not reset the sequences: %1 %2")
                  .arg(seq).arg(updateSeq.lastError().text());
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

