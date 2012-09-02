/*
 * RecipeListModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2012.
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

#include "RecipeListModel.h"
#include <QWidget>
#include "database.h"

RecipeListModel::RecipeListModel(QWidget* parent)
   : QAbstractListModel(parent)
{
   //connect( &(Database::instance()), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(dbChanged(QMetaProperty,QVariant)) );
   connect( &(Database::instance()), SIGNAL(newRecipeSignal(Recipe*)), this, SLOT(addRecipe(Recipe*)) );
   connect( &(Database::instance()), SIGNAL(deletedRecipeSignal(Recipe*)), this, SLOT(removeRecipe(Recipe*)) );
   repopulateList();
}

void RecipeListModel::addRecipe(Recipe* recipe)
{
   if( !recipe ||
      recipes.contains(recipe) ||
      recipe->deleted() ||
      !recipe->display()
   )
      return;
   
   int size = recipes.size();
   beginInsertRows( QModelIndex(), size, size );
   recipes.append(recipe);
   connect( recipe, SIGNAL(changedName(const QString&)), this, SLOT(recNameChanged(const QString&)) );
   endInsertRows();
}

void RecipeListModel::addRecipes(QList<Recipe*> recs)
{
   QList<Recipe*>::iterator i;
   QList<Recipe*> tmp;
   
   for( i = recs.begin(); i != recs.end(); ++i )
   {
      // if the recipe is not already in the list and
      // if the recipe has not been deleted and
      // if the recipe is to be displayed, then append it
      if( !recipes.contains(*i) &&
         !(*i)->deleted()       &&
         (*i)->display()
      )
         tmp.append(*i);
   }
   
   int size = recipes.size();
   beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
   recipes.append(tmp);
   
   for( i = tmp.begin(); i != tmp.end(); i++ )
      connect( *i, SIGNAL(changedName(const QString&)), this, SLOT(recNameChanged(const QString&)) );
   
   endInsertRows();
}

void RecipeListModel::removeRecipe(Recipe* recipe)
{
   int ndx = recipes.indexOf(recipe);
   if( ndx > 0 )
   {
      beginRemoveRows( QModelIndex(), ndx, ndx );
      disconnect( recipe, 0, this, 0 );
      recipes.removeAt(ndx);
      endRemoveRows();
   }
}

void RecipeListModel::removeAll()
{
   beginRemoveRows( QModelIndex(), 0, recipes.size()-1 );
   while( !recipes.isEmpty() )
      disconnect( recipes.takeLast(), 0, this, 0 );
   endRemoveRows();
}

void RecipeListModel::recNameChanged(const QString& name)
{
   Recipe* rec = qobject_cast<Recipe*>(sender());
   int ndx;
   
   if( !rec )
      return;
   
   ndx = recipes.indexOf(rec);
   if( ndx >= 0 )
      emit dataChanged( createIndex(ndx,0), createIndex(ndx,0) );
}

void RecipeListModel::repopulateList()
{
   removeAll();
   addRecipes( Database::instance().recipes() );
}

int RecipeListModel::rowCount( QModelIndex const& parent ) const
{
   return recipes.size();
}

QVariant RecipeListModel::data( QModelIndex const& index, int role ) const
{
   int row = index.row();
   int col = index.column();
   if( col == 0 && role == Qt::DisplayRole )
      return QVariant(recipes.at(row)->name());
   else
      return QVariant();
}

QVariant RecipeListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   return QVariant();
}
