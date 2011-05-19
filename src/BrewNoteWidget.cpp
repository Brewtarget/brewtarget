
#include <QWidget>
#include <QDate>
#include "BrewNoteWidget.h"
#include "brewnote.h"
#include "brewtarget.h"
#include "observable.h"

BrewNoteWidget::BrewNoteWidget(QWidget *parent) : QWidget(parent)
{
   setupUi(this);
   bNoteObs = 0;

   connect(lineEdit_SG,SIGNAL(editingFinished()),this,SLOT(updateSG()));
   connect(lineEdit_volIntoBK,SIGNAL(editingFinished()),this,SLOT(updateVolumeIntoBK()));
   connect(lineEdit_strikeTemp,SIGNAL(editingFinished()),this,SLOT(updateStrikeTemp()));
   connect(lineEdit_mashFinTemp,SIGNAL(editingFinished()),this,SLOT(updateMashFinTemp()));

   connect(lineEdit_OG,SIGNAL(editingFinished()),this,SLOT(updateOG()));
   connect(lineEdit_postBoilVol,SIGNAL(editingFinished()),this,SLOT(updatePostBoilVolume()));
   connect(lineEdit_volIntoFerm,SIGNAL(editingFinished()),this,SLOT(updateVolumeIntoFerm()));
   connect(lineEdit_pitchTemp,SIGNAL(editingFinished()),this,SLOT(updatePitchTemp()));

   connect(lineEdit_FG,SIGNAL(editingFinished()),this,SLOT(updateFG()));
   connect(lineEdit_finalVol,SIGNAL(editingFinished()),this,SLOT(updateFinalVolume()));
   connect(lineEdit_fermentDate,SIGNAL(editingFinished()),this,SLOT(updateFermentDate()));

   connect(plainTextEdit_brewNotes,SIGNAL(textChanged()), this, SLOT(updateNotes()));
}

BrewNoteWidget::~BrewNoteWidget()
{
}

void BrewNoteWidget::setBrewNote(BrewNote* bNote)
{
   double low = 0.95;
   double high = 1.05;

   if ( bNote && bNote != bNoteObs )
   {
      bNoteObs = bNote;
      setObserved(bNoteObs);

      // Set the highs and the lows for the lcds
      lcdnumber_effBK->setLowLim(bNoteObs->getProjEff() * low);
      lcdnumber_effBK->setHighLim(bNoteObs->getProjEff() * high);

      lcdnumber_projectedOG->setLowLim( bNoteObs->getProjOG() * low);
      lcdnumber_projectedOG->setHighLim( bNoteObs->getProjOG() * high);

      lcdnumber_brewhouseEff->setLowLim(bNoteObs->getProjEff() * low);
      lcdnumber_brewhouseEff->setHighLim(bNoteObs->getProjEff() * high);

      lcdnumber_projABV->setLowLim( bNoteObs->getProjABV() * low);
      lcdnumber_projABV->setHighLim( bNoteObs->getProjABV() * high);

      lcdnumber_abv->setLowLim( bNoteObs->getProjABV() * low);
      lcdnumber_abv->setHighLim( bNoteObs->getProjABV() * high);

      showChanges();
   }
}

// TBD
void BrewNoteWidget::updateBrewDate()
{
}

void BrewNoteWidget::updateSG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setSG(lineEdit_SG->text().toDouble());
   showChanges();
}

void BrewNoteWidget::updateVolumeIntoBK()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setVolumeIntoBK(Brewtarget::volQStringToSI(lineEdit_volIntoBK->text()));
   showChanges();
}

void BrewNoteWidget::updateStrikeTemp()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setStrikeTemp(Brewtarget::tempQStringToSI(lineEdit_strikeTemp->text()));
   showChanges();
}

void BrewNoteWidget::updateMashFinTemp()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setMashFinTemp(Brewtarget::tempQStringToSI(lineEdit_mashFinTemp->text()));
   showChanges();
}

void BrewNoteWidget::updateOG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setOG(lineEdit_OG->text().toDouble());
   showChanges();
}

