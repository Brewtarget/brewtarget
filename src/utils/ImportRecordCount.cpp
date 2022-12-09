/*
 * utils/ImportRecordCount.cpp is part of Brewtarget, and is Copyright the following
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
#include "utils/ImportRecordCount.h"

ImportRecordCount::ImportRecordCount() : skips{}, oks{} {
   return;
}

void ImportRecordCount::skipped(QString recordName) {
   // If we already have a count, get it and add one, otherwise start from 1
   // If QMap holds an item with key recordName then insert() will just replace its existing value
   this->skips.insert(recordName,
                      this->skips.contains(recordName) ? (this->skips.value(recordName) + 1) : 1);
   return;
}

void ImportRecordCount::processedOk(QString recordName) {
   // Same implementation as skipped() above, but not (IMHO) enough code duplication to pull out into a common
   // function
   this->oks.insert(recordName,
                    this->oks.contains(recordName) ? (this->oks.value(recordName) + 1) : 1);
   return;
}

bool ImportRecordCount::writeToUserMessage(QTextStream & userMessage) {

   if (this->oks.isEmpty() && this->skips.isEmpty()) {
      //
      // For BeerXML imports, we haven't managed to get the XSD to enforce that there is at least some recognisable
      // content in the file, so we need to handle this case ourselves.
      //
      userMessage << tr("Couldn't find any recognisable data in the file!");
      return false;
   }

   if (!this->oks.isEmpty()) {
      userMessage << tr("🗸 Read ");
      int totalRecordsRead = 0;
      int typesOfRecordsRead = 0;
      for (auto ii = this->oks.constBegin(); ii != this->oks.constEnd(); ++ii, ++typesOfRecordsRead) {
         if (0 != typesOfRecordsRead) {
            userMessage << ", ";
         }
         // NB key will typically be class name, so force lower case in the output to get "3 hop records" rather than
         // "3 Hop records" etc.
         userMessage << ii.value() << " " << ii.key().toLower();
         totalRecordsRead += ii.value();
      }
      userMessage << (1 == totalRecordsRead ? tr(" record") : tr(" records"));
   }

   if (!this->skips.isEmpty()) {
      // If we read some records _and_ skipped some, then we need some space between the two messages (about what we
      // read and what we skipped).
      if (!this->oks.isEmpty()) {
         userMessage << tr("\n\n");
      }

      userMessage << tr("↷ Skipped ");
      int totalRecordsSkipped = 0;
      int typesOfRecordsSkipped = 0;
      for (auto ii = this->skips.constBegin(); ii != this->skips.constEnd(); ++ii, ++typesOfRecordsSkipped) {
         if (0 != typesOfRecordsSkipped) {
            userMessage << ", ";
         }
         // NB key will typically be class name, so force lower case in the output to get "3 hop records" rather than
         // "3 Hop records" etc.
         userMessage << ii.value() << " " << ii.key().toLower();
         totalRecordsSkipped += ii.value();
      }
      userMessage <<
         (1 == totalRecordsSkipped ? tr(" record") : tr(" records")) << " already in database";
   }

   return true;
}
