/*
 * BrewTargetTreeModel.h is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RECEIPTREEMODEL_H_
#define RECEIPTREEMODEL_H_

class BrewTargetTreeModel;

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <QMetaProperty>
#include <QVariant>
#include <QObject>
#include <QSqlRelationalTableModel>

// Forward declarations
class BeerXMLElement;
class Recipe;
class BrewTargetTreeItem;
class BrewTargetTreeView;
class BrewNote;
class Equipment;
class Fermentable;
class Hop;
class Misc;
class Yeast;

/*!
 * \class BrewTargetTreeModel
 * \author Mik Firestone
 *
 * \brief Model for a tree of Recipes, Equipments, Fermentables, Hops, Miscs, Yeasts, and BrewNotes.
 */
class BrewTargetTreeModel : public QAbstractItemModel
{
   Q_OBJECT

public:
   //! \brief Describes what items this tree will show.
   enum TypeMasks
   {
      //! Show recipes
      RECIPEMASK        = 1,
      //! Show equipments
      EQUIPMASK         = 2,
      //! Show fermentables
      FERMENTMASK       = 4,
      //! Show hops
      HOPMASK           = 8,
      //! Show miscs
      MISCMASK          = 16,
      //! Show yeasts
      YEASTMASK         = 32,
      //! Show brewnotes
      BREWNOTEMASK      = 64,
      //! Show everything
      ALLMASK           = 127
   };
   
   BrewTargetTreeModel(BrewTargetTreeView *parent = 0, TypeMasks type = ALLMASK);
   virtual ~BrewTargetTreeModel();
   
   //! \brief Reimplemented from QAbstractItemModel
   virtual QVariant data(const QModelIndex &index, int role) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual Qt::ItemFlags flags( const QModelIndex &index) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual int rowCount( const QModelIndex &parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual int columnCount( const QModelIndex &index = QModelIndex()) const;

   //! \brief Reimplemented from QAbstractItemModel
   virtual QModelIndex index( int row, int col, const QModelIndex &parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual QModelIndex parent( const QModelIndex &index) const;

   //! \brief Reimplemented from QAbstractItemModel
   bool insertRow(int row, const QModelIndex &parent = QModelIndex(), QObject* victim = 0, int victimType = -1);
   //! \brief Reimplemented from QAbstractItemModel
   virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

   //! \brief Get the upper-left index for the given \c type.
   QModelIndex getFirst(int type);

   //! \brief Test type at \c index.
   bool isRecipe(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isEquipment(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isFermentable(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isHop(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isMisc(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isYeast(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isBrewNote(const QModelIndex &index);

   //! \brief Test type at \c index.
   int getType(const QModelIndex &index);
   //! \brief Return the type mask for this tree. \sa BrewTargetTreeModel::TypeMasks
   int getMask();

   //! \brief Get Recipe at \c index.
   Recipe* getRecipe(const QModelIndex &index) const;
   //! \brief Get Equipment at \c index.
   Equipment* getEquipment(const QModelIndex &index) const;
   //! \brief Get Fermentable at \c index.
   Fermentable* getFermentable(const QModelIndex &index) const;
   //! \brief Get Hop at \c index.
   Hop* getHop(const QModelIndex &index) const;
   //! \brief Get Misc at \c index.
   Misc* getMisc(const QModelIndex &index) const;
   //! \brief Get Yeast at \c index.
   Yeast* getYeast(const QModelIndex &index) const;
   //! \brief Get BrewNote at \c index.
   BrewNote* getBrewNote(const QModelIndex &index) const;

   //! \brief Get index of \c rec.
   QModelIndex findRecipe(Recipe* rec);
   //! \brief Get index of \c kit.
   QModelIndex findEquipment(Equipment* kit);
   //! \brief Get index of \c ferm.
   QModelIndex findFermentable(Fermentable* ferm);
   //! \brief Get index of \c hop.
   QModelIndex findHop(Hop* hop);
   //! \brief Get index of \c misc.
   QModelIndex findMisc(Misc* misc);
   //! \brief Get index of \c yeast.
   QModelIndex findYeast(Yeast* yeast);
   //! \brief Get index of \c bNote.
   QModelIndex findBrewNote(BrewNote* bNote);

private slots:
   // Called when new items are added to database.
   void equipmentAdded(Equipment* victim);
   void fermentableAdded(Fermentable* victim);
   void hopAdded(Hop* victim);
   void miscAdded(Misc* victim);
   void recipeAdded(Recipe* victim);
   void yeastAdded(Yeast* victim);
   void brewNoteAdded(BrewNote* victim);
   
   // Called when one of our items change.
   void equipmentChanged();
   void fermentableChanged();
   void hopChanged();
   void miscChanged();
   void recipeChanged();
   void yeastChanged();
   void brewNoteChanged();
   
   // Called when an item is removed from the database.
   void equipmentRemoved(Equipment* victim);
   void fermentableRemoved(Fermentable* victim);
   void hopRemoved(Hop* victim);
   void miscRemoved(Misc* victim);
   void recipeRemoved(Recipe* victim);
   void yeastRemoved(Yeast* victim);
   void brewNoteRemoved(BrewNote* victim);

private:
   BrewTargetTreeItem *getItem(const QModelIndex &index) const;
   // Loads the data. Empty \b propname means load all trees.
   void loadTreeModel(QString propName = "");
   // Unloads the data. Empty \b propname means unload all trees.
   void unloadTreeModel(QString propName = "");
   
   // Helpers to connect signals in the underlying data to model slots that catch
   // and re-emit model changes.
   void observeEquipment(Equipment*);
   void observeFermentable(Fermentable*);
   void observeHop(Hop*);
   void observeMisc(Misc*);
   void observeRecipe(Recipe*);
   void observeYeast(Yeast*);
   void observeBrewNote(BrewNote*);
   
   // Helper methods for recipe headers
   QVariant getRecipeHeader(int section) const;
   QVariant getEquipmentHeader(int section) const;
   QVariant getFermentableHeader(int section) const;
   QVariant getHopHeader(int section) const;
   QVariant getMiscHeader(int section) const;
   QVariant getYeastHeader(int section) const;
   
   BrewTargetTreeItem* rootItem;
   QHash<TypeMasks, int> trees;
   BrewTargetTreeView *parentTree;
   TypeMasks treeMask;

};

#endif /* RECEIPTREEMODEL_H_ */
