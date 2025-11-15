/*======================================================================================================================
 * utils/ColumnInfo.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef UTILS_COLUMNINFO_H
#define UTILS_COLUMNINFO_H
#pragma once

#include <QString>

#include "measurement/PhysicalQuantity.h"
#include "measurement/SystemOfMeasurement.h"
#include "measurement/UnitSystem.h"
#include "utils/PropertyPath.h"
#include "utils/TypeInfo.h"

/**
 * \class ColumnInfo
 *
 * \brief This per-column struct / mini-class holds basic info about each column in a table (see \c BtTableModel
 *        subclasses) or a tree (see \c TreeModel subclasses).  It also plays a slightly similar role as \c SmartLabel.
 *        However, there are several important differences, including that \c ColumnInfo is \b not a \c QWidget and
 *        therefore not a signal emitter.
 *
 *        As mentioned below, it is \c QHeaderView that sends us the signal about the user having right-clicked on a
 *        column header.  We then act on the pop-up menu selections directly, rather than \c SmartLabel sending a
 *        signal that \c SmartLineEdit (and sometimes others) pick up.
 *
 *        NOTE that you usually want to use the \c TABLE_MODEL_HEADER or \c TREE_NODE_HEADER macro when constructing
 */
struct ColumnInfo {
   /**
    * \brief By analogy with \c editorName in \c SmartLabel and \c SmartLineEdit
    */
   char const * const modelName;

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

///private:
///   /**
///    * \brief The localised text to display in this column header
///    *
///    *        TBD: We probably ought to be able to get this from \c typeInfo, but need to check how we're going to handle
///    *             property paths such as {PropertyNames::Recipe::style, PropertyNames::NamedEntity::name} (which we
///    *             would want to show as "Style" rather than "Name").  I think the answer is either that we have an
///    *             optional label.  We probably don't want to populate the optional label directly.  Rather, for a non-
///    *             trivial property path, we specify the index of the path element to use as the name -- eg on a StockUse
///    *             subclass, in {PropertyNames::StockUse::brewNote, PropertyNames::OwnedByRecipe::recipe,
///    *             PropertyNames::NamedEntity::name} we'd want the middle element of the path.
///    */
///   QString const label;
///public:

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
   using Extras = std::optional<Measurement::ChoiceOfPhysicalQuantity>;
   Extras extras;

   ColumnInfo(char const * const   modelName   ,
              char const * const   columnName  ,
              char const * const   columnFqName,
              size_t       const   index       ,
///              QString      const   label       ,
              TypeLookup   const & typeLookup  ,
              PropertyPath         propertyPath,
              Extras       const   extras = std::nullopt);
   ~ColumnInfo();

   // Stuff for setting display units and scales -- per column
   // I know it looks odd to have const setters, but they are const because they do not change the data in the struct
   void setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement) const;
   void setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const;
   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement() const;
   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale() const;
};

/**
 * \brief This macro saves a bit of copy-and-paste when calling the \c ColumnInfo constructor.  Eg instead
 *        of writing:
 *
 *           ColumnInfo{"HopTableModel",
 *                      "Alpha",
 *                      "HopTableModel::ColumnIndex::Alpha",
 *                      static_cast<size_t>(HopTableModel::ColumnIndex::Alpha),
 *                      tr("Alpha %"),
 *                      Hop::typeLookup.getType(PropertyNames::Hop::alpha)}
 *
 *        you write:
 *
 *           TABLE_MODEL_HEADER(Hop, Alpha, tr("Alpha %"), PropertyNames::Hop::alpha);
 *
 *        Arguments not specified below are passed in to initialise \c ColumnInfo::propertyPath and, if present,
 *        \c ColumnInfo::extras.
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
#define TABLE_MODEL_HEADER(modelClass, columnName, /* labelText, */ ...) \
   ColumnInfo{#modelClass "TableModel", \
              #columnName, \
              #modelClass "TableModel::ColumnIndex::" #columnName, \
              static_cast<size_t>(modelClass##TableModel::ColumnIndex::columnName), \
              /* modelClass::labelText, */ \
              modelClass::typeLookup, \
              __VA_ARGS__}

/**
 * \brief Same as TABLE_MODEL_HEADER but for use with \c TreeNode subclasses rather than \c BtTableModel subclasses
 */
#define TREE_NODE_HEADER(nodeType, modelClass, columnName, /* labelText, */ ...) \
   ColumnInfo{#nodeType "<" #modelClass ">", \
              #columnName, \
              #nodeType "<" #modelClass ">::ColumnIndex::" #columnName, \
              static_cast<size_t>(nodeType<modelClass>::ColumnIndex::columnName), \
              /* modelClass::labelText, */ \
              modelClass::typeLookup, \
              __VA_ARGS__}

#endif
