/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtHorizontalTabs.h is part of Brewtarget, and is Copyright the following authors 2021-2024:
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
#ifndef BTHORIZONTALTABS_H
#define BTHORIZONTALTABS_H
#pragma once

#include <QProxyStyle>
#include <QStyleOption>
#include <QSize>

/**
 * \brief A custom style class for \c QTabBar (eg of a \c QTabWidget with tabs on the left \c QTabBar::RoundedWest) to
 *        rotate the tabs so that their text is horizontal instead of the default vertical.  This looks neater when we
 *        have potentially long tab text on a widget that is not hugely tall.
 *
 *        To use on, say, \c myQTabWidget, a pointer to \c QTabWidget, call
 *        \c myQTabWidget->tabBar()->setStyle(new BtHorizontalTabs);
 *
 *        NOTE: This is a commonly-used approach that works on Windows and Linux, but not entirely on Mac.  It's fine on
 *              Mac for tabs with square icons in, but doesn't work properly for tabs with text. (If you look at the Qt
 *              source code, eg https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/widgets/qtabbar.cpp?h=5.15,
 *              you'll see some special handling for Mac OS, so perhaps this is something to do with it.)
 *
 *              If we wanted a more sure-fire way of achieving horizontal text in left-hand tabs, we could, in
 *              principle, create our own classes that inherit from \c QTabWidget and \c QTabBar.  But I fear we'd end
 *              up re-implementing a lot of the inner workings of those classes, which would be rather painful.  For
 *              now, we leave text in its default orientation on Mac.
 */
class BtHorizontalTabs : public QProxyStyle {

public:

   /**
    * \brief This is a slightly horrible way of getting things to work on Mac OS
    *
    * \param forceRotate If set to \c true then the class will do it's stuff even on Mac, which is what you want
    *                    for tabs with square icons in.  Otherwise, on Mac, the class will do nothing, leaving the
    *                    default behaviour of side tabs, which is what you want for text tabs.
    *
    *                    NOTE that, on other platforms (ie Linux and Windows), this parameter has no effect -- ie we'll
    *                    always try to rotate the side tab text to horizontal.
    */
   BtHorizontalTabs(bool const forceRotate = false);
   virtual ~BtHorizontalTabs();

   /**
    * \brief Reimplements \c QStyle::sizeFromContents (and various derivatives thereof)
    *
    * \return the size of the element described by the specified option and type, based on the provided \c contentsSize.
    */
   virtual QSize sizeFromContents(ContentsType type,
                                  QStyleOption const * option,
                                  QSize const & contentsSize,
                                  QWidget const * widget) const;

   /**
    * \brief Reimplements \c QStyle::drawControl (and various derivatives thereof) to draw the given \c element with the
    *        provided \c painter with the style options specified by \c option.
    */
   virtual void drawControl(ControlElement element,
                            QStyleOption const * option,
                            QPainter * painter,
                            QWidget const * widget) const;

private:
   bool const m_forceRotate;
};

#endif
