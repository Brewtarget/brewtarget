/*
 * BtTreeView.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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
#include <QDebug>
#include <QHeaderView>
#include "BtTreeView.h"
#include "BtTreeModel.h"
#include "BtTreeFilterProxyModel.h"
#include "database.h"
#include "recipe.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "yeast.h"
#include "brewnote.h"
#include "style.h"

BtTreeView::BtTreeView(QWidget *parent) :
   QTreeView(parent)
{
   // Set some global properties that all the kids will use.
   setAllColumnsShowFocus(true);
   setContextMenuPolicy(Qt::CustomContextMenu);
   setRootIsDecorated(false);
   setSelectionMode(QAbstractItemView::ExtendedSelection);

}

BtTreeModel* BtTreeView::getModel()
{
   return model;
}

bool BtTreeView::removeRow(const QModelIndex &index)
{
   QModelIndex modelIndex = filter->mapToSource(index);
   QModelIndex parent = model->parent(modelIndex);
   int position       = modelIndex.row();

   return model->removeRows(position,1,parent);
}

bool BtTreeView::isParent(const QModelIndex& parent, const QModelIndex& child)
{
   QModelIndex modelParent = filter->mapToSource(parent);
   QModelIndex modelChild = filter->mapToSource(child);
   return modelParent == model->parent(modelChild);
}

QModelIndex BtTreeView::getParent(const QModelIndex& child)
{
   if ( ! child.isValid() )
      return QModelIndex();

   QModelIndex modelChild = filter->mapToSource(child);
   if ( modelChild.isValid())
      return filter->mapFromSource(model->parent(modelChild));

   return QModelIndex();
}

QModelIndex BtTreeView::getFirst()
{
   return filter->mapFromSource(model->getFirst());
}

Recipe* BtTreeView::getRecipe(const QModelIndex &index) const
{
   return model->getRecipe(filter->mapToSource(index));
}

QModelIndex BtTreeView::findRecipe(Recipe* rec)
{
   return filter->mapFromSource(model->findRecipe(rec));
}

Equipment* BtTreeView::getEquipment(const QModelIndex &index) const
{
   return model->getEquipment(filter->mapToSource(index));
}

QModelIndex BtTreeView::findEquipment(Equipment* kit)
{
   return filter->mapFromSource(model->findEquipment(kit));
}

Fermentable* BtTreeView::getFermentable(const QModelIndex &index) const
{
   return model->getFermentable(filter->mapToSource(index));
}

QModelIndex BtTreeView::findFermentable(Fermentable* ferm)
{
   return filter->mapFromSource(model->findFermentable(ferm));
}

Hop* BtTreeView::getHop(const QModelIndex &index) const
{
   return model->getHop(filter->mapToSource(index));
}

QModelIndex BtTreeView::findHop(Hop* hop)
{
   return filter->mapFromSource(model->findHop(hop));
}

Misc* BtTreeView::getMisc(const QModelIndex &index) const
{
   return model->getMisc(filter->mapToSource(index));
}

QModelIndex BtTreeView::findMisc(Misc* misc)
{
   return filter->mapFromSource(model->findMisc(misc));
}

Yeast* BtTreeView::getYeast(const QModelIndex &index) const
{
   return model->getYeast(filter->mapToSource(index));
}

QModelIndex BtTreeView::findYeast(Yeast* yeast)
{
   return filter->mapFromSource(model->findYeast(yeast));
}

Style* BtTreeView::getStyle(const QModelIndex &index) const
{
   return model->getStyle(filter->mapToSource(index));
}

QModelIndex BtTreeView::findStyle(Style* style)
{
   return filter->mapFromSource(model->findStyle(style));
}

BrewNote* BtTreeView::getBrewNote(const QModelIndex &index) const
{
   if ( ! index.isValid() ) 
      return NULL;

   return model->getBrewNote(filter->mapToSource(index));
}

QModelIndex BtTreeView::findBrewNote(BrewNote* bNote)
{
   return filter->mapFromSource(model->findBrewNote(bNote));
}

int BtTreeView::type(const QModelIndex &index)
{
   return model->type(filter->mapToSource(index));
}

void BtTreeView::mousePressEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton)
   {
      dragStart = event->pos();
      doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mousePressEvent(event);
}

void BtTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton)
      doubleClick = true;

   // Send the event on its way up to the parent
   QTreeView::mouseDoubleClickEvent(event);
}

void BtTreeView::mouseMoveEvent(QMouseEvent *event)
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

void BtTreeView::keyPressEvent(QKeyEvent *event)
{
   switch( event->key() )
   {
      case Qt::Key_Space:
      case Qt::Key_Select:
      case Qt::Key_Enter:
      case Qt::Key_Return:
         emit BtTreeView::doubleClicked(selectedIndexes().first());
         return;
   }
   QTreeView::keyPressEvent(event);
}

QMimeData *BtTreeView::mimeData(QModelIndexList indexes) 
{
   QMimeData *mimeData = new QMimeData();
   QByteArray encodedData;
   QString name = "";
   int _type;

   Fermentable *fermentable = 0;
   Equipment *equipment = 0;
   Hop *hop = 0;
   Misc *misc = 0;
   Yeast *yeast = 0;

   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   // All of the calls like getType, getEquipment, etc. will translate between
   // the model and the proxy indexes, so we don't have to here
   foreach (QModelIndex index, indexes)
   {
      if (index.isValid())
      {
         _type = type(index);
         switch(_type)
         {
            case BtTreeItem::EQUIPMENT:
               equipment = model->getEquipment(index);
               if (equipment)
               {
                  name = equipment->name();
               }
               break;
            case BtTreeItem::FERMENTABLE:
               fermentable = model->getFermentable(index);
               if (fermentable)
               {
                  name = fermentable->name();
               }
               break;
            case BtTreeItem::HOP:
               hop = model->getHop(index);
               if (hop)
               {
                  name = hop->name();
               }
               break;
            case BtTreeItem::MISC:
               misc = model->getMisc(index);
               if (misc)
               {
                  name = misc->name();
               }
               break;
            case BtTreeItem::YEAST:
               yeast = model->getYeast(index);
               if (yeast)
               {
                  name = yeast->name();
               }
               break;
            default:
               name = "";
        }
        stream << _type << name;
      }
   }

   mimeData->setData("application/x-brewtarget", encodedData);
   return mimeData;
}

bool BtTreeView::multiSelected()
{
   QModelIndexList selected = selectionModel()->selectedRows();
   bool hasRecipe, hasSomethingElse;

   hasRecipe        = false;
   hasSomethingElse = false;

   if ( selected.count() == 0 ) 
      return false;

   foreach (QModelIndex selection, selected)
   {
      QModelIndex selectModel = filter->mapToSource(selection);
      if (model->isRecipe(selectModel))
         hasRecipe = true;
      else
         hasSomethingElse = true;
   }

   return hasRecipe && hasSomethingElse;
}

void BtTreeView::setupContextMenu(QWidget* top, QWidget* editor, QMenu *sMenu,int type)
{

   contextMenu = new QMenu(this);
   subMenu = new QMenu(this);

   switch(type) 
   {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      case BtTreeItem::RECIPE:
         contextMenu->addAction(tr("New Recipe"), editor, SLOT(newRecipe()));
         contextMenu->addAction(tr("Brew It!"), top, SLOT(newBrewNote()));
         contextMenu->addSeparator();

         subMenu->addAction(tr("Brew Again"), top, SLOT(reBrewNote()));
         subMenu->addAction(tr("Change date"), top, SLOT(changeBrewDate()));
         subMenu->addAction(tr("Recalculate eff"), top, SLOT(fixBrewNote()));
         subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

         break;
      case BtTreeItem::EQUIPMENT:
         contextMenu->addAction(tr("New Equipment"), editor, SLOT(newEquipment()));
         contextMenu->addSeparator();
         break;
      case BtTreeItem::FERMENTABLE:
         contextMenu->addAction(tr("New Fermentable"), editor, SLOT(newFermentable()));
         contextMenu->addSeparator();
         break;
      case BtTreeItem::HOP:
         contextMenu->addAction(tr("New Hop"), editor, SLOT(newHop()));
         contextMenu->addSeparator();
         break;
      case BtTreeItem::MISC:
         contextMenu->addAction(tr("New Misc"), editor, SLOT(newMisc()));
         contextMenu->addSeparator();
         break;
      case BtTreeItem::STYLE:
         contextMenu->addAction(tr("New Style"), editor, SLOT(newStyle()));
         contextMenu->addSeparator();
         break;
      case BtTreeItem::YEAST:
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

QMenu* BtTreeView::getContextMenu(QModelIndex selected)
{
   if ( type(selected) == BtTreeItem::BREWNOTE )
      return subMenu;

   return contextMenu;
}

// Bad form likely

RecipeTreeView::RecipeTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::RECIPEMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::RECIPEMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findRecipe(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   // Resizing before you set the model doesn't do much.
   resizeColumnToContents(0);
}

EquipmentTreeView::EquipmentTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::EQUIPMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::EQUIPMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);

   setExpanded(findEquipment(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}

// Icky ick ikcy
FermentableTreeView::FermentableTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::FERMENTMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::FERMENTMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
  
   filter->dumpObjectInfo(); 
   setExpanded(findFermentable(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}

// More Ick
HopTreeView::HopTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::HOPMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::HOPMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findHop(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}

// Ick some more
MiscTreeView::MiscTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::MISCMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::MISCMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findMisc(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}

// Will this ick never end?
YeastTreeView::YeastTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::YEASTMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::YEASTMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findYeast(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}

// Nope. Apparently not, cause I keep adding more
StyleTreeView::StyleTreeView(QWidget *parent)
   : BtTreeView(parent)
{
   model = new BtTreeModel(this, BtTreeModel::STYLEMASK);
   filter = new BtTreeFilterProxyModel(this, BtTreeModel::STYLEMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findStyle(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}
