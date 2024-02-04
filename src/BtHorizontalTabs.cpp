/*
 * BtHorizontalTabs.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021-2024
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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
#include "BtHorizontalTabs.h"

#include <QDebug>
#include <QStyleOptionTab>

QSize BtHorizontalTabs::sizeFromContents(ContentsType type,
                                         QStyleOption const * option,
                                         QSize const & contentsSize,
                                         QWidget const * widget) const {
   // By default, we're just going to return the same size as our parent class...
   QSize size = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
   if (type == QStyle::CT_TabBarTab) {
      // ...but, if it's a tab, then we want to swap the X and Y of the size rectangle we return
//      qDebug() << Q_FUNC_INFO << "Transposing width:" << size.width() << ", height:" << size.height();
      size.transpose();
   }
   return size;
}

void BtHorizontalTabs::drawControl(ControlElement element,
                                   QStyleOption const * option,
                                   QPainter * painter,
                                   QWidget const * widget) const {
   // Special handling is only for tabs
   if (element == QStyle::CE_TabBarTabLabel) {
      QStyleOptionTab const * tabOption = qstyleoption_cast<QStyleOptionTab const *>(option);
      if (tabOption) {
         QStyleOptionTab modifiedTabOption{*tabOption};
         //
         // Per https://doc.qt.io/qt-5/qtabbar.html#Shape-enum, the QTabBar::Shape enum type lists the built-in shapes
         // supported by QTabBar.  In particular:
         //   - QTabBar::RoundedNorth = The normal rounded look above the pages
         //   - QTabBar::RoundedWest  = The normal rounded look on the left side of the pages
         //
         // Normally, when the tabs are on the left (QTabBar::RoundedWest), the text is rotated so that (in
         // left-to-right languages such as English) it reads bottom-to-top.  If we change the shape attribute for
         // drawing the tab label to QTabBar::RoundedNorth, then the text should be drawn unrotated as though the tab
         // were above the widget.
         //
         // Normally keep the debug statements below commented out as they generates a lot of output!
         //
//         qDebug() << Q_FUNC_INFO << "Changing shape from" << modifiedTabOption.shape << "to" << QTabBar::RoundedNorth;
         modifiedTabOption.shape = QTabBar::RoundedNorth;
         QProxyStyle::drawControl(element, &modifiedTabOption, painter, widget);
         return;
      }
   }

   // For everything except tabs, just let the parent class do the drawing -- ie don't change anything
   QProxyStyle::drawControl(element, option, painter, widget);
   return;

}
