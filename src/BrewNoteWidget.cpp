
#include <QWidget>
#include <QDate>
#include "BrewNoteWidget.h"
#include "brewnote.h"
#include "brewtarget.h"

BrewNoteWidget::BrewNoteWidget(QWidget *parent) : QWidget(parent)
{
   setupUi(this);
   bNoteObs = 0;

   connect(lineEdit_SG,SIGNAL(editingFinished()),this,SLOT(updateSG()));
   connect(lineEdit_volIntoBK,SIGNAL(editingFinished()),this,SLOT(updateVolumeIntoBK_l()));
   connect(lineEdit_strikeTemp,SIGNAL(editingFinished()),this,SLOT(updateStrikeTemp_c()));
   connect(lineEdit_mashFinTemp,SIGNAL(editingFinished()),this,SLOT(updateMashFinTemp_c()));

   connect(lineEdit_OG,SIGNAL(editingFinished()),this,SLOT(updateOG()));
   connect(lineEdit_postBoilVol,SIGNAL(editingFinished()),this,SLOT(updatePostBoilVolume_l()));
   connect(lineEdit_volIntoFerm,SIGNAL(editingFinished()),this,SLOT(updateVolumeIntoFerm_l()));
   connect(lineEdit_pitchTemp,SIGNAL(editingFinished()),this,SLOT(updatePitchTemp_c()));

   connect(lineEdit_FG,SIGNAL(editingFinished()),this,SLOT(updateFG()));
   connect(lineEdit_finalVol,SIGNAL(editingFinished()),this,SLOT(updateFinalVolume_l()));
   connect(lineEdit_fermentDate,SIGNAL(editingFinished()),this,SLOT(updateFermentDate()));

   connect(plainTextEdit_brewNotes,SIGNAL(textChanged()), this, SLOT(updateNotes()));
}

void BrewNoteWidget::setBrewNote(BrewNote* bNote)
{
   double low = 0.95;
   double high = 1.05;

   if( bNoteObs != 0 )
      disconnect( bNoteObs, 0, this, 0 );
   
   if ( bNote )
   {
      bNoteObs = bNote;
      connect( bNoteObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );

      // Set the highs and the lows for the lcds
      lcdnumber_effBK->setLowLim(bNoteObs->getProjEff_pct() * low);
      lcdnumber_effBK->setHighLim(bNoteObs->getProjEff_pct() * high);

      lcdnumber_projectedOG->setLowLim( bNoteObs->getProjOG() * low);
      lcdnumber_projectedOG->setHighLim( bNoteObs->getProjOG() * high);

      lcdnumber_brewhouseEff->setLowLim(bNoteObs->getProjEff_pct() * low);
      lcdnumber_brewhouseEff->setHighLim(bNoteObs->getProjEff_pct() * high);

      lcdnumber_projABV->setLowLim( bNoteObs->getProjABV_pct() * low);
      lcdnumber_projABV->setHighLim( bNoteObs->getProjABV_pct() * high);

      lcdnumber_abv->setLowLim( bNoteObs->getProjABV_pct() * low);
      lcdnumber_abv->setHighLim( bNoteObs->getProjABV_pct() * high);

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

   bNoteObs->setSG(lineEdit_SG->text());
   showChanges();
}

void BrewNoteWidget::updateVolumeIntoBK_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setVolumeIntoBK_l(Brewtarget::volQStringToSI(lineEdit_volIntoBK->text()));
   showChanges();
}

void BrewNoteWidget::updateStrikeTemp_c()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setStrikeTemp_c(Brewtarget::tempQStringToSI(lineEdit_strikeTemp->text()));
   showChanges();
}

void BrewNoteWidget::updateMashFinTemp_c()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setMashFinTemp_c(Brewtarget::tempQStringToSI(lineEdit_mashFinTemp->text()));
   showChanges();
}

