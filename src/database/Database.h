/*
 * database/Database.h is part of Brewtarget, and is copyright the following
 * authors 2009-2021:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#ifndef DATABASE_H
#define DATABASE_H
#pragma once

#include <memory> // For PImpl

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QString>

class BtStringConst;

/*!
 * \class Database
 *
 * \brief Handles connections to the database.
 *
 * This class is a sort-of singleton, in that there is one instance for each type of DB.
 */
class Database {
   Q_DECLARE_TR_FUNCTIONS(Database)

public:

   //! \brief Supported databases. I am not 100% sure I'm digging this
   //  solution, but this is more extensible than what I was doing previously
   enum DbType {
      NODB = 0,  // Popularity was over rated
      SQLITE,    // compact, fast and a little loose
      PGSQL,     // big, powerful, uptight and a little stodgy
      ALLDB      // Keep this one the last one, or bad things will happen
   };

   /*!
    * \brief This should be the ONLY way you get an instance.
    *
    * \param dbType Which type of database object you want to get.  If not specified (or set to Database::NODB) then
    *               the default configured type will be returned
    */
   static Database& instance(Database::DbType dbType = Database::NODB);

   /**
    * \brief Check for new default ingredients etc
    */
   void checkForNewDefaultData();

   /*! \brief Get the right database connection for the calling thread.
    *
    *         Note the following from https://doc.qt.io/qt-5/qsqldatabase.html#database:
    *            "An instance of QSqlDatabase represents [a] connection ... to the database. ... It is highly
    *            recommended that you do not keep a copy of [a] QSqlDatabase [object] around as a member of a class,
    *            as this will prevent the instance from being correctly cleaned up on shutdown."
    *
    *         Moreover, there can be multiple instances of a QSqlDatabase object for a single connection.  (Copying
    *         the object does not create a new connection, it just creates a new object that references the same
    *         underlying connection.)
    *
    *         Per https://doc.qt.io/qt-5/qsqldatabase.html#removeDatabase, ALL QSqlDatabase objects (and QSqlQuery
    *         objects) for a given database connection MUST be destroyed BEFORE the underlying database connection is
    *         removed from Qt's list of database connections (via QSqlDatabase::removeDatabase() static function),
    *         otherwise errors of the form "QSqlDatabasePrivate::removeDatabase: connection ... is still in use, all
    *         queries will cease to work" will be logged followed by messy raw data dumps (ie where binary data is
    *         written to the logs without interpretation).
    *
    *         Thus, all this function does really is (a) generate a thread-specific name for this thread's connection,
    *         (b) have create and register a new connection for this thread if none exists, (c) return a new stack-
    *         allocated QSqlDatabase object for this thread's DB connection.
    *
    *         Callers should not copy the returned QSqlDatabase object nor retain it for longer than is necessary.
    *
    * \return A stack-allocated \c QSqlDatabase object through which this thread's database connection can be accessed.
    */
   QSqlDatabase sqlDatabase() const;

   //! \brief Should be called when we are about to close down.
   void unload();

   //! \brief Create a blank database in the given file
   bool createBlank(QString const& filename);

   static char const * getDefaultBackupFileName();

   //! backs up database to chosen file
   bool backupToFile(QString newDbFileName);

   //! backs up database to 'dir' in chosen directory
   bool backupToDir(QString dir, QString filename="");

   //! \brief Reverts database to that of chosen file.
   bool restoreFromFile(QString newDbFileStr);

   static bool verifyDbConnection(Database::DbType testDb,
                                  QString const& hostname,
                                  int portnum = 5432,
                                  QString const & schema="public",
                                  QString const & database="brewtarget",
                                  QString const & username="brewtarget",
                                  QString const & password="brewtarget");
   bool loadSuccessful();

   //! \brief Figures out what databases we are copying to and from, opens what
   //   needs opens and then calls the appropriate workhorse to get it done.
   void convertDatabase(QString const& Hostname, QString const& DbName,
                        QString const& Username, QString const& Password,
                        int Portnum, Database::DbType newType);

   /*!
    * \brief If we are supporting multiple databases, we need some way to
    * figure out which database we are using. I still don't know that this
    * will be the final implementation -- I can't help but think I should be
    * subclassing something
    */
   Database::DbType dbType() const;

   /**
    * \brief Turn foreign key constraints on or off.  Typically, turning them off is only required during copying the
    *        contents of one DB to another.
    */
   void setForeignKeysEnabled(bool enabled, QSqlDatabase connection, Database::DbType whichDb = Database::NODB);

   /**
    * \brief For a given base type, return the typename to use for the corresponding columns when creating tables.
    *
    *        Note that there is no general implementation of this template, just specialisations (defined in
    *        Database.cpp).  Supported types are:
    *           bool
    *           int
    *           unsigned int
    *           double
    *           QString
    *           QDate
    */
   template<typename T> char const * getDbNativeTypeName() const;

   /**
    * \brief Returns the text we need to use to specify an integer column as primary key when creating a table, eg:
    *           "INTEGER PRIMARY KEY" for SQLite
    *           "SERIAL PRIMARY KEY" for PostgreSQL
    *           "AUTO_INCREMENT PRIMARY KEY" for MySQL / MariaDB
    */
   char const * getDbNativePrimaryKeyDeclaration() const;

   /**
    * \brief Returns a text template for an ALTER TABLE query to add a foreign key column to a table.  Callers should
    *        create a QString from the result and append .arg() calls to set:
    *           • table name (for table to modify) as argument 1
    *           • column name (to add) as argument 2
    *           • foreign key table name as argument 3
    *           • foreign key column name as argument 4
    */
   char const * getSqlToAddColumnAsForeignKey() const;

   /*! Stores the date that we last asked the user to merge the
    *  data-space database to the user-space database.
    */
   static QDateTime lastDbMergeRequest;

   /**
    * \brief Returns a displayable set of name-value pairs for the connection details for the current database,
    *        \b excluding password
    */
   QList<QPair<QString, QString>> displayableConnectionParms() const;

   /**
    * \brief This member function should be called after you have manually inserted into a primary key column that is
    *        normally automatically populated by the database.  When you do such manual inserts, some databases (eg
    *        PostgreSQL) need to be told to update the value they would use for the next automatically generated ID.
    *
    * \param connection The connection you used to do the manual inserts
    * \param tableName  The table you inserted into
    * \param columName  The primary key column on that table
    *
    * \return \c false if there was an error, \c true otherwise
    */
   bool updatePrimaryKeySequenceIfNecessary(QSqlDatabase & connection,
                                            BtStringConst const & tableName,
                                            BtStringConst const & columnName) const;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! Hidden constructor.
   Database(DbType dbType);
   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   Database(Database const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   Database& operator=(Database const&) = delete;
   //! No move constructor
   Database(Database &&) = delete;
   //! No move assignment
   Database& operator=(Database &&) = delete;
   //! Destructor hidden.
   ~Database();

   //! Load database from file.
   bool load();
};

namespace DatabaseHelper {

   /**
    * \return displayable name for a given DB type
    */
   char const * getNameFromDbTypeName(Database::DbType whichDb);

}

#endif
