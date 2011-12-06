/*  
 * brewnote.cpp is part of Brewtarget, written by Mik Firestone
 * (mikfire@gmail.com).   Copyright is given to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDomNode>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QDateTime>
#include <algorithm>
#include <QRegExp>
#include "brewnote.h"
#include "brewtarget.h"
#include "Algorithms.h"
#include "mashstep.h"

// operators for sorts and things
bool operator<(BrewNote const& lhs, BrewNote const& rhs)
{
   return lhs.brewDate() < rhs.brewDate();
}

bool operator==(BrewNote const& lhs, BrewNote const& rhs)
{
   return lhs.brewDate() == rhs.brewDate();
}

/*
void BrewNote::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement bNtNode;
   QDomElement tmpElement;
   QDomText tmpText;
   QHash<QString,QString> tags = XMLTagToName();
   QHashIterator<QString,QString> i(tags);

   bNtNode = doc.createElement("BREWNOTE");

   while (i.hasNext())
   {
      i.next();

      tmpElement = doc.createElement(i.key());
      if ( i.key() == "BREWDATE" )
         tmpText    = doc.createTextNode(brewDate.toString(Qt::ISODate));
      else if ( i.key() == "DATE_FERMENTED_OUT" )
         tmpText = doc.createTextNode(fermentDate.toString(Qt::ISODate));
      else if (i.key() == "VERSION")
         tmpText = doc.createTextNode(QString("%1").arg(version));
      else if (i.key() == "NOTES")
         tmpText = doc.createTextNode(QString("%1").arg(notes));
      else
         tmpText    = doc.createTextNode(text(info.value(i.value())));

      tmpElement.appendChild(tmpText);
      bNtNode.appendChild(tmpElement);
   }

   parent.appendChild(bNtNode);
}
*/

/*
void BrewNote::fromNode(const QDomNode& bNoteNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   QHash<QString,QString> tags = XMLTagToName();
   
   for( node = bNoteNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() )
         continue;
      
      textNode = child.isText() ? child.toText()       : QDomText();
      value    = child.isText() ? textNode.nodeValue() : QString();
     
      property = node.nodeName();

      if( property == "BREWDATE" )
         setBrewDate(value);
      else if( property == "VERSION" && version != getInt(textNode) )
         Brewtarget::log(Brewtarget::ERROR, QString("BREWNOTE says it is version %1, not version %2. Line %3")
               .arg(getInt(textNode)).arg(version).arg(textNode.lineNumber()) );
      else if ( property == "DATE_FERMENTED_OUT" )
         setFermentDate(value);
      else if ( property == "NOTES")
         setNotes(value);
      else if ( tags.contains(property) ) // make sure we have that property defined.
         setInfo( tags.value(property), value.toDouble());
   }
   //hasChanged();
}
*/

/*
void BrewNote::setDefaults(Recipe* parent)
{
   Mash* mash = 0;
   MashStep* temp = 0;
   Yeast *yeast = 0;
   Equipment* equip = 0;
   double atten_pct = -1.0;
   int numYeast;
   QList<Yeast*> yeasts;

   int i;

   brewDate = QDateTime::currentDateTime();
   fermentDate = brewDate.addDays(7);
 
   setInfo("SG",parent->boilGrav());
   setInfo("projBoilGrav",parent->boilGrav());

   setInfo("volumeIntoBK",parent->boilSize_l());
   setInfo("projVolIntoBK",parent->boilSize_l());

   mash = parent->mash();

   if ( mash )
   {
      QList<MashStep*> mashSteps = mash->mashSteps();
      int size = mashSteps.size();

      if ( size > 0 ) 
      {
         temp = mashSteps[0];
         setInfo("strikeTemp",temp->endTemp_c());
         setInfo("projStrikeTemp",temp->endTemp_c());

         setInfo("mashFinTemp",temp->endTemp_c());
         setInfo("projMashFinTemp",temp->endTemp_c());
      }

      if ( size - 2 >= 0 )
      {
         temp = mashSteps[size-2];
         setInfo("mashFinTemp",temp->endTemp_c());
         setInfo("projMashFinTemp",temp->endTemp_c());
      }
   }

   setInfo("OG",parent->og());
   setInfo("projOG",parent->og());

   setInfo("postBoilVolume",parent->postBoilVolume_l());
   setInfo("volumeIntoFerm",parent->finalVolume_l());
   setInfo("projVolIntoFerm",parent->finalVolume_l());

   setInfo("pitchTemp",parent->primaryTemp_c());

   setInfo("FG",parent->fg());
   setInfo("projFG",parent->fg());

   setInfo("finalVolume",parent->finalVolume_l());

   setInfo("projEff",parent->efficiency_pct());
   setInfo("projPoints",parent->points(parent->boilSize_l()));
   setInfo("projABV", parent->ABV_pct());

   yeasts = parent->yeasts();
   numYeast = yeasts.size();
   for( i = 0; i < numYeast; ++i )
   {
      yeast = yeasts[i];
      if ( yeast->attenuation_pct() > atten_pct )
         atten_pct = yeast->attenuation_pct();
   }

   if ( numYeast == 0 || atten_pct < 0.0 )
      atten_pct = 75;
   setInfo("projAtten", atten_pct);

   equip = parent->equipment();
   if ( equip ) 
   {
      double boiloff_hr = equip->evapRate_lHr();
      double boil_time  = equip->boilTime_min();

      setInfo("boilOff", boiloff_hr * (boil_time/60));
   }

   setInfo("effIntoBK",0.0);
   setInfo("calculatedOG",0.0);
   setInfo("brewhouseEff",0.0);
   setInfo("calculatedABV",0.0);
   setInfo("abv",0.0);
}
*/