void BrewNoteWidget::updateOG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setOG(lineEdit_OG->text());
   showChanges();
}

void BrewNoteWidget::updatePostBoilVolume_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setPostBoilVolume_l(Brewtarget::volQStringToSI(lineEdit_postBoilVol->text()));
   showChanges();
}

void BrewNoteWidget::updateVolumeIntoFerm_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setVolumeIntoFerm_l(Brewtarget::volQStringToSI(lineEdit_volIntoFerm->text()));
   showChanges();
}

void BrewNoteWidget::updatePitchTemp_c()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setPitchTemp_c(Brewtarget::tempQStringToSI(lineEdit_pitchTemp->text()));
   showChanges();
}

void BrewNoteWidget::updateFG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFG(lineEdit_FG->text());
   showChanges();
}

void BrewNoteWidget::updateFinalVolume_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFinalVolume_l(Brewtarget::volQStringToSI(lineEdit_finalVol->text()));
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

   bNoteObs->setNotes(plainTextEdit_brewNotes->toPlainText(), false);
}

void BrewNoteWidget::changed(QMetaProperty /*prop*/, QVariant /*val*/)
{
   if ( sender() != bNoteObs )
      return;

   showChanges();
}

void BrewNoteWidget::saveAll()
{
   if ( ! bNoteObs )
      return;
   
   //bNoteObs->disableNotification();

   updateSG();
   updateVolumeIntoBK_l();
   updateStrikeTemp_c();
   updateMashFinTemp_c();
   updateOG();
   updatePostBoilVolume_l();
   updateVolumeIntoFerm_l();
   updatePitchTemp_c();
   updateFG();
   updateFinalVolume_l();
   updateFermentDate();
   updateNotes();

   //bNoteObs->reenableNotification();
   //bNoteObs->forceNotify();

   hide();
}

void BrewNoteWidget::showChanges()
{
   if (bNoteObs == 0)
      return;

   lineEdit_SG->setText(Brewtarget::displayOG(bNoteObs->getSG(),true));
   lineEdit_volIntoBK->setText(Brewtarget::displayAmount(bNoteObs->getVolumeIntoBK_l(),Units::liters));
   lineEdit_strikeTemp->setText(Brewtarget::displayAmount(bNoteObs->getStrikeTemp_c(),Units::celsius));
   lineEdit_mashFinTemp->setText(Brewtarget::displayAmount(bNoteObs->getMashFinTemp_c(),Units::celsius));
   lineEdit_OG->setText(Brewtarget::displayOG(bNoteObs->getOG(),true));
   lineEdit_postBoilVol->setText(Brewtarget::displayAmount(bNoteObs->getPostBoilVolume_l(),Units::liters));
   lineEdit_volIntoFerm->setText(Brewtarget::displayAmount(bNoteObs->getVolumeIntoFerm_l(),Units::liters));
   lineEdit_pitchTemp->setText(Brewtarget::displayAmount(bNoteObs->getPitchTemp_c(),Units::celsius));
   lineEdit_FG->setText(Brewtarget::displayOG(bNoteObs->getFG(),true));
   lineEdit_finalVol->setText(Brewtarget::displayAmount(bNoteObs->getFinalVolume_l(),Units::liters));
   lineEdit_fermentDate->setText(bNoteObs->getFermentDate_short());
   plainTextEdit_brewNotes->setPlainText(bNoteObs->getNotes());

   // Now with the calculated stuff
   lcdnumber_effBK->display(bNoteObs->calculateEffIntoBK_pct(),2);
   lcdnumber_projectedOG->display( Brewtarget::displayOG(bNoteObs->calculateOG()));
   lcdnumber_brewhouseEff->display(bNoteObs->calculateBrewHouseEff_pct(),2);
   lcdnumber_projABV->display(bNoteObs->calculateABV_pct(),2);
   lcdnumber_abv->display(bNoteObs->actualABV_pct(),2);
   
}

