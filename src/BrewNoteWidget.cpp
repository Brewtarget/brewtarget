
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

   // Labels
   connect( btLabel_Sg, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_volIntoBk, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_strikeTemp, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_mashFinTemp, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_Og, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_volIntoFerm, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_pitchTemp, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_postFermentFg, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_finalVolume, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_projectedOg, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
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
      lcdnumber_effBK->setLowLim(bNoteObs->projEff_pct() * low);
      lcdnumber_effBK->setHighLim(bNoteObs->projEff_pct() * high);

      lcdnumber_projectedOG->setLowLim( bNoteObs->projOg() * low);
      lcdnumber_projectedOG->setHighLim( bNoteObs->projOg() * high);

      lcdnumber_brewhouseEff->setLowLim(bNoteObs->projEff_pct() * low);
      lcdnumber_brewhouseEff->setHighLim(bNoteObs->projEff_pct() * high);

      lcdnumber_projABV->setLowLim( bNoteObs->projABV_pct() * low);
      lcdnumber_projABV->setHighLim( bNoteObs->projABV_pct() * high);

      lcdnumber_abv->setLowLim( bNoteObs->projABV_pct() * low);
      lcdnumber_abv->setHighLim( bNoteObs->projABV_pct() * high);

      showChanges();
   }
}

// TODO: what's this?
// TODO: In answer to the question, this is a place holder for when I figure
// out how to allow people to reset the brewdate.
void BrewNoteWidget::updateBrewDate()
{
}

void BrewNoteWidget::updateSG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setSg(BrewNote::translateSG(lineEdit_SG->text()));
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

   bNoteObs->setOg(BrewNote::translateSG(lineEdit_OG->text()));
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

   bNoteObs->setFg(BrewNote::translateSG(lineEdit_FG->text()));
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

   bNoteObs->setFermentDate( BeerXMLElement::getDateTime(lineEdit_fermentDate->text()) );
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

void BrewNoteWidget::showChanges(QString field)
{
   if (bNoteObs == 0)
      return;

   lineEdit_SG->setText(Brewtarget::displayOG(bNoteObs->sg(),false,"btLabel_Sg"));
   lineEdit_volIntoBK->setText(Brewtarget::displayAmount(bNoteObs->volumeIntoBK_l(),"btLabel_volIntoBk",Units::liters));
   lineEdit_strikeTemp->setText(Brewtarget::displayAmount(bNoteObs->strikeTemp_c(),"btLabel_strikeTemp", Units::celsius));
   lineEdit_mashFinTemp->setText(Brewtarget::displayAmount(bNoteObs->mashFinTemp_c(),"btLabel_mashFinTemp", Units::celsius));
   lineEdit_OG->setText(Brewtarget::displayOG(bNoteObs->og(),false,"btLabel_Og"));
   lineEdit_postBoilVol->setText(Brewtarget::displayAmount(bNoteObs->postBoilVolume_l(),"btLabel_postBoilVolume", Units::liters));
   lineEdit_volIntoFerm->setText(Brewtarget::displayAmount(bNoteObs->volumeIntoFerm_l(),"btLabel_volumeIntoFerm", Units::liters));
   lineEdit_pitchTemp->setText(Brewtarget::displayAmount(bNoteObs->pitchTemp_c(),"btLabel_pitchTemp",Units::celsius));
   lineEdit_FG->setText(Brewtarget::displayOG(bNoteObs->fg(),false,"btLabel_Fg"));
   lineEdit_finalVol->setText(Brewtarget::displayAmount(bNoteObs->finalVolume_l(),"btLabel_finalVolume", Units::liters));
   lineEdit_fermentDate->setText(bNoteObs->fermentDate_short());
   plainTextEdit_brewNotes->setPlainText(bNoteObs->notes());

   // Now with the calculated stuff
   lcdnumber_effBK->display(bNoteObs->effIntoBK_pct(),2);
   lcdnumber_projectedOG->display( Brewtarget::displayOG(bNoteObs->og(),false,"btLabel_projectedOg"));
   lcdnumber_brewhouseEff->display(bNoteObs->brewhouseEff_pct(),2);
   lcdnumber_projABV->display(bNoteObs->projABV_pct(),2);
   lcdnumber_abv->display(bNoteObs->abv(),2);
   
}

