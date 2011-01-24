/*
 * tabletest.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QApplication>
#include <QVector>
#include <iostream>
#include "MiscTableWidget.h"
#include "MiscTableModel.h"
#include "xmltree.h"
#include "xmlnode.h"
#include "hop.h"

int main( int argc, char **argv )
{
   QVector<const XmlNode*> nodes;
   Misc *misc;
   XmlTree tree = XmlTree( std::cin );
   std::cout << "Is valid: " << tree.isValid() << std::endl;
   
   tree.getNodesWithTag( nodes, "MISC" );
   misc = new Misc(nodes[0]);

   QApplication app(argc,argv);
   MiscTableWidget* table = new MiscTableWidget();
   MiscTableModel* model = table->getModel();
   model->addMisc(misc);
   model->notify(misc);

   table->show();
   return app.exec();
}

