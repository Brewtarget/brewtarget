/*
 * xmlnode.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "xmlnode.h"
#include "xml.h"

void XmlNode::initMembers()
{
   tag = "";
   attributes = std::vector<std::string>();
   parent = NULL;
   children = std::vector<XmlNode*>();
   leafText = "";
}

XmlNode::XmlNode()
{
   initMembers();
}

bool isWhiteSpace( char c )
{
   return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t'
           || c == '\v' );
}

void getAttributes( std::vector<std::string>& attrib, std::string::const_iterator &iter )
{
   std::string::const_iterator b;
   
   // Get the attributes.
   while(true)
   {
      // We are now one past the end of the tag.
      if( *iter == '>' )
         break;
      iterateUntilNotDelimiter( iter );
      b = iter;
      iterateUntilDelimiter(iter);
      attrib.push_back( replaceXmlCodes(removeQuotes(std::string(b,iter))) );
      
      // Want to push back the equals signs.
      if( *iter == '=' )
         attrib.push_back("=");
   }
}

void iterateUntilDelimiter( std::string::const_iterator &iter )
{
   while( !(isWhiteSpace(*iter) || *iter == '=' || *iter == '>') )
          ++iter;
}

void iterateUntilNotDelimiter( std::string::const_iterator &iter )
{
   while( isWhiteSpace(*iter) || *iter == '=' || *iter == '>' )
          ++iter;
}

void iterateUntilCharFound( std::string::const_iterator &iter, char c )
{
   while( *iter != c )
      ++iter;
}

std::string getNextTag( std::string::const_iterator &iter )
{
   std::string::const_iterator b;
   
   iterateUntilCharFound( iter, '<' );
   ++iter;
   b = iter;
   
   iterateUntilDelimiter( iter );
   
   return std::string(b, iter);
}

// iter should be on the end of the previous tag or
// the beginning of this tag.
XmlNode::XmlNode( std::string::const_iterator &iter, const XmlNode* parent )
{
   std::string::const_iterator b;
   std::string tmpTag;
   XmlNode *tmpNode;
   
   initMembers();
   
   while(true)
   {
      iterateUntilNotDelimiter(iter);
      if( *iter != '<' )
      {
         // Now we now that we are a leaf and only contain text.
         b = iter;
         iterateUntilCharFound( iter, '<' );
         leafText = replaceXmlCodes( std::string(b, iter) );
         return;
      }
      
      tag = getNextTag( iter ); // Not a leaf, so get tag.
      if( tag == "!--" ) // If this is an XML comment...
      {
         tag = "";
         // skip until the end of the comment...
         iterateUntilCharFound( iter, '>' );
         continue; // and try again.
      }
      else if( beginsWith(tag, "/") ) // Previous XmlNode should have parsed its own ending tag.
      {
         throw XmlNodeException("unexpected end tag: " + tag);
      }
      else
         break;
   } // end while(true)
   
   getAttributes( attributes, iter );
   
   // Get the children.
   while(true)
   {
      iterateUntilNotDelimiter(iter);
      b = iter;
      
      // Break if we reach the end tag.
      if( *iter == '<' && getNextTag( iter ) == ("/" + tag) )
         break;
      else
         iter = b; // Reset iter.
      
      tmpNode = new XmlNode(iter, this);
      children.push_back(tmpNode);
   }
}

bool XmlNode::isLeaf() const
{
   return children.size() == 0;
}

void XmlNode::printLeaves() const
{
   if( isLeaf() )
   {
      std::cout << "Leaf: \"" +leafText + "\" ";
      printParentTags();
      return;
   }
   
   unsigned int i, size = children.size();
   for( i = 0; i < size; ++i )
      children[i]->printLeaves();
}

void XmlNode::getNodesWithTag( std::vector<const XmlNode*> &nodes, const std::string &inTag ) const
{
   if( tag == inTag )
      nodes.push_back(this);
   
   // Recursion base case.
   if( isLeaf() )
      return;
   
   unsigned int i, size = children.size();
   for( i = 0; i < size; ++i )
      children[i]->getNodesWithTag( nodes, inTag );
}

void XmlNode::getChildren( std::vector<XmlNode*>& v) const
{
   v.clear();
   copy( children.begin(), children.end(), back_inserter(v) );
   return;
}

const std::string& XmlNode::getTag() const
{
   return tag;
}

void XmlNode::printParentTags() const
{
   if( parent == NULL )
      return;
      
   std::cout << "<" + parent->tag + ">, ";
   parent->printParentTags();
}

void XmlNode::toXml( std::ostream& os, int indentLevel ) const
{
   unsigned int i;
   std::string indent( 3*indentLevel, ' ' );
   
   if( isLeaf() )
   {
      os << indent << leafText << std::endl;
      return;
   }
   
   // Output the beginning of the tag...
   os << indent << "<" + tag;
   // ...now all of the attributes...
   for( i = 0; i < attributes.size(); ++i )
   {
      if( attributes[i] == "=" )
         os << attributes[i];
      else
      {
         if( i > 0 && attributes[i-1] == "=" )
            os << attributes[i];
         else
            os << " " + attributes[i];
      }
   }
   // ...and finally the end of the tag.
   os << ">" << endl;
   
   // Now print all the children.
   for( i = 0; i < children.size(); ++i )
      children[i]->toXml( os, indentLevel+1 );
   
   // Now the closing tag.
   os << indent << "</" + tag + ">" << std::endl;
   
   return;
}

std::string XmlNode::getLeafText() const
{
   return leafText;
}
