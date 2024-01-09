/*
 * RecipeExtrasWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2024
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Peter Buelow <goballstate@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "RecipeExtrasWidget.h"

#include <QDate>
#include <QWidget>
#include <QDebug>

#include "MainWindow.h"
#include "measurement/Unit.h"
#include "model/Recipe.h"

RecipeExtrasWidget::RecipeExtrasWidget(QWidget* parent) :
   QWidget(parent),
   recipe(nullptr),
   ratingChanged{false} {

   this->setupUi(this);

   // Note that label_primaryAge, label_secAge, label_tertAge, label_age are QLabel, not SmartLabel, as we're "forcing"
   // the measurement to be in days rather than allowing the usual units of PhysicalQuantity::Time
   SMART_FIELD_INIT(RecipeExtrasWidget, label_brewer     , lineEdit_brewer     , Recipe, PropertyNames::Recipe::brewer              );
   SMART_FIELD_INIT(RecipeExtrasWidget, label_asstBrewer , lineEdit_asstBrewer , Recipe, PropertyNames::Recipe::asstBrewer          );
   SMART_FIELD_INIT(RecipeExtrasWidget, label_primaryAge , lineEdit_primaryAge , Recipe, PropertyNames::Recipe::primaryAge_days  , 0);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_primaryTemp, lineEdit_primaryTemp, Recipe, PropertyNames::Recipe::primaryTemp_c    , 1);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_secAge     , lineEdit_secAge     , Recipe, PropertyNames::Recipe::secondaryAge_days, 0);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_secTemp    , lineEdit_secTemp    , Recipe, PropertyNames::Recipe::secondaryTemp_c  , 1);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_tertAge    , lineEdit_tertAge    , Recipe, PropertyNames::Recipe::tertiaryAge_days , 0);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_tertTemp   , lineEdit_tertTemp   , Recipe, PropertyNames::Recipe::tertiaryTemp_c   , 1);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_age        , lineEdit_age        , Recipe, PropertyNames::Recipe::age_days         , 0);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_ageTemp    , lineEdit_ageTemp    , Recipe, PropertyNames::Recipe::ageTemp_c        , 1);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_carbVols   , lineEdit_carbVols   , Recipe, PropertyNames::Recipe::carbonation_vols    );

   // See comment in model/Recipe.cpp about things we measure in days.  If we switched them from Dimensionless to Time,
   // we would need something like this
   // this->lineEdit_primaryAge->getSmartField().setForcedRelativeScale(Measurement::UnitSystem::RelativeScale::Large);
   // this->lineEdit_secAge    ->getSmartField().setForcedRelativeScale(Measurement::UnitSystem::RelativeScale::Large);
   // this->lineEdit_tertAge   ->getSmartField().setForcedRelativeScale(Measurement::UnitSystem::RelativeScale::Large);
   // this->lineEdit_age       ->getSmartField().setForcedRelativeScale(Measurement::UnitSystem::RelativeScale::Large);

   connect(this->lineEdit_age         , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateAge          );
   connect(this->lineEdit_ageTemp     , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateAgeTemp      );
   connect(this->lineEdit_asstBrewer  , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateBrewerAsst   );
   connect(this->lineEdit_brewer      , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateBrewer       );
   connect(this->lineEdit_carbVols    , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateCarbonation  );
   connect(this->lineEdit_primaryAge  , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updatePrimaryAge   );
   connect(this->lineEdit_primaryTemp , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updatePrimaryTemp  );
   connect(this->lineEdit_secAge      , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateSecondaryAge );
   connect(this->lineEdit_secTemp     , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateSecondaryTemp);
   connect(this->lineEdit_tertAge     , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateTertiaryAge  );
   connect(this->lineEdit_tertTemp    , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateTertiaryTemp );
   connect(this->spinBox_tasteRating  , QOverload<int>::of(&QSpinBox::valueChanged), this, &RecipeExtrasWidget::changeRatings      );
   connect(this->spinBox_tasteRating  , &QAbstractSpinBox::editingFinished         , this, &RecipeExtrasWidget::updateTasteRating  );
   connect(this->dateEdit_date        , &QDateTimeEdit::dateChanged                , this, &RecipeExtrasWidget::updateDate         );
   connect(this->btTextEdit_notes     , &BtTextEdit::textModified                  , this, &RecipeExtrasWidget::updateNotes        );
   connect(this->btTextEdit_tasteNotes, &BtTextEdit::textModified                  , this, &RecipeExtrasWidget::updateTasteNotes   );
   return;
}

void RecipeExtrasWidget::setRecipe(Recipe* rec) {
   if (this->recipe) {
      disconnect(this->recipe, 0, this, 0);
   }

   if (rec) {
      this->recipe = rec;
      connect(this->recipe, &NamedEntity::changed, this, &RecipeExtrasWidget::changed);
      this->showChanges();
   }
   return;
}

void RecipeExtrasWidget::updateBrewer() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::brewer, lineEdit_brewer->text(), tr("Change Brewer"));
   return;
}

void RecipeExtrasWidget::updateBrewerAsst() {
   if (!this->recipe) { return;}
   if ( lineEdit_asstBrewer->isModified() ) {
      MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::asstBrewer, lineEdit_asstBrewer->text(), tr("Change Assistant Brewer"));
   }
   return;
}

void RecipeExtrasWidget::changeRatings([[maybe_unused]] int rating) { ratingChanged = true; }

void RecipeExtrasWidget::updateTasteRating() {
   if (!this->recipe) { return;}
   if ( ratingChanged )
   {
      MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::tasteRating, spinBox_tasteRating->value(), tr("Change Taste Rating"));
      ratingChanged = false;
   }
   return;
}

void RecipeExtrasWidget::updatePrimaryAge() {
   if (!this->recipe) { return;}
   // See comment in model/Recipe.cpp for why age_days, primaryAge_days, secondaryAge_days, tertiaryAge_days properties
   // are dimensionless
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::primaryAge_days, lineEdit_primaryAge->getNonOptValueAs<double>(), tr("Change Primary Age"));
}

void RecipeExtrasWidget::updatePrimaryTemp() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::primaryTemp_c, lineEdit_primaryTemp->toCanonical().quantity(), tr("Change Primary Temp"));
}

void RecipeExtrasWidget::updateSecondaryAge() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::secondaryAge_days, lineEdit_secAge->getNonOptValueAs<double>(), tr("Change Secondary Age"));
}

void RecipeExtrasWidget::updateSecondaryTemp() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::secondaryTemp_c, lineEdit_secTemp->toCanonical().quantity(), tr("Change Secondary Temp"));
}

void RecipeExtrasWidget::updateTertiaryAge() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::tertiaryAge_days, lineEdit_tertAge->getNonOptValueAs<double>(), tr("Change Tertiary Age"));
}

void RecipeExtrasWidget::updateTertiaryTemp() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::tertiaryTemp_c, lineEdit_tertTemp->toCanonical().quantity(), tr("Change Tertiary Temp"));
}

void RecipeExtrasWidget::updateAge() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::age_days, lineEdit_age->getNonOptValueAs<double>(), tr("Change Age"));
}

void RecipeExtrasWidget::updateAgeTemp() {
   if (!this->recipe) { return;}
   MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::ageTemp_c, lineEdit_ageTemp->toCanonical().quantity(), tr("Change Age Temp"));
}

void RecipeExtrasWidget::updateDate(QDate const & date) {
   if (!this->recipe) { return;}

   if (date.isNull()) {
      MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::date, dateEdit_date->date(), tr("Change Date"));
   } else {
      // We have to be careful to avoid going round in circles here.  When we call
      // this->dateEdit_date->setDate(this->recipe->date()) to show the Recipe date in the UI, that will generate a
      // signal that ends up calling this function to say the date on the Recipe has changed, which it hasn't.
      if (date != this->recipe->date()) {
         MainWindow::instance().doOrRedoUpdate(*recipe, PropertyNames::Recipe::date, date, tr("Change Date"));
      }
   }
   return;
}

void RecipeExtrasWidget::updateCarbonation() {
   if (!this->recipe) { return;}

   MainWindow::instance().doOrRedoUpdate(*recipe,
                                         PropertyNames::Recipe::carbonation_vols,
                                         lineEdit_carbVols->toCanonical().quantity(),
                                         tr("Change Carbonation"));
}

void RecipeExtrasWidget::updateTasteNotes() {
   if (!this->recipe) { return;}

   MainWindow::instance().doOrRedoUpdate(*recipe,
                                         PropertyNames::Recipe::tasteNotes,
                                         btTextEdit_tasteNotes->toPlainText(),
                                         tr("Edit Taste Notes"));
}

void RecipeExtrasWidget::updateNotes() {
   if (!this->recipe) { return;}

   MainWindow::instance().doOrRedoUpdate(*recipe,
                                         PropertyNames::Recipe::notes,
                                         btTextEdit_notes->toPlainText(),
                                         tr("Edit Notes"));
}

void RecipeExtrasWidget::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() != this->recipe) {
      return;
   }

   showChanges(&prop);
   return;
}

void RecipeExtrasWidget::saveAll() {
   //recObs->disableNotification();

   updateBrewer();
   updateBrewerAsst();
   updateTasteRating();
   updatePrimaryAge();
   updatePrimaryTemp();
   updateSecondaryAge();
   updateSecondaryTemp();
   updateTertiaryAge();
   updateTertiaryTemp();
   updateAge();
   updateAgeTemp();
   updateDate();
   updateCarbonation();
   updateTasteNotes();
   updateNotes();

   //recObs->reenableNotification();
   //recObs->forceNotify();

   hide();
   return;
}

void RecipeExtrasWidget::showChanges(QMetaProperty* prop) {
   if (!this->recipe) { return;}

   bool updateAll = true;
   QString propName;
   if (prop) {
      updateAll = false;
      propName = prop->name();
   }

   // I think we may be going circular here? LineEdit says "change is made",
   // which signals the widget which changes the db, which signals "change is
   // made" which signals the widget, which changes the LineEdit, which says
   // "change is made" ... rinse, lather, repeat
   // Unlike other editors, this one needs to read from recipe when it gets an
   // updateAll
   if (updateAll || propName == PropertyNames::Recipe::age_days         ) { this->lineEdit_age         ->setAmount   (recipe->age_days         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::ageTemp_c        ) { this->lineEdit_ageTemp     ->setAmount   (recipe->ageTemp_c        ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::asstBrewer       ) { this->lineEdit_asstBrewer  ->setText     (recipe->asstBrewer       ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::brewer           ) { this->lineEdit_brewer      ->setText     (recipe->brewer           ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::carbonation_vols ) { this->lineEdit_carbVols    ->setAmount   (recipe->carbonation_vols ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::primaryAge_days  ) { this->lineEdit_primaryAge  ->setAmount   (recipe->primaryAge_days  ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::primaryTemp_c    ) { this->lineEdit_primaryTemp ->setAmount   (recipe->primaryTemp_c    ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::secondaryAge_days) { this->lineEdit_secAge      ->setAmount   (recipe->secondaryAge_days()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::secondaryTemp_c  ) { this->lineEdit_secTemp     ->setAmount   (recipe->secondaryTemp_c  ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::tertiaryAge_days ) { this->lineEdit_tertAge     ->setAmount   (recipe->tertiaryAge_days ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::tertiaryTemp_c   ) { this->lineEdit_tertTemp    ->setAmount   (recipe->tertiaryTemp_c   ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::tasteRating      ) { this->spinBox_tasteRating  ->setValue    (recipe->tasteRating      ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::date             ) { this->dateEdit_date        ->setDate     (recipe->date             ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::notes            ) { this->btTextEdit_notes     ->setPlainText(recipe->notes            ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::tasteNotes       ) { this->btTextEdit_tasteNotes->setPlainText(recipe->tasteNotes       ()); if (!updateAll) { return; } }

   return;
}
