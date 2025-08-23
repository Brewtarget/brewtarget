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

#include <QComboBox>
#include <QDebug>
#include <QString>
#include <QTextStream>

#include "utils/BtStringConst.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::BtComboBoxNamedEntity { inline BtStringConst const property{#property}; }
AddPropertyName(currentId)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Extends \c QComboBox to show a list of a particular \c NamedEntity - eg \c Style for a \c Recipe.  Used in
 *        conjunction with \c BtComboBoxNamedEntityBase to make \c BtComboBoxNamedStyle etc, because classes inheriting
 *        from QObject can't be templated, as the Qt MOC won't be able to process them.  (See comment in
 *        qtModels/tableModels/BtTableModel.h for more info.)
 */
class BtComboBoxNamedEntity : public QComboBox {
   Q_OBJECT

protected:
   BtComboBoxNamedEntity(char const * const name, QWidget * parent);
   virtual ~BtComboBoxNamedEntity();

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief The ID of the selected \c NamedEntity.
    *        This read-only property is only used by \c ScaleRecipeTool so we can use \c QWizardPage::registerField
    */
   Q_PROPERTY(int currentId    READ getCurrentId)

   /**
    * \brief Set the ID of the selected \c NamedEntity.  This should only be called via
    *        \c BtComboBoxNamedEntityBase::setItem() (which does additional checks).
    *
    * \param value -1 means nothing selected
    */
   void setCurrentId(int value);

   int getCurrentId() const;

private:
   QString const m_name;
};


#include "database/ObjectStoreWrapper.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief CRTP base for \c TreeModel subclasses.  See comment on \c TreeModel class for more info.
 *
 * \param Derived - The derived class
 * \param NE      - The \c NamedEntity subclass (eg \c Style) of objects listed in this combo box
 * \param NeListModel - The subclass of \c QlistModel for this type (eg \c StyleListModel)
 * \param NeSortFilterProxyModel - The subclass of \c QSortFilterProxyModel for this type (eg \c
 *                                 StyleSortFilterProxyModel)
 */
template<class Derived> class BtComboBoxNamedEntityPhantom;
template<class Derived, class NE, class NeListModel, class NeSortFilterProxyModel>
class BtComboBoxNamedEntityBase : public CuriouslyRecurringTemplateBase<BtComboBoxNamedEntityPhantom, Derived> {
   friend Derived;

public:
   /**
    * \brief Post-construction initialisation.
    *
    *        See comment in widgets/BtComboBoxEnum.h for why data cannot be specified in .ui file.
    *
    *        Unlike the other combo boxes, there aren't lots of different instances of each specialisation of
    *        \c BtComboBoxNamedEntity, so we don't store editor name, combo box name, etc.
    */
   void init() {
      //
      // Unlike BtComboBoxBool and BtComboBoxEnum, we don't populate the combo box items directly.  Rather, we use a
      // QSortFilterProxyModel (which in turn uses QAbstractItemModel), because then Qt can automatically sort the
      // entries.
      //
      // Note, also, that ListModelBase already handles updates from the object store (signalObjectInserted,
      // signalObjectDeleted), so we don't have to worry here about keeping the combo box contents in-sync with the DB.
      //
      // NB: Since we are managing object lifetime via unique_ptr, we don't pass in a parent pointer (as that would ask
      //     Qt to manage the object destruction.
      //
      this->m_listModel            = std::make_unique<NeListModel           >();
      this->m_sortFilterProxyModel = std::make_unique<NeSortFilterProxyModel>();

      //
      // Setting the dynamicSortFilter property to true means that the proxy model is dynamically sorted and filtered
      // whenever the contents of the source model change.  Per the Qt docs, "you should not update the source model
      // through the proxy model when dynamicSortFilter is true".
      //
      this->m_sortFilterProxyModel->setDynamicSortFilter(false);
      this->m_sortFilterProxyModel->setSortLocaleAware(true);
      this->m_sortFilterProxyModel->setSourceModel(this->m_listModel.get());
      this->m_sortFilterProxyModel->sort(0);

      this->derived().setModel(this->m_sortFilterProxyModel.get());

      //
      // The current limitation of using our own QSortFilterProxyModel etc to control what the combo box displays is
      // that we can't add an explicit "empty item" to the list of items, because EquipmentSortFilterProxyModel etc
      // don't support this.  Eg, putting the following here will have no effect:
      //
      //    this->derived().insertItem(0, "", QVariant::fromValue<int>(-1));
      //
      // However, even when a QComboBox combo box doesn't have such an "empty item", you can still set the current
      // selection to be empty (ie nothing selected) by passing -1 to QComboBox::setCurrentIndex.  It's just that, once
      // it is set to something, the user won't be able to unset it.  On the whole, that is actually what we want.  Eg,
      // once the user has set the Equipment on a new Recipe for the first time, they can change it but not unset it.
      //

      qDebug() << Q_FUNC_INFO << "Number of items:" << this->derived().count();

      // Uncomment the following block for diagnosing initialisation problems
//      {
//         QString debugOutput;
//         QTextStream debugOutputStream{&debugOutput};
//         for (int ii = 0; ii < this->derived().count(); ++ii) {
//            //
//            // Annoyingly, QMetaType implements operator<< only for QDebug output stream (and only since Qt 6.5), and
//            // doesn't have a toString() or similar member function.  However, we can lookup the names of the id()
//            // values at https://doc.qt.io/qt-6/qmetatype.html#Type-enum, which is good enough for our diagnostic
//            // purposes here.
//            //
//            QVariant data {this->derived().itemData(ii)};
//            debugOutputStream <<
//               "[" << ii << "] : (" << data.metaType().id() << ") " << data.typeName() << " : " << data.toString() << "\n";
//         }
//         qDebug().noquote() << Q_FUNC_INFO << "Values:\n" << debugOutput;
//      }

      //
      // By default, a QComboBox "will adjust to its contents the first time it is shown", which means that, on some
      // platforms at least, if it somehow gets shown before it is populated, then it will be far too narrow.
      //
      this->derived().QComboBox::setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return;
   }

