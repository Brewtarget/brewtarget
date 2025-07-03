/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/listModels/ListModelBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef LISTMODELS_LISTMODELBASE_H
#define LISTMODELS_LISTMODELBASE_H
#pragma once

#include <memory>

#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/Recipe.h"
#include "database/ObjectStoreWrapper.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief Curiously Recurring Template Pattern (CRTP) base class for EquipmentListModel, StyleListModel, etc
 *
 *           QAbstractListModel     ListModelBase<StyleListModel, Style>
 *                           \            /
 *                            \          /
 *                           StyleListModel
 *
 *        Note that, although Qt is sufficiently flexible to allow you to use \c QAbstractListModel to build tables, we
 *        stick to \c QAbstractTableModel for that.  We only use \c QAbstractListModel for building lists of names of
 *        things.  Currently this is for the benefit of \c BtComboBoxNamedEntity subclasses.
 */
template<class Derived> class ListModelPhantom;
template<class Derived, class NE>
class ListModelBase : public CuriouslyRecurringTemplateBase<ListModelPhantom, Derived> {
public:
   ListModelBase() :
      m_items{},
      m_recipe{nullptr} {
      this->derived().connect(&ObjectStoreTyped<NE>::getInstance(), &ObjectStoreTyped<NE>::signalObjectInserted, &this->derived(), &Derived::addItem);
      this->derived().connect(&ObjectStoreTyped<NE>::getInstance(), &ObjectStoreTyped<NE>::signalObjectDeleted , &this->derived(), &Derived::removeItem);
      return;
   }

   //! \brief Add items to the list model
   void addItems(QList<NE *> items) {
      QList<NE *> tmp;
      for (NE * ii : items) {
         // if the item is not already in the list and
         // if the item has not been deleted, then append it
         if (!m_items.contains(ii) && !ii->deleted()) {
            tmp.append(ii);
         }
      }

      if (tmp.size() > 0) {
         int size = m_items.size();
         this->derived().beginInsertRows(QModelIndex(), size, size + tmp.size());
         m_items.append(tmp);

         for (NE * ii : tmp) {
            this->derived().connect(ii, &NamedEntity::changed, &this->derived(), &Derived::itemChanged);
         }

         this->derived().endInsertRows();
      }
      return;
   }

   //! \brief Remove all items from the list model
   void removeAll() {
      if (m_items.size() > 0) {
         this->derived().beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
         while (!m_items.isEmpty()) {
            this->derived().disconnect(m_items.takeLast(), nullptr, &this->derived(), nullptr);
         }
         this->derived().endRemoveRows();
      }
      return;
   }

   //! \return the item at \c ndx
   NE * at(int ndx) const {
      if (ndx >= 0 && ndx < m_items.size()) {
         return m_items[ndx];
      }
      return nullptr;
   }

   //! \return the index of the specified item
   [[deprecated]] int indexOf(NE * item) const {
      return m_items.indexOf(item);
   }

   //! \return the index of the specified item
   QModelIndex find(NE * item) const {
      int indx = m_items.indexOf(item);
      if (indx < 0) {
         return QModelIndex();
      }

      return this->derived().index(indx, 0);
   }

   void remove(NE * item) {
      int ndx = m_items.indexOf(item);
      if (ndx >= 0) {
         this->derived().beginRemoveRows(QModelIndex(), ndx, ndx);
         this->derived().disconnect(item, nullptr, &this->derived(), nullptr);
         m_items.removeAt(ndx);
         this->derived().endRemoveRows();
      }
      return;
   }

   void observeRecipe(Recipe * rec) {
      if (m_recipe) {
         this->derived().disconnect(m_recipe, nullptr, &this->derived(), nullptr);
      }
      m_recipe = rec;

      if (m_recipe )
         this->derived().connect(m_recipe, &NamedEntity::changed, &this->derived(), &Derived::recipeChanged);
      return;
   }

protected:

   int doRowCount([[maybe_unused]] QModelIndex const & parent) const {
      return m_items.size();
   }

   QVariant doData(QModelIndex const & index, int role) const {
      if (index.column() == 0) {
         //
         // See https://doc.qt.io/qt-6/qt.html#ItemDataRole-enum for more on Qt::ItemDataRole.  For our purposes:
         //
         //    Qt::DisplayRole = we want the name of the stored object (to show on the screen)
         //    Qt::UserRole    = we want the ID of the stored object (to uniquely identify it)
         //
         if (role == Qt::DisplayRole) {
            return QVariant(m_items.at(index.row())->name());
         } else if (role == Qt::UserRole) {
            return QVariant(m_items.at(index.row())->key());
         }
      }
      return QVariant();
   }

   QVariant doHeaderData([[maybe_unused]] int section,
                         [[maybe_unused]] Qt::Orientation orientation,
                         [[maybe_unused]] int role) const {
      return QVariant(QString("Header Data..."));
   }

