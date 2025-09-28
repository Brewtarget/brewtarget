/*======================================================================================================================
 * trees/NamedEntityTreeModel.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef TREES_NAMEDENTITYTREEMODEL_H
#define TREES_NAMEDENTITYTREEMODEL_H
#pragma once

#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/InventoryFermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "trees/TreeModel.h"
#include "trees/TreeModelBase.h"

// See comment in trees/NamedEntityTreeView.h for why we can't remove the repetition below with a macro

class EquipmentTreeModel : public TreeModel, public TreeModelBase<EquipmentTreeModel, Equipment> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Equipment)
};

class FermentableTreeModel : public TreeModel, public TreeModelBase<FermentableTreeModel, Fermentable> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Fermentable)
};

class InventoryFermentableTreeModel : public TreeModel, public TreeModelBase<InventoryFermentableTreeModel, InventoryFermentable> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(InventoryFermentable)
};

class MashTreeModel : public TreeModel, public TreeModelBase<MashTreeModel, Mash, MashStep> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Mash, MashStep)
};

class BoilTreeModel : public TreeModel, public TreeModelBase<BoilTreeModel, Boil, BoilStep> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Boil, BoilStep)
};

class FermentationTreeModel : public TreeModel, public TreeModelBase<FermentationTreeModel, Fermentation, FermentationStep> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Fermentation, FermentationStep)
};

class HopTreeModel : public TreeModel, public TreeModelBase<HopTreeModel, Hop> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Hop)
};

class MiscTreeModel : public TreeModel, public TreeModelBase<MiscTreeModel, Misc> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Misc)
};

class SaltTreeModel : public TreeModel, public TreeModelBase<SaltTreeModel, Salt> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Salt)
};

class StyleTreeModel : public TreeModel, public TreeModelBase<StyleTreeModel, Style> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Style)
};

class WaterTreeModel : public TreeModel, public TreeModelBase<WaterTreeModel, Water> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Water)
};

class YeastTreeModel : public TreeModel, public TreeModelBase<YeastTreeModel, Yeast> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Yeast)
};

#endif
