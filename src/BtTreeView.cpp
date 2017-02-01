/*
 * BtTreeView.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QDrag>
#include <QMenu>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QInputDialog>

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
#include "FermentableDialog.h"
#include "EquipmentEditor.h"
#include "HopDialog.h"
#include "MiscDialog.h"
#include "StyleEditor.h"
#include "YeastDialog.h"

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
   connect( _model, &BtTreeModel::expandFolder, this, &BtTreeView::expandFolder);
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

   BeerXMLElement* thing = _model->thing(filter->mapToSource(index));
   if ( thing )
      return _model->thing(filter->mapToSource(index))->folder();
   else 
      return "";
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
   else
      doubleClick = false;

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

void BtTreeView::newIngredient() {

   QString folder;
   QModelIndexList indexes = selectionModel()->selectedRows();
   // This is a little weird. There is an edge case where nothing is
   // selected and you click the big blue + button.
   if ( indexes.size() > 0 )
      folder = folderName(indexes.at(0));

   switch(_type) {
      case BtTreeModel::EQUIPMASK:
         qobject_cast<EquipmentEditor*>(_editor)->newEquipment(folder);
         break;
      case BtTreeModel::FERMENTMASK:
         qobject_cast<FermentableDialog*>(_editor)->newFermentable(folder);
         break;
      case BtTreeModel::HOPMASK:
         qobject_cast<HopDialog*>(_editor)->newHop(folder);
         break;
      case BtTreeModel::MISCMASK:
         qobject_cast<MiscDialog*>(_editor)->newMisc(folder);
         break;
      case BtTreeModel::STYLEMASK:
         qobject_cast<StyleEditor*>(_editor)->newStyle(folder);
         break;
      case BtTreeModel::YEASTMASK:
         qobject_cast<YeastDialog*>(_editor)->newYeast(folder);
         break;
      default:
         Brewtarget::logW(QString("BtTreeView::setupContextMenu unrecognized mask %1").arg(_type));
   }

}

void BtTreeView::setupContextMenu(QWidget* top, QWidget* editor)
{
   QMenu* _newMenu = new QMenu(this);
   QMenu* _exportMenu = new QMenu(this);

   _contextMenu = new QMenu(this);
   subMenu = new QMenu(this);

   _editor = editor;

   _newMenu->setTitle(tr("New"));
   _contextMenu->addMenu(_newMenu);
   _contextMenu->addSeparator();

   switch(_type) 
   {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      case BtTreeModel::RECIPEMASK:
         _newMenu->addAction(tr("Recipe"), editor, SLOT(newRecipe()));

         _contextMenu->addAction(tr("Brew It!"), top, SLOT(brewItHelper()));
         _contextMenu->addSeparator();

         subMenu->addAction(tr("Brew Again"), top, SLOT(brewAgainHelper()));
         subMenu->addAction(tr("Change date"), top, SLOT(changeBrewDate()));
         subMenu->addAction(tr("Recalculate eff"), top, SLOT(fixBrewNote()));
         subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

         break;
      case BtTreeModel::EQUIPMASK:
         _newMenu->addAction(tr("Equipment"), this, SLOT(newIngredient()));
         break;
      case BtTreeModel::FERMENTMASK:
         _newMenu->addAction(tr("Fermentable"), this, SLOT(newIngredient()));
         break;
      case BtTreeModel::HOPMASK:
         _newMenu->addAction(tr("Hop"), this, SLOT(newIngredient()));
         break;
      case BtTreeModel::MISCMASK:
         _newMenu->addAction(tr("Misc"), this, SLOT(newIngredient()));
         break;
      case BtTreeModel::STYLEMASK:
         _newMenu->addAction(tr("Style"), this, SLOT(newIngredient()));
         break;
      case BtTreeModel::YEASTMASK:
         _newMenu->addAction(tr("Yeast"), this, SLOT(newIngredient()));
         break;
      default:
         Brewtarget::logW(QString("BtTreeView::setupContextMenu unrecognized mask %1").arg(_type));
   }

   _newMenu->addAction(tr("Folder"), top, SLOT(newFolder()));
   // Copy
   _contextMenu->addAction(tr("Copy"), top, SLOT(copySelected()));
   // Delete
   _contextMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));
   // export and import
   _contextMenu->addSeparator();
   _exportMenu->setTitle(tr("Export"));
   _exportMenu->addAction(tr("To XML"), top, SLOT(exportSelected()));
   _exportMenu->addAction(tr("To HTML"), top, SLOT(exportSelectedHtml()));
   _contextMenu->addMenu(_exportMenu);
   _contextMenu->addAction(tr("Import"), top, SLOT(importFiles()));
   
}

QMenu* BtTreeView::contextMenu(QModelIndex selected)
{
   if ( type(selected) == BtTreeItem::BREWNOTE )
      return subMenu;

   return _contextMenu;
}

QString BtTreeView::verifyCopy(QString tag, QString name, bool *abort)
{
   QInputDialog askEm;

   // Gotta build this hard, so we can say "cancel all"
   askEm.setCancelButtonText( tr("Cancel All") );
   askEm.setWindowTitle( tr("Copy %1").arg(tag) );
   askEm.setLabelText(tr("Enter a unique name for the copy of %1.").arg(name));
   askEm.setToolTip(tr("An empty name will skip copying this %1.").arg(tag));

   if ( askEm.exec() == QDialog::Accepted )
   {
      if ( abort )
         *abort = false;

      name = askEm.textValue();
   }
   else 
   {
      if ( abort )
         *abort = true;
   }

   return name;
}

void BtTreeView::copySelected(QModelIndexList selected)
{
   QList< QPair<QModelIndex, QString> > names;
   QString newName;
   QModelIndexList translated;
   bool abort = false;

   // Time to lay down the boogie 
   foreach( QModelIndex at, selected )
   {
      // If somebody said cancel, bug out
      if ( abort == true )
         return;

      // First, we should translate from proxy to model, because I need this index a lot.
      QModelIndex trans = filter->mapToSource(at);

      // You can't delete the root element
      if ( trans == findElement(0) )
         continue;

      // Otherwise prompt
      switch(_model->type(trans))
      {
         case BtTreeItem::EQUIPMENT:
            newName = verifyCopy(tr("Equipment"),_model->name(trans), &abort);
            break;
         case BtTreeItem::FERMENTABLE:
            newName = verifyCopy(tr("Fermentable"),_model->name(trans), &abort);
            break;
         case BtTreeItem::HOP:
            newName = verifyCopy(tr("Hop"),_model->name(trans), &abort);
            break;
         case BtTreeItem::MISC:
            newName = verifyCopy(tr("Misc"),_model->name(trans), &abort);
            break;
         case BtTreeItem::RECIPE:
            newName = verifyCopy(tr("Recipe"),_model->name(trans), &abort);
            break;
         case BtTreeItem::STYLE:
            newName = verifyCopy(tr("Style"),_model->name(trans), &abort);
            break;
         case BtTreeItem::YEAST:
            newName = verifyCopy(tr("Yeast"),_model->name(trans), &abort);
            break;
         default:
            Brewtarget::logW( QString("BtTreeView::copySelected Unknown type: %1").arg(_model->type(trans)));
      }
      if ( !abort && !newName.isEmpty() )
         names.append(qMakePair(trans,newName));
   }
   // If we get here, call the model to do the copy
   _model->copySelected(names);
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
            Brewtarget::logW( QString("BtTreeView::deleteSelected Unknown type: %1").arg(_model->type(trans)));
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
