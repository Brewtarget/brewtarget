/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeView.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "trees/TreeView.h"

#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

#include "catalogs/FermentableCatalog.h"
#include "catalogs/HopCatalog.h"
#include "catalogs/MiscCatalog.h"
#include "catalogs/YeastCatalog.h"
#include "editors/EquipmentEditor.h"
#include "editors/StyleEditor.h"
#include "editors/WaterEditor.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Folder.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "trees/TreeModel.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_TreeView.cpp"

TreeView::TreeView(QWidget * parent, TreeModel::TypeMasks type) :
   QTreeView{parent},
   m_type{type} {
   qDebug() << Q_FUNC_INFO << "type=" << type;
   // Set some global properties that all the kids will use.
   setAllColumnsShowFocus(true);
   setContextMenuPolicy(Qt::CustomContextMenu);
   setRootIsDecorated(false);

   setDragEnabled(true);
   setAcceptDrops(true);
   setDropIndicatorShown(true);
   setSelectionMode(QAbstractItemView::ExtendedSelection);

   this->m_type = type;
   this->m_model = new TreeModel(this, m_type);
   this->m_filter = new TreeFilterProxyModel(this, m_type);
   this->m_filter->setSourceModel(m_model);
   this->setModel(m_filter);
   this->m_filter->setDynamicSortFilter(true);

   setExpanded(findElement(nullptr), true);
   setSortingEnabled(true);
   sortByColumn(0, Qt::AscendingOrder);
   resizeColumnToContents(0);

   // and one wee connection
   connect(m_model, &TreeModel::expandFolder, this, &TreeView::expandFolder);
   return;
}

TreeModel * TreeView::model() {
   return m_model;
}

TreeFilterProxyModel * TreeView::filter() {
   return m_filter;
}

bool TreeView::removeRow(const QModelIndex & index) {
   QModelIndex modelIndex = m_filter->mapToSource(index);
   QModelIndex parent = m_model->parent(modelIndex);
   int position       = modelIndex.row();

   return m_model->removeRows(position, 1, parent);
}

bool TreeView::isParent(const QModelIndex & parent, const QModelIndex & child) {
   QModelIndex modelParent = m_filter->mapToSource(parent);
   QModelIndex modelChild = m_filter->mapToSource(child);
   return modelParent == m_model->parent(modelChild);
}

QModelIndex TreeView::parent(const QModelIndex & child) {
   if (! child.isValid()) {
      return QModelIndex();
   }

   QModelIndex modelChild = m_filter->mapToSource(child);
   if (modelChild.isValid()) {
      return m_filter->mapFromSource(m_model->parent(modelChild));
   }

   return QModelIndex();
}

QModelIndex TreeView::first() {
   return m_filter->mapFromSource(m_model->first());
}

QString TreeView::folderName(QModelIndex index) {
   if (m_model->type(m_filter->mapToSource(index)) == TreeNode::Type::Folder) {
      return m_model->getItem<Folder>(m_filter->mapToSource(index))->fullPath();
   }

   //
   // TBD: The qobject_cast here is a bit clunky but, for the moment, is necessary now that we removed folders from
   //      BrewNotes.
   //
   auto folder = FolderUtils::getFolder(m_model->thing(m_filter->mapToSource(index)));
   if (folder) {
      return *folder;
   } else {
      return "";
   }
}

QModelIndex TreeView::findElement(NamedEntity * thing) {
   return m_filter->mapFromSource(m_model->findElement(thing));
}

template<class T>
T * TreeView::getItem(QModelIndex const & index) const {
   if (! index.isValid()) {
      return nullptr;
   }

   return this->m_model->getItem<T>(this->m_filter->mapToSource(index));
}
//
// Instantiate the above template function for the types that are going to use it
//
template Recipe      * TreeView::getItem<Recipe     >(QModelIndex const & index) const;
template Equipment   * TreeView::getItem<Equipment  >(QModelIndex const & index) const;
template Fermentable * TreeView::getItem<Fermentable>(QModelIndex const & index) const;
template Hop         * TreeView::getItem<Hop        >(QModelIndex const & index) const;
template Misc        * TreeView::getItem<Misc       >(QModelIndex const & index) const;
template Yeast       * TreeView::getItem<Yeast      >(QModelIndex const & index) const;
template Style       * TreeView::getItem<Style      >(QModelIndex const & index) const;
template Water       * TreeView::getItem<Water      >(QModelIndex const & index) const;
template BrewNote    * TreeView::getItem<BrewNote   >(QModelIndex const & index) const;
template Folder    * TreeView::getItem<Folder   >(QModelIndex const & index) const;

