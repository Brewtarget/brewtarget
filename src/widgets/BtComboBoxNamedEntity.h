/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtComboBoxNamedEntity.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef WIDGETS_BTCOMBOBOXNAMEDENTITY_H
#define WIDGETS_BTCOMBOBOXNAMEDENTITY_H
#pragma once

#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

#include "qtModels/listModels/BoilListModel.h"
#include "qtModels/listModels/EquipmentListModel.h"
#include "qtModels/listModels/MashListModel.h"
#include "qtModels/listModels/FermentableListModel.h"
#include "qtModels/listModels/FermentationListModel.h"
#include "qtModels/listModels/HopListModel.h"
#include "qtModels/listModels/MiscListModel.h"
#include "qtModels/listModels/SaltListModel.h"
#include "qtModels/listModels/StyleListModel.h"
#include "qtModels/listModels/WaterListModel.h"
#include "qtModels/listModels/YeastListModel.h"

#include "qtModels/sortFilterProxyModels/BoilSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/EquipmentSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/MashSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/FermentableSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/FermentationSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/HopSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/MiscSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/SaltSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/YeastSortFilterProxyModel.h"

#include "widgets/BtComboBoxObject.h"
#include "widgets/BtComboBoxObjectBase.h"

// See comment in trees/NamedEntityTreeView.h for why we can't remove the repetition below with a macro
class BtComboBoxBoil : public BtComboBoxObject,
                       public BtComboBoxObjectBase<BtComboBoxBoil,
                                                   Boil          ,
                                                   BoilListModel ,
                                                   BoilSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Boil)
};

class BtComboBoxEquipment : public BtComboBoxObject,
                            public BtComboBoxObjectBase<BtComboBoxEquipment,
                                                        Equipment          ,
                                                        EquipmentListModel ,
                                                        EquipmentSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Equipment)
};

class BtComboBoxMash : public BtComboBoxObject,
                       public BtComboBoxObjectBase<BtComboBoxMash,
                                                   Mash          ,
                                                   MashListModel ,
                                                   MashSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Mash)
};

class BtComboBoxFermentable : public BtComboBoxObject,
                              public BtComboBoxObjectBase<BtComboBoxFermentable,
                                                          Fermentable          ,
                                                          FermentableListModel ,
                                                          FermentableSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Fermentable)
};

class BtComboBoxFermentation : public BtComboBoxObject,
                               public BtComboBoxObjectBase<BtComboBoxFermentation,
                                                           Fermentation          ,
                                                           FermentationListModel ,
                                                           FermentationSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Fermentation)
};

class BtComboBoxHop : public BtComboBoxObject,
                      public BtComboBoxObjectBase<BtComboBoxHop,
                                                  Hop          ,
                                                  HopListModel ,
                                                  HopSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Hop)
};

class BtComboBoxMisc : public BtComboBoxObject,
                       public BtComboBoxObjectBase<BtComboBoxMisc,
                                                   Misc          ,
                                                   MiscListModel ,
                                                   MiscSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Misc)
};

class BtComboBoxSalt : public BtComboBoxObject,
                       public BtComboBoxObjectBase<BtComboBoxSalt,
                                                   Salt          ,
                                                   SaltListModel ,
                                                   SaltSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Salt)
};

class BtComboBoxStyle : public BtComboBoxObject,
                        public BtComboBoxObjectBase<BtComboBoxStyle,
                                                    Style          ,
                                                    StyleListModel ,
                                                    StyleSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Style)
};

class BtComboBoxWater : public BtComboBoxObject,
                        public BtComboBoxObjectBase<BtComboBoxWater,
                                                    Water          ,
                                                    WaterListModel ,
                                                    WaterSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Water)
};

class BtComboBoxYeast : public BtComboBoxObject,
                        public BtComboBoxObjectBase<BtComboBoxYeast,
                                                    Yeast          ,
                                                    YeastListModel ,
                                                    YeastSortFilterProxyModel> {
   Q_OBJECT
   BT_COMBO_BOX_OBJECT_DECL(Yeast)
};

#endif
