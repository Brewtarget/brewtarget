/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/DefaultContentLoader.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "database/DefaultContentLoader.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QTextStream>

#include "Application.h"
#include "config.h"
#include "database/DatabaseSchemaHelper.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Recipe.h"
#include "serialization/ImportExport.h"

namespace {
   // TODO It would be neat to be able to supply folder name as a parameter to XML/JSON import
   char const * const FOLDER_FOR_SUPPLIED_RECIPES = CONFIG_APPLICATION_NAME_LC;
}

int constexpr DefaultContentLoader::availableContentVersion = 3;

DefaultContentLoader::UpdateResult DefaultContentLoader::updateContentIfNecessary(QSqlDatabase & db,
                                                                                  QTextStream  & userMessage) {
   //
   // Note that it's a coding error to call this function before the DB schema has been updated to whatever the most
   // current version is.  This is because all the various bits of code that we use to import data will be assuming
   // the DB schema is up-to-date.
   //
   // It's quick to check here, so it doesn't hurt.
   //
   int const dbSchemaVersion = DatabaseSchemaHelper::schemaVersion(db);
   Q_ASSERT(dbSchemaVersion == DatabaseSchemaHelper::latestVersion);

   //
   // In older versions of the software, default data was copied from a SQLite database file (default_db.sqlite) into
   // the user's database (which could be SQLite or PostgreSQL), and special tables (bt_hop, bt_fermentable, etc) kept
   // track of which records in the user's database had been copies from the default database.  This served two
   // purposes.  One was to know which default records were present in the user's database, so we could copy across any
   // new ones when the default data set is augmented.  The other was to allow us to attempt to modify the user's
   // records when corresponding records in the default data set were changed (typically to make corrections).  However,
   // it's risky to modify the user's existing data because you might overwrite changes they've made themselves since
   // the record was imported.  So we stopped trying to do that, and used the special tables just to track which default
   // records had and hadn't yet been imported.
   //
   // The next step was to move the default data a single BeerXML file which, besides simplifying the data import, had a
   // couple of advantages:
   //    - Being a text rather than a binary format, it's much easier in the source code repository to make (and
   //      see) changes to default data.
   //    - Our XML import code already does duplicate detection, so don't need the special tracking tables any more.
   //      We just try to import all the default data, and any records that the user already has will be skipped
   //      over.
   //
   // The plan was then to convert this to BeerJSON once we had support for that (which we now do).  However, there are
   // a couple of downsides to this approach:
   //    - As we add data over time, using a single default data file starts to become cumbersome to maintain.
   //    - Although BeerJSON is generally "better" than BeerXML, there are different nuances to both formats (eg BeerXML
   //      includes Equipment in a Recipe, where as BeerJSON does not).  Where we have source data in one format, it's
   //      better to retain it in that format than convert it.
   //
   // So, now we load a sequence of "default content" files, each of which can be BeerXML or BeerJSON.  Each filename
   // begins "DefaultContentNNN-" where NNN is a unique three-digit number from a sequence starting 001.  So, eg, we can
   // have:
   //    DefaultContent001-OriginalBtContent.xml
   //    DefaultContent002-BJCP_2021_Styles.json
   //    DefaultContent003-Yeasts.json
   //    etc
   //
   // NOTE that when a new DefaultContentNNN- is added, there are changes to make to the following files:
   //    meson.build
   //    CMakeLists.txt
   //
   // We store in the DB settings table what file number we've already reached.
   //
   int const defaultContentAlreadyLoaded = DatabaseSchemaHelper::getDefaultContentVersionFromDb(db);
   if (defaultContentAlreadyLoaded < 0) {
      qWarning() << Q_FUNC_INFO << "Could not read default_content column from settings table";
      userMessage << "Error reading settings from DB";
      return DefaultContentLoader::UpdateResult::Failed;
   }

   qInfo() <<
      Q_FUNC_INFO << "availableContentVersion:" << DefaultContentLoader::availableContentVersion << ", defaultContentAlreadyLoaded:" <<
      defaultContentAlreadyLoaded;

   bool succeeded = true;
   if (defaultContentAlreadyLoaded < DefaultContentLoader::availableContentVersion &&
       Application::isInteractive() &&
       QMessageBox::question(nullptr,
                             QObject::tr("Merge Database"),
                             QObject::tr("New ingredients etc are available. Would you like to add them to your database?"),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {

      QStringList inputFiles;
      QDir const dir = Application::getResourceDir();
      for (auto ii = defaultContentAlreadyLoaded + 1; ii <= DefaultContentLoader::availableContentVersion; ++ii) {
         QString const globPattern = QString{"DefaultContent%1-*"}.arg(ii, 3, 10, QChar{'0'});
         QStringList const nameFilters{globPattern};
         QStringList const matchingFiles = dir.entryList(nameFilters, QDir::Files);
         if (matchingFiles.size() != 1) {
            //
            // This is typically a coding error or a packaging error, unless our data directory has been messed with.
            //
            qCritical() <<
               Q_FUNC_INFO << "Search for" << globPattern << "in directory" << dir << "yielded" <<
               matchingFiles.size() << "results (expecting 1):" << matchingFiles.join(", ");
            userMessage << QObject::tr("Error matching %1 file pattern in %2 directory").arg(globPattern, dir.absolutePath());
            return DefaultContentLoader::UpdateResult::Failed;
         }
         qDebug() << Q_FUNC_INFO << "Will read in" << matchingFiles.at(0);
         inputFiles << dir.absoluteFilePath(matchingFiles.at(0));
      }

      if (inputFiles.size() > 0) {
         //
         // We'd like to put any newly-imported default Recipes in the same folder as the other default ones.  To do this, we
         // first make a note of which Recipes exist already, then, after the import, any new ones need to go in the default
         // folder.
         //
         QList<Recipe *> allRecipesBeforeImport = ObjectStoreWrapper::getAllRaw<Recipe>();
         qDebug() << Q_FUNC_INFO << allRecipesBeforeImport.size() << "Recipes before import";

         succeeded = ImportExport::importFromFiles(inputFiles);

         if (succeeded) {
            //
            // Now see what Recipes exist that weren't there before the import
            //
            QList<Recipe *> allRecipesAfterImport = ObjectStoreWrapper::getAllRaw<Recipe>();
            qDebug() << Q_FUNC_INFO << allRecipesAfterImport.size() << "Recipes after import";

            //
            // Once the lists are sorted, finding the difference is just a library call
            // (Note that std::set_difference requires the longer list as its first parameter pair.)
            //
            std::sort(allRecipesBeforeImport.begin(), allRecipesBeforeImport.end());
            std::sort(allRecipesAfterImport.begin(), allRecipesAfterImport.end());
            QList<Recipe *> newlyImportedRecipes;
            std::set_difference(allRecipesAfterImport.begin(), allRecipesAfterImport.end(),
                                allRecipesBeforeImport.begin(), allRecipesBeforeImport.end(),
                                std::back_inserter(newlyImportedRecipes));
            qDebug() << Q_FUNC_INFO << newlyImportedRecipes.size() << "newly imported Recipes";
            for (auto recipe : newlyImportedRecipes) {
               recipe->setFolder(FOLDER_FOR_SUPPLIED_RECIPES);
            }

            //
            // Record that we're up-to-date on default content, so we don't try to read the files in again next time.
            //
            succeeded &= DatabaseSchemaHelper::setDefaultContentVersionFromDb(
               db,
               DefaultContentLoader::availableContentVersion
            );
         }

      }

      //
      // ImportExport::importFromFiles will already have shown success/failure pop-ups, so we don't need to interact
      // further with the user here.
      //
      return succeeded ? DefaultContentLoader::UpdateResult::Succeeded : DefaultContentLoader::UpdateResult::Failed;
   }

   return DefaultContentLoader::UpdateResult::NothingToDo;
}
