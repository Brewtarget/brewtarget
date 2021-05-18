/*
 * HopSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#include "brewtarget.h"
#include "HopSortFilterProxyModel.h"
#include "HopTableModel.h"
#include "model/Hop.h"
#include "Unit.h"
#include <iostream>

HopSortFilterProxyModel::HopSortFilterProxyModel(QObject *parent, bool filt)
: QSortFilterProxyModel(parent)
{
   filter = filt;
}

bool HopSortFilterProxyModel::lessThan(const QModelIndex &left,
                                         const QModelIndex &right) const
{
    QVariant leftHop = sourceModel()->data(left);
    QVariant rightHop = sourceModel()->data(right);
    QStringList uses = QStringList() << "Dry Hop" << "Aroma" << "Boil" << "First Wort" << "Mash";
    QModelIndex lSibling, rSibling;
    int lUse, rUse;
    double lAlpha, rAlpha;
    bool ok = false;
    Unit const * unit = &Units::kilograms;

   switch( left.column() )
   {
      case HOPALPHACOL:
         lAlpha = Brewtarget::toDouble(leftHop.toString(), &ok );
         if ( ! ok )
            qWarning() << QString("HopSortFilterProxyModel::lessThan() could not convert %1 to double").arg(leftHop.toString());

         rAlpha = Brewtarget::toDouble(rightHop.toString(), &ok );
         if ( ! ok )
            qWarning() << QString("HopSortFilterProxyModel::lessThan() could not convert %1 to double").arg(rightHop.toString());

         return lAlpha < rAlpha;

      case HOPINVENTORYCOL:
         if (Brewtarget::qStringToSI(leftHop.toString(), unit) == 0.0 && this->sortOrder() == Qt::AscendingOrder)
            return false;
         else
            return Brewtarget::qStringToSI(leftHop.toString(),unit) < Brewtarget::qStringToSI(rightHop.toString(),unit);
      case HOPAMOUNTCOL:
         return Brewtarget::qStringToSI(leftHop.toString(),unit) < Brewtarget::qStringToSI(rightHop.toString(),unit);
      case HOPTIMECOL:
        // Get the indexes of the Use column
        lSibling = left.sibling(left.row(), HOPUSECOL);
        rSibling = right.sibling(right.row(), HOPUSECOL);
        // We are talking to the model, so we get the strings associated with
        // the names, not the Hop::Use enums. We need those translated into
        // ints to make this work
        lUse = uses.indexOf( (sourceModel()->data(lSibling)).toString() );
        rUse = uses.indexOf( (sourceModel()->data(rSibling)).toString() );

        unit = &Units::minutes; // not &Units::kilogram
        if ( lUse == rUse )
            return Brewtarget::qStringToSI(leftHop.toString(),unit) < Brewtarget::qStringToSI(rightHop.toString(),unit);

        return lUse < rUse;
    }

    return leftHop.toString() < rightHop.toString();
}

bool HopSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent) const
{
   HopTableModel* model = qobject_cast<HopTableModel*>(sourceModel());
   QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

   return !filter
          ||
           ( sourceModel()->data(index).toString().contains(filterRegExp())
             && model->getHop(source_row)->display()
           );
}
