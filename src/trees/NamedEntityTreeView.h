/*======================================================================================================================
 * trees/NamedEntityTreeView.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef TREES_NAMEDENTITYTREEVIEW_H
#define TREES_NAMEDENTITYTREEVIEW_H
#pragma once

#include "editors/BoilEditor.h"
#include "editors/EquipmentEditor.h"
#include "editors/FermentableEditor.h"
#include "editors/FermentationEditor.h"
#include "editors/HopEditor.h"
#include "editors/InventoryFermentableEditor.h"
#include "editors/MashEditor.h"
#include "editors/MiscEditor.h"
#include "editors/SaltEditor.h"
#include "editors/StyleEditor.h"
#include "editors/WaterEditor.h"
#include "editors/YeastEditor.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/InventoryFermentable.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "trees/NamedEntityTreeModel.h"
#include "trees/NamedEntityTreeSortFilterProxyModel.h"
#include "trees/TreeView.h"
#include "trees/TreeViewBase.h"
#include "trees/TreeModelBase.h"


// Although the class definitions below look like ideal candidates for a macro, this would confuse the Qt MOC, so we
// live with the small amount of repetition here.

class EquipmentTreeView : public TreeView,
                          public TreeViewBase<EquipmentTreeView,
                                              EquipmentTreeModel,
                                              EquipmentTreeSortFilterProxyModel,
                                              EquipmentEditor,
                                              Equipment> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Equipment)
};

class MashTreeView : public TreeView,
                     public TreeViewBase<MashTreeView,
                                         MashTreeModel,
                                         MashTreeSortFilterProxyModel,
                                         MashEditor,
                                         Mash,
                                         MashStep> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Mash, MashStep)
};

class BoilTreeView : public TreeView,
                     public TreeViewBase<BoilTreeView,
                                         BoilTreeModel,
                                         BoilTreeSortFilterProxyModel,
                                         BoilEditor,
                                         Boil,
                                         BoilStep> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Boil, BoilStep)
};

class FermentationTreeView : public TreeView,
                             public TreeViewBase<FermentationTreeView,
                                                 FermentationTreeModel,
                                                 FermentationTreeSortFilterProxyModel,
                                                 FermentationEditor,
                                                 Fermentation,
                                                 FermentationStep> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Fermentation, FermentationStep)
};

class FermentableTreeView : public TreeView,
                            public TreeViewBase<FermentableTreeView,
                                                FermentableTreeModel,
                                                FermentableTreeSortFilterProxyModel,
                                                FermentableEditor,
                                                Fermentable> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Fermentable)
};

class HopTreeView : public TreeView,
                    public TreeViewBase<HopTreeView,
                                        HopTreeModel,
                                        HopTreeSortFilterProxyModel,
                                        HopEditor,
                                        Hop> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Hop)
};

class InventoryFermentableTreeView : public TreeView,
                                     public TreeViewBase<InventoryFermentableTreeView,
                                                         InventoryFermentableTreeModel,
                                                         InventoryFermentableTreeSortFilterProxyModel,
                                                         InventoryFermentableEditor,
                                                         InventoryFermentable> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(InventoryFermentable)
};

class MiscTreeView : public TreeView,
                     public TreeViewBase<MiscTreeView,
                                         MiscTreeModel,
                                         MiscTreeSortFilterProxyModel,
                                         MiscEditor,
                                         Misc> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Misc)
};

class SaltTreeView : public TreeView,
                     public TreeViewBase<SaltTreeView,
                                          SaltTreeModel,
                                          SaltTreeSortFilterProxyModel,
                                          SaltEditor,
                                          Salt> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Salt)
};

class StyleTreeView : public TreeView,
                      public TreeViewBase<StyleTreeView,
                                          StyleTreeModel,
                                          StyleTreeSortFilterProxyModel,
                                          StyleEditor,
                                          Style> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Style)
};

class WaterTreeView : public TreeView,
                      public TreeViewBase<WaterTreeView,
                                          WaterTreeModel,
                                          WaterTreeSortFilterProxyModel,
                                          WaterEditor,
                                          Water> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Water)
};

class YeastTreeView : public TreeView,
                      public TreeViewBase<YeastTreeView,
                                          YeastTreeModel,
                                          YeastTreeSortFilterProxyModel,
                                          YeastEditor,
                                          Yeast> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Yeast)
};


#endif
