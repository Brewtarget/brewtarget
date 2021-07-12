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
#include "model/Recipe.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Yeast.h"
#include "model/BrewNote.h"
#include "model/Style.h"
#include "model/Water.h"
#include "FermentableDialog.h"
#include "EquipmentEditor.h"
#include "HopDialog.h"
#include "MiscDialog.h"
#include "StyleEditor.h"
#include "YeastDialog.h"
#include "WaterEditor.h"

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

   m_type = type;
   m_model = new BtTreeModel(this, m_type);
   m_filter = new BtTreeFilterProxyModel(this, m_type);
   m_filter->setSourceModel(m_model);
   setModel(m_filter);
   m_filter->setDynamicSortFilter(true);

   setExpanded(findElement(nullptr), true);
   setSortingEnabled(true);
   sortByColumn(0,Qt::AscendingOrder);
   resizeColumnToContents(0);

   // and one wee connection
   connect( m_model, &BtTreeModel::expandFolder, this, &BtTreeView::expandFolder);
}

BtTreeModel* BtTreeView::model()
{
   return m_model;
}

BtTreeFilterProxyModel* BtTreeView::filter() { return m_filter; }

bool BtTreeView::removeRow(const QModelIndex &index)
{
   QModelIndex modelIndex = m_filter->mapToSource(index);
   QModelIndex parent = m_model->parent(modelIndex);
   int position       = modelIndex.row();

   return m_model->removeRows(position,1,parent);
}

bool BtTreeView::isParent(const QModelIndex& parent, const QModelIndex& child)
{
   QModelIndex modelParent = m_filter->mapToSource(parent);
   QModelIndex modelChild = m_filter->mapToSource(child);
   return modelParent == m_model->parent(modelChild);
}

QModelIndex BtTreeView::parent(const QModelIndex& child)
{
   if ( ! child.isValid() )
      return QModelIndex();

   QModelIndex modelChild = m_filter->mapToSource(child);
   if ( modelChild.isValid())
      return m_filter->mapFromSource(m_model->parent(modelChild));

   return QModelIndex();
}

QModelIndex BtTreeView::first()
{
   return m_filter->mapFromSource(m_model->first());
}

Recipe* BtTreeView::recipe(const QModelIndex &index) const
{
   return m_model->recipe(m_filter->mapToSource(index));
}

QString BtTreeView::folderName(QModelIndex index)
{
   if ( m_model->type(m_filter->mapToSource(index)) == BtTreeItem::FOLDER)
      return m_model->folder(m_filter->mapToSource(index))->fullPath();

   NamedEntity* thing = m_model->thing(m_filter->mapToSource(index));
   if ( thing )
      return m_model->thing(m_filter->mapToSource(index))->folder();
   else
      return "";
}

QModelIndex BtTreeView::findElement(NamedEntity* thing)
{
   return m_filter->mapFromSource(m_model->findElement(thing));
}

Equipment* BtTreeView::equipment(const QModelIndex &index) const
{
   return m_model->equipment(m_filter->mapToSource(index));
}

Fermentable* BtTreeView::fermentable(const QModelIndex &index) const
{
   return m_model->fermentable(m_filter->mapToSource(index));
}

Hop* BtTreeView::hop(const QModelIndex &index) const
{
   return m_model->hop(m_filter->mapToSource(index));
}

Misc* BtTreeView::misc(const QModelIndex &index) const
{
   return m_model->misc(m_filter->mapToSource(index));
}

Yeast* BtTreeView::yeast(const QModelIndex &index) const
{
   return m_model->yeast(m_filter->mapToSource(index));
}

Style* BtTreeView::style(const QModelIndex &index) const
{
   return m_model->style(m_filter->mapToSource(index));
}

Water* BtTreeView::water(const QModelIndex &index) const
{
   return m_model->water(m_filter->mapToSource(index));
}

BrewNote* BtTreeView::brewNote(const QModelIndex &index) const
{
   if ( ! index.isValid() )
      return nullptr;

   return m_model->brewNote(m_filter->mapToSource(index));
}

