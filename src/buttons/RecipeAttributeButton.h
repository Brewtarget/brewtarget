/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * buttons/RecipeAttributeButton.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef BUTTONS_RECIPEATTRIBUTEBUTTON_H
#define BUTTONS_RECIPEATTRIBUTEBUTTON_H
#pragma once

#include <QPushButton>
#include <QMetaProperty>
#include <QVariant>

#include "model/Recipe.h"
#include "model/RecipeUseOfWater.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

//=============================================== RecipeAttributeButton ================================================

/**
 * \brief Base class for button that shows a named related object (\c Mash / \c Style / \c Equipment / \c Boil /
 *        \c Fermentation, \c Water) for a \c Recipe.
 *
 *        Subclasses must also inherit from the CRTP class \c RecipeAttributeButtonBase (see below) which provides the
 *        substance.   (This class just declares Qt slots etc.)
 */
class RecipeAttributeButton : public QPushButton {
   Q_OBJECT
public:
   RecipeAttributeButton(QWidget * parent);
   virtual ~RecipeAttributeButton();

protected slots:
   virtual void          recipeChanged(QMetaProperty, QVariant) = 0;
   virtual void whatButtonShowsChanged(QMetaProperty, QVariant) = 0;
};

//============================================= RecipeAttributeButtonBase ==============================================

/**
 * \brief Buttons that inherit from \c RecipeAttributeButton (see above) also inherit from this CRTP class so they can
 *        have strongly-typed member functions such as \c MashButton::setMash, \c StyleButton::setStyle, etc.
 */
template<class Derived> class RecipeAttributeButtonBasePhantom;
template<class Derived, class WhatButtonShows>
class RecipeAttributeButtonBase : public CuriouslyRecurringTemplateBase<RecipeAttributeButtonBasePhantom, Derived> {
protected:
   RecipeAttributeButtonBase() :
      m_recipe{nullptr},
      m_whatButtonShows{nullptr} {
      return;
   }

   ~RecipeAttributeButtonBase() = default;

public:
   /**
    * \brief Observe \c Recipe
    */
   void setRecipe(Recipe * recipe) {
      if (this->m_recipe) {
         this->derived().disconnect(this->m_recipe, nullptr, &this->derived(), nullptr);
      }

      this->m_recipe = recipe;
      if (this->m_recipe) {
         this->derived().connect(this->m_recipe, &NamedEntity::changed, &this->derived(), &Derived::recipeChanged );
         // According to Clang, we need to "use 'template' keyword to treat 'get' as a dependent template name" here
         this->setWhatButtonShows(this->m_recipe->template get<WhatButtonShows>());
      } else {
         this->setWhatButtonShows(nullptr);
      }
      return;

   }

   /**
    * \brief Observe \c Mash, \c Style etc (according to what \c WhatButtonShows is).
    *
    *        Note that, typically, class users should not need to call this function directly.  It should suffice to
    *        call \c setRecipe.
    */
   void setWhatButtonShows(std::shared_ptr<WhatButtonShows> whatButtonShows) {
      if (this->m_whatButtonShows) {
         this->derived().disconnect(this->m_whatButtonShows.get(), nullptr, &this->derived(), nullptr);
      }

      this->m_whatButtonShows = whatButtonShows;
      if(this->m_whatButtonShows) {
         this->derived().connect(this->m_whatButtonShows.get(), &NamedEntity::changed, &this->derived(), &Derived::whatButtonShowsChanged);
         this->derived().setText   (Derived::tr("Edit «%1»").arg(this->m_whatButtonShows->name()));
         this->derived().setToolTip(Derived::tr("Edit «%1» %2").arg(this->m_whatButtonShows->name()).arg(WhatButtonShows::localisedName()));
      } else {
         this->derived().setText("");
         this->derived().setToolTip("");
      }
      return;
   }

   //! \return the observed \c Mash, \c Style etc (according to what \c WhatButtonShows is).
   std::shared_ptr<WhatButtonShows> getWhatButtonShows() const {
      return this->m_whatButtonShows;
   }

protected:
   //! \brief \c Derived should call this from its \c recipeChanged slot
   void doRecipeChanged(QMetaProperty prop, [[maybe_unused]] QVariant val) {
      if (prop.name() == Recipe::propertyNameFor<WhatButtonShows>()) {
         this->setWhatButtonShows(this->m_recipe->template get<WhatButtonShows>());
      }
      return;
   }

   //! \brief \c Derived should call this from its \c whatButtonShowsChanged slot
   void doWhatButtonShowsChanged(QMetaProperty prop, QVariant val) {
      if (prop.name() == PropertyNames::NamedEntity::name) {
         this->derived().setText(val.toString());
      }
      return;
   }

protected:
   Recipe * m_recipe;
   std::shared_ptr<WhatButtonShows> m_whatButtonShows;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define RECIPE_ATTRIBUTE_BUTTON_BASE_DECL(WhatButtonShows) \
   /* This allows RecipeAttributeButtonBase to call protected and private members of the derived class. */ \
   friend class RecipeAttributeButtonBase<WhatButtonShows##Button, WhatButtonShows>;          \
                                                                                              \
   public:                                                                                    \
   WhatButtonShows##Button(QWidget * parent = nullptr);                                       \
   virtual ~WhatButtonShows##Button();                                                        \
                                                                                              \
   std::shared_ptr<WhatButtonShows> get##WhatButtonShows();                                   \
   void set##WhatButtonShows(std::shared_ptr<WhatButtonShows> val);                           \
                                                                                              \
   private slots:                                                                             \
   virtual void          recipeChanged(QMetaProperty, QVariant);                              \
   virtual void whatButtonShowsChanged(QMetaProperty, QVariant);                              \

/**
 * \brief WhatButtonShows##Button classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define RECIPE_ATTRIBUTE_BUTTON_BASE_COMMON_CODE(WhatButtonShows)  \
   WhatButtonShows##Button::WhatButtonShows##Button(QWidget * parent) :                       \
      RecipeAttributeButton{parent},                                                          \
      RecipeAttributeButtonBase<WhatButtonShows##Button, WhatButtonShows>{} {                 \
      return;                                                                                 \
   }                                                                                          \
   WhatButtonShows##Button::~WhatButtonShows##Button() = default;                             \
                                                                                              \
   std::shared_ptr<WhatButtonShows> WhatButtonShows##Button::get##WhatButtonShows() {         \
      return this->getWhatButtonShows();                                                      \
   }                                                                                          \
   void WhatButtonShows##Button::set##WhatButtonShows(std::shared_ptr<WhatButtonShows> val) { \
      this->setWhatButtonShows(val);                                                          \
      return;                                                                                 \
   }                                                                                          \
                                                                                              \
   void WhatButtonShows##Button::         recipeChanged(QMetaProperty prop, QVariant val) {   \
      this->doRecipeChanged(prop, val);                                                       \
      return;                                                                                 \
   }                                                                                          \
   void WhatButtonShows##Button::whatButtonShowsChanged(QMetaProperty prop, QVariant val) {   \
      this->doWhatButtonShowsChanged(prop, val);                                              \
      return;                                                                                 \
   }                                                                                          \

#endif