QModelIndex TreeView::findFolder(Folder * folder) {
   return m_filter->mapFromSource(m_model->findFolder(folder->fullPath(), nullptr, false));
}

void TreeView::addFolder(QString folder) {
   m_model->addFolder(folder);
}

void TreeView::renameFolder(Folder * victim, QString newName) {
   m_model->renameFolder(victim, newName);
}

std::optional<TreeNode::Type> TreeView::type(const QModelIndex & index) const {
   return this->m_model->type(this->m_filter->mapToSource(index));
}

void TreeView::mousePressEvent(QMouseEvent * event) {
   if (event->button() == Qt::LeftButton) {
      this->dragStart = event->pos();
      this->doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mousePressEvent(event);
}

void TreeView::mouseDoubleClickEvent(QMouseEvent * event) {

   if (event->button() == Qt::LeftButton) {
      this->doubleClick = true;
   } else {
      this->doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mouseDoubleClickEvent(event);
}

void TreeView::mouseMoveEvent(QMouseEvent * event) {
   // Return if the left button isn't down
   if (!(event->buttons() & Qt::LeftButton)) {
      return;
   }

   // Return if the length of movement isn't far enough.
   if ((event->pos() - dragStart).manhattanLength() < QApplication::startDragDistance()) {
      return;
   }

   if (this->doubleClick) {
      return;
   }

   QDrag * drag = new QDrag(this);
   QMimeData * data = mimeData(selectionModel()->selectedRows());

   drag->setMimeData(data);
   drag->exec(Qt::CopyAction);
}

void TreeView::keyPressEvent(QKeyEvent * event) {
   switch (event->key()) {
      case Qt::Key_Space:
      case Qt::Key_Select:
      case Qt::Key_Enter:
      case Qt::Key_Return:
         emit TreeView::doubleClicked(selectedIndexes().first());
         return;
   }
   QTreeView::keyPressEvent(event);
}

QMimeData * TreeView::mimeData(QModelIndexList indexes) {
   QString name = "";

   QByteArray encodedData;
   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   // From what I've been able to tell, the drop events are homogenous -- a
   // single drop event will be all equipment or all recipe or ...
   std::optional<TreeNode::Type> itsa;
   for (QModelIndex index : indexes) {

      if (!index.isValid()) {
         continue;
      }

      int id;
      auto nodeType = this->type(index);
      if (nodeType != TreeNode::Type::Folder) {
         if (m_model->thing(m_filter->mapToSource(index)) == nullptr) {
            qWarning() << Q_FUNC_INFO << QString("Couldn't map that thing");
            id = -1;
         } else {
            id   = m_model->thing(m_filter->mapToSource(index))->key();
            name = m_model->name(m_filter->mapToSource(index));
            // Save this for later reference
            if (!itsa) {
               itsa = nodeType;
            }
         }
      } else {
         id = -1;
         name = m_model->getItem<Folder>(m_filter->mapToSource(index))->fullPath();
      }
      stream << static_cast<int>(*nodeType) << id << name;
   }

   if (*itsa == TreeNode::Type::Recipe || *itsa == TreeNode::Type::Style || *itsa == TreeNode::Type::Equipment) {
      // Recipes, equipment and styles get dropped on the recipe pane
      name = "application/x-brewtarget-recipe";
   } else if ( *itsa == TreeNode::Type::Folder ) {
      // folders will be handled by themselves.
      name = "application/x-brewtarget-folder";
   } else if ( *itsa != TreeNode::Type::Water ) {
      // Everything other than folders get dropped on the ingredients pane
      name = "application/x-brewtarget-ingredient";
   } else {
      // This isn't used yet, but maybe some day I will fix that
      name = "application/x-brewtarget-water";
   }

   QMimeData * mimeData = new QMimeData();
   mimeData->setData(name, encodedData);
   return mimeData;
}

bool TreeView::multiSelected() {
   QModelIndexList selected = selectionModel()->selectedRows();
   if (selected.count() == 0) {
      return false;
   }

   bool hasRecipe        = false;
   bool hasSomethingElse = false;

   for (QModelIndex selection : selected) {
      QModelIndex selectModel = m_filter->mapToSource(selection);
      if (m_model->itemIs<Recipe>(selectModel)) {
         hasRecipe = true;
      } else {
         hasSomethingElse = true;
      }
   }

   return hasRecipe && hasSomethingElse;
}

void TreeView::newNamedEntity() {

   QString folder;
   QModelIndexList indexes = selectionModel()->selectedRows();
   // This is a little weird. There is an edge case where nothing is
   // selected and you click the big blue + button.
   if (indexes.size() > 0) {
      folder = folderName(indexes.at(0));
   }

   if (m_type.testFlag(TreeModel::TypeMask::Equipment  )) { qobject_cast<EquipmentEditor   *>(m_editor)->newEditItem(folder); return; }
   if (m_type.testFlag(TreeModel::TypeMask::Fermentable)) { qobject_cast<FermentableEditor *>(m_editor)->newEditItem(folder); return; }
   if (m_type.testFlag(TreeModel::TypeMask::Hop        )) { qobject_cast<HopEditor         *>(m_editor)->newEditItem(folder); return; }
   if (m_type.testFlag(TreeModel::TypeMask::Misc       )) { qobject_cast<MiscEditor        *>(m_editor)->newEditItem(folder); return; }
   if (m_type.testFlag(TreeModel::TypeMask::Style      )) { qobject_cast<StyleEditor       *>(m_editor)->newEditItem(folder); return; }
   if (m_type.testFlag(TreeModel::TypeMask::Yeast      )) { qobject_cast<YeastEditor       *>(m_editor)->newEditItem(folder); return; }
   if (m_type.testFlag(TreeModel::TypeMask::Water      )) { qobject_cast<WaterEditor       *>(m_editor)->newEditItem(folder); return; }

   qWarning() << Q_FUNC_INFO << "Unrecognized mask" << m_type;
   return;
}

void TreeView::showAncestors() {
   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach (QModelIndex selected, ndxs) {
         m_model->showAncestors(m_filter->mapToSource(selected));
      }
   }
}

void TreeView::hideAncestors() {
   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach (QModelIndex selected, ndxs) {
         // make sure we add the ancestors to the exclusion list
         m_model->hideAncestors(m_filter->mapToSource(selected));
      }
   }
}

