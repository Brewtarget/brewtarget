/*======================================================================================================================
 * tools/OgCorrectionTool.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Matt Young <mfsy@yahoo.com>
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
 =====================================================================================================================*/
#ifndef TOOLS_OGCORRECTIONTOOL_H
#define TOOLS_OGCORRECTIONTOOL_H
#pragma once

#include <QDialog>

#include "ui_ogCorrectionTool.h"

class Recipe;

/*!
 * \brief The Original Gravity Correction Tool helps you correct your OG on brew day, as follows:
 *           - If your gravity is too high you can add water to the boil.
 *           - If it is too low you can either add DME/sugar or boil for longer.
 *
 *        Of course, if things have gone wrong, then correcting the gravity won't guarantee to get you the taste you
 *        were aiming for, but it might help.
 */
class OgCorrectionTool : public QDialog, public Ui::ogCorrectionTool {
   Q_OBJECT

public:
   explicit OgCorrectionTool(QWidget * parent = nullptr);
   ~OgCorrectionTool() override;

   //! Set the recipe whose OG to correct.
   void setRecipe(Recipe * rec);

public slots:
   void newlySelectedRecipe();
   void calculate();

private:
   // This is technically const because it doesn't modify member variables, even though it changes state of things they
   // point to!
   void hideOutputs() const;

   //! Set the text that depends on \c m_outcome
   void showOutcomeTexts();

   Recipe * m_recObs = nullptr;

   /**
    * \brief What the last calculation found, so that we can redo the text that depends on it if the user changes
    *        language.  (This is \c mutable because \c hideOutputs needs to reset it.)
    */
   enum class Outcome {
      NotCalculated,
      OnTarget     ,
      AboveTarget  ,
      BelowTarget  ,
   };
   mutable Outcome m_outcome = Outcome::NotCalculated;
protected:
   /**
    * \brief Overrides \c QWidget::changeEvent so that, on \c QEvent::LanguageChange, we update our text to the new
    *        language.
    */
   void changeEvent(QEvent * event) override;
};

#endif