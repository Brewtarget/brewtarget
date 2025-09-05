/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/ImportExport.cpp is part of Brewtarget, and is copyright the following authors 2013-2025:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "serialization/ImportExport.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>

#include "MainWindow.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUseOfWater.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "serialization/json/BeerJson.h"
#include "serialization/xml/BeerXml.h"

namespace {
   enum class ImportOrExport {
      EXPORT,
      IMPORT
   };

   //
   // Note that we cannot guarantee the file extension is at the end of the input string -- eg we might be matching
   // against "BeerJSON format (*.json)".
   //
   // In the regex below, we assume:
   //   - File extension starts with dot -- "[.]" (Saves trying to work out how many backslashes to use to escape dot!)
   //   - File extensions are only latin letters but may be upper or lower case or both -- "[a-zA-Z]"
   //   - File extensions are any non-zero length -- "+"
   //   - File extension is either at end of string or followed by something that is not a letter or a
   //     digit -- "[^a-zA-Z0-9]|$"
   //
   // This is a bit overkill for what we need for now, but not hugely, and is potentially useful for future expansion
   // (eg if we want to generalise file extension handling).
   //
   static QRegularExpression const fileExtensionRegexp {"[.]([a-zA-Z]+)[^a-zA-Z0-9]|$"};

   /**
    * \brief Display a file dialog for selecting file(s) for reading / writing
    */
   std::optional<QStringList> selectFiles(ImportOrExport const importOrExport) {
      //
      // Set up the file chooser dialog.  In previous versions of the code, this was created once and reused every time
      // we want to open a file.  The advantage of that is that, on subsequent uses, the file dialog is going to open
      // wherever you navigated to when you last opened a file.  However, as at 2020-12-30, there is a known bug in Qt
      // (https://bugreports.qt.io/browse/QTBUG-88971) which means you cannot make a QFileDialog "forget" previous
      // files you have selected with it.  So each time you you show it, the subsequent list returned from
      // selectedFiles() is actually all files _ever_ selected with this dialog object.  (The bug report is a bit bare
      // bones, but https://forum.qt.io/topic/121235/qfiledialog-has-memory has more detail.)
      //
      // Our workaround is to use a new QFileDialog each time, and manually keep track of the current directory.  This
      // also has the advantage that we remember the same directory for both reading and writing.
      //
      static QString fileChooserDirectory{QDir::homePath()};
      bool const importing{importOrExport == ImportOrExport::IMPORT};
      // Note that we can't have the "both" formats filter for exporting, because QFileDialog will just slap a .json
      // extension on, even if you specified myfile.xml.  .:TBD:. There is probably a better way of handling this that
      // we should look into at some point.
      QFileDialog fileChooser{
         &MainWindow::instance(),
         importing ? QObject::tr("Open") : QObject::tr("Save"),
         fileChooserDirectory,
         importing ?
            QObject::tr("BeerJSON and BeerXML files (*.json *.xml);;BeerJSON files (*.json);;BeerXML files (*.xml)") :
            QObject::tr("BeerJSON format (*.json);;BeerXML format (*.xml)")
      };
      fileChooser.setViewMode(QFileDialog::List);
      if (importing) {
         fileChooser.setAcceptMode(QFileDialog::AcceptOpen);
         fileChooser.setFileMode(QFileDialog::ExistingFiles);
      } else {
         Q_ASSERT(importOrExport == ImportOrExport::EXPORT);
         fileChooser.setAcceptMode(QFileDialog::AcceptSave);
         fileChooser.setFileMode(QFileDialog::AnyFile);
         //
         // If the user doesn't specify a suffix, we choose one for them.  By default it's "json" to match the first
         // filter in the list.  If the user changes the filter, then we get a signal and change the default suffix
         // accordingly.
         //
         // I think in practice QFileDialog::filterSelected will always get sent when exec() is run below, so we don't
         // need the call to setDefaultSuffix here as the one in the lambda below will suffice.  But it doesn't hurt to
         // have this initial one, so we'll leave it for now.
         //
         fileChooser.setDefaultSuffix(QString("json"));
         fileChooser.connect(
            &fileChooser,
            &QFileDialog::filterSelected,
            [&fileChooser](QString const & filter) {
               //
               // The Qt docs are a bit silent about what the parameter is for the QFileDialog::filterSelected signal.
               // By adding a logging statement here, we discovered in Qt5 that we got either "*.json" or "*.xml".
               // However, in Qt6, we got "BeerJSON format (*.json)" or "BeerXML format (*.xml)".
               //
               QRegularExpressionMatch match = fileExtensionRegexp.match(filter);
               if (!match.hasMatch()) {
                  //
                  // This shouldn't happen because we should be parsing one of the options we gave in the fileChooser
                  // constructor above.
                  //
                  qCritical() << Q_FUNC_INFO << "Unable to parse" << filter;
                  return;
               }

               QString const suffix = match.captured(1).toLower();
               qDebug() << Q_FUNC_INFO << "Export filter is:" << filter << ".  From this, suffix is:" << suffix;
               fileChooser.setDefaultSuffix(suffix);
               return;
            }
         );
      }

      if (!fileChooser.exec() ) {
         // User clicked cancel, so nothing more to do
         return std::nullopt;
      }

      //
      // QFileDialog::defaultSuffix() excludes the leading dot on the suffix, but things are easier for us below if we
      // include it.
      //
      QString const defaultSuffix = QString{".%1"}.arg(fileChooser.defaultSuffix());
      QList<QString> selectedFiles = fileChooser.selectedFiles();

      qDebug() <<
         Q_FUNC_INFO << "Selected" << selectedFiles.length() << "file(s) (from directory" << fileChooser.directory() <<
         "):" << fileChooser.selectedFiles() << ". Default suffix:" << defaultSuffix;

      // Remember the directory for next time
      fileChooserDirectory = fileChooser.directory().canonicalPath();

      //
      // If we are writing, then, for any files that do not have a suffix, add the default one
      //
      if (!importing) {
         for (QString & file : selectedFiles) {
            if (!file.endsWith(".json", Qt::CaseInsensitive) &&
                !file.endsWith(".xml" , Qt::CaseInsensitive)) {
               file.append(defaultSuffix);
            }
         }
      }

      return selectedFiles;
   }

