/*
 * utils/BtStringConst.h is part of Brewtarget, and is Copyright the following
 * authors 2021
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
#ifndef UTILS_BTSTRINGCONST_H
#define UTILS_BTSTRINGCONST_H
#pragma once

class QDebug;
class QString;
class QTextStream;

/**
 * \brief Class for compile-time constant ASCII strings
 *
 *        This is a thin wrapper around char const * const that ensures the right thing happens when you do comparisons
 *        with ==, ie meaning you don't have to remember to use std::strcmp.  This is useful, because comparing two
 *        compile-time constant strings with == often "works" almost all the time (because two identical strings are
 *        just pointers to the same memory location) but not always (because one particular compiler will have done its
 *        optimisation differently and there are actually two different locations in memory holding identical strings).
 *
 *        In a Qt application such as ours, you might thing we could use QString const instead of char const * const.
 *        This has two disadvantages.  Firstly, we sometimes need the constant as a char const * -- eg to pass to Qt
 *        property functions such as QObject::property() and QObject::setProperty().  Although char const * into a
 *        QString is trivial, doing the reverse, ie getting getting char const * out of a QString, is a bit painful
 *        (because QString is inherently UTF-16 so you end up creating implicit or explicit temporaries to hold char *
 *        data etc).
 *
 *        The second disadvantage of QString for string constants is that QString does clever reference counting
 *        internally (see https://doc.qt.io/qt-5/implicit-sharing.html).  In theory this is invisible to users of
 *        QString and never a problem.  In practice, you have to be careful about, say, a struct containing
 *        QString const &, as you can break the reference-counting logic and get a segfault (at least on Clang on Mac OS
 *        with Qt 5.9.5).
 */
class BtStringConst {
public:
   // NB: Constructors are all explicit as we don't want to construct with implicit conversions
   explicit BtStringConst(char const * const cString);
   //! Copy constructor OK
   explicit BtStringConst(BtStringConst const &);
   //! Move constructor OK
   explicit BtStringConst(BtStringConst &&);
   ~BtStringConst();

   /**
    * \brief Compare two \c BtStringConst for equality using \c std::strcmp internally after checking for null pointers
    */
   bool operator==(BtStringConst const & rhs) const;
   template<class T>
   bool operator!=(T const & rhs) const {
      return !(*this == rhs);
   }

   /**
    * \brief Returns \c true if the contained char const * const pointer is null
    */
   bool isNull() const;

   /**
    * \brief Returns the contained char const * const pointer
    */
   char const * const operator*() const;

   /**
    * \brief Generic output streaming for \c BtStringConst, including sensible output if the contained pointer is null
    *        Note that we can't template operator<< as such a template would match too many things and create errors
    *        along the lines of "ambiguous overload for ‘operator<<’" elsewhere in the code.  (Specifically, the problem
    *        is that the compiler sees, say, an int and decides "I could implicitly convert int to to char const * const
    *        and use that to construct a BtStringConst, which would then match this template, but now I've got multiple
    *        matches and don't know what to do, so stop compilation with an error.")
    */
   template<class OS>
   OS & writeTo(OS & outputStream) const {
      if (this->isNull()) {
         outputStream << "[nullptr]";
      } else {
         outputStream << **this;
      }
      return outputStream;
   }

private:
   char const * const cString;

   //! No assignment operator
   BtStringConst & operator=(BtStringConst const &) = delete;
   //! No move assignment
   BtStringConst & operator=(BtStringConst &&) = delete;
};

/**
 * \brief If you want to initialise a \c BtStringConst with \c nullptr you need to pass
 *        static_cast<char const * const>(nullptr), which is a bit cumbersome, so instead, this pre-defined constant
 *        allows you to reference a null-containing \c BtStringConst with BtString::NULL_STR.
 */
namespace BtString { extern BtStringConst const NULL_STR; }

/**
 * \brief Output \c BtStringConst to \c QTextStream
 */
QTextStream & operator<<(QTextStream & outputStream, BtStringConst const & btStringConst);
QDebug & operator<<(QDebug & outputStream, BtStringConst const & btStringConst);

/**
 * \brief Compare char const * const to BtStringConst
 */
bool operator==(char const * const lhs, BtStringConst const & rhs);

/**
 * \brief Compare BtStringConst to char const * const
 */
bool operator==(BtStringConst const & lhs, char const * const rhs);

/**
 * \brief Compare QString to BtStringConst
 */
bool operator==(QString const & lhs, BtStringConst const & rhs);

/**
 * \brief Compare BtStringConst to QString
 */
bool operator==(BtStringConst const & lhs, QString const & rhs);

/**
 * \brief Not equals is implemented in terms of equals, which saves a little boilerplate
 *        Note that we can't template operator!= as such a template would match too many things and create errors
 *        along the lines of "ambiguous overload for ‘operator!=’" elsewhere in the code
 *
 *        .:TODO:. Look at whether spaceship operator can reduce this boilerplate more, once we're using C++20)
 */
bool operator!=(QString const & lhs, BtStringConst const & rhs);

#endif
