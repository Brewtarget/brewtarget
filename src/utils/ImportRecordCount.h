/*
 * utils/ImportRecordCount.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2022:
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef UTILS_IMPORTRECORDCOUNT_H
#define UTILS_IMPORTRECORDCOUNT_H
#pragma once

#include <QCoreApplication> // For Q_DECLARE_TR_FUNCTIONS
#include <QMap>
#include <QString>
#include <QTextStream>

/**
 * \brief This class keeps tallies of records processed in loading a JSON or XML document so that we can tell the
 *        user how many objects (hops, recipes, etc) we (a) skipped over (eg because they were duplicates) and (b)
 *        loaded successfully.  (We don't need to do this for exports as there's no duplicate management to do in that
 *        direction.)
 *
 * Note that we use a QMap and not a QHash here as it's nice to be able to run through the keys in alphabetical
 * order when generating the summary message for the user.  (See eg code in xml/XmlCoding.cpp that does this.)
 */
class ImportRecordCount {
   // Per https://doc.qt.io/qt-5/i18n-source-translation.html#translating-non-qt-classes, this gives us a tr() function
   // without having to inherit from QObject.
   Q_DECLARE_TR_FUNCTIONS(ImportRecordCount)

public:
   ImportRecordCount();

   /**
    * \brief Call this to mark that we skipped over a record
    * \param recordName The name of the record (typically the class name of the object being read in).  This will be
    *                   rendered in lower case in \c writeToUserMessage.
    *                   TBD could we enhance to do this in the user's language (eg "hop" "houblon", "hopfen"?
    */
   void skipped(QString recordName);

   /**
    * \brief Call this to mark that we successfully saved a record
    * \param recordName The name of the record (typically the class name of the object being read in).  This will be
    *                   rendered in lower case in \c writeToUserMessage.
    *                   TBD could we enhance to do this in the user's language (eg "hop" "houblon", "hopfen"?
    */
   void processedOk(QString recordName);

   /**
    * \brief Construct a user-readable string summarising how many records of each type were skipped and/or successfully
    *        processed.
    * \param userMessage Where to write the text suitable for showing on-screen to the user
    * \return \b false if no records at all were skipped or processed, \b true otherwise
    */
   bool writeToUserMessage(QTextStream & userMessage);

private:
   QMap<QString, int> skips;
   QMap<QString, int> oks;
};

#endif
