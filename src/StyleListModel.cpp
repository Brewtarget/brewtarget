/*
 * StyleListModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - swstim <swstim@gmail.com>
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

#include "StyleListModel.h"
#include "style.h"
#include "database.h"
#include "recipe.h"

StyleListModel::StyleListModel(QWidget* parent)
   : QAbstractListModel(parent), recipe(0)
{
   connect( &(Database::instance()), &Database::newStyleSignal, this, &StyleListModel::addStyle );
   connect( &(Database::instance()), SIGNAL(deletedSignal(Style*)), this, SLOT(removeStyle(Style*)) );
   repopulateList();
}

void StyleListModel::addStyle(Style* s)
{
   if( !s || !s->display() || s->deleted() )
      return;
   
   if( !styles.contains(s) )
   {
      int size = styles.size();
      beginInsertRows( QModelIndex(), size, size );
      styles.append(s);
      connect( s, &BeerXMLElement::changed, this, &StyleListModel::styleChanged );
      endInsertRows();
   }
}

void StyleListModel::addStyles(QList<Style*> s)
{
   QList<Style*>::iterator i;
   QList<Style*> tmp;
   
   for( i = s.begin(); i != s.end(); i++ )
   {
      if( !styles.contains(*i) )
         tmp.append(*i);
   }
   
   int size = styles.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      styles.append(tmp);
      
      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, &BeerXMLElement::changed, this, &StyleListModel::styleChanged );
      
      endInsertRows();
   }
}

void StyleListModel::removeStyle(Style* style)
{
   int ndx = styles.indexOf(style);
   if( ndx >= 0 )
   {
      beginRemoveRows( QModelIndex(), ndx, ndx );
      disconnect( style, 0, this, 0 );
      styles.removeAt(ndx);
      endRemoveRows();
   }
}

void StyleListModel::removeAll()
{
   if (styles.size())
   {
      beginRemoveRows( QModelIndex(), 0, styles.size()-1 );
      while( !styles.isEmpty() )
         disconnect( styles.takeLast(), 0, this, 0 );
      endRemoveRows();
   }
}

void StyleListModel::styleChanged(QMetaProperty prop, QVariant val)
{   
   Style* sSend = qobject_cast<Style*>(sender());
   
   // NOTE: how to get around the issue that the sender might live in
   // a different thread and therefore always cause sSend == 0?
   if( sSend == 0 )
      return;
   
   QString propName(prop.name());
   if( propName == "name" )
   {
      int ndx = styles.indexOf(sSend);
      if( ndx >= 0 )
         emit dataChanged( createIndex(ndx,0), createIndex(ndx,0) );
   }
}

void StyleListModel::repopulateList()
{
   removeAll();
   addStyles( Database::instance().styles() );
}

Style* StyleListModel::at(int ndx)
{
   if( ndx >= 0 && ndx < styles.size() )
      return styles[ndx];
   else
      return 0;
}

int StyleListModel::indexOf(Style* s)
{
   return styles.indexOf(s);
}

int StyleListModel::rowCount( QModelIndex const& parent ) const
{
   return styles.size();
}

QVariant StyleListModel::data( QModelIndex const& index, int role ) const
{
   int row = index.row();
   int col = index.column();
   if( col == 0 && role == Qt::DisplayRole )
      return QVariant(styles.at(row)->name());
   else
      return QVariant();
}

QVariant StyleListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   return QVariant(QString("Header Data..."));
}
