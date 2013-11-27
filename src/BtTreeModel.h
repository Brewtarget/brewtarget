/*
 * BtTreeModel.h is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#ifndef BTTREEMODEL_H_
#define BTTREEMODEL_H_

class BtTreeModel;

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
class BtFolder;
class BtTreeItem;
class BtTreeView;
class BrewNote;
class Equipment;
class Fermentable;
class Hop;
class Misc;
class Yeast;
class Style;

/*!
 * \class BtTreeModel
 * \author Mik Firestone
 *
 * \brief Model for a tree of Recipes, Equipments, Fermentables, Hops, Miscs and Yeasts
 *
 * Provides the necessary model so we can build the trees. It extends the
 * QAbstractItemModel, so it has to implement some of the virtual methods
 * required.
 */
class BtTreeModel : public QAbstractItemModel
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
      //! Show styles
      STYLEMASK         = 128,
      //! folders. This may actually have worked better than expected.
      FOLDERMASK        = 256
   };
   
   BtTreeModel(BtTreeView *parent = 0, TypeMasks type = RECIPEMASK);
   virtual ~BtTreeModel();
   
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

   //! \brief Get the upper-left index for the tree
   QModelIndex getFirst();

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
   bool isStyle(const QModelIndex &index);
   //! \brief Test type at \c index.
   bool isFolder(const QModelIndex &index) const;

   //! \brief Gets the type of item at \c index
   int type(const QModelIndex &index);
   //! \brief Return the type mask for this tree. \sa BtTreeModel::TypeMasks
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
   //! \brief Get Style at \c index.
   Style* getStyle(const QModelIndex &index) const;
   //! \brief Get folder at \c index
   BtFolder* getFolder(const QModelIndex &index) const;
   //! \brief Get BeerXMLElement at \c index.
   BeerXMLElement* getThing(const QModelIndex &index) const;

   //! \brief Get index of \c rec.
   QModelIndex findRecipe(Recipe* rec, BtTreeItem* parent = NULL);
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
   //! \brief Get index of \c Style
   QModelIndex findStyle(Style* style);
   //! \brief Get index of \c Folder
   // I'm not quite sure of this signature yet. I want something that can:
   // a) recurse the tree to see if a folder exists
   // b) return the proper index to the folder if it does
   // c) optionally create the tree as it goes.
   // What I don't know is do I send it a QString for the name, or a BtFolder?
   QModelIndex findFolder(QString folder, BtTreeItem* parent=NULL, bool create=false, QString pPath = "" );

private slots:
   //! \brief slot to catch a newEquipmentSignal
   void equipmentAdded(Equipment* victim);
   //! \brief slot to catch a newFermentableSignal
   void fermentableAdded(Fermentable* victim);
   //! \brief slot to catch a newHopSignal
   void hopAdded(Hop* victim);
   //! \brief slot to catch a newMiscSignal
   void miscAdded(Misc* victim);
   //! \brief slot to catch a newRecipeSignal
   void recipeAdded(Recipe* victim);
   //! \brief slot to catch a newYeastSignal
   void yeastAdded(Yeast* victim);
   //! \brief slot to catch a newBrewNoteSignal
   void brewNoteAdded(BrewNote* victim);
   //! \brief slot to catch a newStyleSignal
   void styleAdded(Style* victim);
   //! \brief slot to catch a new folder signal. Folders are odd, because they
   // can hold .. anything, including other folders. So I need the most generic
   // pointer I can get. I hope this works.
   // void folderAdded(QObject* victim);
   
   //! \brief slot to catch a changed signal from an equipment
   void equipmentChanged();
   //! \brief slot to catch a changed signal from a fermentable
   void fermentableChanged();
   //! \brief slot to catch a changed signal from a hop
   void hopChanged();
   //! \brief slot to catch a changed signal from a misc
   void miscChanged();
   //! \brief slot to catch a changed signal from a recipe
   void recipeChanged();
   //! \brief slot to catch a changed signal from a yeast
   void yeastChanged();
   //! \brief slot to catch a changed signal from a brewnote
   void brewNoteChanged();
   //! \brief slot to catch a changed signal from a style
   void styleChanged();
   //! \brief slot to catch a changed folder signal. Folders are odd, because they
   // can hold .. anything, including other folders. So I need the most generic
   // pointer I can get. I hope this works.
   void folderChanged(QString name);
   
   //! \brief slot to catch a deletedEquipmentSignal
   void equipmentRemoved(Equipment* victim);
   //! \brief slot to catch a deletedFermentableSignal
   void fermentableRemoved(Fermentable* victim);
   //! \brief slot to catch a deletedHopSignal
   void hopRemoved(Hop* victim);
   //! \brief slot to catch a deletedMiscSignal
   void miscRemoved(Misc* victim);
   //! \brief slot to catch a deletedRecipeSignal
   void recipeRemoved(Recipe* victim);
   //! \brief slot to catch a deletedYeastSignal
   void yeastRemoved(Yeast* victim);
   //! \brief slot to catch a deletedBrewNoteSignal
   void brewNoteRemoved(BrewNote* victim);
   //! \brief slot to catch a deletedStyleSignal
   void styleRemoved(Style* victim);
   //! \brief slot to catch a changed folder signal. Folders are odd, because they
   // can hold .. anything, including other folders. So I need the most generic
   // pointer I can get. I hope this works.
   //void folderRemoved(QObject* victim);

