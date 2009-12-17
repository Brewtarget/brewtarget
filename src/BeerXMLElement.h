/*
 * BeerXMLElement.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _BEERXMLELEMENT_H
#define	_BEERXMLELEMENT_H

class BeerXMLElement;

#include <QDomText>
#include <QDomNode>
#include <QDomDocument>
#include <QString>

class BeerXMLElement
{
public:

   double getDouble( const QDomText& textNode );
   bool getBool( const QDomText& textNode );
   int getInt( const QDomText& textNode );

   QString text(bool val);
   QString text(double val);
   QString text(int val);

   void deepCopy( BeerXMLElement* other ); // Constructs a deep copy of this element.
   
   virtual void fromNode(const QDomNode& node) = 0; // Should initialize this element from the node.
   virtual void toXml(QDomDocument& doc, QDomNode& parent) = 0;
};

#endif	/* _BEERXMLELEMENT_H */

