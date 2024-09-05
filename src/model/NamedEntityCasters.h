/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/NamedEntityCasters.h is part of Brewtarget, and is copyright the following authors 2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_NAMEDENTITYCASTERS_H
#define MODEL_NAMEDENTITYCASTERS_H
#pragma once

#include <memory>

#include <QVariant>

class NamedEntity;

/**
 * \brief With raw pointers, as long as we know \c foo is a raw pointer to some subclass of \c NamedEntity, we can
 *        downcast \c foo to be a raw pointer to \c NamedEntity without needing to know the exact type of object it
 *        actually points to.
 *
 *        With shared pointers, we can't do this, because you need to know what you're casting from.  This struct
 *        allows, eg, a subclass of \c NamedEntity to generate helper functions that can be accessed in a generic way
 *        (eg via \c TypeInfo) to do the equivalent shared pointer casting.
 *
 *        This struct needs to be a separate from \c NamedEntity to avoid circular dependencies between \c NamedEntity
 *        and \c TypeInfo.  Fortunately, shared_ptr is implemented in a clever way that it only needs a forward
 *        declaration of the class it is wrapping, so we can use shared_ptr<NamedEntity> here without having to include
 *        model/NamedEntity.h.
 *
 *        NOTE: Instances of this \c NamedEntityCasters should be constructed via \c NamedEntityCasters::construct
 */
struct NamedEntityCasters {
   //! Pointer to a \c downcastPointer function
   std::shared_ptr<NamedEntity>        (*m_pointerDowncaster)(QVariant const &                           );
   //! Pointer to a \c upcastPointer function
   QVariant                            (*m_pointerUpcaster  )(std::shared_ptr<NamedEntity>               );
   //! Pointer to a \c upcastListToVariant function
   QVariant                            (*m_listUpcaster     )(QList<std::shared_ptr<NamedEntity>> const &);
   //! Pointer to a \c downcastList function
   QList<std::shared_ptr<NamedEntity>> (*m_listDowncaster   )(QVariant const &                           );

   /**
    * \brief Converts a QVariant containing `std::shared_ptr<Hop>` or `std::shared_ptr<Fermentable>` etc to
    *        `std::shared_ptr<NamedEntity>`.
    */
   template<typename T>
   static std::shared_ptr<NamedEntity> downcastPointer(QVariant const & input) {
      return std::static_pointer_cast<NamedEntity>(input.value<std::shared_ptr<T>>());
   }

   /**
    * \brief Opposite of \c downcastVariant.  Converts `std::shared_ptr<NamedEntity>` to a QVariant containing
    *        `std::shared_ptr<Hop>` or `std::shared_ptr<Fermentable>` etc.
    */
   template<typename T>
   static QVariant upcastPointer(std::shared_ptr<NamedEntity> input) {
      return QVariant::fromValue(std::static_pointer_cast<T>(input));
   }

   /**
    * \brief Converts `QList<shared_ptr<Hop>>` or `QList<shared_ptr<Fermentable>>` etc to
    *        `QList<shared_ptr<NamedEntity>>`.
    */
   template<typename T>
   static QList< std::shared_ptr<NamedEntity> > downcastList(QList<std::shared_ptr<T>> const & inputList) {
      QList< std::shared_ptr<NamedEntity> > outputList;
      outputList.reserve(inputList.size());
      for (std::shared_ptr<T> ii : inputList) {
         outputList.append(std::static_pointer_cast<NamedEntity>(ii));
      }
      return outputList;
   }

   /**
    * \brief Converts `QList<shared_ptr<NamedEntity>>` to `QList<shared_ptr<Hop>>` or `QList<shared_ptr<Fermentable>>`
    *        etc.
    */
   template<typename T>
   static QList< std::shared_ptr<T> > upcastList(QList<std::shared_ptr<NamedEntity>> const & inputList) {
      QList< std::shared_ptr<T> > outputList;
      outputList.reserve(inputList.size());
      for (std::shared_ptr<NamedEntity> ii : inputList) {
         outputList.append(std::static_pointer_cast<T>(ii));
      }
      return outputList;
   }

   /**
    * \brief In various parts of the generic serialisation code (for XML and JSON), it is useful, for a given subclass
    *        \c T of \c NamedEntity, to have a pointer to a function that can cast a list of base pointers to derived
    *        ones.  This is typically because we want to pass such a list in to the property system so that it can call
    *        a setter function.  This is fortunate because it means we can avoid having the function pointer signature
    *        depend on T (even though it points to a templated function).
    */
   template<typename T>
   static QVariant upcastListToVariant(QList<std::shared_ptr<NamedEntity>> const & inputList) {
      return QVariant::fromValue(NamedEntityCasters::upcastList<T>(inputList));
   }

   /**
    * \brief In counterpart to \c upcastListToVariant, we need to be able to cast in the opposite direction.  Again, we
    *        don't want the function \b signature to depend on T, and again the use of \c QVariant allows this.
    *
    * \param inputList A \c QVariant holding QList< std::shared_ptr<T>>
    */
   template<typename T>
   static QList<std::shared_ptr<NamedEntity>> downcastListFromVariant(QVariant const & inputList) {
      return NamedEntityCasters::downcastList<T>(inputList.value<QList<std::shared_ptr<T>>>());
   }

   /**
    * \brief And because we can't template the constructor of a non-templated class/struct, we need a templated factory
    *        function.
    */
   template<typename T>
   static NamedEntityCasters construct() {
      return {
         NamedEntityCasters::downcastPointer        <T>,
         NamedEntityCasters::upcastPointer          <T>,
         NamedEntityCasters::upcastListToVariant    <T>,
         NamedEntityCasters::downcastListFromVariant<T>
      };
   }

};


/**
 * \brief The downside of the \c NamedEntityCasters is that we now need to declare all sorts of permutations of
 *        Q_DECLARE_METATYPE, including a lot that we'll never actually use in practice.  So it's simpler to have our
 *        own macro that generates all the Q_DECLARE_METATYPE macros we think we'll need for a class.
 *
 *        NOTE: we also need to ensure shared pointers to most subclasses of \c NamedEntity are registered in
 *              utils/MetaTypes.cpp, otherwise we risk "QMetaProperty::read: Unable to handle unregistered datatype"
 *              errors at runtime.
 */
#define BT_DECLARE_METATYPES(ClassName) \
Q_DECLARE_METATYPE(std::shared_ptr<ClassName> ) \
Q_DECLARE_METATYPE(QList<                ClassName *>) \
Q_DECLARE_METATYPE(QList<std::shared_ptr<ClassName> >)

#endif
