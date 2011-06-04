/*
 * BrewTargetTreeModel.h is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include <Qt>
#include <QObject>

#include "recipe.h"
#include "BrewTargetTreeItem.h"
#include "BrewTargetTreeView.h"
#include "database.h"
#include "observable.h"
#include "brewnote.h"

class BrewTargetTreeModel : public QAbstractItemModel, public MultipleObserver
{
   Q_OBJECT;

public:
   BrewTargetTreeModel(BrewTargetTreeView *parent = 0);
   virtual ~BrewTargetTreeModel();

   // Methods required for read-only stuff
   QVariant data(const QModelIndex &index, int role) const;
   QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   Qt::ItemFlags flags( const QModelIndex &index) const;
   int rowCount( const QModelIndex &parent = QModelIndex()) const;
   int columnCount( const QModelIndex &index = QModelIndex()) const;

    // Methods required for tree views (odd we are worried about the view in
    // the model).
   QModelIndex index( int row, int col, const QModelIndex &parent = QModelIndex()) const;
   QModelIndex parent( const QModelIndex &index) const;


   QModelIndex getFirst(int type);
   // Methods required for read-write access.  We are not implementing adding
    // or removing columns because that doesn't make sense for this model.
   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

   // Good stuff to have.  Like a Ruination clone happily dry hopping
   bool isRecipe(const QModelIndex &index);
   bool isEquipment(const QModelIndex &index);
   bool isFermentable(const QModelIndex &index);
   bool isHop(const QModelIndex &index);
   bool isMisc(const QModelIndex &index);
   bool isYeast(const QModelIndex &index);
   bool isBrewNote(const QModelIndex &index);

   int getType(const QModelIndex &index);

   // Methods required for observable
   virtual void notify(Observable *notifier, QVariant info = QVariant());
   void startObservingDB();

   // Convenience functions to make the rest of the software play nice
   Recipe* getRecipe(const QModelIndex &index) const;
   Equipment* getEquipment(const QModelIndex &index) const;
   Fermentable* getFermentable(const QModelIndex &index) const;
   Hop* getHop(const QModelIndex &index) const;
   Misc* getMisc(const QModelIndex &index) const;
   Yeast* getYeast(const QModelIndex &index) const;
   BrewNote* getBrewNote(const QModelIndex &index) const;

   QModelIndex findRecipe(Recipe* rec);
   QModelIndex findEquipment(Equipment* kit);
   QModelIndex findFermentable(Fermentable* ferm);
   QModelIndex findHop(Hop* hop);
   QModelIndex findMisc(Misc* misc);
   QModelIndex findYeast(Yeast* yeast);
   QModelIndex findBrewNote(BrewNote* bNote);

private:
   BrewTargetTreeItem *getItem(const QModelIndex &index) const;
   void loadTreeModel(int reload);
   void unloadTreeModel(int unload);

   BrewTargetTreeItem* rootItem;
   BrewTargetTreeView *parentTree;
   Database* dbObs;
   Recipe* recObs;
};

#endif /* RECEIPTREEMODEL_H_ */