BtFolder* BtTreeView::folder(const QModelIndex &index) const
{
   if ( ! index.isValid() )
      return nullptr;

   return m_model->folder(m_filter->mapToSource(index));
}

QModelIndex BtTreeView::findFolder(BtFolder* folder)
{
   return m_filter->mapFromSource(m_model->findFolder(folder->fullPath(), nullptr, false));
}

void BtTreeView::addFolder(QString folder)
{
   m_model->addFolder(folder);
}

void BtTreeView::renameFolder(BtFolder* victim, QString newName)
{
   m_model->renameFolder(victim,newName);
}

int BtTreeView::type(const QModelIndex &index)
{
   return m_model->type(m_filter->mapToSource(index));
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
   drag->exec(Qt::CopyAction);
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
         if ( m_model->thing( m_filter->mapToSource(index)) == nullptr ) {
            qWarning() << QString("Couldn't map that thing");
            id = -1;
         }
         else {
            id   = m_model->thing(m_filter->mapToSource(index))->key();
            name = m_model->name(m_filter->mapToSource(index));
            // Save this for later reference
            if ( itsa == -1 )
               itsa = _type;
         }
      }
      else
      {
         id = -1;
         name = m_model->folder(m_filter->mapToSource(index))->fullPath();
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
      QModelIndex selectModel = m_filter->mapToSource(selection);
      if (m_model->isRecipe(selectModel))
         hasRecipe = true;
      else
         hasSomethingElse = true;
   }

   return hasRecipe && hasSomethingElse;
}

void BtTreeView::newNamedEntity() {

   QString folder;
   QModelIndexList indexes = selectionModel()->selectedRows();
   // This is a little weird. There is an edge case where nothing is
   // selected and you click the big blue + button.
   if ( indexes.size() > 0 )
      folder = folderName(indexes.at(0));

   switch(m_type) {
      case BtTreeModel::EQUIPMASK:
         qobject_cast<EquipmentEditor*>(m_editor)->newEquipment(folder);
         break;
      case BtTreeModel::FERMENTMASK:
         qobject_cast<FermentableDialog*>(m_editor)->newFermentable(folder);
         break;
      case BtTreeModel::HOPMASK:
         qobject_cast<HopDialog*>(m_editor)->newHop(folder);
         break;
      case BtTreeModel::MISCMASK:
         qobject_cast<MiscDialog*>(m_editor)->newMisc(folder);
         break;
      case BtTreeModel::STYLEMASK:
         qobject_cast<StyleEditor*>(m_editor)->newStyle(folder);
         break;
      case BtTreeModel::YEASTMASK:
         qobject_cast<YeastDialog*>(m_editor)->newYeast(folder);
         break;
      case BtTreeModel::WATERMASK:
         qobject_cast<WaterEditor*>(m_editor)->newWater(folder);
         break;
      default:
         qWarning() << QString("BtTreeView::setupContextMenu unrecognized mask %1").arg(m_type);
   }

}

void BtTreeView::showAncestors()
{
   if ( m_type == BtTreeModel::RECIPEMASK ) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach( QModelIndex selected, ndxs ) {
         m_model->showAncestors(m_filter->mapToSource(selected));
      }
   }
}

void BtTreeView::hideAncestors()
{
   if ( m_type == BtTreeModel::RECIPEMASK ) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach( QModelIndex selected, ndxs ) {
         // make sure we add the ancestors to the exclusion list
         m_model->hideAncestors(m_filter->mapToSource(selected));
      }
   }
}

void BtTreeView::orphanRecipe()
{
   if ( m_type == BtTreeModel::RECIPEMASK ) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach( QModelIndex selected, ndxs ) {
         // make sure we add the ancestors to the exclusion list
         m_model->orphanRecipe(m_filter->mapToSource(selected));
      }
   }
}

