/*
 * WaterEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#include "WaterEditor.h"

#include <QDebug>
#include <QInputDialog>

#include "database/ObjectStoreWrapper.h"
#include "model/Water.h"

// This private implementation class holds all private non-virtual members of WaterEditor
class WaterEditor::impl {
public:
   /**
    * Constructor
    */
   impl(QString const editorName) : editorName{editorName},
                                    observedWater{},
                                    editedWater{} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   QString const editorName;

   // This is the Water object we are "observing" and to which our edits will be committed if and when the user clicks
   // OK
   std::shared_ptr<Water> observedWater;
   // This is a temporary copy of the "observed" Water that holds the live edits (which will be saved if the user clicks
   // OK and lost if the user clicks Cancel)
   std::unique_ptr<Water> editedWater;
};

WaterEditor::WaterEditor(QWidget *parent,
                         QString const editorName) : QDialog(parent),
                                                     pimpl{std::make_unique<impl>(editorName)} {
   setupUi(this);

   // .:TBD:. The QLineEdit::textEdited and QPlainTextEdit::textChanged signals below are sent somewhat more frequently
   // than we really need - ie every time you type a character in the name or notes field.  We should perhaps look at
   // changing the corresponding field types...
   connect(this->buttonBox,           &QDialogButtonBox::accepted,    this, &WaterEditor::saveAndClose);
   connect(this->buttonBox,           &QDialogButtonBox::rejected,    this, &WaterEditor::clearAndClose);
   connect(this->comboBox_alk,        &QComboBox::currentTextChanged, this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_alk,        &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_ca,         &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_cl,         &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_mg,         &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_na,         &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_name,       &QLineEdit::textEdited,         this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_ph,         &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->lineEdit_so4,        &BtLineEdit::textModified,      this, &WaterEditor::inputFieldModified);
   connect(this->plainTextEdit_notes, &QPlainTextEdit::textChanged,   this, &WaterEditor::inputFieldModified);

   this->waterEditRadarChart->init(
      tr("PPM"),
      50,
      {
         {PropertyNames::Water::calcium_ppm,     tr("Calcium")},
         {PropertyNames::Water::bicarbonate_ppm, tr("Bicarbonate")},
         {PropertyNames::Water::sulfate_ppm,     tr("Sulfate")},
         {PropertyNames::Water::chloride_ppm,    tr("Chloride")},
         {PropertyNames::Water::sodium_ppm,      tr("Sodium")},
         {PropertyNames::Water::magnesium_ppm,   tr("Magnesium")}
      }
   );

   return;
}

//WaterEditor::~WaterEditor() = default;
WaterEditor::~WaterEditor() {
   qDebug() << Q_FUNC_INFO << "Cleaning up";
   if (this->pimpl->observedWater) {
      qDebug() <<
         Q_FUNC_INFO << this->pimpl->editorName << ": Was observing" << this->pimpl->observedWater->name() <<
         "#" << this->pimpl->observedWater->key() << " @" << static_cast<void *>(this->pimpl->observedWater.get()) <<
         " (use count" << this->pimpl->observedWater.use_count() << ")";
   }
   if (this->pimpl->editedWater) {
      qDebug() <<
         Q_FUNC_INFO << this->pimpl->editorName << ": Was editing" << this->pimpl->editedWater->name() <<
         "#" << this->pimpl->editedWater->key() << " @" << static_cast<void *>(this->pimpl->editedWater.get());
   }
   return;
}

void WaterEditor::setWater(std::optional<std::shared_ptr<Water>> water) {

   if (this->pimpl->observedWater) {
      qDebug() <<
         Q_FUNC_INFO << this->pimpl->editorName << ": Stop observing" << this->pimpl->observedWater->name() <<
         "#" << this->pimpl->observedWater->key() << " @" << static_cast<void *>(this->pimpl->observedWater.get()) <<
         " (use count" << this->pimpl->observedWater.use_count() << ")";
      disconnect(this->pimpl->observedWater.get(), nullptr, this, nullptr);
      this->pimpl->observedWater.reset();
   }

   if (water) {
      this->pimpl->observedWater = water.value();
      qDebug() <<
         Q_FUNC_INFO << this->pimpl->editorName << ": Now observing" << this->pimpl->observedWater->name() <<
         "#" << this->pimpl->observedWater->key() << " @" << static_cast<void *>(this->pimpl->observedWater.get()) <<
         " (use count" << this->pimpl->observedWater.use_count() << ")";
      this->waterEditRadarChart->addSeries(tr("Current"), Qt::darkGreen, *this->pimpl->observedWater);
      connect(this->pimpl->observedWater.get(), &NamedEntity::changed, this, &WaterEditor::changed);

      // Make a copy of the Water object we are observing
      this->pimpl->editedWater = std::make_unique<Water>(*this->pimpl->observedWater);
      this->pimpl->editedWater->setAmount(0.0);
      this->waterEditRadarChart->addSeries(tr("Modified"), Qt::green, *this->pimpl->editedWater);

      this->showChanges();
   } else {
      qDebug() << Q_FUNC_INFO << this->pimpl->editorName << ": Observing Nothing";
   }

   return;
}