void TreeView::revertRecipeToPreviousVersion() {
   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach (QModelIndex selected, ndxs) {
         // make sure we add the ancestors to the exclusion list
         m_model->revertRecipeToPreviousVersion(m_filter->mapToSource(selected));
      }
   }
}

void TreeView::orphanRecipe() {
   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      // I hear a noise at the door, as of some immense slippery body
      // lumbering against it
      foreach( QModelIndex selected, ndxs ) {
         // make sure we add the ancestors to the exclusion list
         m_model->orphanRecipe(m_filter->mapToSource(selected));
      }
   }
}

void TreeView::spawnRecipe() {
   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      QModelIndexList ndxs = selectionModel()->selectedRows();

      foreach (QModelIndex selected, ndxs) {
         // make sure we add the ancestors to the exclusion list
         m_model->spawnRecipe(m_filter->mapToSource(selected));
      }
   }
}

bool TreeView::ancestorsAreShowing(QModelIndex ndx) {
   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      QModelIndex translated = m_filter->mapToSource(ndx);
      return m_model->showChild(translated);
   }

   return false;
}
void TreeView::enableDelete(bool enable)       {
   m_deleteAction->setEnabled(enable);
}
void TreeView::enableShowAncestor(bool enable) {
   m_showAncestorAction->setEnabled(enable);
}
void TreeView::enableHideAncestor(bool enable) {
   m_hideAncestorAction->setEnabled(enable);
}
void TreeView::enableOrphan(bool enable)       {
   m_orphanAction->setEnabled(enable);
}
void TreeView::enableSpawn(bool enable)        {
   m_spawnAction->setEnabled(enable);
}

