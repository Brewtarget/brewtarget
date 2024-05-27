/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/CuriouslyRecurringTemplateBase.h is part of Brewtarget, and is copyright the following authors 2023:
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
#ifndef UTILS_CURIOUSLYRECURRINGTEMPLATEBASE_H
#define UTILS_CURIOUSLYRECURRINGTEMPLATEBASE_H
#pragma once

/**
 * \brief Trivial base class to implement the common functionality of Curiously Recurring Template Pattern (CRTP).
 *
 *        CRTP base classes can inherit from this class to avoid having to implement the standard \c derived member
 *        function.  (The inheritance needs to be public, otherwise you'll get compiler errors about inaccessible
 *        bases.)
 *
 *        The first template parameter here is a phantom type to avoid diamond inheritance if a class uses more than
 *        one CRTP.  It's described in more detail at https://www.fluentcpp.com/2017/05/19/crtp-helper/.
 *
 *        Usage is:
 *
 *           template<class Derived>
 *           class MyCrtpClass : public CuriouslyRecurringTemplateBase<MyCrtpClass, Derived> { ... }
 *
 *           ...
 *
 *           class Concrete : public MyCrtpClass<Concrete> { ... }
 *
 *        This means we're using CRTP in the CRTP helper.
 *
 *        HOWEVER, there is one more twist.  If the template class directly inheriting from
 *        \c CuriouslyRecurringTemplateBase has more than one template parameter, then it cannot be used as the phantom
 *        type parameter.  Instead, a dedicated type must be used:
 *
 *           template<class Derived> MyCrtpClassPhantom;
 *           template<class Derived, class Foo, class Bar>
 *           class MyCrtpClass : public CuriouslyRecurringTemplateBase<MyCrtpClassPhantom, Derived> { ... }
 *
 *        In a few cases we already have a traits class for the class directly inheriting from
 *        \c CuriouslyRecurringTemplateBase, so that can be used as the phantom type.
 *
 *        There is doubtless some more elegant way of doing this without the inheriting class having to declare phantom
 *        types, but I haven't discovered it yet.
 */
template<template<typename> class PhantomType, class Derived>
class CuriouslyRecurringTemplateBase {
protected:
   /**
    * \brief We need const and non-const versions of the function to downcast 'this' pointer to the derived class, which
    *        allows us to call non-virtual member functions in the derived class from the templated base class.
    */
   Derived       & derived()       { return static_cast<Derived      &>(*this); }
   Derived const & derived() const { return static_cast<Derived const&>(*this); }
};

#endif
