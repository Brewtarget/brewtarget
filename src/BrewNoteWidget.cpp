
#include <QWidget>
#include <QDate>
#include <QDebug>
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

   connect(btTextEdit_brewNotes,SIGNAL(textModified()), this, SLOT(updateNotes()));

   // Labels
   connect( btLabel_Sg, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_volIntoBk, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_strikeTemp, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_mashFinTemp, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_Og, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_volIntoFerm, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_pitchTemp, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
   connect( btLabel_postBoilVol, SIGNAL(labelChanged(QString)), this, SLOT(showChanges(QString)));
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

   if ( lineEdit_SG->isModified() )
   {
      bNoteObs->setSg(BrewNote::translateSG(lineEdit_SG->text()));
      showChanges();
   }
}

void BrewNoteWidget::updateVolumeIntoBK_l()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if ( lineEdit_volIntoBK->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("volumeIntoBK_l", noUnit,page_preboil,Brewtarget::UNIT).toInt();
      bNoteObs->setVolumeIntoBK_l(Brewtarget::volQStringToSI(lineEdit_volIntoBK->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updateStrikeTemp_c()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if ( lineEdit_strikeTemp->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("strikeTemp_c",noUnit,page_preboil,Brewtarget::UNIT).toInt();
      bNoteObs->setStrikeTemp_c(Brewtarget::tempQStringToSI(lineEdit_strikeTemp->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updateMashFinTemp_c()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if ( lineEdit_mashFinTemp->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("mashFinTemp_c",noUnit,page_preboil,Brewtarget::UNIT).toInt();
      bNoteObs->setMashFinTemp_c(Brewtarget::tempQStringToSI(lineEdit_mashFinTemp->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updateOG()
{
   if (bNoteObs == 0)
      return;

   if ( lineEdit_OG->isModified() )
   {
      bNoteObs->setOg(BrewNote::translateSG(lineEdit_OG->text()));
      showChanges();
   }
}

void BrewNoteWidget::updatePostBoilVolume_l()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if (lineEdit_postBoilVol->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("postBoilVolume_l",noUnit,page_postboil,Brewtarget::UNIT).toInt();
      bNoteObs->setPostBoilVolume_l(Brewtarget::volQStringToSI(lineEdit_postBoilVol->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updateVolumeIntoFerm_l()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if ( lineEdit_volIntoFerm->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("volumeIntoFerm_l",noUnit,page_postboil,Brewtarget::UNIT).toInt();
      bNoteObs->setVolumeIntoFerm_l(Brewtarget::volQStringToSI(lineEdit_volIntoFerm->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updatePitchTemp_c()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if ( lineEdit_pitchTemp->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("pitchTemp_c",noUnit,page_postboil,Brewtarget::UNIT).toInt();
      bNoteObs->setPitchTemp_c(Brewtarget::tempQStringToSI(lineEdit_pitchTemp->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updateFG()
{
   if (bNoteObs == 0)
      return;

   if ( lineEdit_FG->isModified() )
   {
      bNoteObs->setFg(BrewNote::translateSG(lineEdit_FG->text()));
      showChanges();
   }
}

void BrewNoteWidget::updateFinalVolume_l()
{
   unitDisplay dispUnit;
   if (bNoteObs == 0)
      return;

   if ( lineEdit_finalVol->isModified() )
   {
      dispUnit  = (unitDisplay)Brewtarget::option("finalVolume_l",noUnit,page_postferment,Brewtarget::UNIT).toInt();
      bNoteObs->setFinalVolume_l(Brewtarget::volQStringToSI(lineEdit_finalVol->text(),dispUnit));
      showChanges();
   }
}

void BrewNoteWidget::updateFermentDate()
{
   if (bNoteObs == 0)
      return;

   if (lineEdit_fermentDate->isModified() )
   {
      bNoteObs->setFermentDate( BeerXMLElement::getDateTime(lineEdit_fermentDate->text()) );
      showChanges();
   }
}

void BrewNoteWidget::updateNotes()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setNotes(btTextEdit_brewNotes->toPlainText(), false);
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

   hide();
}

void BrewNoteWidget::showChanges(QString field)
{
   double low  = 0.95;
   double high = 1.05;

   if (bNoteObs == 0)
      return;

   lineEdit_SG->setText(Brewtarget::displayOG(bNoteObs, page_preboil, "sg",false));
   lineEdit_volIntoBK->setText(Brewtarget::displayAmount(bNoteObs,page_preboil,"volumeIntoBK_l",Units::liters));
   lineEdit_strikeTemp->setText(Brewtarget::displayAmount(bNoteObs, page_preboil, "strikeTemp_c", Units::celsius));
   lineEdit_mashFinTemp->setText(Brewtarget::displayAmount(bNoteObs, page_preboil, "mashFinTemp_c", Units::celsius));
   lineEdit_OG->setText(Brewtarget::displayOG(bNoteObs, page_postboil, "og",false));
   lineEdit_postBoilVol->setText(Brewtarget::displayAmount(bNoteObs, page_postboil, "postBoilVolume_l", Units::liters));
   lineEdit_volIntoFerm->setText(Brewtarget::displayAmount(bNoteObs, page_postboil, "volumeIntoFerm_l", Units::liters));
   lineEdit_pitchTemp->setText(Brewtarget::displayAmount(bNoteObs, page_postboil, "pitchTemp_c",Units::celsius));

   lineEdit_FG->setText(Brewtarget::displayOG(bNoteObs, page_postferment, "fg",false));
   lineEdit_finalVol->setText(Brewtarget::displayAmount(bNoteObs, page_postferment, "finalVolume_l", Units::liters));
   lineEdit_fermentDate->setText(bNoteObs->fermentDate_short());
   btTextEdit_brewNotes->setPlainText(bNoteObs->notes());

   // Now with the calculated stuff
   lcdnumber_effBK->display(bNoteObs->effIntoBK_pct(),2);

   lcdnumber_projectedOG->setLowLim( low * Brewtarget::displayOG(bNoteObs, scrollAreaWidgetContents, "projOg",false).toDouble() );
   lcdnumber_projectedOG->setHighLim( high * Brewtarget::displayOG(bNoteObs, scrollAreaWidgetContents, "projOg",false).toDouble() );
   lcdnumber_projectedOG->display( Brewtarget::displayOG(bNoteObs, scrollAreaWidgetContents, "projOg",false));

   lcdnumber_brewhouseEff->display(bNoteObs->brewhouseEff_pct(),2);
   lcdnumber_projABV->display(bNoteObs->projABV_pct(),2);
   lcdnumber_abv->display(bNoteObs->abv(),2);
   
}

void BrewNoteWidget::focusOutEvent(QFocusEvent *e)
{
   //qDebug() << "Notes lost focus";
}
