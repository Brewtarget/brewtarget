/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtHorizontalTabs.cpp is part of Brewtarget, and is Copyright the following authors 2021-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "BtHorizontalTabs.h"

#include <QDebug>
#include <QStyleOptionTab>

BtHorizontalTabs::BtHorizontalTabs([[maybe_unused]] bool const forceRotate) :
   QProxyStyle{},
   m_forceRotate{
//
// This may look a bit odd, but it saves having lots of other #ifdefs below
//
// You can sort of simulate Mac behaviour by changing this #ifdef to #ifndef
//
#ifdef Q_OS_MACOS
      // On Mac, we only attempt to rotate the side tabs if you insist (eg because you know they are icons)
      forceRotate
#else
      // On other platforms, it's safe always to rotate the size tabs
      true
#endif
   } {
   return;
}

BtHorizontalTabs::~BtHorizontalTabs() = default;


QSize BtHorizontalTabs::sizeFromContents(ContentsType type,
                                         QStyleOption const * option,
                                         QSize const & contentsSize,
                                         QWidget const * widget) const {
   // By default, we're just going to return the same size as our parent class...
   QSize size = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);

   //
   // On Mac, the results of this function are correctly used by QTabBar to reserve the right size and shape space to
   // draw a horizontal-oriented tab.  However, when QTabBar comes to draw the tab, it seems to ignore the width
   // returned from this function.
   //
   // See https://github.com/Brewtarget/brewtarget/issues/787#issuecomment-1921341649 for example screenshot (after
   // overriding the behaviour that would otherwise prevent text that does not fit in the tab from being displayed).
   //
   if (this->m_forceRotate && type == QStyle::CT_TabBarTab ) {
      // ...but, if it's a tab, then we want to swap the X and Y of the size rectangle we return
      size.transpose();
   }
   return size;
}

void BtHorizontalTabs::drawControl(ControlElement element,
                                   QStyleOption const * option,
                                   QPainter * painter,
                                   QWidget const * widget) const {
   //
   // Special handling is only for the text or icon inside tabs.  We want the tab itself (and the highlighting for
   // "current tab") to be drawn as a "side" tab -- so we leave default handling for QStyle::CE_TabBarTabShape and
   // QStyle::CE_TabBarTab.
   //
   // Of course, it would be cute, on Mac OS, to be able to force the correct widths of QStyle::CE_TabBarTabShape and
   // QStyle::CE_TabBarTab by consulting widget->rect() (yes, of course, rect is a member function on QWidget even
   // though it's a member variable on QStyleOption).  However, we cannot modify option.rect because option is read-only
   // for us, and we do not own the QStyleOption object, so it is not safe for us to cast away its "const".
   //
   if (this->m_forceRotate && element == QStyle::CE_TabBarTabLabel) {
      QStyleOptionTab const * tabOption = qstyleoption_cast<QStyleOptionTab const *>(option);
      if (tabOption) {
         QStyleOptionTab modifiedTabOption{*tabOption};
         //
         // Per https://doc.qt.io/qt-5/qtabbar.html#Shape-enum, the QTabBar::Shape enum type lists the built-in shapes
         // supported by QTabBar.  In particular:
         //   - QTabBar::RoundedNorth = The normal rounded look above the pages
         //   - QTabBar::RoundedWest  = The normal rounded look on the left side of the pages
         //
         // Normally, when the tabs are on the left (QTabBar::RoundedWest), the text or icon is rotated 90°
         // anticlockwise, so that (in left-to-right languages such as English) the text reads bottom-to-top.  If we
         // change the shape attribute for drawing the tab label to QTabBar::RoundedNorth, then the text or icon should
         // be drawn unrotated as though the tab were above the widget.  And, the work in sizeFromContents() above
         // should have ensured there is enough space for this.
         //
         modifiedTabOption.shape = QTabBar::RoundedNorth;
         QProxyStyle::drawControl(element, &modifiedTabOption, painter, widget);
         return;
      }
   }

   // For everything except tabs, just let the parent class do the drawing -- ie don't change anything
   QProxyStyle::drawControl(element, option, painter, widget);
   return;

}