void BtTreeView::spawnRecipe()
{
   if ( m_type == BtTreeModel::RECIPEMASK ) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      foreach( QModelIndex selected, ndxs ) {
         // make sure we add the ancestors to the exclusion list
         m_model->spawnRecipe(m_filter->mapToSource(selected));
      }
   }
}

bool BtTreeView::ancestorsAreShowing(QModelIndex ndx)
{
   if ( m_type == BtTreeModel::RECIPEMASK ) {
      QModelIndex translated = m_filter->mapToSource(ndx);
      return m_model->showChild(translated);
   }

   return false;
}
void BtTreeView::enableDelete(bool enable)       { m_deleteAction->setEnabled(enable); }
void BtTreeView::enableShowAncestor(bool enable) { m_showAncestorAction->setEnabled(enable); }
void BtTreeView::enableHideAncestor(bool enable) { m_hideAncestorAction->setEnabled(enable); }
void BtTreeView::enableOrphan(bool enable)       { m_orphanAction->setEnabled(enable); }
void BtTreeView::enableSpawn(bool enable)        { m_spawnAction->setEnabled(enable); }

void BtTreeView::setupContextMenu(QWidget* top, QWidget* editor)
{
   QMenu* m_newMenu = new QMenu(this);

   m_exportMenu = new QMenu(this);
   m_contextMenu = new QMenu(this);
   subMenu = new QMenu(this);
   m_versionMenu = new QMenu(this);

   m_editor = editor;

   m_newMenu->setTitle(tr("New"));
   m_contextMenu->addMenu(m_newMenu);

   switch(m_type)
   {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      case BtTreeModel::RECIPEMASK:
         m_newMenu->addAction(tr("Recipe"), editor, SLOT(newRecipe()));

         // version menu
         m_versionMenu->setTitle("Snapshots");
         m_showAncestorAction = m_versionMenu->addAction( tr("Show Snapshots"), this, SLOT(showAncestors()));
         m_hideAncestorAction = m_versionMenu->addAction( tr("Hide Snapshots"), this, SLOT(hideAncestors()));
         m_orphanAction = m_versionMenu->addAction( tr("Detach Recipe"), this, SLOT(orphanRecipe()));
         m_spawnAction  = m_versionMenu->addAction( tr("Snapshot Recipe"), this, SLOT(spawnRecipe()));
         m_contextMenu->addMenu(m_versionMenu);

         m_contextMenu->addSeparator();
         m_brewItAction = m_contextMenu->addAction(tr("Brew It!"), top, SLOT(brewItHelper()));
         m_contextMenu->addSeparator();

         subMenu->addAction(tr("Brew Again"), top, SLOT(brewAgainHelper()));
         subMenu->addAction(tr("Change date"), top, SLOT(changeBrewDate()));
         subMenu->addAction(tr("Recalculate eff"), top, SLOT(fixBrewNote()));
         subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

         break;
      case BtTreeModel::EQUIPMASK:
         m_newMenu->addAction(tr("Equipment"), this, SLOT(newNamedEntity()));
         break;
      case BtTreeModel::FERMENTMASK:
         m_newMenu->addAction(tr("Fermentable"), this, SLOT(newNamedEntity()));
         break;
      case BtTreeModel::HOPMASK:
         m_newMenu->addAction(tr("Hop"), this, SLOT(newNamedEntity()));
         break;
      case BtTreeModel::MISCMASK:
         m_newMenu->addAction(tr("Misc"), this, SLOT(newNamedEntity()));
         break;
      case BtTreeModel::STYLEMASK:
         m_newMenu->addAction(tr("Style"), this, SLOT(newNamedEntity()));
         break;
      case BtTreeModel::YEASTMASK:
         m_newMenu->addAction(tr("Yeast"), this, SLOT(newNamedEntity()));
         break;
      case BtTreeModel::WATERMASK:
         m_newMenu->addAction(tr("Water"), this, SLOT(newNamedEntity()));
         break;
      default:
         qWarning() << QString("BtTreeView::setupContextMenu unrecognized mask %1").arg(m_type);
   }

   m_contextMenu->addSeparator();
   m_newMenu->addAction(tr("Folder"), top, SLOT(newFolder()));
   // Copy
   m_copyAction = m_contextMenu->addAction(tr("Copy"), top, SLOT(copySelected()));
   // m_deleteAction makes it easier to find this later to disable it
   m_deleteAction = m_contextMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));
   // export and import
   m_contextMenu->addSeparator();
   m_exportMenu->setTitle(tr("Export"));
   m_exportMenu->addAction(tr("To XML"), top, SLOT(exportSelected()));
   m_exportMenu->addAction(tr("To HTML"), top, SLOT(exportSelectedHtml()));
   m_contextMenu->addMenu(m_exportMenu);
   m_contextMenu->addAction(tr("Import"), top, SLOT(importFiles()));

}

