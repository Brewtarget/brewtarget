/*
 * MashComboBox.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Julein <j2bweb@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include "MashComboBox.h"
#include <QList>
#include "database.h"
#include "model/Mash.h"

MashComboBox::MashComboBox(QWidget* parent)
   : QComboBox(parent)
{
   setCurrentIndex(-1);
   connect( &(Database::instance()), SIGNAL(newMashSignal(Mash*)), this, SLOT(addMash(Mash*)) );
   connect( &(Database::instance()), SIGNAL(deletedSignal(Mash*)), this, SLOT(removeMash(Mash*)) );
   repopulateList();
}

void MashComboBox::addMash(Mash* m)
{
   if( m && !mashObs.contains(m) && m->display() && !m->deleted() )
   {
      mashObs.append(m);
      connect( m, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   }

   addItem( m->name() );
}

void MashComboBox::removeMash(Mash* m)
{
   if( m )
      disconnect( m, 0, this, 0 );
   int ndx = mashObs.indexOf(m);
   if( ndx >= 0 )
   {
      mashObs.removeAt(ndx);
      removeItem(ndx);
   }
}

void MashComboBox::removeAllMashs()
{
   QList<Mash*> tmpMashs(mashObs);
   int i;

   for( i = 0; i < tmpMashs.size(); ++i )
      removeMash(tmpMashs[i]);
}

void MashComboBox::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   i = mashObs.indexOf( qobject_cast<Mash*>(sender()) );
   if( i >= 0 )
   {
      // Notice we assume 'i' is an index into both 'mashObs' and also
      // to the text list in this combo box...
      setItemText( i, mashObs[i]->name() );
   }
}

void MashComboBox::setIndexByMash(Mash* mash)
{
   int ndx;

   ndx = mashObs.indexOf(mash);
   setCurrentIndex(ndx);
}

void MashComboBox::setIndex(int ndx)
{
   setCurrentIndex(ndx);
}

void MashComboBox::repopulateList()
{
   unsigned int i, size;
   clear();

   QList<Mash*> tmpMashs(mashObs);
   size = tmpMashs.size();
   for( i = 0; i < size; ++i )
      removeMash( tmpMashs[i] );

   tmpMashs.clear();
   tmpMashs = Database::instance().mashs();

   size = tmpMashs.size();
   for( i = 0; i < size; ++i )
      addMash(tmpMashs[i]);

   setCurrentIndex(-1);
}

Mash* MashComboBox::getSelectedMash()
{
   if( currentIndex() >= 0 )
      return mashObs[currentIndex()];
   else
      return 0;
}
