/*
 * Html.cpp is part of Brewtarget, and was written by
 * Mark de Wever (koraq@xs4all.nl), copyright 2016
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Html.h"

#include <QFile>
#include <QString>
#include <QTextStream>

namespace Html
{

QString getCss(const QString& resourceName)
{
   QFile cssInput(resourceName);
   QString result;

   if (cssInput.open(QFile::ReadOnly))
   {
      QTextStream inStream(&cssInput);
      while (!inStream.atEnd())
      {
         result += inStream.readLine();
      }
   }
   return result;
}

QString createHeader(const QString& title, const QString& cssResourceName)
{
   return QString(
                "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
                "\"http://www.w3.org/TR/1998/REC-html40-19980424/strict.dtd\">"
                "<html>"
                "<head>"
                "<meta http-equiv=\"Content-Type\" content=\"text/html; "
                "charset=utf-8\">"
                "<title>%1</title>"
                "<style type=\"text/css\">%2</style>"
                "</head>"
                "<body>")
         .arg(title)
         .arg(getCss(cssResourceName));
}

QString createFooter()
{
   return "</body></html>";
}

} // namespace Html