QMenu* BtTreeView::contextMenu(QModelIndex selected)
{
   bool disableDelete = false;

   BtTreeItem::ITEMTYPE t_type = static_cast<BtTreeItem::ITEMTYPE>(type(selected));
   if ( t_type == BtTreeItem::BREWNOTE )
      return subMenu;

   if ( t_type == BtTreeItem::RECIPE ) {
      Recipe *rec = recipe(selected);
      QModelIndex translated = m_filter->mapToSource(selected);
      // OK! Recipe/folder was found! yey let's display a context menu.
      // This happens when a Recipe or folder was clicked in the treeview. i.e. not the Top-level 'Recipies' item!
      if ( rec != nullptr ) {
         // you can not delete a locked recipe
         enableDelete( ! rec->locked() );

         // if we have ancestors and are showing them but are not an actual
         // ancestor, then enable hide
         enableHideAncestor(rec->hasAncestors() && m_model->showChild(translated) && rec->display());

         // if we have ancestors and are not showing them, enable showAncestors
         enableShowAncestor(rec->hasAncestors() && ! m_model->showChild(translated));

         // if we have ancestors and are not locked, then we are a leaf node and
         // allow orphaning
         enableOrphan(rec->hasAncestors() && ! rec->locked() );

         // if display is true, we can spawn it. This should mean we cannot spawn
         // ancestors directly, which is what I want.
         enableSpawn( rec->display() );

         // If user has clickec the Top-level Item 'Recipies' Once this menu Item will be forever disabled if we don't enable it.
         m_exportMenu->setEnabled( true );
         m_copyAction->setEnabled( true );
         m_brewItAction->setEnabled( true );
      }
      // This case will happen if user Right-click the top most item in the list, as that will yield a rec == nullptr.
      // In this case we will treat it like a folder and disable a bunch of options.
      else {
         t_type = BtTreeItem::FOLDER;
         disableDelete = true;
      }
   }
   if ( t_type == BtTreeItem::FOLDER ) {
      enableDelete( ! disableDelete );
      enableHideAncestor( false );
      enableShowAncestor( false );
      enableOrphan( false );
      enableSpawn( false );
      m_exportMenu->setEnabled( false );
      m_copyAction->setEnabled( false );
      m_brewItAction->setEnabled( false );
   }

   return m_contextMenu;
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
      QModelIndex trans = m_filter->mapToSource(at);

      // You can't delete the root element
      if ( trans == findElement(nullptr) )
         continue;

      // Otherwise prompt
      switch(m_model->type(trans))
      {
         case BtTreeItem::EQUIPMENT:
            newName = verifyCopy(tr("Equipment"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::FERMENTABLE:
            newName = verifyCopy(tr("Fermentable"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::HOP:
            newName = verifyCopy(tr("Hop"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::MISC:
            newName = verifyCopy(tr("Misc"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::RECIPE:
            newName = verifyCopy(tr("Recipe"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::STYLE:
            newName = verifyCopy(tr("Style"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::YEAST:
            newName = verifyCopy(tr("Yeast"),m_model->name(trans), &abort);
            break;
         case BtTreeItem::WATER:
            newName = verifyCopy(tr("Water"),m_model->name(trans), &abort);
            break;
         default:
            qWarning() << QString("BtTreeView::copySelected Unknown type: %1").arg(m_model->type(trans));
      }
      if ( !abort && !newName.isEmpty() )
         names.append(qMakePair(trans,newName));
   }
   // If we get here, call the model to do the copy
   m_model->copySelected(names);
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
      QModelIndex trans = m_filter->mapToSource(at);

      // You can't delete the root element
      if ( trans == findElement(nullptr) )
         continue;

      // If we have alread said "Yes To All", just append and go
      if ( confirmDelete == QMessageBox::YesToAll )
      {
         translated.append(trans);
         continue;
      }

      // Otherwise prompt
      switch(m_model->type(trans))
      {
         case BtTreeItem::RECIPE:
            confirmDelete = verifyDelete(confirmDelete,tr("Recipe"),m_model->name(trans));
            break;
         case BtTreeItem::EQUIPMENT:
            confirmDelete = verifyDelete(confirmDelete,tr("Equipment"),m_model->name(trans));
            break;
         case BtTreeItem::FERMENTABLE:
            confirmDelete = verifyDelete(confirmDelete,tr("Fermentable"),m_model->name(trans));
            break;
         case BtTreeItem::HOP:
            confirmDelete = verifyDelete(confirmDelete,tr("Hop"),m_model->name(trans));
            break;
         case BtTreeItem::MISC:
            confirmDelete = verifyDelete(confirmDelete,tr("Misc"),m_model->name(trans));
            break;
         case BtTreeItem::STYLE:
            confirmDelete = verifyDelete(confirmDelete,tr("Style"),m_model->name(trans));
            break;
         case BtTreeItem::YEAST:
            confirmDelete = verifyDelete(confirmDelete,tr("Yeast"),m_model->name(trans));
            break;
         case BtTreeItem::BREWNOTE:
            confirmDelete = verifyDelete(confirmDelete,tr("BrewNote"),m_model->brewNote(trans)->brewDate_short());
            break;
         case BtTreeItem::FOLDER:
            confirmDelete = verifyDelete(confirmDelete,tr("Folder"),m_model->folder(trans)->fullPath());
            break;
         case BtTreeItem::WATER:
            confirmDelete = verifyDelete(confirmDelete,tr("Water"),m_model->name(trans));
            break;
         default:
            qWarning() << QString("BtTreeView::deleteSelected Unknown type: %1").arg(m_model->type(trans));
      }
      // If they selected "Yes" or "Yes To All", push and loop
      if ( confirmDelete == QMessageBox::Yes || confirmDelete == QMessageBox::YesToAll )
         translated.append(trans);
   }
   // If we get here, call the model to delete the victims
   m_model->deleteSelected(translated);
}

void BtTreeView::setFilter(BtTreeFilterProxyModel *newFilter)
{
   m_filter = newFilter;
}

BtTreeFilterProxyModel* BtTreeView::filter() const
{
   return m_filter;
}

void BtTreeView::expandFolder(BtTreeModel::TypeMasks kindaThing, QModelIndex fIdx)
{
   // FUN! I get to map from source this time.
   // I don't have to check if this is a folder (I think?)
   if ( kindaThing & m_type && fIdx.isValid() && ! isExpanded(m_filter->mapFromSource(fIdx) ))
      setExpanded(m_filter->mapFromSource(fIdx),true);
}

void BtTreeView::versionedRecipe(Recipe* descendant) { emit recipeSpawn(descendant); }

// Bad form likely

RecipeTreeView::RecipeTreeView(QWidget *parent)
   : BtTreeView(parent, BtTreeModel::RECIPEMASK)
{
   connect( m_model, &BtTreeModel::recipeSpawn, this, &BtTreeView::versionedRecipe );
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

// Cthulhu take me
WaterTreeView::WaterTreeView(QWidget *parent)
   : BtTreeView(parent,BtTreeModel::WATERMASK)
{
}