void BrewNoteWidget::updatePostBoilVolume()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setPostBoilVolume(Brewtarget::volQStringToSI(lineEdit_postBoilVol->text()));
   showChanges();
}

void BrewNoteWidget::updateVolumeIntoFerm()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setVolumeIntoFerm(Brewtarget::volQStringToSI(lineEdit_volIntoFerm->text()));
   showChanges();
}

void BrewNoteWidget::updatePitchTemp()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setPitchTemp(Brewtarget::tempQStringToSI(lineEdit_pitchTemp->text()));
   showChanges();
}

void BrewNoteWidget::updateFG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFG(lineEdit_FG->text().toDouble());
   showChanges();
}

void BrewNoteWidget::updateFinalVolume()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFinalVolume(Brewtarget::volQStringToSI(lineEdit_finalVol->text()));
   showChanges();
}

void BrewNoteWidget::updateFermentDate()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFermentDate(lineEdit_fermentDate->text());
   showChanges();
}

void BrewNoteWidget::updateNotes()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->disableNotification();
   bNoteObs->setNotes(plainTextEdit_brewNotes->toPlainText());
   bNoteObs->reenableNotification();
}

void BrewNoteWidget::notify(Observable* notifier, QVariant /*info*/)
{
   if ( notifier != bNoteObs )
      return;

   showChanges();
}

void BrewNoteWidget::saveAll()
{
   if ( ! bNoteObs )
      return;
   
   bNoteObs->disableNotification();

   updateSG();
   updateVolumeIntoBK();
   updateStrikeTemp();
   updateMashFinTemp();
   updateOG();
   updatePostBoilVolume();
   updateVolumeIntoFerm();
   updatePitchTemp();
   updateFG();
   updateFinalVolume();
   updateFermentDate();
   updateNotes();

   bNoteObs->reenableNotification();
   bNoteObs->forceNotify();

   hide();
}

void BrewNoteWidget::showChanges()
{
   if (bNoteObs == 0)
      return;

   lineEdit_SG->setText(Brewtarget::displayOG(bNoteObs->getSG()));
   lineEdit_volIntoBK->setText(Brewtarget::displayAmount(bNoteObs->getVolumeIntoBK(),Units::liters));
   lineEdit_strikeTemp->setText(Brewtarget::displayAmount(bNoteObs->getStrikeTemp(),Units::celsius));
   lineEdit_mashFinTemp->setText(Brewtarget::displayAmount(bNoteObs->getMashFinTemp(),Units::celsius));
   lineEdit_OG->setText(Brewtarget::displayOG(bNoteObs->getOG()));
   lineEdit_postBoilVol->setText(Brewtarget::displayAmount(bNoteObs->getPostBoilVolume(),Units::liters));
   lineEdit_volIntoFerm->setText(Brewtarget::displayAmount(bNoteObs->getVolumeIntoFerm(),Units::liters));
   lineEdit_pitchTemp->setText(Brewtarget::displayAmount(bNoteObs->getPitchTemp(),Units::celsius));
   lineEdit_FG->setText(Brewtarget::displayOG(bNoteObs->getFG()));
   lineEdit_finalVol->setText(Brewtarget::displayAmount(bNoteObs->getFinalVolume(),Units::liters));
   lineEdit_fermentDate->setText(bNoteObs->getFermentDate().toString(Qt::ISODate));
   plainTextEdit_brewNotes->setPlainText(bNoteObs->getNotes());

   // Now with the calculated stuff
   lcdnumber_effBK->display(bNoteObs->calculateEffIntoBK(),2);
   lcdnumber_projectedOG->display( Brewtarget::displayOG(bNoteObs->calculateOG()));
   lcdnumber_brewhouseEff->display(bNoteObs->calculateBrewHouseEff(),2);
   lcdnumber_projABV->display(bNoteObs->calculateABV(),2);
   lcdnumber_abv->display(bNoteObs->actualABV(),2);
   
}

/*
QPalette selectPalette( int highLow )
{
   return 0;
   switch(highLow)
   {
      case -1:
         return btColors.value("tooLow");
      case 1:
         return btColors.value("tooHigh");
   }
   return btColors.value("good");
}
*/