// Initializers
BrewNote::BrewNote()
   : BeerXMLElement()
{
}

BrewNote::BrewNote(BrewNote const& other)
   : BeerXMLElement(other)
{
}

// Setters=====================================================================
void BrewNote::setBrewDate(QDateTime const& date)
{
   set("brewDate", "brewDate", date.toString(Qt::ISODate));
}

void BrewNote::setFermentDate(QDateTime const& date)
{
   set("fermentDate", "fermentDate", date.toString(Qt::ISODate));
}

void BrewNote::setNotes(QString const& var, bool notify)
{
   set("notes", "notes", var, notify);
}

double BrewNote::translateSG(QString qstr)
{
   double var;
   QString unit;
   QRegExp numUnit;

   // Try to make some guesses about what is there.
   numUnit.setPattern("(\\d+(?:\\.\\d+)?|\\.\\d+)\\s*(\\w+)?");
   numUnit.setCaseSensitivity(Qt::CaseInsensitive);

   if ( qstr.contains(numUnit) )
   {
      var  = numUnit.capturedTexts()[1].toDouble();
      unit = numUnit.capturedTexts()[2];
   }
   else 
      var = qstr.toDouble();


   if ( unit.contains("p", Qt::CaseInsensitive) || var > 1.2)
      return Algorithms::Instance().PlatoToSG_20C20C(var);

   return var;
}

void BrewNote::setSg(double var)              { set("sg", "sg", var); }
void BrewNote::setABV(double var)               { set("abv", "abv", var); }
void BrewNote::setVolumeIntoBK_l(double var)    { set("volumeIntoBK_l", "volume_into_bk", var); }
void BrewNote::setEffIntoBK_pct(double var)     { set("effIntoBK_pct", "eff_into_bk", var); }
void BrewNote::setBrewhouseEff_pct(double var)  { set("brewhouseEff_pct", "brewhouse_eff", var); }
void BrewNote::setStrikeTemp_c(double var)      { set("strikeTemp_c", "strike_temp", var); }
void BrewNote::setMashFinTemp_c(double var)     { set("mashFinTemp_c", "mash_final_temp", var); }
void BrewNote::setOg(double var)              { set("og", "og", var); }
void BrewNote::setPostBoilVolume_l(double var)  { set("postBoilVolume_l", "post_boil_volume", var); }
void BrewNote::setVolumeIntoFerm_l(double var)  { set("volumeIntoFerm_l", "volume_into_fermenter", var); }
void BrewNote::setPitchTemp_c(double var)       { set("pitchTemp_c", "pitch_temp", var); }
void BrewNote::setFg(double var)              { set("fg", "fg", var); }
void BrewNote::setFinalVolume_l(double var)     { set("finalVolume_l", "final_volume", var); }
void BrewNote::setProjBoilGrav(double var)      { set("projBoilGrav", "projected_boil_grav", var); }
void BrewNote::setProjVolIntoBK_l(double var)   { set("projVolIntoBK_l", "projected_vol_into_bk", var); }
void BrewNote::setProjStrikeTemp_c(double var)  { set("projStrikeTemp_c", "projected_strike_temp", var); }
void BrewNote::setProjMashFinTemp_c(double var) { set("projMashFinTemp_c", "projected_mash_fin_temp", var); }
void BrewNote::setProjOg(double var)            { set("projOg", "projected_og", var); }
void BrewNote::setProjVolIntoFerm_l(double var) { set("projVolIntoFerm_l", "projected_vol_into_ferm", var); }
void BrewNote::setProjFg(double var)          { set("projFg", "projected_fg", var); }
void BrewNote::setProjEff_pct(double var)         { set("projEff_pct", "projected_eff", var); }
void BrewNote::setProjABV_pct(double var)         { set("projABV_pct", "projected_abv", var); }
void BrewNote::setProjPoints(double var)      { set("projPoints", "projected_points", var); }
void BrewNote::setProjAtten(double var)       { set("projAtten", "projected_atten", var); }
void BrewNote::setBoilOff_l(double var)         { set("boilOff_l", "boil_off", var); }

