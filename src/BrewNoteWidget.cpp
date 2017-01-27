/*
 * BrewNoteWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
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
   setObjectName("BrewNoteWidget");

   connect(lineEdit_SG, &BtLineEdit::textModified, this, &BrewNoteWidget::updateSG);
   connect(lineEdit_volIntoBK, &BtLineEdit::textModified, this, &BrewNoteWidget::updateVolumeIntoBK_l);
   connect(lineEdit_strikeTemp, &BtLineEdit::textModified, this, &BrewNoteWidget::updateStrikeTemp_c);
   connect(lineEdit_mashFinTemp, &BtLineEdit::textModified, this, &BrewNoteWidget::updateMashFinTemp_c);

   connect(lineEdit_OG, &BtLineEdit::textModified, this, &BrewNoteWidget::updateOG);
   connect(lineEdit_postBoilVol, &BtLineEdit::textModified, this, &BrewNoteWidget::updatePostBoilVolume_l);
   connect(lineEdit_volIntoFerm, &BtLineEdit::textModified, this, &BrewNoteWidget::updateVolumeIntoFerm_l);
   connect(lineEdit_pitchTemp, &BtLineEdit::textModified, this, &BrewNoteWidget::updatePitchTemp_c);

   connect(lineEdit_FG, &BtLineEdit::textModified, this, &BrewNoteWidget::updateFG);
   connect(lineEdit_finalVol, &BtLineEdit::textModified, this, &BrewNoteWidget::updateFinalVolume_l);
   connect(lineEdit_fermentDate, &QDateTimeEdit::dateTimeChanged, this, &BrewNoteWidget::updateFermentDate);

   connect(btTextEdit_brewNotes, &BtTextEdit::textModified, this, &BrewNoteWidget::updateNotes);

   // A few labels on this page need special handling, so I connect them here
   // instead of how we would normally do this.
   connect(btLabel_projectedOg, &BtLabel::labelChanged, this, &BrewNoteWidget::updateProjOg);
   connect(btLabel_fermentDate, &BtLabel::labelChanged, this, &BrewNoteWidget::updateDateFormat);

   // I think this might work
   updateDateFormat(Unit::noUnit, Unit::noScale);
}

// I should really do this better, but I really cannot bring myself to do
// another UnitSystem for one input field.
void BrewNoteWidget::updateDateFormat(Unit::unitDisplay display,Unit::unitScale scale)
{
   QString format;
   // I need the new unit, not the old
   Unit::unitDisplay unitDsp = (Unit::unitDisplay)Brewtarget::option("fermentDate", Brewtarget::getDateFormat(), "page_postferment", Brewtarget::UNIT).toInt();

   switch(unitDsp)
   {
      case Unit::displayUS:
         format = "MM-dd-yyyy";
         break;
      case Unit::displayImp:
         format = "dd-MM-yyyy";
         break;
      case Unit::displaySI:
      default:
         format = "yyyy-MM-dd";
   }
   lineEdit_fermentDate->setDisplayFormat(format);
}


void BrewNoteWidget::updateProjOg(Unit::unitDisplay oldUnit, Unit::unitScale oldScale)
{
   double low  = 0.95;
   double high = 1.05;
   double quant;
   int precision = 3;

   // I don't think we care about the old unit or scale, just the new ones
   Unit::unitDisplay unitDsp = (Unit::unitDisplay)Brewtarget::option("projOg", Unit::noUnit, "page_preboil", Brewtarget::UNIT).toInt();


   if ( unitDsp == Unit::noUnit )
      unitDsp = Brewtarget::getDensityUnit();

   if ( unitDsp == Unit::displayPlato )
      precision = 0;

   quant = Brewtarget::amountDisplay(bNoteObs, page_preboil, "projOg",Units::sp_grav);
   lcdnumber_projectedOG->setLowLim(  low  * quant );
   lcdnumber_projectedOG->setHighLim( high * quant );
   lcdnumber_projectedOG->display(quant, precision);
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
      connect( bNoteObs, &BeerXMLElement::changed, this, &BrewNoteWidget::changed );

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

bool BrewNoteWidget::isBrewNote(BrewNote* note) { return bNoteObs == note; }

void BrewNoteWidget::updateSG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setSg(lineEdit_SG->toSI());
}

void BrewNoteWidget::updateVolumeIntoBK_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setVolumeIntoBK_l(lineEdit_volIntoBK->toSI());
}

void BrewNoteWidget::updateStrikeTemp_c()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setStrikeTemp_c(lineEdit_strikeTemp->toSI());
}

void BrewNoteWidget::updateMashFinTemp_c()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setMashFinTemp_c(lineEdit_mashFinTemp->toSI());
}

void BrewNoteWidget::updateOG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setOg(lineEdit_OG->toSI());
}

void BrewNoteWidget::updatePostBoilVolume_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setPostBoilVolume_l(lineEdit_postBoilVol->toSI());
   showChanges();
}

void BrewNoteWidget::updateVolumeIntoFerm_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setVolumeIntoFerm_l(lineEdit_volIntoFerm->toSI());
   showChanges();
}

void BrewNoteWidget::updatePitchTemp_c()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setPitchTemp_c(lineEdit_pitchTemp->toSI());
   showChanges();
}

void BrewNoteWidget::updateFG()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFg(lineEdit_FG->toSI());
   showChanges();
}

void BrewNoteWidget::updateFinalVolume_l()
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFinalVolume_l(lineEdit_finalVol->toSI());
//   showChanges();
}

void BrewNoteWidget::updateFermentDate(const QDateTime& datetime)
{
   if (bNoteObs == 0)
      return;

   bNoteObs->setFermentDate(datetime);
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

/*
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
*/

void BrewNoteWidget::showChanges(QString field)
{
   if (bNoteObs == 0)
      return;

   lineEdit_SG->setText(bNoteObs);
   lineEdit_volIntoBK->setText(bNoteObs);
   lineEdit_strikeTemp->setText(bNoteObs);
   lineEdit_mashFinTemp->setText(bNoteObs);
   lineEdit_OG->setText(bNoteObs);
   lineEdit_postBoilVol->setText(bNoteObs);
   lineEdit_volIntoFerm->setText(bNoteObs);
   lineEdit_pitchTemp->setText(bNoteObs);
   lineEdit_FG->setText(bNoteObs);
   lineEdit_finalVol->setText(bNoteObs);

   lineEdit_fermentDate->setDateTime(bNoteObs->fermentDate());
   btTextEdit_brewNotes->setPlainText(bNoteObs->notes());

   // Now with the calculated stuff
   lcdnumber_effBK->display(bNoteObs->effIntoBK_pct(),2);

   // Need to think about these? Maybe use the bubbles?
   updateProjOg(Unit::noUnit,Unit::noScale); // this requires more work, but updateProj does it

   lcdnumber_brewhouseEff->display(bNoteObs->brewhouseEff_pct(),2);
   lcdnumber_projABV->display(bNoteObs->projABV_pct(),2);
   lcdnumber_abv->display(bNoteObs->abv(),2);
   
}

void BrewNoteWidget::focusOutEvent(QFocusEvent *e)
{
   //qDebug() << "Notes lost focus";
}
