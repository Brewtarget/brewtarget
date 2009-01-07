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
#include <vector>
#include <iostream>
#include "FermentableTableWidget.h"
#include "FermentableTableModel.h"
#include "xmltree.h"
#include "xmlnode.h"
#include "fermentable.h"

int main( int argc, char **argv )
{
   std::vector<const XmlNode*> nodes;
   Fermentable *ferm;
   XmlTree tree = XmlTree( std::cin );
   std::cout << "Is valid: " << tree.isValid() << std::endl;
   
   tree.getNodesWithTag( nodes, "FERMENTABLE" );
   ferm = new Fermentable(nodes[0]);

   QApplication app(argc,argv);
   FermentableTableWidget* table = new FermentableTableWidget();
   FermentableTableModel* model = table->getModel();
   model->addFermentable(ferm);
   model->notify(ferm);

   table->show();
   return app.exec();
}

