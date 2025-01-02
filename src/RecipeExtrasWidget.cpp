/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * RecipeExtrasWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Peter Buelow <goballstate@gmail.com>
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
#include "RecipeExtrasWidget.h"

#include <QDate>
#include <QWidget>
#include <QDebug>

#include "measurement/Unit.h"
#include "model/Recipe.h"
#include "undoRedo/Undoable.h"
#include "utils/OptionalHelpers.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_RecipeExtrasWidget.cpp"

RecipeExtrasWidget::RecipeExtrasWidget(QWidget* parent) :
   QWidget(parent),
   recipe(nullptr),
   ratingChanged{false} {

   this->setupUi(this);

   // Note that label_age is QLabel, not SmartLabel, as we're "forcing" the measurement to be in days rather than
   // allowing the usual units of PhysicalQuantity::Time
   SMART_FIELD_INIT(RecipeExtrasWidget, label_brewer     , lineEdit_brewer     , Recipe, PropertyNames::Recipe::brewer              );
   SMART_FIELD_INIT(RecipeExtrasWidget, label_asstBrewer , lineEdit_asstBrewer , Recipe, PropertyNames::Recipe::asstBrewer          );
   SMART_FIELD_INIT(RecipeExtrasWidget, label_age        , lineEdit_age        , Recipe, PropertyNames::Recipe::age_days         , 0);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_ageTemp    , lineEdit_ageTemp    , Recipe, PropertyNames::Recipe::ageTemp_c        , 1);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_carbVols   , lineEdit_carbVols   , Recipe, PropertyNames::Recipe::carbonation_vols    );
   SMART_FIELD_INIT(RecipeExtrasWidget, label_acidity    , lineEdit_acidity    , Recipe, PropertyNames::Recipe::beerAcidity_pH         , 1);
   SMART_FIELD_INIT(RecipeExtrasWidget, label_attenuation, lineEdit_attenuation, Recipe, PropertyNames::Recipe::apparentAttenuation_pct, 1);

   // See comment in model/Recipe.cpp about things we measure in days.  If we switched them from Dimensionless to Time,
   // we would need something like this
   // this->lineEdit_age       ->getSmartField().setForcedRelativeScale(Measurement::UnitSystem::RelativeScale::Large);
   connect(this->lineEdit_age         , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateAge          );

   connect(this->lineEdit_ageTemp     , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateAgeTemp      );
   connect(this->lineEdit_asstBrewer  , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateBrewerAsst   );
   connect(this->lineEdit_brewer      , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateBrewer       );
   connect(this->lineEdit_carbVols    , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateCarbonation  );
   connect(this->spinBox_tasteRating  , QOverload<int>::of(&QSpinBox::valueChanged), this, &RecipeExtrasWidget::changeRatings      );
   connect(this->spinBox_tasteRating  , &QAbstractSpinBox::editingFinished         , this, &RecipeExtrasWidget::updateTasteRating  );
   connect(this->dateEdit_date        , &BtOptionalDateEdit::optionalDateChanged   , this, &RecipeExtrasWidget::updateDate         );
   connect(this->btTextEdit_notes     , &BtTextEdit::textModified                  , this, &RecipeExtrasWidget::updateNotes        );
   connect(this->btTextEdit_tasteNotes, &BtTextEdit::textModified                  , this, &RecipeExtrasWidget::updateTasteNotes   );
   connect(this->lineEdit_acidity     , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateAcidity      );
   connect(this->lineEdit_attenuation , &SmartLineEdit::textModified               , this, &RecipeExtrasWidget::updateAttenuation  );

   return;
}

RecipeExtrasWidget::~RecipeExtrasWidget() = default;

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
   if (!this->recipe) { return; }
   Undoable::doOrRedoUpdate(*recipe, TYPE_INFO(Recipe, brewer), lineEdit_brewer->text(), tr("Change Brewer"));
   return;
}

void RecipeExtrasWidget::updateBrewerAsst() {
   if (!this->recipe) { return; }
   if ( lineEdit_asstBrewer->isModified() ) {
      Undoable::doOrRedoUpdate(*recipe, TYPE_INFO(Recipe, asstBrewer), lineEdit_asstBrewer->text(), tr("Change Assistant Brewer"));
   }
   return;
}

void RecipeExtrasWidget::changeRatings([[maybe_unused]] int rating) { ratingChanged = true; }

void RecipeExtrasWidget::updateTasteRating() {
   if (!this->recipe) { return; }
   if ( ratingChanged )
   {
      Undoable::doOrRedoUpdate(*recipe, TYPE_INFO(Recipe, tasteRating), spinBox_tasteRating->value(), tr("Change Taste Rating"));
      ratingChanged = false;
   }
   return;
}

void RecipeExtrasWidget::updateAge() {
   if (!this->recipe) { return; }
   Undoable::doOrRedoUpdate(*recipe, TYPE_INFO(Recipe, age_days), lineEdit_age->getNonOptValue<double>(), tr("Change Age"));
}

