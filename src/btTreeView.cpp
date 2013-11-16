/*
 * btTreeView.cpp is part of Brewtarget and was written by Mik
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
#include "btTreeView.h"
#include "btTreeModel.h"
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

btTreeView::btTreeView(QWidget *parent) :
   QTreeView(parent)
{
   // Set some global properties that all the kids will use.
   setAllColumnsShowFocus(true);
   setContextMenuPolicy(Qt::CustomContextMenu);
   setRootIsDecorated(false);
   setSelectionMode(QAbstractItemView::ExtendedSelection);

}

btTreeModel* btTreeView::getModel()
{
   return model;
}

bool btTreeView::removeRow(const QModelIndex &index)
{
   QModelIndex modelIndex = filter->mapToSource(index);
   QModelIndex parent = model->parent(modelIndex);
   int position       = modelIndex.row();

   return model->removeRows(position,1,parent);
}

bool btTreeView::isParent(const QModelIndex& parent, const QModelIndex& child)
{
   QModelIndex modelParent = filter->mapToSource(parent);
   QModelIndex modelChild = filter->mapToSource(child);
   return modelParent == model->parent(modelChild);
}

QModelIndex btTreeView::getParent(const QModelIndex& child)
{
   if ( ! child.isValid() )
      return QModelIndex();

   QModelIndex modelChild = filter->mapToSource(child);
   if ( modelChild.isValid())
      return filter->mapFromSource(model->parent(modelChild));

   return QModelIndex();
}

QModelIndex btTreeView::getFirst()
{
   return filter->mapFromSource(model->getFirst());
}

Recipe* btTreeView::getRecipe(const QModelIndex &index) const
{
   return model->getRecipe(filter->mapToSource(index));
}

QModelIndex btTreeView::findRecipe(Recipe* rec)
{
   return filter->mapFromSource(model->findRecipe(rec));
}

Equipment* btTreeView::getEquipment(const QModelIndex &index) const
{
   return model->getEquipment(filter->mapToSource(index));
}

QModelIndex btTreeView::findEquipment(Equipment* kit)
{
   return filter->mapFromSource(model->findEquipment(kit));
}

Fermentable* btTreeView::getFermentable(const QModelIndex &index) const
{
   return model->getFermentable(filter->mapToSource(index));
}

QModelIndex btTreeView::findFermentable(Fermentable* ferm)
{
   return filter->mapFromSource(model->findFermentable(ferm));
}

Hop* btTreeView::getHop(const QModelIndex &index) const
{
   return model->getHop(filter->mapToSource(index));
}

QModelIndex btTreeView::findHop(Hop* hop)
{
   return filter->mapFromSource(model->findHop(hop));
}

Misc* btTreeView::getMisc(const QModelIndex &index) const
{
   return model->getMisc(filter->mapToSource(index));
}

QModelIndex btTreeView::findMisc(Misc* misc)
{
   return filter->mapFromSource(model->findMisc(misc));
}

Yeast* btTreeView::getYeast(const QModelIndex &index) const
{
   return model->getYeast(filter->mapToSource(index));
}

QModelIndex btTreeView::findYeast(Yeast* yeast)
{
   return filter->mapFromSource(model->findYeast(yeast));
}

Style* btTreeView::getStyle(const QModelIndex &index) const
{
   return model->getStyle(filter->mapToSource(index));
}

QModelIndex btTreeView::findStyle(Style* style)
{
   return filter->mapFromSource(model->findStyle(style));
}

BrewNote* btTreeView::getBrewNote(const QModelIndex &index) const
{
   if ( ! index.isValid() ) 
      return NULL;

   return model->getBrewNote(filter->mapToSource(index));
}

QModelIndex btTreeView::findBrewNote(BrewNote* bNote)
{
   return filter->mapFromSource(model->findBrewNote(bNote));
}

int btTreeView::type(const QModelIndex &index)
{
   return model->type(filter->mapToSource(index));
}

void btTreeView::mousePressEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton)
   {
      dragStart = event->pos();
      doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mousePressEvent(event);
}

void btTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton)
      doubleClick = true;

   // Send the event on its way up to the parent
   QTreeView::mouseDoubleClickEvent(event);
}

void btTreeView::mouseMoveEvent(QMouseEvent *event)
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

void btTreeView::keyPressEvent(QKeyEvent *event)
{
   switch( event->key() )
   {
      case Qt::Key_Space:
      case Qt::Key_Select:
      case Qt::Key_Enter:
      case Qt::Key_Return:
         emit btTreeView::doubleClicked(selectedIndexes().first());
         return;
   }
   QTreeView::keyPressEvent(event);
}

QMimeData *btTreeView::mimeData(QModelIndexList indexes) 
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
            case btTreeItem::EQUIPMENT:
               equipment = model->getEquipment(index);
               if (equipment)
               {
                  name = equipment->name();
               }
               break;
            case btTreeItem::FERMENTABLE:
               fermentable = model->getFermentable(index);
               if (fermentable)
               {
                  name = fermentable->name();
               }
               break;
            case btTreeItem::HOP:
               hop = model->getHop(index);
               if (hop)
               {
                  name = hop->name();
               }
               break;
            case btTreeItem::MISC:
               misc = model->getMisc(index);
               if (misc)
               {
                  name = misc->name();
               }
               break;
            case btTreeItem::YEAST:
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

bool btTreeView::multiSelected()
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

void btTreeView::setupContextMenu(QWidget* top, QWidget* editor, QMenu *sMenu,int type)
{

   contextMenu = new QMenu(this);
   subMenu = new QMenu(this);

   switch(type) 
   {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      case btTreeItem::RECIPE:
         contextMenu->addAction(tr("New Recipe"), editor, SLOT(newRecipe()));
         contextMenu->addAction(tr("Brew It!"), top, SLOT(newBrewNote()));
         contextMenu->addSeparator();

         subMenu->addAction(tr("Brew Again"), top, SLOT(reBrewNote()));
         subMenu->addAction(tr("Change date"), top, SLOT(changeBrewDate()));
         subMenu->addAction(tr("Recalculate eff"), top, SLOT(fixBrewNote()));
         subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

         break;
      case btTreeItem::EQUIPMENT:
         contextMenu->addAction(tr("New Equipment"), editor, SLOT(newEquipment()));
         contextMenu->addSeparator();
         break;
      case btTreeItem::FERMENTABLE:
         contextMenu->addAction(tr("New Fermentable"), editor, SLOT(newFermentable()));
         contextMenu->addSeparator();
         break;
      case btTreeItem::HOP:
         contextMenu->addAction(tr("New Hop"), editor, SLOT(newHop()));
         contextMenu->addSeparator();
         break;
      case btTreeItem::MISC:
         contextMenu->addAction(tr("New Misc"), editor, SLOT(newMisc()));
         contextMenu->addSeparator();
         break;
      case btTreeItem::STYLE:
         contextMenu->addAction(tr("New Style"), editor, SLOT(newStyle()));
         contextMenu->addSeparator();
         break;
      case btTreeItem::YEAST:
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

QMenu* btTreeView::getContextMenu(QModelIndex selected)
{
   if ( type(selected) == btTreeItem::BREWNOTE )
      return subMenu;

   return contextMenu;
}

// Bad form likely

RecipeTreeView::RecipeTreeView(QWidget *parent)
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::RECIPEMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::RECIPEMASK);
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
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::EQUIPMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::EQUIPMASK);
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
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::FERMENTMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::FERMENTMASK);
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
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::HOPMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::HOPMASK);
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
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::MISCMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::MISCMASK);
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
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::YEASTMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::YEASTMASK);
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
   : btTreeView(parent)
{
   model = new btTreeModel(this, btTreeModel::STYLEMASK);
   filter = new BtTreeFilterProxyModel(this, btTreeModel::STYLEMASK);
   filter->setSourceModel(model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findStyle(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);
}
