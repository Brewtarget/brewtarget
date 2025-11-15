/*======================================================================================================================
 * utils/MultiVector.h is part of Brewtarget, and is copyright the following authors 2025:
 *   • Matt Young <mfsy@yahoo.com>
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
 =====================================================================================================================*/
#ifndef UTILS_MULTIVECTOR_H
#define UTILS_MULTIVECTOR_H
#pragma once

#include <iterator>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>

#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

/**
 * \brief This is the native way to refer to an element of a \c MultiVector.  We define it outside the class so that we
 *        can define \c operator<< for it before the \c MultiVector template class definition (and thus can use the
 *        operator inside the template class).
 *
 *        The limitation of doing this is that we cannot use \c MultiVector::size_type as the type for the members of
 *        this struct.  However, for our purposes, it is reasonable to assume \c std::size_t, as this is what is
 *        "usually" the type of \c std::vector::size_type
 */
struct MvCoordinates {
   //! Index into the list of vectors
   std::size_t vecNum = 0;
   //! Index into the individual vector
   std::size_t index = 0;

   bool operator==(MvCoordinates const & rhs) const = default;

};

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, MvCoordinates const & coordinates) {
   stream << "MvCoordinates (vecNum:" << coordinates.vecNum << ", index:" << coordinates.index << ")";
   return stream;
}

/**
 * \class MultiVector
 *
 * \brief This class behaves mostly like a vector but internally holds its data in one or more vectors (of differing
 *        lengths).  The internal vectors can be a mix of ones owned by this class and external ones.
 *
 *        We primarily use this where we want to be able to initialise a list from several sources and there are reasons
 *        we don't want to be able to copy the contents.  Eg in \c ObjectStore, we want to be able to share some field
 *        definitions between multiple tables, but we don't want to try copy one vector of definitions into another
 *        because \c BtStringConst is (intentionally) not copyable.
 *
 *        NOTE: Because our use cases are only for constant data, we have not implemented insertion, deletion etc.
 */