void WaterEditor::newWater(QString folder) {
   QString name = QInputDialog::getText(this, tr("Water name"),
                                              tr("Water name:"));
   if (name.isEmpty()) {
      return;
   }

   qDebug() << Q_FUNC_INFO << this->pimpl->editorName << ": Creating new Water, " << name;

   this->setWater(std::make_shared<Water>(name));
   if (!folder.isEmpty()) {
      this->pimpl->observedWater->setFolder(folder);
   }

   setVisible(true);

   return;
}

void WaterEditor::showChanges(QMetaProperty const * prop) {

   if (!this->pimpl->observedWater) {
      return;
   }

   QString propName;

   bool updateAll = false;

   if (prop == nullptr) {
      qDebug() << Q_FUNC_INFO << this->pimpl->editorName << ": Update all";
      updateAll = true;
   }
   else {
      propName = prop->name();
      qDebug() << Q_FUNC_INFO << this->pimpl->editorName << ": Changed" << propName;
   }

   if (propName == PropertyNames::NamedEntity::name || updateAll) {
      lineEdit_name->setText(this->pimpl->observedWater->name());
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::calcium_ppm || updateAll) {
      lineEdit_ca->setText(this->pimpl->observedWater->calcium_ppm(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::magnesium_ppm || updateAll) {
      lineEdit_mg->setText(this->pimpl->observedWater->magnesium_ppm(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::sulfate_ppm || updateAll) {
      lineEdit_so4->setText(this->pimpl->observedWater->sulfate_ppm(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::sodium_ppm || updateAll) {
      lineEdit_na->setText(this->pimpl->observedWater->sodium_ppm(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::chloride_ppm || updateAll) {
      lineEdit_cl->setText(this->pimpl->observedWater->chloride_ppm(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::bicarbonate_ppm || updateAll) {
      lineEdit_alk->setText(this->pimpl->observedWater->bicarbonate_ppm(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::ph || updateAll) {
      lineEdit_ph->setText(this->pimpl->observedWater->ph(),2);
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::alkalinityAsHCO3 || updateAll) {
      bool typeless = this->pimpl->observedWater->alkalinityAsHCO3();
      comboBox_alk->setCurrentIndex(comboBox_alk->findText(typeless ? "HCO3" : "CaCO3"));
      if (!updateAll) return;
   }
   if (propName == PropertyNames::Water::notes || updateAll) {
      plainTextEdit_notes->setPlainText(this->pimpl->observedWater->notes());
      if (!updateAll) return;
   }

   return;
}

void WaterEditor::inputFieldModified() {
   //
   // What we're doing here is, if one of the input fields on the dialog is modified, we update the corresponding
   // field(s) on this->pimpl->editedWater and replot the radar chart.  That way the user can see the "shape" of their
   // changes in real time.
   //
   // When we come to close the window, depending on whether the user clicked "OK" or "Cancel" we then either copy the
   // changes to the "observed" water (this->pimpl->observedWater) or discard them (resetting this->pimpl->editedWater
   // to be the same as this->pimpl->observedWater).
   //
   QObject const * const signalSender = this->sender();
   // Usually leave the next line commented as otherwise get too much logging when user is typing in notes or name
   // fields.
//   qDebug() << Q_FUNC_INFO << this->pimpl->editorName << ": signal from" << signalSender;
   if (signalSender && signalSender->parent() == this) {
      // .:TBD:. Need to get to the bottom of the relationship between Water::alkalinity and Water::bicarbonate_ppm.  It
      //         feels wrong that we just set both from the same input, but probably needs some more profound thought
      //         about what exactly correct behaviour should be.
      if      (signalSender == this->comboBox_alk)         {this->pimpl->editedWater->setAlkalinityAsHCO3(this->comboBox_alk->currentText() == QString("HCO3"));}
      else if (signalSender == this->lineEdit_alk)         {this->pimpl->editedWater->setBicarbonate_ppm (this->lineEdit_alk->toCanonical().quantity());  // NB continues on next line!
                                                            this->pimpl->editedWater->setAlkalinity      (this->lineEdit_alk->toCanonical().quantity());                 }
      else if (signalSender == this->lineEdit_ca)          {this->pimpl->editedWater->setCalcium_ppm     (this->lineEdit_ca->toCanonical().quantity());                  }
      else if (signalSender == this->lineEdit_cl)          {this->pimpl->editedWater->setChloride_ppm    (this->lineEdit_cl->toCanonical().quantity());                  }
      else if (signalSender == this->lineEdit_mg)          {this->pimpl->editedWater->setMagnesium_ppm   (this->lineEdit_mg->toCanonical().quantity());                  }
      else if (signalSender == this->lineEdit_na)          {this->pimpl->editedWater->setSodium_ppm      (this->lineEdit_na->toCanonical().quantity());                  }
      else if (signalSender == this->lineEdit_name)        {this->pimpl->editedWater->setName            (this->lineEdit_name->text());                         }
      else if (signalSender == this->lineEdit_ph)          {this->pimpl->editedWater->setPh              (this->lineEdit_ph->toCanonical().quantity());                  }
      else if (signalSender == this->lineEdit_so4)         {this->pimpl->editedWater->setSulfate_ppm     (this->lineEdit_so4->toCanonical().quantity());                 }
      else if (signalSender == this->plainTextEdit_notes)  {this->pimpl->editedWater->setNotes           (this->plainTextEdit_notes->toPlainText());            }
      else {
         // If we get here, it's probably a coding error
         qWarning() << Q_FUNC_INFO << "Unrecognised child";
      }

      //
      // Strictly speaking we don't always need to replot the radar chart - eg if a text field changed it doesn't affect
      // the chart - but, for the moment, we just keep things simple and always replot.
      //
      this->waterEditRadarChart->replot();
   }
   return;
}

void WaterEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == this->pimpl->observedWater.get()) {
      this->showChanges(&prop);
   }

   this->waterEditRadarChart->replot();
   return;
}

void WaterEditor::saveAndClose() {
   qDebug() << Q_FUNC_INFO << this->pimpl->editorName;
   if (!this->pimpl->observedWater) {
      // For the moment, if we weren't given a Water object (via setWater) then we don't try to save any changes when
      // the editor is closed.  Arguably, if the user has actually filled in a bunch of data, then we should use that
      // to create and save a new Water object.
      qDebug() << Q_FUNC_INFO << "Save and close with no Water specified, so discarding any inputs";
      return;
   }

   // Apply all the edits
   if (this->pimpl->editedWater) {
      *this->pimpl->observedWater = *this->pimpl->editedWater;
      qDebug() <<
         Q_FUNC_INFO << this->pimpl->editorName << ": Applied edits to Water #" << this->pimpl->observedWater->key() <<
         ":" << this->pimpl->observedWater->name();
   }

   // This is deliberately commented out for now at least as, when we're called from WaterDialog, it is that window that
   // is responsible for adding new Water objects to the Recipe (which results in the Water object being saved in the
   // DB).  If we save the Water object here then the current logic in WaterDialog won't pick up that it needs to be added to the Recipe.
//   if (this->pimpl->observedWater->key() < 0) {
//      qDebug() << Q_FUNC_INFO << "Writing new Water:" << this->pimpl->observedWater->name();
//      ObjectStoreWrapper::insert(this->pimpl->observedWater);
//   }

   setVisible(false);
   return;
}

void WaterEditor::clearAndClose() {
   qDebug() << Q_FUNC_INFO << this->pimpl->editorName;

   // At this point, we want to clear edits, but we _don't_ want to stop observing the Water that's been given to us as
   // our creator (eg WaterDialog) may redisplay us without a repeat call to setWater.

   // This reverts all the input fields
   this->showChanges();

   // Revert all the edits in our temporary copy of the "observed" Water
   if (this->pimpl->observedWater && this->pimpl->editedWater) {
      *this->pimpl->editedWater = *this->pimpl->observedWater;
      qDebug() <<
         Q_FUNC_INFO << this->pimpl->editorName << ": Discarded edits to Water #" <<
         this->pimpl->observedWater->key() << ":" << this->pimpl->observedWater->name();
   }

   setVisible(false); // Hide the window.
   return;
}
