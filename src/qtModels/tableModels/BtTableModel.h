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
#include <vector>

#include <QAbstractTableModel>
#include <QDebug>
#include <QHeaderView>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QTableView>

#include "BtFieldType.h"
#include "measurement/UnitSystem.h"
#include "model/NamedEntity.h"
#include "utils/BtStringConst.h"
#include "utils/EnumStringMapping.h"
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
class BtTableModel : public QAbstractTableModel {
   Q_OBJECT

   // There are quite a few protected members of this class (including many inherited from QAbstractItemModel) that
   // TableModelBase needs to access (to save copy-and-pasting code in all the derived classes.  It's better to make it
   // a friend than make all those members public.
   template<class Derived, class NE> friend class TableModelBase;

public:
   /**
    * \brief Extra info stored in \c ColumnInfo (see below) for enum types
    */
   struct EnumInfo {
      /**
       * \brief Values to store in combo box
       */
      EnumStringMapping const & stringMapping;

      /**
       * \brief Localised display names to show on combo box
       */
      EnumStringMapping const & displayNames;
   };

   /**
    * \brief Extra info stored in \c ColumnInfo (see below) for boolean types
    *
    *        For most boolean types, we show a combo box (eg "Not mashed" / "Mashed"; "Weight" / "Volume")
    *
    *        These are QString rather than reference to QString as typically initialised with an rvalue (the result of
    *        calling \c QObject::tr).
    */
   struct BoolInfo {
      QString const unsetDisplay;
      QString const setDisplay;
   };

   /**
    * \brief I know we don't need a struct for this, but it's more consistent to use one
    */
   struct PrecisionInfo {
      unsigned int const precision;
   };

   /**
    * \brief This per-column struct / mini-class holds basic info about each column in the table.  It also plays a
    *        slightly similar role as \c SmartLabel.  However, there are several important differences, including that
    *        \c ColumnInfo is \b not a \c QWidget and therefore not a signal emitter.
    *
    *        As mentioned below, it is \c QHeaderView that sends us the signal about the user having right-clicked on a
    *        column header.  We then act on the pop-up menu selections directly, rather than \c SmartLabel sending a
    *        signal that \c SmartLineEdit (and sometimes others) pick up.
    *
    *        NOTE that you usually want to use the TABLE_MODEL_HEADER macro when constructing
    */
   struct ColumnInfo {
      /**
       * \brief By analogy with \c editorName in \c SmartLabel and \c SmartLineEdit
       */
      char const * const tableModelName;

      /**
       * \brief By analogy with \c labelName in \c SmartLabel and \c lineEditName in \c SmartLineEdit
       */
      char const * const columnName;

      /**
       * \brief By analogy with \c labelFqName in \c SmartLabel and \c lineEditFqName in \c SmartLineEdit
       */
      char const * const columnFqName;

      /**
       * \brief Each subclass should normally declare its own \c enum \c class \c ColumnIndex to identify its columns.
       *        We store the column index here as a cross-check that we've got everything in the right order.
       */
      size_t const index;

      /**
       * \brief The localised text to display in this column header
       */
      QString const label;

      /**
       * \brief What type of data is shown in this column
       */
      TypeInfo const & typeInfo;

      /**
       * \brief Parameters to construct the \c PropertyPath to which this field relates.  Usually this is just a
       *        property name, eg \c PropertyNames::NamedEntity::name.  But, eg, in \c RecipeAdditionHopTableModel and
       *        similar places, we need `{PropertyNames::RecipeAdditionHop::hop, PropertyNames::Hop::alpha_pct}`, etc.
       */
      PropertyPath propertyPath;

      /**
       * \brief Extra info, dependent on the type of value in the column
       *
       *        Note that we use a \c Measurement::ChoiceOfPhysicalQuantity value for an "amount type" column for
       *        selecting a physical quantity for a paired column (with the same property path).
       */
      using Extras = std::optional<std::variant<EnumInfo,
                                                BoolInfo,
                                                PrecisionInfo,
                                                Measurement::ChoiceOfPhysicalQuantity>>;
      Extras extras;

      ColumnInfo(char const * const   tableModelName,
                 char const * const   columnName    ,
                 char const * const   columnFqName  ,
                 size_t       const   index         ,
                 QString      const   label         ,
                 TypeLookup   const & typeLookup    ,
                 PropertyPath         propertyPath  ,
                 Extras       const   extras = std::nullopt) :
         tableModelName{tableModelName                      },
         columnName    {columnName                          },
         columnFqName  {columnFqName                        },
         index         {index                               },
         label         {label                               },
         typeInfo      {propertyPath.getTypeInfo(typeLookup)},
         propertyPath  {propertyPath                        },
         extras        {extras                              } {
         return;
      }