void TreeView::setupContextMenu(QWidget * top, QWidget * editor) {
   QMenu * newMenu = new QMenu(this);
   m_exportMenu = new QMenu(this);
   m_contextMenu = new QMenu(this);
   subMenu = new QMenu(this);
   m_versionMenu = new QMenu(this);

   m_editor = editor;

   newMenu->setTitle(tr("New"));
   m_contextMenu->addMenu(newMenu);


   if (m_type.testFlag(TreeModel::TypeMask::Recipe)) {
      // the recipe case is a bit more complex, because we need to handle the brewnotes too
      newMenu->addAction(tr("Recipe"), editor, SLOT(newRecipe()));

      // version menu
      m_versionMenu->setTitle("Snapshots");
      m_showAncestorAction = m_versionMenu->addAction(tr("Show Snapshots"), this, SLOT(showAncestors()));
      m_hideAncestorAction = m_versionMenu->addAction(tr("Hide Snapshots"), this, SLOT(hideAncestors()));
      m_orphanAction = m_versionMenu->addAction(tr("Detach Recipe"), this, SLOT(orphanRecipe()));
      m_spawnAction  = m_versionMenu->addAction(tr("Snapshot Recipe"), this, SLOT(spawnRecipe()));
      m_contextMenu->addMenu(m_versionMenu);

      m_contextMenu->addSeparator();
      m_brewItAction = m_contextMenu->addAction(tr("Brew It!"), top, SLOT(brewItHelper()));
      m_contextMenu->addSeparator();

      subMenu->addAction(tr("Brew Again"), top, SLOT(brewAgainHelper()));
      subMenu->addAction(tr("Change date"), top, SLOT(changeBrewDate()));
      subMenu->addAction(tr("Recalculate eff"), top, SLOT(fixBrewNote()));
      subMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));

   }
   else if (m_type.testFlag(TreeModel::TypeMask::Equipment  )) { newMenu->addAction(tr("Equipment"  ), this, SLOT(newNamedEntity())); }
   else if (m_type.testFlag(TreeModel::TypeMask::Fermentable)) { newMenu->addAction(tr("Fermentable"), this, SLOT(newNamedEntity())); }
   else if (m_type.testFlag(TreeModel::TypeMask::Hop        )) { newMenu->addAction(tr("Hop"        ), this, SLOT(newNamedEntity())); }
   else if (m_type.testFlag(TreeModel::TypeMask::Misc       )) { newMenu->addAction(tr("Misc"       ), this, SLOT(newNamedEntity())); }
   else if (m_type.testFlag(TreeModel::TypeMask::Style      )) { newMenu->addAction(tr("Style"      ), this, SLOT(newNamedEntity())); }
   else if (m_type.testFlag(TreeModel::TypeMask::Yeast      )) { newMenu->addAction(tr("Yeast"      ), this, SLOT(newNamedEntity())); }
   else if (m_type.testFlag(TreeModel::TypeMask::Water      )) { newMenu->addAction(tr("Water"      ), this, SLOT(newNamedEntity())); }
   else {
      qWarning() << Q_FUNC_INFO << "Unrecognized mask" << m_type;
   }

   m_contextMenu->addSeparator();
   newMenu->addAction(tr("Folder"), top, SLOT(newFolder()));
   // Copy
   m_copyAction = m_contextMenu->addAction(tr("Copy"), top, SLOT(copySelected()));
   // m_deleteAction makes it easier to find this later to disable it
   m_deleteAction = m_contextMenu->addAction(tr("Delete"), top, SLOT(deleteSelected()));
   // export and import
   m_contextMenu->addSeparator();
   m_exportMenu->setTitle(tr("Export"));
   m_exportMenu->addAction(tr("To File (BeerXML or BeerJSON)"), top, SLOT(exportSelected()));
//   m_exportMenu->addAction(tr("To HTML"), top, SLOT(exportSelectedHtml()));
   m_contextMenu->addMenu(m_exportMenu);
   m_contextMenu->addAction(tr("Import"), top, SLOT(importFiles()));
   return;
}

