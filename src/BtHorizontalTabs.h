/*
 * BtHorizontalTabs.h is part of Brewtarget, and is Copyright the following
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

#ifndef BTHORIZONTALTABS_H
#define BTHORIZONTALTABS_H
#pragma once

#include <QProxyStyle>
#include <QStyleOption>
#include <QSize>

/**
 * \brief A custom style class for \c QTabWidget with tabs on the left (\c QTabBar::RoundedWest) to rotate the tab so
 *        that its text is horizontal instead of the default vertical.  This looks neater when we have potentially long
 *        tab text on a widget that is not hugely tall.
 */
class BtHorizontalTabs : public QProxyStyle {
public:
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
};
#endif
