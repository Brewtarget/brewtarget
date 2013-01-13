/*
 * RecipeListModel.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2012-2013.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RECIPELISTMODEL_H
#define _RECIPELISTMODEL_H

#include <QAbstractListModel>
#include <QList>
class Recipe;

/*!
 * \brief A list of the database's available recipes.
 * \author Philip G. Lee
 */
class RecipeListModel : public QAbstractListModel
{
   Q_OBJECT
   
public:
   RecipeListModel(QWidget* parent = 0);
   
   //! Reimplemented from QAbstractListModel.
   virtual int rowCount( QModelIndex const& parent = QModelIndex() ) const;
   //! Reimplemented from QAbstractListModel.
   virtual QVariant data( QModelIndex const& index, int role = Qt::DisplayRole ) const;
   //! Reimplemented from QAbstractListModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

   //! Remove all equipments from the list.
   void removeAll();
   
private slots:
   //! To catch when one of our recipes' names changed.
   void recNameChanged(const QString&);
   //! Add a recipe to the list.
   void addRecipe(Recipe* recipe);
   //! Add many recipes to the list.
   void addRecipes(QList<Recipe*> recs);
   //! Remove a recipe from the list.
   void removeRecipe(Recipe* recipe);
   
private:
   QList<Recipe*> recipes;
   
   void repopulateList();
};

#endif /*_RECIPELISTMODEL_H*/
