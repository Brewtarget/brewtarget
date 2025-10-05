/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/BtTableModel.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef TABLEMODELS_BTTABLEMODEL_H
#define TABLEMODELS_BTTABLEMODEL_H
#pragma once

#include <memory>
#include <optional>

#include <QAbstractTableModel>
#include <QDebug>
#include <QHeaderView>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QTableView>

#include "measurement/UnitSystem.h"
#include "model/NamedEntity.h"
#include "utils/BtStringConst.h"
#include "utils/ColumnInfo.h"
#include "utils/EnumStringMapping.h"
#include "utils/NoCopy.h"
#include "utils/PropertyPath.h"

class Recipe;
template<class Derived, class NE> class TableModelBase; // This forward declaration is so we can make it a friend

/**
 * \class BtTableModel
 *
 * \brief Typically used to show a grid list of \c Hop or \c Fermentable, etc in one of two contexts:
 *           * Listing all items of this type in the database
 *           * Listing all items of this type used in the current \c Recipe
 *        The grid list shows certain attributes of the item (which can usually be edited in place) and will launch the
 *        relevant editor (eg \c HopEditor, \c FermentableEditor, etc) when you double-click on the name.
 *
 *        Unfortunately we can't template \c BtTableModel because it inherits from a \c QObject and the Qt meta-object
 *        compiler (moc) can't handle templated classes in QObject-derived classes (though it is fine with templated
 *        member functions in such classes, as long as they are not signals or slots).  We might one day look at
 *        https://github.com/woboq/verdigris, which overcomes these limitations, but, for now, we live within Qt's
 *        constraints and try to pull out as much common code as possible using a limited form of multiple inheritance
 *        and the Curiously Recurring Template Pattern.
 *
 *              QObject
 *                   \
 *                   ...
 *                     \
 *              QAbstractTableModel
 *                           \
 *                            \
 *                          BtTableModel                TableModelBase<NE, xxxTableModel>
 *                                /   \                 /        /
 *                               /     \               /        /
 *                              /   FermentableTableModel      /
 *                             /        HopTableModel         /
 *                            /      MashStepTableModel      /
 *                           /         MiscTableModel       /
 *                          /         YeastTableModel      /
 *                         /           SaltTableModel     /
 *                        /           WaterTableModel    /
 *                       /                              /
 *              BtTableModelRecipeObserver             /
 *                                    \               /
 *                                     \             /
 *                     RecipeAdditionFermentableTableModel
 *                        RecipeAdditionHopTableModel
 *                        RecipeAdditionMiscTableModel
 *                        RecipeAdditionYeastTableModel
 *
 *
 *        Eg RecipeAdditionHopTableModel inherits from BtTableModelRecipeObserver and
 *        TableModelBase<RecipeAdditionHopTableModel, RecipeAdditionHop>
 *
 *        The \c BtTableModelRecipeObserver class adds a pointer to a \c Recipe to \c BtTableModel.
 *
 *        You might be wondering why there isn't a corresponding inheritance hierarchy in \c TableModelBase.  This is
 *        because, by the magic of template metaprogramming \c TableModelBase "knows" whether its derived class inherits
 *        from \c BtTableModelRecipeObserver or directly from \c BtTableModel, and can therefore adapt accordingly.
 */
class BtTableModel : public QAbstractTableModel/*, public ColumnOwner*/ {
   Q_OBJECT

   // There are quite a few protected members of this class (including many inherited from QAbstractItemModel) that
   // TableModelBase needs to access (to save copy-and-pasting code in all the derived classes.  It's better to make it
   // a friend than make all those members public.
   template<class Derived, class NE> friend class TableModelBase;

public:

   /**
    * \brief
    *
    * \param parent
    * \param editable
    * \param columnInfos Needs to be in order
    */
   BtTableModel(QTableView * parent,
                bool editable/*,
                std::initializer_list<ColumnInfo> columnInfos*/);
   virtual ~BtTableModel();

   // Per https://doc.qt.io/qt-5/qabstracttablemodel.html, when subclassing QAbstractTableModel, you must implement
   // rowCount(), columnCount(), and data(). Default implementations of the index() and parent() functions are provided
   // by QAbstractTableModel. Well behaved models will also implement headerData().

   //! \brief Child classes must override this member function of \c QAbstractTableModel
   virtual int columnCount(QModelIndex const & parent = QModelIndex()) const override = 0;

   //! \brief Child classes must implement this
   virtual QVariant columnLabel(int section) const = 0;

   //! \brief Child classes must implement this
   virtual ColumnInfo columnInfo(int section) const = 0;

   /**
    * \brief Reimplemented from \c QAbstractTableModel
    *
    *        This handles the simple case of static horizontal headers.  For more complex headers, child classes should
    *        override this member function.
    */
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
   /**
    * \brief Receives the \c QWidget::customContextMenuRequested signal from \c QHeaderView to pops the context menu
    *        for changing units and scales
    */
   void contextMenu(QPoint const & point);

private:
   // Insert all the usual boilerplate to prevent copy/assignment/move
   NO_COPY_DECLARATIONS(BtTableModel)

protected:
   QTableView * m_parentTableWidget;
   bool m_editable;
};

/**
 * \class BtTableModelRecipeObserver
 */
class BtTableModelRecipeObserver : public BtTableModel {
public:
   BtTableModelRecipeObserver(QTableView * parent,
                              bool editable/*,
                              std::initializer_list<ColumnInfo> columnInfos*/);
   ~BtTableModelRecipeObserver();

   /**
    * \brief The \c Recipe, if any, in which this table's objects (eg RecipeAdditionHop objects) are used.
    *
    *        Normally this would be protected, but it needs to be public for TableModelBase to access
    */
   Recipe * recObs;
};

#endif
