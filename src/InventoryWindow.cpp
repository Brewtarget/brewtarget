/*======================================================================================================================
 * InventoryWindow.cpp is part of Brewtarget, and is copyright the following authors 2025:
 *   • Matt Young <mfsy@yahoo.com>
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
 =====================================================================================================================*/
#include "InventoryWindow.h"

#include "editors/InventoryFermentableEditor.h"

// This private implementation class holds all private non-virtual members of InventoryWindow
class InventoryWindow::impl {
public:
   impl(InventoryWindow & self) :
      m_self{self},
      m_inventoryFermentableEditor{std::make_unique<InventoryFermentableEditor>(&m_self)} {
      return;
   }

   ~impl() = default;

   //================================================ MEMBER VARIABLES =================================================
   InventoryWindow & m_self;
   std::unique_ptr<InventoryFermentableEditor> m_inventoryFermentableEditor;
};

InventoryWindow::InventoryWindow(QWidget * parent) :
   QDialog{parent},
   Ui::inventoryWindow{},
   pimpl{std::make_unique<impl>(*this)} {

   // This sets various things from the inventoryWindow.ui file, including window title
   this->setupUi(this);

   this->treeInvView_fermentable->init(*this->pimpl->m_inventoryFermentableEditor);

   return;
}

InventoryWindow::~InventoryWindow() = default;