private:
   //! \brief returns the BtTreeItem at \c index
   BtTreeItem *getItem(const QModelIndex &index) const;
   //! \brief Loads the data. Empty \c propname means load all trees.
   void loadTreeModel(QString propName = "");
   //! \brief Unloads the data. Empty \c propname means unload all trees.
   void unloadTreeModel(QString propName = "");
   
   //! \brief connects the changedName() signal from \c Equipment to the equipmentChanged() slot
   void observeEquipment(Equipment*);
   //! \brief connects the changedName() signal from \c Fermentable to the fermentableChanged() slot
   void observeFermentable(Fermentable*);
   //! \brief connects the changedName() signal from \c Hop to the hopChanged() slot
   void observeHop(Hop*);
   //! \brief connects the changedName() signal from \c Misc to the miscChanged() slot
   void observeMisc(Misc*);
   //! \brief connects the changedName() signal from \c Recipe to the recipeChanged() slot
   void observeRecipe(Recipe*);
   //! \brief connects the changedName() signal from \c Yeast to the yeastChanged() slot
   void observeYeast(Yeast*);
   //! \brief connects the changedName() signal from \c BrewNote to the brewnoteChanged() slot
   void observeBrewNote(BrewNote*);
   //! \brief connects the changedName() signal from \c Style to the styleChanged() slot
   void observeStyle(Style*);
   
   //! \brief returns the \c section header from a recipe
   QVariant getRecipeHeader(int section) const;
   //! \brief returns the \c section header from an equipment
   QVariant getEquipmentHeader(int section) const;
   //! \brief returns the \c section header from a fermentable
   QVariant getFermentableHeader(int section) const;
   //! \brief returns the \c section header from a hop
   QVariant getHopHeader(int section) const;
   //! \brief returns the \c section header from a misc
   QVariant getMiscHeader(int section) const;
   //! \brief returns the \c section header from a yeast
   QVariant getYeastHeader(int section) const;
   //! \brief returns the \c section header from a style
   QVariant getStyleHeader(int section) const;
   //! \brief returns the \c section header for a folder. If that makes sense.
   // and I'm not sure it does
   QVariant getFolderHeader(int section) const;
  
   //! \brief creates a folder tree. It's mostly a helper function.
   QModelIndex createFolderTree( QStringList dirs, BtTreeItem* parent, QString pPath);

   BtTreeItem* rootItem;
   BtTreeView *parentTree;
   TypeMasks treeMask;

};

#endif /* RECEIPTREEMODEL_H_ */
