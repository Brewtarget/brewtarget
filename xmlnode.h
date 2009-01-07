/*
 * xmlnode.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef XMLNODE_H_
#define XMLNODE_H_

#include <string>
#include <vector>
#include <exception>

void iterateUntilDelimiter( std::string::const_iterator &iter );
void iterateUntilNotDelimiter( std::string::const_iterator &iter );
void iterateUntilCharFound( std::string::const_iterator &iter, char c );
std::string getNextTag( std::string::const_iterator &iter );
void getAttributes( std::vector<std::string>& attrib, std::string::const_iterator &iter );
bool isWhiteSpace( char c );

class XmlNode;
class XmlNodeException;

class XmlNode
{
   public:
      XmlNode();
      XmlNode( std::string::const_iterator &iter, const XmlNode* parent );
      bool isLeaf() const;
      void printLeaves() const;
      void printParentTags() const;
      void getNodesWithTag( std::vector<const XmlNode*> &nodes, const std::string &inTag ) const;
      void getChildren( std::vector<XmlNode*>& v) const;
      const std::string& getTag() const;
      std::string getLeafText() const;
      void toXml( std::ostream& os, int indentLevel ) const;
      
   private:
      std::string tag;
      std::vector<std::string> attributes;
      XmlNode *parent;
      std::vector<XmlNode*> children;
      std::string leafText; // Only valid if node has no children.
      
      void initMembers();
};

class XmlNodeException : public std::exception
{
public:

   virtual const char* what() const throw()
   {
      return std::string("XmlNode error: " + _err + "\n").c_str();
   }

   XmlNodeException( std::string message )
   {
      _err = message;
   }

   ~XmlNodeException() throw() {}

private:

   std::string _err;
};

#endif /*XMLNODE_H_*/
