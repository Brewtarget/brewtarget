/*
 * PageText.h is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Mattias Måhl <mattias@kejsarsten.com>
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
#ifndef BTPAGE_PAGETEXT_H
#define BTPAGE_PAGETEXT_H
#include "PageChildObject.h"
#include <QPainter>
#include <QString>
#include <QRectF>
#include <QTextOption>
#include <QFont>
#include <QDebug>

namespace BtPage
{
   /* \!brief
   * class PageText
   *
   * BtPage text object that needs to have a Value (text) and a Font (Will default to Application Font if not set.)
   */
   class PageText
      : public PageChildObject
   {

   public:
      PageText(Page *parent, QString value, QFont font);
      QString Value;
      QTextOption Options;

      int count();

      //Enforced by PageChildObject
      void render(QPainter *painter);
      QSize getSize();
      void calculateBoundingBox( double scalex = 0.0, double scaley = 0.0 );
   };
}
#endif /* BTPAGE_PAGETEXT_H */