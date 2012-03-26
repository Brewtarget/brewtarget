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
#include <QMenu>
#include "BrewTargetTreeView.h"
#include "BrewTargetTreeModel.h"
#include "recipe.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "yeast.h"
#include "brewnote.h"

BrewTargetTreeView::BrewTargetTreeView(QWidget *parent) :
   QTreeView(parent)
{
   // Set some global properties that all the kids will use.
   setContextMenuPolicy(Qt::CustomContextMenu);
   setRootIsDecorated(false);
   resizeColumnToContents(0);
   setSelectionMode(QAbstractItemView::ExtendedSelection);
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
               name = model->getEquipment(index)->name();
               break;
            case BrewTargetTreeItem::FERMENTABLE:
               name = model->getFermentable(index)->name();
               break;
            case BrewTargetTreeItem::HOP:
               name = model->getHop(index)->name();
               break;
            case BrewTargetTreeItem::MISC:
               name = model->getMisc(index)->name();
               break;
            case BrewTargetTreeItem::YEAST:
               name = model->getYeast(index)->name();
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

void BrewTargetTreeView::setupContextMenu(QWidget* top, QWidget* editor, QMenu *sMenu,int type)
{

   contextMenu = new QMenu(this);
   subMenu = new QMenu(this);

   switch(type) 
   {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      case BrewTargetTreeItem::RECIPE:
         contextMenu->addAction(tr("New Recipe"), editor, SLOT(newRecipe()));
         contextMenu->addAction(tr("Brew It!"), top, SLOT(newBrewNote()));
         contextMenu->addSeparator();

         subMenu->addAction(tr("Brew Again"), top, SLOT(reBrewNote()));
         subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

         break;
      case BrewTargetTreeItem::EQUIPMENT:
         contextMenu->addAction(tr("New Equipment"), editor, SLOT(newEquipment()));
         contextMenu->addSeparator();
         break;
      case BrewTargetTreeItem::FERMENTABLE:
         contextMenu->addAction(tr("New Fermentable"), editor, SLOT(newFermentable()));
         contextMenu->addSeparator();
         break;
      case BrewTargetTreeItem::HOP:
         contextMenu->addAction(tr("New Hop"), editor, SLOT(newHop()));
         contextMenu->addSeparator();
         break;
      case BrewTargetTreeItem::MISC:
         contextMenu->addAction(tr("New Misc"), editor, SLOT(newMisc()));
         contextMenu->addSeparator();
         break;
      case BrewTargetTreeItem::YEAST:
         contextMenu->addAction(tr("New Yeast"), editor, SLOT(newYeast()));
         contextMenu->addSeparator();
         break;
   }

   contextMenu->addMenu(sMenu);
   // Copy
   contextMenu->addAction(tr("Copy"), top, SLOT(copySelected()));
   // Delete
   contextMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));
   // export and import
   contextMenu->addSeparator();
   contextMenu->addAction(tr("Export"), top, SLOT(exportSelected()));
   contextMenu->addAction(tr("Import"), top, SLOT(importFiles()));
   
}

QMenu* BrewTargetTreeView::getContextMenu(QModelIndex selected)
{
   if ( getType(selected) == BrewTargetTreeItem::BREWNOTE )
      return subMenu;

   return contextMenu;
}

// Bad form likely

RecipeTreeView::RecipeTreeView(QWidget *parent)
   : BrewTargetTreeView(parent)
{
   model = new BrewTargetTreeModel(this, BrewTargetTreeModel::RECIPEMASK);

   setModel(model);
   setExpanded(findRecipe(0), true);
}

EquipmentTreeView::EquipmentTreeView(QWidget *parent)
   : BrewTargetTreeView(parent)
{
   model = new BrewTargetTreeModel(this, BrewTargetTreeModel::EQUIPMASK);

   setModel(model);
   setExpanded(findEquipment(0), true);

}

// Icky ick ikcy
FermentableTreeView::FermentableTreeView(QWidget *parent)
   : BrewTargetTreeView(parent)
{
   model = new BrewTargetTreeModel(this, BrewTargetTreeModel::FERMENTMASK);

   setModel(model);
   setExpanded(findFermentable(0), true);
}

// More Ick
HopTreeView::HopTreeView(QWidget *parent)
   : BrewTargetTreeView(parent)
{
   model = new BrewTargetTreeModel(this, BrewTargetTreeModel::HOPMASK);

   setModel(model);
   setExpanded(findHop(0), true);
}

// Ick some more
MiscTreeView::MiscTreeView(QWidget *parent)
   : BrewTargetTreeView(parent)
{
   model = new BrewTargetTreeModel(this, BrewTargetTreeModel::MISCMASK);

   setModel(model);
   setExpanded(findMisc(0), true);
}

// Will this ick never end?
YeastTreeView::YeastTreeView(QWidget *parent)
   : BrewTargetTreeView(parent)
{
   model = new BrewTargetTreeModel(this, BrewTargetTreeModel::YEASTMASK);

   setModel(model);
   setExpanded(findYeast(0), true);
}