   void doItemChanged(QMetaProperty prop, [[maybe_unused]] QVariant val) {
      NE * neSender = qobject_cast<NE *>(this->derived().sender());
      if (!neSender) {
         return;
      }

      QString propName(prop.name());
      if (propName == PropertyNames::NamedEntity::name) {
         int ndx = m_items.indexOf(neSender);
         if (ndx >= 0 ) {
            this->derived().emit dataChanged(this->derived().createIndex(ndx, 0),
                                        this->derived().createIndex(ndx, 0));
         }
      }
      return;
   }

   void doAddItem(int itemId) {
      qDebug() << Q_FUNC_INFO << "New" << NE::staticMetaObject.className() << "#" << itemId;
      NE * ne = ObjectStoreWrapper::getByIdRaw<NE>(itemId);
      if (!ne || ne->deleted()) {
         return;
      }

      if (!m_items.contains(ne) ) {
         int size = m_items.size();
         this->derived().beginInsertRows(QModelIndex(), size, size);
         m_items.append(ne);
         this->derived().connect(ne, &NamedEntity::changed, &this->derived(), &Derived::itemChanged );
         this->derived().endInsertRows();
      }
      return;
   }

   void doRemoveItem([[maybe_unused]] int itemId,
                     std::shared_ptr<QObject> object) {
      NE * item = std::static_pointer_cast<NE>(object).get();
      this->remove(item);
      return;
   }

   void doRecipeChanged(QMetaProperty prop, QVariant val, BtStringConst const & propNameInRecipe) {
      if (prop.name() == propNameInRecipe) {
         NE * newItem = val.value<NE *>();
         // .:TODO:. Now do something with the changed item
         Q_UNUSED(newItem); // Until then, this will keep the compiler happy
      }
      return;
   }

private:
   QList<NE *> m_items ;
   Recipe *    m_recipe;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define LIST_MODEL_COMMON_DECL(NeName)                                                                      \
   /* This allows ListModelBase to call protected and private members of Derived */                         \
   friend class ListModelBase<NeName##ListModel, NeName>;                                                   \
                                                                                                            \
   public:                                                                                                  \
      NeName##ListModel(QWidget * parent = nullptr);                                                        \
      virtual ~NeName##ListModel();                                                                         \
                                                                                                            \
   /** Reimplemented from QAbstractListModel. */                                                            \
   virtual int rowCount(QModelIndex const & parent = QModelIndex()) const;                                  \
   /** Reimplemented from QAbstractListModel. */                                                            \
   virtual QVariant data(QModelIndex const & index, int role = Qt::DisplayRole) const;                      \
   /** Reimplemented from QAbstractListModel. */                                                            \
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; \
                                                                                                            \
   public slots:                                                                                            \
      void itemChanged(QMetaProperty prop, QVariant val);                                                   \
      void addItem(int itemId);                                                                             \
      void removeItem(int itemId, std::shared_ptr<QObject> object);                                         \
      void recipeChanged(QMetaProperty prop, QVariant val);                                                 \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 */
#define LIST_MODEL_COMMON_CODE(NeName, RecipePropertyName)                                            \
   NeName##ListModel::NeName##ListModel(QWidget * parent) :                                           \
      QAbstractListModel(parent),                                                                     \
      ListModelBase<NeName##ListModel, NeName>() {                                                    \
      /* Note that the following line cannot be moved to the ListModelBase constructor as, if we */   \
      /* do, we'll get a "pure virtual method called" error during the execution of              */   \
      /* this->derived().beginInsertRows.  This will be because                                  */   \
      /* QAbstractItemModel::beginInsertRows calls a virtual function but the mechanism for      */   \
      /* making virtual calls (often a vtable) on an object is not guaranteed to be set up until */   \
      /* all the object's base class constructors have been executed.                            */   \
      this->addItems(ObjectStoreTyped<NeName>::getInstance().getAllRaw());                            \
      return;                                                                                         \
   }                                                                                                  \
   NeName##ListModel::~NeName##ListModel() = default;                                                 \
   int NeName##ListModel::rowCount(QModelIndex const & parent) const {                                \
      return this->doRowCount(parent);                                                                \
   }                                                                                                  \
   QVariant NeName##ListModel::data(QModelIndex const & index, int role) const {                      \
      return this->doData(index, role);                                                               \
   }                                                                                                  \
   QVariant NeName##ListModel::headerData(int section, Qt::Orientation orientation, int role) const { \
      return this->doHeaderData(section, orientation, role);                                          \
   }                                                                                                  \
   void NeName##ListModel::itemChanged(QMetaProperty prop, QVariant val) {                            \
      this->doItemChanged(prop, val);                                                                 \
      return;                                                                                         \
   }                                                                                                  \
   void NeName##ListModel::addItem(int itemId) {                                                      \
      this->doAddItem(itemId);                                                                        \
      return;                                                                                         \
   }                                                                                                  \
   void NeName##ListModel::removeItem(int itemId, std::shared_ptr<QObject> object) {                  \
      this->doRemoveItem(itemId, object);                                                             \
      return;                                                                                         \
   }                                                                                                  \
   void NeName##ListModel::recipeChanged(QMetaProperty prop, QVariant val) {                          \
      this->doRecipeChanged(prop, val, RecipePropertyName);                                           \
      return;                                                                                         \
   }                                                                                                  \

#endif