   void setItem(std::shared_ptr<NE> item) {
      if (item) {
         this->derived().setCurrentId(item->key());
         return;
      }

      this->derived().setCurrentId(-1);
      return;
   }

   std::shared_ptr<NE> getItem() const {
      int const itemId = this->derived().getCurrentId();
      if (itemId < 0) {
         return std::shared_ptr<NE>{nullptr};
      }
      return ObjectStoreWrapper::getById<NE>(itemId);
   }

   //================================================ Member Variables =================================================
   std::unique_ptr<NeListModel           > m_listModel            = nullptr;
   std::unique_ptr<NeSortFilterProxyModel> m_sortFilterProxyModel = nullptr;
};

/**
 * \brief Generates derived class declaration.
 */
#define BT_COMBO_BOX_NAMED_ENTITY_DECL(NeName) \
class BtComboBox##NeName : public BtComboBoxNamedEntity, \
                           public BtComboBoxNamedEntityBase<BtComboBox##NeName          ,   \
                                                            NeName                      ,   \
                                                            NeName##ListModel           ,   \
                                                            NeName##SortFilterProxyModel> { \
   Q_OBJECT                                                                                 \
   /* Allows BtComboBoxNamedEntityBase to call protected and private members of Derived */  \
   friend class BtComboBoxNamedEntityBase<BtComboBox##NeName          ,  \
                                          NeName                      ,  \
                                          NeName##ListModel           ,  \
                                          NeName##SortFilterProxyModel>; \
                                                      \
   public:                                            \
      BtComboBox##NeName(QWidget * parent = nullptr); \
      virtual ~BtComboBox##NeName();                  \
};                                                    \

/**
 * \brief Generates derived class member function definitions.  Used in BtComboBoxNamedEntity.cpp.
 */
#define BT_COMBO_BOX_NAMED_ENTITY_CODE(NeName) \
   BtComboBox##NeName::BtComboBox##NeName(QWidget * parent) :      \
      BtComboBoxNamedEntity{"BtComboBox" #NeName, parent},         \
      BtComboBoxNamedEntityBase<BtComboBox##NeName          ,      \
                                NeName                      ,      \
                                NeName##ListModel           ,      \
                                NeName##SortFilterProxyModel> {} { \
      return;                                                      \
   }                                                               \
   BtComboBox##NeName::~BtComboBox##NeName() = default;            \

#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Fermentation.h"
#include "model/Style.h"
#include "model/Water.h"

#include "qtModels/listModels/BoilListModel.h"
#include "qtModels/listModels/EquipmentListModel.h"
#include "qtModels/listModels/MashListModel.h"
#include "qtModels/listModels/FermentationListModel.h"
#include "qtModels/listModels/StyleListModel.h"
#include "qtModels/listModels/WaterListModel.h"

#include "qtModels/sortFilterProxyModels/BoilSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/EquipmentSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/MashSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/FermentationSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.h"

BT_COMBO_BOX_NAMED_ENTITY_DECL(Boil        )
BT_COMBO_BOX_NAMED_ENTITY_DECL(Equipment   )
BT_COMBO_BOX_NAMED_ENTITY_DECL(Mash        )
BT_COMBO_BOX_NAMED_ENTITY_DECL(Fermentation)
BT_COMBO_BOX_NAMED_ENTITY_DECL(Style       )
BT_COMBO_BOX_NAMED_ENTITY_DECL(Water       )

#endif
