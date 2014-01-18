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
#include <QMessageBox>
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

BtTreeView::BtTreeView(QWidget *parent, BtTreeModel::TypeMasks type) :
   QTreeView(parent)
{
   // Set some global properties that all the kids will use.
   setAllColumnsShowFocus(true);
   setContextMenuPolicy(Qt::CustomContextMenu);
   setRootIsDecorated(false);

   setDragEnabled(true);
   setAcceptDrops(true);
   setDropIndicatorShown(true);
   setSelectionMode(QAbstractItemView::ExtendedSelection);

   _type = type;
   _model = new BtTreeModel(this, _type);
   filter = new BtTreeFilterProxyModel(this, _type);
   filter->setSourceModel(_model);
   setModel(filter);
   filter->setDynamicSortFilter(true);
   
   setExpanded(findElement(0), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);

   // and one wee connection
   connect( _model, SIGNAL(expandFolder(BtTreeModel::TypeMasks, QModelIndex)), this, SLOT(expandFolder(BtTreeModel::TypeMasks, QModelIndex)));
}

BtTreeModel* BtTreeView::model()
{
   return _model;
}

bool BtTreeView::removeRow(const QModelIndex &index)
{
   QModelIndex modelIndex = filter->mapToSource(index);
   QModelIndex parent = _model->parent(modelIndex);
   int position       = modelIndex.row();

   return _model->removeRows(position,1,parent);
}

bool BtTreeView::isParent(const QModelIndex& parent, const QModelIndex& child)
{
   QModelIndex modelParent = filter->mapToSource(parent);
   QModelIndex modelChild = filter->mapToSource(child);
   return modelParent == _model->parent(modelChild);
}

QModelIndex BtTreeView::parent(const QModelIndex& child)
{
   if ( ! child.isValid() )
      return QModelIndex();

   QModelIndex modelChild = filter->mapToSource(child);
   if ( modelChild.isValid())
      return filter->mapFromSource(_model->parent(modelChild));

   return QModelIndex();
}

QModelIndex BtTreeView::first()
{
   return filter->mapFromSource(_model->first());
}

Recipe* BtTreeView::recipe(const QModelIndex &index) const
{
   return _model->recipe(filter->mapToSource(index));
}

QString BtTreeView::folderName(QModelIndex index)
{
   if ( _model->type(filter->mapToSource(index)) == BtTreeItem::FOLDER)
      return _model->folder(filter->mapToSource(index))->fullPath();

   return _model->thing(filter->mapToSource(index))->folder();
}

QModelIndex BtTreeView::findElement(BeerXMLElement* thing)
{
   return filter->mapFromSource(_model->findElement(thing));
}

Equipment* BtTreeView::equipment(const QModelIndex &index) const
{
   return _model->equipment(filter->mapToSource(index));
}

Fermentable* BtTreeView::fermentable(const QModelIndex &index) const
{
   return _model->fermentable(filter->mapToSource(index));
}

Hop* BtTreeView::hop(const QModelIndex &index) const
{
   return _model->hop(filter->mapToSource(index));
}

Misc* BtTreeView::misc(const QModelIndex &index) const
{
   return _model->misc(filter->mapToSource(index));
}

Yeast* BtTreeView::yeast(const QModelIndex &index) const
{
   return _model->yeast(filter->mapToSource(index));
}

Style* BtTreeView::style(const QModelIndex &index) const
{
   return _model->style(filter->mapToSource(index));
}

BrewNote* BtTreeView::brewNote(const QModelIndex &index) const
{
   if ( ! index.isValid() ) 
      return NULL;

   return _model->brewNote(filter->mapToSource(index));
}

BtFolder* BtTreeView::folder(const QModelIndex &index) const
{
   if ( ! index.isValid() ) 
      return NULL;

   return _model->folder(filter->mapToSource(index));
}

QModelIndex BtTreeView::findFolder(BtFolder* folder)
{
   return filter->mapFromSource(_model->findFolder(folder->fullPath(), NULL, false));
}

void BtTreeView::addFolder(QString folder)
{
   _model->addFolder(folder);
}

void BtTreeView::renameFolder(BtFolder* victim, QString newName)
{
   _model->renameFolder(victim,newName);
}