QMenu * TreeView::contextMenu(QModelIndex selected) {
   bool disableDelete = false;

   TreeNode::Type t_type = *this->type(selected);
   if (t_type == TreeNode::Type::BrewNote) {
      return subMenu;
   }

   if (t_type == TreeNode::Type::Recipe) {
      // Right at the top of the tree, it's possible to click on something that is neither a folder nor a recipe, so
      // we have to check for that here.
      auto rec = this->getItem<Recipe>(selected);
      if (rec) {
         QModelIndex translated = m_filter->mapToSource(selected);

         // you can not delete a locked recipe
         enableDelete(! rec->locked());

         // if we have ancestors and are showing them but are not an actual
         // ancestor, then enable hide
         enableHideAncestor(rec->hasAncestors() && m_model->showChild(translated) && rec->display());

         // if we have ancestors and are not showing them, enable showAncestors
         enableShowAncestor(rec->hasAncestors() && ! m_model->showChild(translated));

         // if we have ancestors and are not locked, then we are a leaf node and
         // allow orphaning
         enableOrphan(rec->hasAncestors() && ! rec->locked());

         // if display is true, we can spawn it. This should mean we cannot spawn
         // ancestors directly, which is what I want.
         enableSpawn(rec->display());

         // If user has clicked the Top-level Item 'Recipes' Once this menu Item will be forever disabled if we don't enable it.
         m_exportMenu->setEnabled(true);
         m_copyAction->setEnabled(true);
         m_brewItAction->setEnabled(true);
      } else {
         // This case will happen if user Right-click the top most item in the list, as that will yield a rec == nullptr.
         // In this case we will treat it like a folder and disable a bunch of options.
         t_type = TreeNode::Type::Folder;
         disableDelete = true;
      }
      if (t_type == TreeNode::Type::Folder) {
         enableDelete( ! disableDelete );
         enableHideAncestor( false );
         enableShowAncestor( false );
         enableOrphan( false );
         enableSpawn( false );
         m_exportMenu->setEnabled( false );
         m_copyAction->setEnabled( false );
         m_brewItAction->setEnabled( false );
      }
   }

   return m_contextMenu;
}

QString TreeView::verifyCopy(QString tag, QString name, bool * abort) {
   QInputDialog askEm;

   // Gotta build this hard, so we can say "cancel all"
   askEm.setCancelButtonText(tr("Cancel All"));
   askEm.setWindowTitle(tr("Copy %1").arg(tag));
   askEm.setLabelText(tr("Enter a unique name for the copy of %1.").arg(name));
   askEm.setToolTip(tr("An empty name will skip copying this %1.").arg(tag));

   if (askEm.exec() == QDialog::Accepted) {
      if (abort) {
         *abort = false;
      }

      name = askEm.textValue();
   } else {
      if (abort) {
         *abort = true;
      }
   }

   return name;
}

