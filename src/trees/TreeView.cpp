/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeView.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include <QMimeData>

#include "MainWindow.h"
#include "trees/TreeModel.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_TreeView.cpp"
#endif

TreeView::TreeView(QWidget * parent) :
   QTreeView{parent} {
   // Set some global properties that all the kids will use.
   this->setAllColumnsShowFocus(true);
   this->setContextMenuPolicy(Qt::CustomContextMenu);

   // This shows the little arrow next to folders so you can see whether they are expanded or not
   this->setRootIsDecorated(true);

   // In QTreeView "this property should only be set to true if it is guaranteed that all items in the view has the same
   // height. This enables the view to do some optimizations."
   this->setUniformRowHeights(true);

   this->setAnimated(true);

   this->setDragEnabled(true);
   this->setAcceptDrops(true);
   this->setDropIndicatorShown(true);

   this->setSelectionBehavior(QAbstractItemView::SelectRows);
   this->setSelectionMode(QAbstractItemView::ExtendedSelection);

   return;
}

void TreeView::exportSelected() const {
   // TODO: This is a bit of a hack just to avoid circular dependencies between TreeViewBase and MainWindow.  We should
   //       come back and look at this properly at some point.
   MainWindow::instance().exportSelected();
   return;
}

void TreeView::importFiles() {
   // TODO: This is even more of a hack, since import isn't specific to one tree.
   MainWindow::instance().importFiles();
   return;
}

void TreeView::mousePressEvent(QMouseEvent * event) {
   if (event->button() == Qt::LeftButton) {
      this->dragStart = event->pos();
      this->m_doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mousePressEvent(event);
}

void TreeView::mouseDoubleClickEvent(QMouseEvent * event) {

   if (event->button() == Qt::LeftButton) {
      this->m_doubleClick = true;
   } else {
      this->m_doubleClick = false;
   }

   // Send the event on its way up to the parent
   QTreeView::mouseDoubleClickEvent(event);
   return;
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

   if (this->m_doubleClick) {
      return;
   }

   //
   // Note that the call to model() here gets our QSortFilterProxyModel, which, via its mimeData() member function,
   // handles mapping selected rows in the view to the corresponding rows in the source model (ie TreeModel subclass)
   // before calling TreeModel::mimeData().
   //
   QMimeData * data = this->model()->mimeData(this->selectionModel()->selectedRows());

   QDrag * drag = new QDrag(this);
   drag->setMimeData(data);
   drag->exec(Qt::CopyAction);

   return;
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
   return;
}

void TreeView::rowsInserted(QModelIndex const & parent, int start, int end) {
   qDebug() << Q_FUNC_INFO << "Rows" << start << "-" << end << "added to" << parent;
   this->QTreeView::rowsInserted(parent, start, end);
   return;
}
