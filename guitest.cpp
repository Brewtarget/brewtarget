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
#include <string>
#include <QApplication>
#include <QWidget>
#include "MainWindow.h"
#include "recipe.h"
#include "xmlnode.h"
#include "xmltree.h"

int main(int argc, char **argv)
{
   XmlTree* tree;
   std::vector<XmlNode*> nodes;
   Recipe* rec;

   QApplication app(argc,argv);
   MainWindow *dialog = new MainWindow();
   tree = new XmlTree(std::cin);

   tree->getNodesWithTag(nodes, std::string("RECIPE"));
   rec = new Recipe(nodes[0]);
   dialog->setRecipe(rec);
   dialog->notify(rec);

   dialog->show();
   return app.exec();
}