      // Stuff for setting display units and scales -- per column
      // I know it looks odd to have const setters, but they are const because they do not change the data in the struct
      void setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement) const;
      void setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const;
      std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement() const;
      std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale() const;
   };

   /**
    * \brief
    *
    * \param parent
    * \param editable
    * \param columnInfos Needs to be in order
    */
   BtTableModel(QTableView * parent,
                bool editable,
                std::initializer_list<ColumnInfo> columnInfos);
   virtual ~BtTableModel();

   ColumnInfo const & getColumnInfo(size_t const columnIndex) const;

   //! \brief Called from \c headerData()
   QVariant getColumnLabel(size_t const columnIndex) const;

   // Per https://doc.qt.io/qt-5/qabstracttablemodel.html, when subclassing QAbstractTableModel, you must implement
   // rowCount(), columnCount(), and data(). Default implementations of the index() and parent() functions are provided
   // by QAbstractTableModel. Well behaved models will also implement headerData().

   //! \brief Reimplemented from QAbstractTableModel
   virtual int columnCount(QModelIndex const & parent = QModelIndex()) const;

   /**
    * \brief Reimplemented from QAbstractTableModel
    *
    *        This handles the simple case of static horizontal headers.  For more complex headers, child classes should
    *        override this member function.
    */
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
   //! \brief Receives the \c QWidget::customContextMenuRequested signal from \c QHeaderView to pops the context menu
   // for changing units and scales
   void contextMenu(QPoint const & point);

protected:
   QTableView * m_parentTableWidget;
   bool m_editable;
private:
   /**
    * \brief We're using a \c std::vector here because it's easier for constant lists.  (With \c QVector, at last in
    *        Qt 5, the items stored even in a const instance still need to be default constructable and copyable.)
    */
   std::vector<ColumnInfo> const m_columnInfos;
};

class BtTableModelRecipeObserver : public BtTableModel {
public:
   BtTableModelRecipeObserver(QTableView * parent,
                              bool editable,
                              std::initializer_list<ColumnInfo> columnInfos);
   ~BtTableModelRecipeObserver();

   /**
    * \brief The \c Recipe, if any, in which this table's objects (eg RecipeAdditionHop objects) are used.
    *
    *        Normally this would be protected, but it needs to be public for TableModelBase to access
    */
   Recipe * recObs;
};

/**
 * \brief This macro saves a bit of copy-and-paste when calling the \c BtTableModel::ColumnInfo constructor.  Eg instead
 *        of writing:
 *
 *           BtTableModel::ColumnInfo{"HopTableModel",
 *                                    "Alpha",
 *                                    "HopTableModel::ColumnIndex::Alpha",
 *                                    static_cast<size_t>(HopTableModel::ColumnIndex::Alpha),
 *                                    tr("Alpha %"),
 *                                    Hop::typeLookup.getType(PropertyNames::Hop::alpha)}
 *
 *        you write:
 *
 *           TABLE_MODEL_HEADER(HopTableModel, Alpha, tr("Alpha %"), PropertyNames::Hop::alpha);
 *
 *        Arguments not specified below are passed in to initialise \c BtTableModel::ColumnInfo::propertyPath and, if
 *        present, \c BtTableModel::ColumnInfo::extras.
 *
 * \param tableModelClass The class name of the class holding the field we're initialising, eg \c HopTableModel.
 * \param columnName
 * \param labelText
 * \param modelClass The subclass of \c NamedEntity that we're editing.  Eg in \c HopEditor, this will be \c Hop
 *
 * Note that for the ... arguments, if they are non-trivial, we need to explicitly call the relevant constructor, eg we
 * must write `PropertyPath{{PropertyNames::RecipeAdditionHop::hop, PropertyNames::Hop::alpha_pct}}` instead of just
 * `{PropertyNames::RecipeAdditionHop::hop, PropertyNames::Hop::alpha_pct}` as we can in some other places.  (For
 * trivial parameters, such as `PropertyNames::NamedEntity::name`, we don't need to do this.)  This is the price of
 * going via a macro.
 *
 * Note too, unlike some other places, because at least one extra non-named parameter is always required, we don't need
 * the `__VA_OPT__(, )` wrapper around __VA_ARGS__.
 */
#define TABLE_MODEL_HEADER(modelClass, columnName, labelText, ...) \
   BtTableModel::ColumnInfo{#modelClass "TableModel", \
                            #columnName, \
                            #modelClass "TableModel::ColumnIndex::" #columnName, \
                            static_cast<size_t>(modelClass##TableModel::ColumnIndex::columnName), \
                            labelText, \
                            modelClass::typeLookup, \
                            __VA_ARGS__}

#endif
