/*
 * xmltree.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "xmlnode.h"

#include <iostream>
#include <vector>
#include <string>
#include "xmltree.h"

std::ostream& operator<<(std::ostream& os, const XmlTree &rhs)
{
   os << "<?xml version=\"" << rhs.xmlVersion << "\"";
   if( rhs.encoding.size() > 0 )
      os << " encoding=\"" << rhs.encoding << "\"";
   if( rhs.standalone.size() > 0 )
      os << " standalone=\"" << rhs.standalone << "\"";
   
   os << "?>" << std::endl;
   
   // Start the recursion...
   rhs.root.toXml( os, 0 );
   
   return os;
}

XmlTree::XmlTree()
{
   initMembers();
}

XmlTree::XmlTree( std::istream &in )
{
   std::string tmp;
   std::string::const_iterator b;
   std::vector<std::string> tmpVec;
   unsigned int i;
   
   initMembers();
   
   while( getline( in, tmp ) )
      xmlText += tmp;
   
   iter = xmlText.begin();
   
   iterateUntilNotDelimiter(iter); // Go to first real character. Should be '<'.
   b = ++iter;
   iterateUntilDelimiter(iter);
   if( std::string(b,iter) != "?xml" )
   {
     valid = false;
     return;
   }
   
   getAttributes( tmpVec, iter );
   
   // Parse attributes.
   for( i = 0; i < tmpVec.size(); ++i )
   {
      tmp = tmpVec[i];
      
      if( tmp == "version" )
      {
         if( ++i < tmpVec.size() && tmpVec[i] == "=" && ++i < tmpVec.size() )
            xmlVersion = tmpVec[i];
         else
         {
            valid = false;
            return;
         }
      }
      else if( tmp == "encoding" )
      {
         if( ++i < tmpVec.size() && tmpVec[i] == "=" && ++i < tmpVec.size() )
            encoding = tmpVec[i];
         else
         {
            valid = false;
            return;
         }
      }
      else if( tmp == "standalone" )
      {
         if( ++i < tmpVec.size() && tmpVec[i] == "=" && ++i < tmpVec.size() )
            standalone = tmpVec[i];
         else
         {
            valid = false;
            return;
         }
      }
   } // End for(). Done parsing attributes.
   
   if( xmlVersion == "" )
   {
      valid = false;
      return;
   }
   
   root = XmlNode();
   while( iter != xmlText.end() )
   {
      // This 'if' statement prevents us from going past xmlText.end()
      if( *iter != '<' )
      {
         iter++;
         continue;
      }
      root.addChild( new XmlNode(iter, &root ) );
   }
}

bool XmlTree::isValid()
{
   return valid;
}

void XmlTree::initMembers()
{
   xmlText = "";
   xmlVersion = "";
   encoding = "";
   standalone = "";
   iter = xmlText.begin();
   valid = true;
}

// Returns the size of the nodes vector.
unsigned int XmlTree::getNodesWithTag( std::vector<XmlNode*> &nodes, const std::string &tag )
{
   nodes.clear();
   
   root.getNodesWithTag( nodes, tag );
   return nodes.size();
}