int BtTreeView::type(const QModelIndex &index)
{
   return _model->type(filter->mapToSource(index));
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

QMimeData* BtTreeView::mimeData(QModelIndexList indexes) 
{
   QMimeData *mimeData = new QMimeData();
   QByteArray encodedData;
   QString name = "";
   int _type, id, itsa;

   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   // From what I've been able to tell, the drop events are homogenous -- a
   // single drop event will be all equipment or all recipe or ...
   itsa = -1;
   foreach (QModelIndex index, indexes)
   {

      if (! index.isValid())
         continue;

      _type = type(index);
      if ( _type != BtTreeItem::FOLDER ) 
      {
         id   = _model->thing(filter->mapToSource(index))->key();
         name = _model->name(filter->mapToSource(index));
         // Save this for later reference
         if ( itsa == -1 )
            itsa = _type;
      }
      else 
      {
         id = -1;
         name = _model->folder(filter->mapToSource(index))->fullPath();
      }
      stream << _type << id << name;
   }

   // Recipes, equipment and styles get dropped on the recipe pane
   if ( itsa == BtTreeItem::RECIPE || itsa == BtTreeItem::STYLE || itsa == BtTreeItem::EQUIPMENT ) 
      name = "application/x-brewtarget-recipe";
   // Everything other than folders get dropped on the ingredients pane
   else if ( itsa != -1 )
      name = "application/x-brewtarget-ingredient";
   // folders will be handled by themselves.
   else
      name = "application/x-brewtarget-folder";

   mimeData->setData(name,encodedData);
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
      if (_model->isRecipe(selectModel))
         hasRecipe = true;
      else
         hasSomethingElse = true;
   }

   return hasRecipe && hasSomethingElse;
}

void BtTreeView::setupContextMenu(QWidget* top, QWidget* editor, QMenu *sMenu, QMenu *fMenu, int type)
{

   _contextMenu = new QMenu(this);
   subMenu = new QMenu(this);

   switch(type) 
   {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      case BtTreeItem::RECIPE:
         _contextMenu->addAction(tr("New Recipe"), editor, SLOT(newRecipe()));
         _contextMenu->addAction(tr("Brew It!"), top, SLOT(newBrewNote()));
         _contextMenu->addSeparator();

         subMenu->addAction(tr("Brew Again"), top, SLOT(reBrewNote()));
         subMenu->addAction(tr("Change date"), top, SLOT(changeBrewDate()));
         subMenu->addAction(tr("Recalculate eff"), top, SLOT(fixBrewNote()));
         subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

         break;
      case BtTreeItem::EQUIPMENT:
         _contextMenu->addAction(tr("New Equipment"), editor, SLOT(newEquipment()));
         _contextMenu->addSeparator();
         break;
      case BtTreeItem::FERMENTABLE:
         _contextMenu->addAction(tr("New Fermentable"), editor, SLOT(newFermentable()));
         _contextMenu->addSeparator();
         break;
      case BtTreeItem::HOP:
         _contextMenu->addAction(tr("New Hop"), editor, SLOT(newHop()));
         _contextMenu->addSeparator();
         break;
      case BtTreeItem::MISC:
         _contextMenu->addAction(tr("New Misc"), editor, SLOT(newMisc()));
         _contextMenu->addSeparator();
         break;
      case BtTreeItem::STYLE:
         _contextMenu->addAction(tr("New Style"), editor, SLOT(newStyle()));
         _contextMenu->addSeparator();
         break;
      case BtTreeItem::YEAST:
         _contextMenu->addAction(tr("New Yeast"), editor, SLOT(newYeast()));
         _contextMenu->addSeparator();
         break;
   }

   // submenus for new and folders
   _contextMenu->addMenu(sMenu);
   _contextMenu->addMenu(fMenu);
   _contextMenu->addSeparator();

   // Copy
   _contextMenu->addAction(tr("Copy"), top, SLOT(copySelected()));
   // Delete
   _contextMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));
   // export and import
   _contextMenu->addSeparator();
   _contextMenu->addAction(tr("Export"), top, SLOT(exportSelected()));
   _contextMenu->addAction(tr("Import"), top, SLOT(importFiles()));
   
}

QMenu* BtTreeView::contextMenu(QModelIndex selected)
{
   if ( type(selected) == BtTreeItem::BREWNOTE )
      return subMenu;

   return _contextMenu;
}

int BtTreeView::verifyDelete(int confirmDelete, QString tag, QString name)
{
   if ( confirmDelete == QMessageBox::YesToAll )
      return confirmDelete;

   return QMessageBox::question(this, tr("Delete %1").arg(tag), tr("Delete %1 %2?").arg(tag).arg(name),
                                  QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel,
                                  QMessageBox::No);

}

