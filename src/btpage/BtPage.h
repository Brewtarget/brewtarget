/*
 * page.h is part of Brewtarget, and is Copyright the following
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
#ifndef _BTPAGE_H
#define _BTPAGE_H
#include <QFont>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPrinter>
#include "PageTable.h"
#include "PageText.h"

class BtPage
{
public:
   BtPage(QPrinter *printer);

   void addChildObject(PageChildObject *obj);
   void renderPage();

private:
   QPrinter *_printer;
   QList<PageChildObject*> _children;
};

#endif /* _BTPAGE_H */
