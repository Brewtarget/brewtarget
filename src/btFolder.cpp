/*
 * btFolder.cpp is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#include <QString>
#include <QRegExp>  // Yeah, you knew that had to happen

#include "btFolder.h"
#include "brewtarget.h"

btFolder::btFolder() : QObject()
{
   setObjectName("btFolder");
}

btFolder::btFolder( btFolder const& other ) : QObject()
{
   setObjectName("btFolder");
   _name = other.name();
   _path = other.path();
   _fullPath = other.fullPath();
}

QString btFolder::name() const { return _name; }
QString btFolder::path() const { return _path; }
QString btFolder::fullPath() const { return _fullPath; }

// changing the name changes the fullPath
void btFolder::setName(QString var) 
{ 
   _name = var; 
   _fullPath = _path.append("/").append(_name);

}

// changing the path changes the fullPath
void btFolder::setPath(QString var) 
{ 
   _path = var; 
   _fullPath = _path.append("/").append(_name);
}

// changing the full path necessarily changes the name and the path
void btFolder::setfullPath(QString var) 
{
   QStringList pieces = var.split("/");

   if ( ! pieces.isEmpty() )
   {
      _name = pieces.last();
      pieces.removeLast();
      _path = pieces.join("/");

      _fullPath = var;
   }
}

bool btFolder::isFolder(QString var ) 
{
   bool ret = false;

   if ( var == _name )
      ret = true;
   else if ( var == _fullPath )
      ret = true;

   return ret;
}