// Getters
QDateTime BrewNote::brewDate()      const { return QDateTime::fromString(get("brewDate").toString(),Qt::ISODate); }
QString BrewNote::brewDate_str()    const { return get("brewDate").toString(); }
QString BrewNote::brewDate_short()  const { return brewDate().toString("yyyy-MM-dd"); }
QDateTime BrewNote::fermentDate()   const { return QDateTime::fromString(get("fermentDate").toString(),Qt::ISODate); }
QString BrewNote::fermentDate_str() const { return get("fermentDate").toString(); }
QString BrewNote::fermentDate_short() const { return fermentDate().toString("yyyy-MM-dd"); }
QString BrewNote::notes()           const { return get("notes").toString(); }

double BrewNote::sg() const              { return get("sg").toDouble(); }
double BrewNote::abv() const               { return get("abv").toDouble(); }
double BrewNote::volumeIntoBK_l() const    { return get("volume_into_bk").toDouble(); }
double BrewNote::effIntoBK_pct() const     { return get("eff_into_bk").toDouble(); }
double BrewNote::brewhouseEff_pct() const  { return get("brewhouse_eff").toDouble(); }
double BrewNote::strikeTemp_c() const      { return get("strike_temp").toDouble(); }
double BrewNote::mashFinTemp_c() const     { return get("mash_final_temp").toDouble(); }
double BrewNote::og() const              { return get("og").toDouble(); }
double BrewNote::volumeIntoFerm_l() const  { return get("volume_into_fermenter").toDouble(); }
double BrewNote::postBoilVolume_l() const  { return get("post_boil_volume").toDouble(); }
double BrewNote::pitchTemp_c() const       { return get("pitch_temp").toDouble(); }
double BrewNote::fg() const              { return get("fg").toDouble(); }
double BrewNote::finalVolume_l() const     { return get("final_volume").toDouble(); }
double BrewNote::projBoilGrav() const    { return get("projected_boil_grav").toDouble(); }
double BrewNote::projVolIntoBK_l() const   { return get("projected_vol_into_bk").toDouble(); }
double BrewNote::projStrikeTemp_c() const  { return get("projected_strike_temp").toDouble(); }
double BrewNote::projMashFinTemp_c() const { return get("projected_mash_fin_temp").toDouble(); }
double BrewNote::projOg() const          { return get("projected_og").toDouble(); }
double BrewNote::projVolIntoFerm_l() const { return get("projected_vol_into_ferm").toDouble(); }
double BrewNote::projFg() const          { return get("projected_fg").toDouble(); }
double BrewNote::projEff_pct() const         { return get("projected_eff").toDouble(); }
double BrewNote::projABV_pct() const         { return get("projected_abv").toDouble(); }
double BrewNote::projPoints() const      { return get("projected_points").toDouble(); }
double BrewNote::projAtten() const       { return get("projected_atten").toDouble(); }
double BrewNote::boilOff_l() const         { return get("boil_off").toDouble(); }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK_pct()
{
   double effIntoBK;
   double maxPoints, actualPoints;

   maxPoints = (projPoints() * projVolIntoBK_l());
   actualPoints = (sg() - 1) * 1000 * volumeIntoBK_l();

   if (maxPoints <= 0.0)
   {
      Brewtarget::logW(QString("Avoiding div by 0, maxpoints is %1").arg(maxPoints));
      return 0.0;
   }
   effIntoBK = actualPoints/maxPoints * 100;
   setEffIntoBK_pct(effIntoBK);
   
   return effIntoBK;
}

