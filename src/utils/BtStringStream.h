/*
 * utils/BtStringStream.h is part of Brewtarget, and is copyright the following
 * authors 2021:
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef UTILS_BTSTRINGSTREAM_H
#define UTILS_BTSTRINGSTREAM_H
#pragma once

#include <QString>
#include <QTextStream>

/**
 * \class BtStringStream is a small specialisation of \c QTextStream that allows it to be used more like
 *                       \c std::stringstream (ie without having to manually create a QString and then pass its address
 *                       to the QTextStream constructor).
 */
class BtStringStream : public QTextStream {
public:
   BtStringStream();
   ~BtStringStream();

   /**
    * \brief Returns the string representation of this object object.
    */
   QString const & asString() const;

private:
   QString theString;
};
#endif