template<class T, class Allocator = std::allocator<T>> class MultiVector {
   using InnerVector = std::vector<T, Allocator>;

public:
   using const_reference = InnerVector::const_reference;
   using reference       = InnerVector::reference      ;
   using size_type       = InnerVector::size_type      ;

   /**
    * \brief Constructor
    *
    * \param ownedFields
    * \param externalFields  NOTE If this is non-empty, none of the vectors pointed to may itself be empty.
    */
   MultiVector(std::initializer_list<T> const ownedFields,
               std::initializer_list<std::vector<T> const *> externalFields = {}) :
      m_ownedFields{ownedFields},
      m_innerVectors{&m_ownedFields} {
      this->m_innerVectors.insert(this->m_innerVectors.end(), externalFields.begin(), externalFields.end());
      return;
   }

   /**
    * \brief Random access iterator (cf std::random_access_iterator concept)
    *
    *        Writing iterators is a bit painstaking, so we enlist the help of Boost's \c iterator_facade to minimise
    *        the amount of boilerplate we need.
    *
    *        The iterator class itself is templated to minimise the duplication between iterators for const and
    *        non-const \c T.  \c Value is either \c T or \c T \c const.  See
    */
   template<class Value>
   class iteratorImpl : public boost::iterator_facade<MultiVector::iteratorImpl<Value>,
                                                      T const,
                                                      boost::random_access_traversal_tag> {
   public:
      iteratorImpl() {
         return;
      }

      explicit iteratorImpl(MultiVector const * multiVector, MvCoordinates coordinates) :
         m_multiVector{multiVector},
         m_coordinates{coordinates} {
         return;
      }

      //
      // The jiggery pokery here is to enable us to convert from iterator to const_iterator but not vice versa
      //
      template<class OtherValue/*,
               typename std::enable_if_t<std::is_convertible<OtherValue *, Value *>::value, bool> = true*/>
      iteratorImpl(iteratorImpl<OtherValue> const & other) requires (std::is_const_v<Value>) :
         m_multiVector{other.m_multiVector},
         m_coordinates{other.m_coordinates} {
         return;
      }

   private:

      //! \return \c true if we are pointing to the last element, \c false otherwise
      bool isLast() const {
         return (
            this->m_coordinates.vecNum == this->m_multiVector->m_innerVectors.size() - 1 &&
            this->m_coordinates.index  == this->m_multiVector->m_innerVectors[this->m_coordinates.vecNum]->size() - 1
         );
      }

      //! \return \c true if we are pointing to one past last element, \c false otherwise
      bool isEnd() const {
         return (
            this->m_coordinates.vecNum == this->m_multiVector->m_innerVectors.size() - 1 &&
            this->m_coordinates.index  == this->m_multiVector->m_innerVectors[this->m_coordinates.vecNum]->size()
         );
      }

      //! \return \c true if we are pointing to the first element, \c false otherwise
      bool isFirst() const {
         return (
            this->m_coordinates.vecNum == 0 &&
            this->m_coordinates.index  == 0
         );
      }

      friend class boost::iterator_core_access;

      void increment() {
         // NB: We need to be able to go one past the end of the last vector
         if (this->m_coordinates.index < this->m_multiVector->m_innerVectors[this->m_coordinates.vecNum]->size() - 1 ||
             this->isLast()) {
            ++this->m_coordinates.index;
         } else {
            ++this->m_coordinates.vecNum;
            this->m_coordinates.index = 0;
         }
         return;
      }

      void decrement() {
         if (this->m_coordinates.index > 0) {
            --this->m_coordinates.index;
         } else {
            --this->m_coordinates.vecNum;
            this->m_coordinates.index = this->m_multiVector->m_innerVectors[this->m_coordinates.vecNum]->size() - 1;
         }
         return;
      }

      void advance(size_type positions) {
         this->m_coordinates.index += positions;
         while (this->m_coordinates.index >= this->m_multiVector->m_innerVectors[this->m_coordinates.vecNum]->size()) {
            this->m_coordinates.index -= this->m_multiVector->m_innerVectors[this->m_coordinates.vecNum]->size();
            // We need to be able to go one past the end of the last vector
            if (this->m_coordinates.vecNum == this->m_multiVector->m_innerVectors.size() - 1) {
               break;
            }
            ++this->m_coordinates.vecNum;
         }
         return;
      }

      template<class OtherValue>
      int distance_to(iteratorImpl<OtherValue> const & other) const {
         Q_ASSERT(this->m_multiVector == other.m_multiVector);
         return other.m_multiVector->coordinatesToFlatIndex(other.m_coordinates) -
                this->m_multiVector->coordinatesToFlatIndex(this->m_coordinates);
      }

      template<class OtherValue>
      bool equal(iteratorImpl<OtherValue> const & other) const {
         return this->m_multiVector == other.m_multiVector &&
                this->m_coordinates == other.m_coordinates;
      }

      Value & dereference() const {
         return this->m_multiVector->at(this->m_coordinates);
      }

   //
   // Two different instantiations of the same template can't access each other's private member variables, even when
   // it's template<T> and template<T const>.  There is probably a clever way around this, but, for the moment, just
   // making them public gets the job done.
   //
   public:
      // Can't use a reference here as iterator needs to be default constructable
      MultiVector const *      m_multiVector = nullptr;
      MvCoordinates m_coordinates = {-1, -1};

   };

   typedef iteratorImpl<T      >       iterator;
   typedef iteratorImpl<T const> const_iterator;

   MvCoordinates flatIndexToCoordinates(size_type pos) const {
      size_type vectorNumber = 0;
      while (pos >= this->m_innerVectors[vectorNumber]->size()) {
         pos -= this->m_innerVectors[vectorNumber]->size();
         ++vectorNumber;
      }
      return MvCoordinates{vectorNumber, pos};
   }

   size_type coordinatesToFlatIndex(MvCoordinates coordinates) const {
      size_type pos = coordinates.index;
      //
      // We have to be a bit careful here.  We can't just write `while (--coordinates.vecNum >= 0)` because the vecNum
      // member is unsigned (so, infinite loop!).  Moving the decrement to inside the loop is the most concise solution
      // in this instance.
      //
      while (coordinates.vecNum > 0) {
         pos += this->m_innerVectors[--coordinates.vecNum]->size();
      }
      return pos;
   }

   /**
    * \brief Unchecked access to an element
    */
   reference operator[](MvCoordinates const coordinates) {
      return (*this->m_innerVectors[coordinates.vecNum])[coordinates.index];
   }
   const_reference operator[](MvCoordinates const coordinates) const {
      return (*this->m_innerVectors[coordinates.vecNum])[coordinates.index];
   }
   reference operator[](size_type const pos) {
      return (*this)[this->flatIndexToCoordinates(pos)];
   }
   const_reference operator[](size_type const pos) const {
      return (*this)[this->flatIndexToCoordinates(pos)];
   }

   /**
    * \brief Checked access to an element
    */
   reference at(MvCoordinates const coordinates) {
      return this->m_innerVectors.at(coordinates.vecNum)->at(coordinates.index);
   }
   reference at(size_type pos) {
      return this->at(this->flatIndexToCoordinates(pos));
   }
   const_reference at(MvCoordinates const coordinates) const {
      return this->m_innerVectors.at(coordinates.vecNum)->at(coordinates.index);
   }
   const_reference at(size_type pos) const {
      return this->at(this->flatIndexToCoordinates(pos));
   }

         iterator  begin()                { return       iterator(this, MvCoordinates{0, 0}); }
   const_iterator  begin() const          { return const_iterator(this, MvCoordinates{0, 0}); }
   const_iterator cbegin() const noexcept { return const_iterator(this, MvCoordinates{0, 0}); }

   //
   // This returns one past the end of the last vector
   //
   MvCoordinates endCoordinates() const {
      // There is always at least one vector (though it can be empty)
      Q_ASSERT(this->m_innerVectors.size() > 0);
      auto const lastVecNum = this->m_innerVectors.size() - 1;
      return MvCoordinates{lastVecNum, this->m_innerVectors[lastVecNum]->size()};
   }
         iterator  end()                { return       iterator(this, this->endCoordinates()); }
   const_iterator  end() const          { return const_iterator(this, this->endCoordinates()); }
   const_iterator cend() const noexcept { return const_iterator(this, this->endCoordinates()); }

   bool empty() const {
      return this->size() == 0;
   }

   size_type size() const {
      return this->coordinatesToFlatIndex(this->endCoordinates());
   }

private:
   /**
    * \brief We have one vector that we own -- constructed from \c std::initializer_list.  This will be first in the
    *        \c m_innerVectors list.
    */
   std::vector<T, Allocator> m_ownedFields;

   /**
    * \brief Our list-of-lists.
    */
   std::vector<InnerVector const *> m_innerVectors;
};

#endif
