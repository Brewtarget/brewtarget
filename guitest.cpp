/*
 * guitest.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <iostream>
#include <vector>
#include <QApplication>
#include "misc.h"
#include "miscEditor.h"
#include "xmltree.h"
#include "xmlnode.h"

int main(int argc, char **argv)
{
   std::vector<const XmlNode*> nodes;
   QApplication app(argc,argv);
   miscEditor *dialog = new miscEditor();
   XmlTree tree(std::cin);
   
   tree.getNodesWithTag(nodes, "MISC");
   Misc misc(nodes[0]);
   
   dialog->setMisc(&misc);
   
   dialog->show();
   return app.exec();
}