   /**
    * \brief Show a success/failure message to the user after we attempted to import one or more BeerXML files
    */
   void importExportMsg(ImportOrExport const importOrExport,
                        QString const & fileName,
                        bool succeeded,
                        QString const & userMessage) {
      // This will allow us to drop the directory path to the file, as it is often long and makes the message box a
      // "wall of text" that will put a lot of users off.
      QFileInfo fileInfo(fileName);

      QString messageBoxTitle{succeeded ? QObject::tr("Success!") : QObject::tr("ERROR")};
      QString messageBoxText;
      if (succeeded) {
         // The userMessage parameter will tell how many files were imported/exported and/or skipped (as duplicates)
         // Do separate messages for import and export as it makes translations easier
         if (ImportOrExport::IMPORT == importOrExport) {
            messageBoxText = QString(
               QObject::tr("Successfully read \"%1\"\n\n%2").arg(fileInfo.fileName()).arg(userMessage)
            );
         } else {
            messageBoxText = QString(
               QObject::tr("Successfully wrote \"%1\"\n\n%2").arg(fileInfo.fileName()).arg(userMessage)
            );
         }
      } else {
         if (ImportOrExport::IMPORT == importOrExport) {
            messageBoxText = QString(
               QObject::tr("Unable to import data from \"%1\"\n\n"
                           "%2\n\n"
                           "Log file may contain more details.").arg(fileInfo.fileName()).arg(userMessage)
            );
         } else {
            // Some write errors (eg nothing to export) are before the filename was chosen (in which case the name will
            // be blank).
            if (fileName == "") {
               messageBoxText = QString("%2").arg(userMessage);
            } else {
               messageBoxText = QString(
                  QObject::tr("Unable to write data to \"%1\"\n\n"
                              "%2\n\n"
                              "Log file may contain more details.").arg(fileInfo.fileName()).arg(userMessage)
               );
            }
         }
      }
      qDebug() << Q_FUNC_INFO << "Message box text : " << messageBoxText;
      QMessageBox msgBox{succeeded ? QMessageBox::Information : QMessageBox::Critical,
                         messageBoxTitle,
                         messageBoxText};
      msgBox.exec();
      return;
   }