void RecipeExtrasWidget::updateAgeTemp() {
   if (!this->recipe) { return; }
   Undoable::doOrRedoUpdate(*recipe, TYPE_INFO(Recipe, ageTemp_c), lineEdit_ageTemp->getNonOptCanonicalQty(), tr("Change Age Temp"));
}

void RecipeExtrasWidget::updateDate(std::optional<QDate> const date) {
   if (!this->recipe) { return; }

   qDebug() << Q_FUNC_INFO;
   if (date) {
      qDebug() << Q_FUNC_INFO << "date" << *date;
   }
   auto dateFromWidget = this->dateEdit_date->optionalDate();
   if (dateFromWidget) {
      qDebug() << Q_FUNC_INFO << "dateFromWidget" << *dateFromWidget;
   }

   // We have to be careful to avoid going round in circles here.  When we call
   // this->dateEdit_date->setOptionalDate(this->recipe->date()) to show the Recipe date in the UI, that will generate a
   // signal that ends up calling this function to say the date on the Recipe has changed, which it hasn't.
   if (date != this->recipe->date()) {
      qDebug() << Q_FUNC_INFO;
      Undoable::doOrRedoUpdate(*recipe, TYPE_INFO(Recipe, date), date, tr("Change Date"));
   }
   return;
}

void RecipeExtrasWidget::updateCarbonation() {
   if (!this->recipe) { return; }

   Undoable::doOrRedoUpdate(*recipe,
                                         TYPE_INFO(Recipe, carbonation_vols),
                                         lineEdit_carbVols->getNonOptCanonicalQty(),
                                         tr("Change Carbonation"));
}

void RecipeExtrasWidget::updateTasteNotes() {
   if (!this->recipe) { return; }

   Undoable::doOrRedoUpdate(*recipe,
                                         TYPE_INFO(Recipe, tasteNotes),
                                         btTextEdit_tasteNotes->toPlainText(),
                                         tr("Edit Taste Notes"));
}

void RecipeExtrasWidget::updateNotes() {
   if (!this->recipe) { return; }

   Undoable::doOrRedoUpdate(*recipe,
                                         TYPE_INFO(Recipe, notes),
                                         btTextEdit_notes->toPlainText(),
                                         tr("Edit Notes"));
}

void RecipeExtrasWidget::updateAcidity() {
   if (!this->recipe) { return; }

   Undoable::doOrRedoUpdate(*recipe,
                                         TYPE_INFO(Recipe, beerAcidity_pH),
                                         lineEdit_acidity->getOptCanonicalQty(),
                                         tr("Change pH"));
}

void RecipeExtrasWidget::updateAttenuation() {
   if (!this->recipe) { return; }

   Undoable::doOrRedoUpdate(*recipe,
                                         TYPE_INFO(Recipe, apparentAttenuation_pct),
                                         lineEdit_attenuation->getOptValue<double>(),
                                         tr("Change Apparent Attenuation"));
}

void RecipeExtrasWidget::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() != this->recipe) {
      return;
   }

   this->showChanges(&prop);
   return;
}

void RecipeExtrasWidget::saveAll() {
   //recObs->disableNotification();

   this->updateBrewer();
   this->updateBrewerAsst();
   this->updateTasteRating();
   this->updateAge();
   this->updateAgeTemp();
   this->updateDate(dateEdit_date->optionalDate());
   this->updateCarbonation();
   this->updateTasteNotes();
   this->updateNotes();
   this->updateAcidity();
   this->updateAttenuation();

   //recObs->reenableNotification();
   //recObs->forceNotify();

   this->hide();
   return;
}

void RecipeExtrasWidget::showChanges(QMetaProperty* prop) {
   if (!this->recipe) { return; }

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
   if (updateAll || propName == PropertyNames::Recipe::age_days        ) { this->lineEdit_age         ->setQuantity (recipe->age_days        ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::ageTemp_c       ) { this->lineEdit_ageTemp     ->setQuantity (recipe->ageTemp_c       ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::asstBrewer      ) { this->lineEdit_asstBrewer  ->setText     (recipe->asstBrewer      ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::brewer          ) { this->lineEdit_brewer      ->setText     (recipe->brewer          ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::carbonation_vols) { this->lineEdit_carbVols    ->setQuantity (recipe->carbonation_vols()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::tasteRating     ) { this->spinBox_tasteRating  ->setValue       (recipe->tasteRating  ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::date            ) { this->dateEdit_date        ->setOptionalDate(recipe->date         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::notes           ) { this->btTextEdit_notes     ->setPlainText(recipe->notes           ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::tasteNotes      ) { this->btTextEdit_tasteNotes->setPlainText(recipe->tasteNotes      ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::beerAcidity_pH  ) { this->lineEdit_acidity     ->setQuantity (recipe->beerAcidity_pH  ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Recipe::apparentAttenuation_pct) { this->lineEdit_attenuation->setQuantity (recipe->apparentAttenuation_pct()); if (!updateAll) { return; } }

   return;
}
