/*
 * HtmlViewer.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include "HtmlViewer.h"
#include <QUrl>
#include <iostream>

HtmlViewer::HtmlViewer(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
}

void HtmlViewer::setHtml(const QString& fileName)
{
   webView->load( QUrl::fromLocalFile(fileName) );
}