   /**
    * \brief Turn a possibly null list into a set
    */
   template<class NE> QSet<NE const *> makeSet(QList<NE const *> const * nes) {
      QSet<NE const *> neSet;
      if (nes) {
         neSet = QSet<NE const *>{nes->begin(), nes->end()};
      }
      return neSet;
   }

   /**
    * \brief Make a set of Hop/Fermentable/etc from a list of same and a list of Recipes
    *        Used in ImportExport::exportToFile when exporting to BeerJSON.  See comment in that function for why.
    */
   template<class NE> QSet<NE const *> makeSet(QList<NE     const *> const * ingredients,
                                               QList<Recipe const *> const * recipes) {
      QSet<NE const *> ingredientSet{makeSet(ingredients)};
      if (recipes) {
         for (Recipe const * recipe : *recipes) {
            auto ingredientAdditions = recipe->allOwned<typename NE::RecipeAdditionClass>();
            for (auto ingredientAddition : ingredientAdditions) {
               auto ingredient = ingredientAddition->ingredient();
               if (ingredient) {
                  ingredientSet.insert(ingredient.get());
               }
            }
         }
      }
      return ingredientSet;
   }
}

bool ImportExport::importFromFiles(std::optional<QStringList> inputFiles) {
   if (!inputFiles) {
      inputFiles = selectFiles(ImportOrExport::IMPORT);
   }
   if (!inputFiles) {
      return false;
   }

   bool allSucceeded = true;
   for (QString filename : *inputFiles) {
      //
      // I guess if the user were importing a lot of files in one go, it might be annoying to have a separate result
      // message for each one, but TBD whether that's much of a use case.  For now, we keep things simple.
      //
      qDebug() << Q_FUNC_INFO << "Importing " << filename;
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      bool succeeded = false;
      if (filename.endsWith("json", Qt::CaseInsensitive)) {
         succeeded = BeerJson::import(filename, userMessageAsStream);
      } else if (filename.endsWith("xml", Qt::CaseInsensitive)) {
         succeeded = BeerXML::getInstance().importFromXML(filename, userMessageAsStream);
      } else {
         qInfo() << Q_FUNC_INFO << "Don't understand file extension on" << filename << "so ignoring!";
         userMessageAsStream <<
            QObject::tr("Did not recognise file extension on \"%1\" so nothing written.").arg(filename);
      }
      qDebug() << Q_FUNC_INFO << "Import " << (succeeded ? "succeeded" : "failed");
      importExportMsg(ImportOrExport::IMPORT, filename, succeeded, userMessage);

      allSucceeded &= succeeded;
   }

   MainWindow::instance().showChanges();

   return allSucceeded;
}