// I should maybe shove this further down the stack. But I prefer to keep the
// confirmation windows at least this high -- models shouldn't be interacting
// with users.
void BtTreeView::deleteSelected(QModelIndexList selected)
{
   QString prompt;
   QModelIndexList translated;

   int confirmDelete = QMessageBox::NoButton;
  
   // Time to lay down the boogie 
   foreach( QModelIndex at, selected )
   {
      // If somebody said cancel, bug out
      if ( confirmDelete == QMessageBox::Cancel )
         return;

      // First, we should translate from proxy to model, because I need this index a lot.
      QModelIndex trans = filter->mapToSource(at);

      // You can't delete the root element
      if ( trans == findElement(0) )
         continue;

      // If we have alread said "Yes To All", just append and go
      if ( confirmDelete == QMessageBox::YesToAll )
      {
         translated.append(trans);
         continue;
      }
      
      // Otherwise prompt
      switch(_model->type(trans))
      {
         case BtTreeItem::RECIPE:
            confirmDelete = verifyDelete(confirmDelete,tr("Recipe"),_model->name(trans));
            break;
         case BtTreeItem::EQUIPMENT:
            confirmDelete = verifyDelete(confirmDelete,tr("Equipment"),_model->name(trans));
            break;
         case BtTreeItem::FERMENTABLE:
            confirmDelete = verifyDelete(confirmDelete,tr("Fermentable"),_model->name(trans));
            break;
         case BtTreeItem::HOP:
            confirmDelete = verifyDelete(confirmDelete,tr("Hop"),_model->name(trans));
            break;
         case BtTreeItem::MISC:
            confirmDelete = verifyDelete(confirmDelete,tr("Misc"),_model->name(trans));
            break;
         case BtTreeItem::STYLE:
            confirmDelete = verifyDelete(confirmDelete,tr("Style"),_model->name(trans));
            break;
         case BtTreeItem::YEAST:
            confirmDelete = verifyDelete(confirmDelete,tr("Yeast"),_model->name(trans));
            break;
         case BtTreeItem::BREWNOTE:
            confirmDelete = verifyDelete(confirmDelete,tr("BrewNote"),_model->brewNote(trans)->brewDate_short());
            break;
         case BtTreeItem::FOLDER:
            confirmDelete = verifyDelete(confirmDelete,tr("Folder"),_model->name(trans));
            break;
         default:
            Brewtarget::log(Brewtarget::WARNING, QString("MainWindow::deleteSelected Unknown type: %1").arg(_model->type(trans)));
      }
      // If they selected "Yes" or "Yes To All", push and loop
      if ( confirmDelete == QMessageBox::Yes || confirmDelete == QMessageBox::YesToAll )
         translated.append(trans);
   }
   // If we get here, call the model to delete the victims
   _model->deleteSelected(translated);
}

void BtTreeView::expandFolder(BtTreeModel::TypeMasks kindaThing, QModelIndex fIdx)
{
   // FUN! I get to map from source this time.
   // I don't have to check if this is a folder (I think?)
   if ( kindaThing & _type && fIdx.isValid() && ! isExpanded(filter->mapFromSource(fIdx) ))
      setExpanded(filter->mapFromSource(fIdx),true);
}
// Bad form likely

RecipeTreeView::RecipeTreeView(QWidget *parent)
   : BtTreeView(parent, BtTreeModel::RECIPEMASK)
{
}

EquipmentTreeView::EquipmentTreeView(QWidget *parent)
   : BtTreeView(parent, BtTreeModel::EQUIPMASK)
{
}

// Icky ick ikcy
FermentableTreeView::FermentableTreeView(QWidget *parent)
   : BtTreeView(parent,BtTreeModel::FERMENTMASK)
{
}

// More Ick
HopTreeView::HopTreeView(QWidget *parent)
   : BtTreeView(parent, BtTreeModel::HOPMASK)
{
}

// Ick some more
MiscTreeView::MiscTreeView(QWidget *parent)
   : BtTreeView(parent,BtTreeModel::MISCMASK)
{
}

// Will this ick never end?
YeastTreeView::YeastTreeView(QWidget *parent)
   : BtTreeView(parent,BtTreeModel::YEASTMASK)
{
}

// Nope. Apparently not, cause I keep adding more
StyleTreeView::StyleTreeView(QWidget *parent)
   : BtTreeView(parent,BtTreeModel::STYLEMASK)
{
}
