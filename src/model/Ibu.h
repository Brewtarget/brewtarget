/*======================================================================================================================
 * model/Ibu.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef MODEL_IBU_H
#define MODEL_IBU_H
#pragma once

/**
 * Common definitions for Ibu estimates and breakdowns.
 *
 * This is slightly ugly, but separating a couple of things out from calculations/IbuMethods.h helps avoid circular
 * dependencies.  The alternative, of making IbuMethods more independent of the model classes requires duplicating
 * definitions inside and outside the model, which is worse IMHO.
 *
 */
namespace Ibu {
   /**
    * \brief The formula used to get IBUs.
    *
    *        Tinseth, Rager, Noonan, Garetz and Daniels are long-established formulae.  mIBU and SMPH are more recent
    *        refinements.
    *
    *        The Tinseth, Rager and Garetz methods are explained and discussed at http://www.realbeer.com/hops/FAQ.html
    *        Garetz and Daniels (along with Tinseth, Rager, Noonan and Mosher) are summarised in Michael L. Hall, "What's
    *        Your IBU?", Zymurgy Special Edition, 1997.
    *
    *        The SMPH and mIBU methods are discussed by John-Paul Hosom at https://byo.com/article/ibu/ and, in more
    *        detail, at
    *         - https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *         - https://jphosom.github.io/alchemyoverlord/blog/11-a-summary-of-factors-affecting-ibus/alchemyoverlord-blog-content11.html
    *         - https://jphosom.github.io/alchemyoverlord/blog/31-ibus-and-the-smph-model/alchemyoverlord-blog-content31.html
    *
    *        For SMPH, the definitive reference is Hosom's own calculator, at
    *        https://jphosom.github.io/alchemyoverlord/ibu_SMPH.html, whose source is
    *        https://github.com/jphosom/alchemyoverlord/blob/master/js/ibu_SMPH.js.  It differs in a few places from the
    *        blog posts -- see comments in IbuMethods.cpp.
    */
   enum class Formula {
      Tinseth,
      Rager  ,
      Noonan ,
      Garetz ,
      Daniels,
      mIbu   ,
      Smph   ,
   };
   // Note that we can't use the Q_ENUM macro here to allow storing the above enum class in a QVariant, because Q_ENUM
   // is designed to be used inside a class that derives from QObject.

   /**
    * \brief IBUs from a hop addition, split between:
    *          - \c boil_ibus: isomerized alpha acids (IAA) from isomerization during the boil proper
    *          - \c postBoil_ibus: IAA from isomerization after flameout (eg during a whirlpool or hop stand, or simply
    *            while the wort is cooling).  Formulae that do not model post-boil isomerization always give 0 for this.
    *          - \c nonIaa_ibus: "auxiliary bittering compounds" (oxidized alpha and beta acids, and polyphenols), which
    *            dissolve quickly rather than needing isomerization.  Only SMPH models these separately -- other
    *            formulae fold them into their utilization curves, and give 0 for this.
    */
   struct Breakdown {
      double boil_ibus     = 0.0;
      double postBoil_ibus = 0.0;
      double nonIaa_ibus   = 0.0;
      double total() const { return this->boil_ibus + this->postBoil_ibus + this->nonIaa_ibus; }
      Breakdown & operator*=(double const factor) {
         this->boil_ibus     *= factor;
         this->postBoil_ibus *= factor;
         this->nonIaa_ibus   *= factor;
         return *this;
      }
   };


}

#endif