bool ImportExport::exportToFile(QList<Recipe      const *> const * recipes,
                                QList<Equipment   const *> const * equipments,
                                QList<Fermentable const *> const * fermentables,
                                QList<Hop         const *> const * hops,
                                QList<Misc        const *> const * miscs,
                                QList<Style       const *> const * styles,
                                QList<Water       const *> const * waters,
                                QList<Yeast       const *> const * yeasts) {
   // It's the caller's responsibility to ensure that at least one list is supplied and that at least one of the
   // supplied lists is non-empty
   Q_ASSERT((recipes      && recipes     ->size() > 0) ||
            (equipments   && equipments  ->size() > 0) ||
            (fermentables && fermentables->size() > 0) ||
            (hops         && hops        ->size() > 0) ||
            (miscs        && miscs       ->size() > 0) ||
            (styles       && styles      ->size() > 0) ||
            (waters       && waters      ->size() > 0) ||
            (yeasts       && yeasts      ->size() > 0));

   auto selectedFiles = selectFiles(ImportOrExport::EXPORT);
   if (!selectedFiles) {
      return false;
   }
   QString filename = (*selectedFiles)[0];

   QString userMessage;
   QTextStream userMessageAsStream{&userMessage};

   // Destructor will close the file if nec when we exit the function
   QFile outFile;
   outFile.setFileName(filename);

   bool succeeded = false;

   if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      qWarning() << Q_FUNC_INFO << "Could not open" << filename << "for writing.";
      userMessageAsStream << QObject::tr("Could not open \"%1\" for writing").arg(filename);

   } else if (filename.endsWith(".json", Qt::CaseInsensitive)) {
      //
      // It's not strictly required by the BeerJSON standard, but we'll get a better export of Recipe if we also
      // explicitly export all the ingredients.  This is because, in BeerJSON (unlike BeerXML), the Recipe specification
      // includes only partial information about each Hop/Fermentable/etc addition.  This is fine if you already have
      // those ingredients in your database when you're reading a Recipe in, but doesn't work so well when you don't.
      //
      QSet<Fermentable const *> setOfFermentable = makeSet(fermentables, recipes);
      QSet<Hop         const *> setOfHop         = makeSet(hops        , recipes);
      QSet<Misc        const *> setOfMisc        = makeSet(miscs       , recipes);
      QSet<Yeast       const *> setOfYeast       = makeSet(yeasts      , recipes);
      QSet<Water       const *> setOfWater       = makeSet(waters      , recipes);
      //
      // Same thing applies for Styles, and we have similar thinking for Equipments.  (Though note that, unlike in
      // BeerXML, Equipment is not part of the Recipe in BeerJSON.)
      //
      QSet<Style       const *> setOfStyle       = makeSet(styles);
      QSet<Equipment   const *> setOfEquipment   = makeSet(equipments);
      if (recipes) {
         for (Recipe const * recipe : *recipes) {
            auto style     = recipe->style    (); if (style    ) { setOfStyle    .insert(style    .get()); }
            auto equipment = recipe->equipment(); if (equipment) { setOfEquipment.insert(equipment.get()); }
         }
      }

      BeerJson::Exporter exporter(outFile, userMessageAsStream);
      if (!setOfFermentable.isEmpty()   ) { exporter.add(setOfFermentable.values()); }
      if (!setOfHop        .isEmpty()   ) { exporter.add(setOfHop        .values()); }
      if (!setOfMisc       .isEmpty()   ) { exporter.add(setOfMisc       .values()); }
      if (!setOfYeast      .isEmpty()   ) { exporter.add(setOfYeast      .values()); }
      if (!setOfStyle      .isEmpty()   ) { exporter.add(setOfStyle      .values()); }
      if (!setOfEquipment  .isEmpty()   ) { exporter.add(setOfEquipment  .values()); }
      if (!setOfWater      .isEmpty()   ) { exporter.add(setOfWater      .values()); }
      if (recipes && recipes->size() > 0) { exporter.add(*recipes                 ); }

      exporter.close();
      succeeded = true;
   } else if (filename.endsWith(".xml", Qt::CaseInsensitive)) {
      BeerXML & bxml = BeerXML::getInstance();
      // The slightly non-standard-XML format of BeerXML means the common bit (which gets written by createXmlFile) is
      // just at the start and there is no "closing" bit to write after we write all the data.
      bxml.createXmlFile(outFile);

      //
      // Not that it matters, but the order things are listed in the BeerXML 1.0 spec is:
      //    HOPS
      //    FERMENTABLES
      //    YEASTS
      //    MISCS
      //    WATERS
      //    STYLES
      //    MASH_STEPS
      //    MASHS
      //    RECIPES
      //    EQUIPMENTS
      //
      if (hops         && hops        ->size() > 0) { bxml.toXml(*hops,         outFile); }
      if (fermentables && fermentables->size() > 0) { bxml.toXml(*fermentables, outFile); }
      if (yeasts       && yeasts      ->size() > 0) { bxml.toXml(*yeasts,       outFile); }
      if (miscs        && miscs       ->size() > 0) { bxml.toXml(*miscs,        outFile); }
      if (waters       && waters      ->size() > 0) { bxml.toXml(*waters,       outFile); }
      if (styles       && styles      ->size() > 0) { bxml.toXml(*styles,       outFile); }
      if (recipes      && recipes     ->size() > 0) { bxml.toXml(*recipes,      outFile); }
      if (equipments   && equipments  ->size() > 0) { bxml.toXml(*equipments,   outFile); }

      succeeded = true;
   } else {
      qInfo() << Q_FUNC_INFO << "Don't understand file extension on" << filename << "so ignoring!";
      userMessageAsStream <<
         QObject::tr("Did not recognise file extension on \"%1\" so nothing read.").arg(filename);
   }


   qDebug() << Q_FUNC_INFO << "Export" << (succeeded ? "succeeded" : "failed");
   importExportMsg(ImportOrExport::EXPORT, filename, succeeded, userMessage);

   return false;
}