// The idea is that based on the preboil gravity, estimate what the actual OG
// will be.
double BrewNote::calculateOg()
{
   double cOG; 
   double points, expectedVol, actualVol;

   points = (sg()-1) * 1000;
   expectedVol = projVolIntoBK_l() - boilOff_l();
   actualVol   = volumeIntoBK_l();

   if ( expectedVol <= 0.0 )
   {
      Brewtarget::logW(QString("calculated OG will be off because of bad expected volume into bk %1").arg(expectedVol));
      return 0.0;
   }

   cOG = 1+ ((points * actualVol / expectedVol) / 1000); 
   setProjOg(cOG);

   return cOG;
}

double BrewNote::calculateBrewHouseEff_pct()
{
   double expectedPoints, actualPoints;
   double brewhouseEff;
   
   actualPoints = (og()-1.0) * 1000.0 * volumeIntoFerm_l();
   expectedPoints = projPoints() * volumeIntoBK_l();

   brewhouseEff = actualPoints/expectedPoints * 100.0;
   setBrewhouseEff_pct(brewhouseEff);

   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewNote::calculateABV_pct()
{
   double atten_pct = projAtten();
   double calculatedABV;
   double estFg;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  = 
   // 1 + [(og - 1) * (1.0 - %/100)]
   estFg = 1 + ((og()-1.0)*(1.0 - atten_pct/100.0));

   calculatedABV = (og()-estFg)*130;
   setProjABV_pct(calculatedABV);

   return calculatedABV;
}

double BrewNote::calculateActualABV_pct()
{
   double abv;

   abv = (og() - fg()) * 130;
   setABV(abv);
   
   return abv;
}

// Pay no attention to these.
/*
QHash<QString,QString> BrewNote::XMLTagToName()
{
   QHash<QString,QString> temp;

   temp.insert("BREWDATE", "brewDate");
   temp.insert("DATE_FERMENTED_OUT", "fermentDate");
   temp.insert("VERSION", "version");
   temp.insert("SG","SG");
   temp.insert("VOLUME_INTO_BK","volumeIntoBK");
   temp.insert("STRIKE_TEMP","strikeTemp");
   temp.insert("MASH_FINAL_TEMP","mashFinTemp");
   temp.insert("OG","OG");
   temp.insert("POST_BOIL_VOLUME","postBoilVolume");
   temp.insert("VOLUME_INTO_FERMENTER","volumeIntoFerm");
   temp.insert("PITCH_TEMP","pitchTemp");
   temp.insert("FG","FG");
   temp.insert("EFF_INTO_BK","effIntoBK");
   temp.insert("PREDICTED_OG","calculatedOG");
   temp.insert("BREWHOUSE_EFF","brewhouseEff");
   temp.insert("PREDICTED_ABV","calculatedABV");
   temp.insert("ACTUAL_ABV","abv");
   temp.insert("PROJECTED_BOIL_GRAV","projBoilGrav");
   temp.insert("PROJECTED_STRIKE_TEMP","projStrikeTemp");
   temp.insert("PROJECTED_MASH_FIN_TEMP","projMashFinTemp");
   temp.insert("PROJECTED_VOL_INTO_BK","projVolIntoBK");
   temp.insert("PROJECTED_OG","projOG");
   temp.insert("PROJECTED_VOL_INTO_FERM","projVolIntoFerm");
   temp.insert("PROJECTED_FG","projFG");
   temp.insert("PROJECTED_EFF","projEff");
   temp.insert("PROJECTED_ABV","projABV");
   temp.insert("PROJECTED_POINTS","projPoints");
   temp.insert("PROJECTED_ATTEN","projAtten");
   temp.insert("BOIL_OFF", "boilOff");
   temp.insert("FINAL_VOLUME","finalVolume");
   temp.insert("NOTES","notes");

   return temp;
}

QHash<QString,QString> BrewNote::NameToXMLTag()
{
   QHash<QString,QString> temp;
   QHash<QString,QString> src = XMLTagToName();
   QHashIterator<QString,QString> i(src);

   while (i.hasNext() )
   {
      i.next();
      temp.insert(i.value(), i.key());
   }
   return temp;
}
*/
