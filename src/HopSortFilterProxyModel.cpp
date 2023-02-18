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
#include "HopSortFilterProxyModel.h"

#include <iostream>

#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Hop.h"
#include "tableModels/HopTableModel.h"

HopSortFilterProxyModel::HopSortFilterProxyModel(QObject *parent, bool filt)
: QSortFilterProxyModel(parent)
{
   filter = filt;
}

bool HopSortFilterProxyModel::lessThan(QModelIndex const & left,
                                       QModelIndex const & right) const {
   QVariant leftHop = sourceModel()->data(left);
   QVariant rightHop = sourceModel()->data(right);
   QStringList uses = QStringList() << "Dry Hop" << "Aroma" << "Boil" << "First Wort" << "Mash";

   switch (left.column()) {
      case HOPALPHACOL:
         {
            double lAlpha = Localization::toDouble(leftHop.toString(), Q_FUNC_INFO);
            double rAlpha = Localization::toDouble(rightHop.toString(), Q_FUNC_INFO);
            return lAlpha < rAlpha;
         }

      case HOPINVENTORYCOL:
         if (Measurement::qStringToSI(leftHop.toString(), Measurement::PhysicalQuantity::Mass).quantity() == 0.0 &&
            this->sortOrder() == Qt::AscendingOrder) {
            return false;
         }
         return Measurement::qStringToSI(leftHop.toString(), Measurement::PhysicalQuantity::Mass) <
                Measurement::qStringToSI(rightHop.toString(), Measurement::PhysicalQuantity::Mass);

      case HOPAMOUNTCOL:
         return Measurement::qStringToSI(leftHop.toString(), Measurement::PhysicalQuantity::Mass) <
                Measurement::qStringToSI(rightHop.toString(), Measurement::PhysicalQuantity::Mass);

      case HOPTIMECOL:
         {
         // Get the indexes of the Use column
         QModelIndex lSibling = left.sibling(left.row(), HOPUSECOL);
         QModelIndex rSibling = right.sibling(right.row(), HOPUSECOL);
         // We are talking to the model, so we get the strings associated with
         // the names, not the Hop::Use enums. We need those translated into
         // ints to make this work
         int lUse = uses.indexOf( (sourceModel()->data(lSibling)).toString() );
         int rUse = uses.indexOf( (sourceModel()->data(rSibling)).toString() );

         if (lUse == rUse) {
               return Measurement::qStringToSI(leftHop.toString(), Measurement::PhysicalQuantity::Time) <
                      Measurement::qStringToSI(rightHop.toString(), Measurement::PhysicalQuantity::Time);
         }

         return lUse < rUse;
         }
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
             && model->getRow(source_row)->display()
           );
}
