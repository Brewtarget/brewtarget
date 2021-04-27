/*
 * BtFolder.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#include <QString>
#include <QRegExp>  // Yeah, you knew that had to happen
#include <QDebug>

#include "BtFolder.h"
#include "brewtarget.h"

BtFolder::BtFolder() : QObject()
{
   setObjectName("BtFolder");
}

BtFolder::BtFolder( BtFolder const& other ) : QObject()
{
   setObjectName("BtFolder");
   _name = other.name();
   _path = other.path();
   _fullPath = other.fullPath();
}

QString BtFolder::name() const { return _name; }
QString BtFolder::path() const { return _path; }
QString BtFolder::fullPath() const { return _fullPath; }

// changing the name changes the fullPath
void BtFolder::setName(QString var) 
{ 
   _name = var; 
   _fullPath = _path.append("/").append(_name);
}

// changing the path changes the fullPath
void BtFolder::setPath(QString var) 
{ 
   _path = var; 
   _fullPath = _path.append("/").append(_name);
}

// changing the full path necessarily changes the name and the path
void BtFolder::setfullPath(QString var) 
{
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QStringList pieces = var.split("/", QString::SkipEmptyParts);
#else
   QStringList pieces = var.split("/", Qt::SkipEmptyParts);
#endif

   if ( ! pieces.isEmpty() )
   {
      _name = pieces.last();
      pieces.removeLast();
      _path = pieces.join("/");

      _fullPath = var;

   }
   else 
   {
      _name = var;
      _path = var;
      _fullPath = var;
   }
}

bool BtFolder::isFolder(QString var ) 
{

   return _fullPath == var;
}

