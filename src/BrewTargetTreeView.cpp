/*
 * BrewTargetTreeView.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include <QDrag>
#include "BrewTargetTreeView.h"
#include "BrewTargetTreeModel.h"

BrewTargetTreeView::BrewTargetTreeView(QWidget *parent) :
   QTreeView(parent)
{
   model = new BrewTargetTreeModel(this);
   model->startObservingDB();

   setModel(model);
}

BrewTargetTreeView::~BrewTargetTreeView()
{
   delete model;
}

BrewTargetTreeModel* BrewTargetTreeView::getModel()
{
   return model;
}

bool BrewTargetTreeView::removeRow(const QModelIndex &index)
{
   QModelIndex parent = model->parent(index);
   int position       = index.row();

   return model->removeRows(position,1,parent);
}

bool BrewTargetTreeView::isParent(const QModelIndex& parent, const QModelIndex& child)
{
   return parent == model->parent(child);
}

QModelIndex BrewTargetTreeView::getParent(const QModelIndex& child)
{
   if ( child.isValid())
      return model->parent(child);

   return QModelIndex();
}

QModelIndex BrewTargetTreeView::getFirst(int type)
{

   return model->getFirst(type);
}

Recipe* BrewTargetTreeView::getRecipe(const QModelIndex &index) const
{
   return model->getRecipe(index);
}

QModelIndex BrewTargetTreeView::findRecipe(Recipe* rec)
{
   return model->findRecipe(rec);
}

Equipment* BrewTargetTreeView::getEquipment(const QModelIndex &index) const
{
   return model->getEquipment(index);
}

QModelIndex BrewTargetTreeView::findEquipment(Equipment* kit)
{
   return model->findEquipment(kit);
}

Fermentable* BrewTargetTreeView::getFermentable(const QModelIndex &index) const
{
   return model->getFermentable(index);
}

QModelIndex BrewTargetTreeView::findFermentable(Fermentable* ferm)
{
   return model->findFermentable(ferm);
}

Hop* BrewTargetTreeView::getHop(const QModelIndex &index) const
{
   return model->getHop(index);
}

QModelIndex BrewTargetTreeView::findHop(Hop* hop)
{
   return model->findHop(hop);
}

Misc* BrewTargetTreeView::getMisc(const QModelIndex &index) const
{
   return model->getMisc(index);
}

QModelIndex BrewTargetTreeView::findMisc(Misc* misc)
{
   return model->findMisc(misc);
}

Yeast* BrewTargetTreeView::getYeast(const QModelIndex &index) const
{
   return model->getYeast(index);
}

QModelIndex BrewTargetTreeView::findYeast(Yeast* yeast)
{
   return model->findYeast(yeast);
}

BrewNote* BrewTargetTreeView::getBrewNote(const QModelIndex &index) const
{
   if ( ! index.isValid() ) 
      return NULL;

   return model->getBrewNote(index);
}

QModelIndex BrewTargetTreeView::findBrewNote(BrewNote* bNote)
{
   return model->findBrewNote(bNote);
}

int BrewTargetTreeView::getType(const QModelIndex &index)
{
   return model->getType(index);
}

void BrewTargetTreeView::mousePressEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton)
   {
      dragStart = event->pos();
      doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mousePressEvent(event);
}

void BrewTargetTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton)
      doubleClick = true;

   // Send the event on its way up to the parent
   QTreeView::mouseDoubleClickEvent(event);
}

void BrewTargetTreeView::mouseMoveEvent(QMouseEvent *event)
{
   // Return if the left button isn't down
   if (!(event->buttons() & Qt::LeftButton))
      return;

   // Return if the length of movement isn't far enough.
   if ((event->pos() - dragStart).manhattanLength() < QApplication::startDragDistance())
      return;

   if ( doubleClick )
      return;

   QDrag *drag = new QDrag(this);
   QMimeData *data = mimeData(selectionModel()->selectedRows());

   drag->setMimeData(data);
   drag->start(Qt::CopyAction);
} 

void BrewTargetTreeView::keyPressEvent(QKeyEvent *event)
{
   switch( event->key() )
   {
      case Qt::Key_Space:
      case Qt::Key_Select:
      case Qt::Key_Enter:
      case Qt::Key_Return:
         emit BrewTargetTreeView::doubleClicked(selectedIndexes().first());
         return;
   }
   QTreeView::keyPressEvent(event);
}

QMimeData *BrewTargetTreeView::mimeData(QModelIndexList indexes) 
{
   QMimeData *mimeData = new QMimeData();
   QByteArray encodedData;
   QString name;
   int type;

   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   foreach (QModelIndex index, indexes)
   {
      if (index.isValid())
      {
         type = getType(index);
         switch(type)
         {
            case BrewTargetTreeItem::EQUIPMENT:
               name = model->getEquipment(index)->getName();
               break;
            case BrewTargetTreeItem::FERMENTABLE:
               name = model->getFermentable(index)->getName();
               break;
            case BrewTargetTreeItem::HOP:
               name = model->getHop(index)->getName();
               break;
            case BrewTargetTreeItem::MISC:
               name = model->getMisc(index)->getName();
               break;
            case BrewTargetTreeItem::YEAST:
               name = model->getYeast(index)->getName();
               break;
            default:
               name = "";
        }
        stream << type << name;
      }
   }

   mimeData->setData("application/x-brewtarget", encodedData);
   return mimeData;
}

bool BrewTargetTreeView::multiSelected()
{
   QModelIndexList selected = selectionModel()->selectedRows();
   bool hasRecipe, hasSomethingElse;

   hasRecipe        = false;
   hasSomethingElse = false;

   if ( selected.count() == 0 ) 
      return false;

   foreach (QModelIndex selection, selected)
   {
      if (model->isRecipe(selection))
         hasRecipe = true;
      else
         hasSomethingElse = true;
   }

   return hasRecipe && hasSomethingElse;
}
