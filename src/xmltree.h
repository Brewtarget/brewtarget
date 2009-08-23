/*
 * xmltree.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _XMLTREE_H
#define	_XMLTREE_H

#include <iostream>
#include <string>
#include "xmlnode.h"

class XmlTree;

class XmlTree
{
   friend std::ostream& operator<<(std::ostream& os, const XmlTree &rhs);
   
public:
   XmlTree();
   XmlTree( std::istream &in );
   bool isValid();
   unsigned int getNodesWithTag( std::vector<XmlNode*> &nodes, const std::string &tag );
   
private:
   std::string xmlText;
   std::string::const_iterator iter;
   std::string xmlVersion;
   std::string encoding;
   std::string standalone;
   bool valid;
   XmlNode root;
   
   void initMembers();
};

#endif	/* _XMLTREE_H */