void TreeView::copySelected(QModelIndexList selected) {
   QList< QPair<QModelIndex, QString>> names;
   QString newName;
   bool abort = false;

   // Time to lay down the boogie
   for (QModelIndex at : selected) {
      // If somebody said cancel, bug out
      if (abort == true) {
         return;
      }

      // First, we should translate from proxy to model, because I need this index a lot.
      QModelIndex trans = m_filter->mapToSource(at);

      // You can't delete the root element
      if (trans == findElement(nullptr)) {
         continue;
      }

      // Otherwise prompt
      auto nodeType = this->m_model->type(trans);
      if (!nodeType) {
         qWarning() << Q_FUNC_INFO << "Unknown type";
      } else {
         switch (*nodeType) {
            case TreeNode::Type::Equipment:
               newName = verifyCopy(tr("Equipment"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Fermentable:
               newName = verifyCopy(tr("Fermentable"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Hop:
               newName = verifyCopy(tr("Hop"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Misc:
               newName = verifyCopy(tr("Misc"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Recipe:
               newName = verifyCopy(tr("Recipe"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Style:
               newName = verifyCopy(tr("Style"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Yeast:
               newName = verifyCopy(tr("Yeast"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::Water:
               newName = verifyCopy(tr("Water"), m_model->name(trans), &abort);
               break;
            case TreeNode::Type::BrewNote:
            case TreeNode::Type::Folder:
               // These cases shouldn't arise (I think!) but the compiler will emit a warning if we don't explicitly
               // have code to handle them (which is good!).
               qWarning() << Q_FUNC_INFO << "Unexpected item type" << static_cast<int>(*nodeType);
               break;
         }
      }
      if (!abort && !newName.isEmpty()) {
         names.append(qMakePair(trans, newName));
      }
   }
   // If we get here, call the model to do the copy
   m_model->copySelected(names);
   return;
}

int TreeView::verifyDelete(int confirmDelete, QString tag, QString name) {
   if (confirmDelete == QMessageBox::YesToAll) {
      return confirmDelete;
   }

   return QMessageBox::question(this, tr("Delete %1").arg(tag), tr("Delete %1 %2?").arg(tag).arg(name),
                                QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel,
                                QMessageBox::No);

}

// I should maybe shove this further down the stack. But I prefer to keep the
// confirmation windows at least this high -- models shouldn't be interacting
// with users.
void TreeView::deleteSelected(QModelIndexList selected) {
   //.:TODO:. Pull out some of the common code from this and copySelected()

   QModelIndexList translated;

   int confirmDelete = QMessageBox::NoButton;

   // Time to lay down the boogie
   for (QModelIndex at : selected) {
      // If somebody said cancel, bug out
      if (confirmDelete == QMessageBox::Cancel) {
         return;
      }

      // First, we should translate from proxy to model, because I need this index a lot.
      QModelIndex trans = m_filter->mapToSource(at);

      // You can't delete the root element
      if (trans == findElement(nullptr)) {
         continue;
      }

      // If we have alread said "Yes To All", just append and go
      if (confirmDelete == QMessageBox::YesToAll) {
         translated.append(trans);
         continue;
      }

      // Otherwise prompt
      auto nodeType = this->m_model->type(trans);
      if (!nodeType) {
         qWarning() << Q_FUNC_INFO << "Unknown type";
      } else {
         switch (*nodeType) {
            case TreeNode::Type::Recipe:
               confirmDelete = verifyDelete(confirmDelete, tr("Recipe"), m_model->name(trans));
               break;
            case TreeNode::Type::Equipment:
               confirmDelete = verifyDelete(confirmDelete, tr("Equipment"), m_model->name(trans));
               break;
            case TreeNode::Type::Fermentable:
               confirmDelete = verifyDelete(confirmDelete, tr("Fermentable"), m_model->name(trans));
               break;
            case TreeNode::Type::Hop:
               confirmDelete = verifyDelete(confirmDelete, tr("Hop"), m_model->name(trans));
               break;
            case TreeNode::Type::Misc:
               confirmDelete = verifyDelete(confirmDelete, tr("Misc"), m_model->name(trans));
               break;
            case TreeNode::Type::Style:
               confirmDelete = verifyDelete(confirmDelete, tr("Style"), m_model->name(trans));
               break;
            case TreeNode::Type::Yeast:
               confirmDelete = verifyDelete(confirmDelete, tr("Yeast"), m_model->name(trans));
               break;
            case TreeNode::Type::BrewNote:
               confirmDelete = verifyDelete(confirmDelete, tr("BrewNote"), m_model->getItem<BrewNote>(trans)->brewDate_short());
               break;
            case TreeNode::Type::Folder:
               confirmDelete = verifyDelete(confirmDelete, tr("Folder"), m_model->getItem<Folder>(trans)->fullPath());
               break;
            case TreeNode::Type::Water:
               confirmDelete = verifyDelete(confirmDelete, tr("Water"), m_model->name(trans));
               break;
         }
      }
      // If they selected "Yes" or "Yes To All", push and loop
      if (confirmDelete == QMessageBox::Yes || confirmDelete == QMessageBox::YesToAll) {
         translated.append(trans);
      }
   }

   // If we get here, call the model to delete the victims
   this->m_model->deleteSelected(translated);

   // NB: In the case of deleting a Recipe, MainWindow::deleteSelected() has the logic that then chooses a new Recipe to
   // show in the main edit pane.

   return;
}

void TreeView::setFilter(TreeFilterProxyModel * newFilter) {
   m_filter = newFilter;
}

TreeFilterProxyModel * TreeView::filter() const {
   return m_filter;
}

void TreeView::expandFolder(TreeModel::TypeMasks kindaThing, QModelIndex fIdx) {
   // FUN! I get to map from source this time.
   // I don't have to check if this is a folder (I think?)
   if (kindaThing & m_type && fIdx.isValid() && ! isExpanded(m_filter->mapFromSource(fIdx))) {
      setExpanded(m_filter->mapFromSource(fIdx), true);
   }
}

void TreeView::versionedRecipe(Recipe * descendant) {
   emit recipeSpawn(descendant);
}

TREE_VIEW_COMMON_CODE(Recipe)
TREE_VIEW_COMMON_CODE(Equipment)
TREE_VIEW_COMMON_CODE(Fermentable)
TREE_VIEW_COMMON_CODE(Hop)
TREE_VIEW_COMMON_CODE(Misc)
TREE_VIEW_COMMON_CODE(Yeast)
TREE_VIEW_COMMON_CODE(Style)
TREE_VIEW_COMMON_CODE(Water)
