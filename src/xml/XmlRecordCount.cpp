/*
 * xml/XmlRecordCount.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
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
#include "xml/XmlRecordCount.h"

XmlRecordCount::XmlRecordCount() : skips{}, oks{} {
   return;
}

void XmlRecordCount::skipped(QString recordName) {
   // If we already have a count, get it and add one, otherwise start from 1
   // If QMap holds an item with key recordName then insert() will just replace its existing value
   this->skips.insert(recordName,
                      this->skips.contains(recordName) ? (this->skips.value(recordName) + 1) : 1);
   return;
}

void XmlRecordCount::processedOk(QString recordName) {
   // Same implementation as skipped() above, but not (IMHO) enough code duplication to pull out into a common
   // function
   this->oks.insert(recordName,
                    this->oks.contains(recordName) ? (this->oks.value(recordName) + 1) : 1);
   return;
}

bool XmlRecordCount::writeToUserMessage(QTextStream & userMessage) {

   if (this->oks.isEmpty() && this->skips.isEmpty()) {
      //
      // Haven't managed to get the XSD to enforce that there is at least some recognisable content in the file, so we
      // need to handle this case ourselves.
      //
      userMessage << this->tr("Couldn't find any recognisable data in the file!");
      return false;
   }

   if (!this->oks.isEmpty()) {
      userMessage << this->tr("🗸 Read ");
      int totalRecordsRead = 0;
      int typesOfRecordsRead = 0;
      for (auto ii = this->oks.constBegin(); ii != this->oks.constEnd(); ++ii, ++typesOfRecordsRead) {
         if (0 != typesOfRecordsRead) {
            userMessage << ", ";
         }
         userMessage << ii.value() << " " << ii.key();
         totalRecordsRead += ii.value();
      }
      userMessage << (1 == totalRecordsRead ? this->tr(" record") : this->tr(" records"));
   }

   if (!this->skips.isEmpty()) {
      // If we read some records _and_ skipped some, then we need some space between the two messages (about what we
      // read and what we skipped).
      if (!this->oks.isEmpty()) {
         userMessage << this->tr("\n\n");
      }

      userMessage << this->tr("↷ Skipped ");
      int totalRecordsSkipped = 0;
      int typesOfRecordsSkipped = 0;
      for (auto ii = this->skips.constBegin(); ii != this->skips.constEnd(); ++ii, ++typesOfRecordsSkipped) {
         if (0 != typesOfRecordsSkipped) {
            userMessage << ", ";
         }
         userMessage << ii.value() << " " << ii.key();
         totalRecordsSkipped += ii.value();
      }
      userMessage <<
         (1 == totalRecordsSkipped ? this->tr(" record") : this->tr(" records")) << " already in database";
   }

   return true;
}
