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

#include <string>
#include <iostream>
#include <QDomNode>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QDate>
#include "brewnote.h"
#include "brewtarget.h"

// operators for sorts and things
bool operator<(BrewNote& lhs, BrewNote& rhs)
{
   return lhs.brewDate < rhs.brewDate;
}

bool operator==(BrewNote& lhs, BrewNote& rhs)
{
   return lhs.brewDate == rhs.brewDate;
}

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
      else
         tmpText    = doc.createTextNode(text(info.value(i.value())));

      tmpElement.appendChild(tmpText);
      bNtNode.appendChild(tmpElement);
   }

   parent.appendChild(bNtNode);
}

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
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
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
         Brewtarget::log(Brewtarget::ERROR, QObject::tr("BREWNOTE says it is version %1, not version %2. Line %3")
               .arg(getInt(textNode)).arg(version).arg(textNode.lineNumber()) );
      else if ( property == "DATE_FERMENTED_OUT" )
         setFermentDate(value);
      else if ( tags.contains(property) ) // make sure we have that property defined.
         setInfo( tags.value(property), value.toDouble());
   }
   hasChanged();
}

void BrewNote::setDefaults(Recipe* recipe)
{
   Mash* mash;
   MashStep* temp;
   Yeast *yeast;
   Equipment* equip;
   int numYeast = recipe->getNumYeasts();
   double atten_pct = -1.0;

   int i;

   brewDate = QDate::currentDate();
   fermentDate = brewDate.addDays(7);
 
   setInfo("SG",recipe->getBoilGrav());
   setInfo("projBoilGrav",recipe->getBoilGrav());

   setInfo("volumeIntoBK",recipe->getBoilSize_l());
   setInfo("projVolIntoBK",recipe->getBoilSize_l());

   mash = recipe->getMash();
   temp = mash->getMashStep(0);

   if ( temp ) 
   {
      setInfo("strikeTemp",temp->getEndTemp_c());
      setInfo("projStrikeTemp",temp->getEndTemp_c());

      setInfo("mashFinTemp",temp->getEndTemp_c());
      setInfo("projMashFinTemp",temp->getEndTemp_c());
   }

   i = mash->getNumMashSteps();
   if ( i - 2 >= 0 )
   {
      temp = recipe->getMash()->getMashStep(i-2);
      if ( temp )
      {
         setInfo("mashFinTemp",temp->getEndTemp_c());
         setInfo("projMashFinTemp",temp->getEndTemp_c());
      }
   }

   setInfo("OG",recipe->getOg());
   setInfo("projOG",recipe->getOg());

   setInfo("volumeIntoFerm",recipe->estimateFinalVolume_l());
   setInfo("projVolIntoFerm",recipe->estimateFinalVolume_l());

   setInfo("pitchTemp",recipe->getPrimaryTemp_c());

   setInfo("FG",recipe->getFg());
   setInfo("projFG",recipe->getFg());

   setInfo("finalVolume",recipe->estimateFinalVolume_l());

   setInfo("projEff",recipe->getEfficiency_pct());
   setInfo("projPoints",recipe->getPoints(recipe->getBoilSize_l()));
   setInfo("projABV", recipe->getABV_pct());

   for( i = 0; i < numYeast; ++i )
   {
      yeast = recipe->getYeast(i);
      if ( yeast->getAttenuation_pct() > atten_pct )
         atten_pct = yeast->getAttenuation_pct();
   }

   if ( numYeast == 0 || atten_pct < 0.0 )
      atten_pct = 75;
   setInfo("projAtten", atten_pct);

   equip = recipe->getEquipment();
   if ( equip ) 
   {
      double boiloff_hr = equip->getEvapRate_lHr();
      double boil_time  = equip->getBoilTime_min();

      setInfo("boilOff", boiloff_hr * (boil_time/60));
   }

   setInfo("effIntoBK",0.0);
   setInfo("calculatedOG",0.0);
   setInfo("brewhouseEff",0.0);
   setInfo("calculatedABV",0.0);
   setInfo("abv",0.0);
}

// Initializers
BrewNote::BrewNote(Recipe* recipe)
{
   setDefaults(recipe);
}

BrewNote::BrewNote(BrewNote& other)
   : Observable()
{
   brewDate    = other.getBrewDate();
   fermentDate = other.getFermentDate();

   setInfo("SG",other.getSG());
   setInfo("volumeIntoBK",other.getVolumeIntoBK());
   setInfo("strikeTemp",other.getStrikeTemp());
   setInfo("mashFinTemp",other.getMashFinTemp());

   setInfo("OG",other.getOG());
   setInfo("volumeIntoFerm",other.getVolumeIntoFerm());
   setInfo("pitchTemp",other.getPitchTemp());

   setInfo("FG",other.getFG());
   setInfo("finalVolume",other.getFinalVolume());

   setInfo("effIntoBK",other.calculateEffIntoBK());
   setInfo("calculatedOG",other.calculateOG());
   setInfo("brewhouseEff",other.calculateBrewHouseEff());
   setInfo("calculatedABV",other.calculateABV());
   setInfo("abv",other.actualABV());

   setInfo("projBoilGrav",other.getProjBoilGrav());
   setInfo("projStrikeTemp",other.getProjStrikeTemp());
   setInfo("projMashFinTemp",other.getProjMashFinTemp());
   setInfo("projVolIntoBK",other.getProjVolIntoBK());
   setInfo("projOG",other.getProjOG());
   setInfo("projVolIntoFerm",other.getProjVolIntoFerm());
   setInfo("projFG",other.getProjFG());
   setInfo("projABV",other.getProjABV());
   setInfo("projPoints",other.getProjPoints());
}

BrewNote::BrewNote(const QDomNode& bnoteNode)
{
   fromNode(bnoteNode);
}

// Setters
void BrewNote::setBrewDate(QString date)
{
   QDate temp;
   
   if ( date != "" ) 
   {
      temp = QDate::fromString(date,Qt::ISODate);
      if ( temp.isValid() ) 
         brewDate = temp;
      else
      {
         Brewtarget::logW(QObject::tr("Invalid date string %1, defaulting to today").arg(date));
         brewDate = QDate::currentDate();
      }
   }
   else
      brewDate = QDate::currentDate();

   hasChanged();
}

void BrewNote::setFermentDate(QString date)
{

   fermentDate = QDate::fromString(date, Qt::ISODate);
   hasChanged();
}

void BrewNote::setNotes(const QString& var)
{
   notes = QString(var);
   hasChanged();
}

void BrewNote::setInfo(QString label, double var)
{
   if ( var < 0.0 ) 
   {
      Brewtarget::logW(QObject::tr("Brewnote: %1 < 0: %2").arg(label).arg(var));
      var = 1.0;
   }
   else 
      info.insert(label,var);

   hasChanged();
}

void BrewNote::setSG(double var)              { setInfo("SG", var); }
void BrewNote::setVolumeIntoBK(double var)    { setInfo("volumeIntoBK", var); }
void BrewNote::setStrikeTemp(double var)      { setInfo("strikeTemp", var); }
void BrewNote::setMashFinTemp(double var)     { setInfo("mashFinTemp", var); }
void BrewNote::setOG(double var)              { setInfo("OG", var); }
void BrewNote::setPostBoilVolume(double var)  { setInfo("postBoilVolume", var); }
void BrewNote::setVolumeIntoFerm(double var)  { setInfo("volumeIntoFerm", var); }
void BrewNote::setPitchTemp(double var)       { setInfo("pitchTemp", var); }
void BrewNote::setFG(double var)              { setInfo("FG", var); }
void BrewNote::setFinalVolume(double var)     { setInfo("finalVolume", var); }
void BrewNote::setProjBoilGrav(double var)    { setInfo("projBoilGrav", var); }
void BrewNote::setProjVolIntoBK(double var)   { setInfo("projVolIntoBK", var); }
void BrewNote::setProjStrikeTemp(double var)  { setInfo("projStrikeTemp", var); }
void BrewNote::setProjMashFinTemp(double var) { setInfo("projMashFinTemp", var); }
void BrewNote::setProjOG(double var)          { setInfo("projOG", var); }
void BrewNote::setProjVolIntoFerm(double var) { setInfo("projVolIntoFerm", var); }
void BrewNote::setProjFG(double var)          { setInfo("projFG", var); }
void BrewNote::setProjEff(double var)         { setInfo("projEff", var); }
void BrewNote::setProjABV(double var)         { setInfo("projABV", var); }
void BrewNote::setProjPoints(double var)      { setInfo("projPoints",var); }
void BrewNote::setProjAtten(double var)       { setInfo("projAtten", var); }
void BrewNote::setBoilOff(double var)         { setInfo("boilOff", var); }

// Getters
QDate BrewNote::getBrewDate()          const { return brewDate; }
QString BrewNote::getBrewDate_str()    const { return brewDate.toString(Qt::ISODate); }
QDate BrewNote::getFermentDate()       const { return fermentDate; }
QString BrewNote::getFermentDate_str() const { return fermentDate.toString(Qt::ISODate); }
QString BrewNote::getNotes()           const { return notes; }

double BrewNote::getSG() const              { return info.value("SG"); }
double BrewNote::getVolumeIntoBK() const    { return info.value("volumeIntoBK"); }
double BrewNote::getStrikeTemp() const      { return info.value("strikeTemp"); }
double BrewNote::getMashFinTemp() const     { return info.value("mashFinTemp"); }
double BrewNote::getOG() const              { return info.value("OG"); }
double BrewNote::getVolumeIntoFerm() const  { return info.value("volumeIntoFerm"); }
double BrewNote::getPostBoilVolume() const  { return info.value("postBoilVolume"); }
double BrewNote::getPitchTemp() const       { return info.value("pitchTemp"); }
double BrewNote::getFG() const              { return info.value("FG"); }
double BrewNote::getFinalVolume() const     { return info.value("finalVolume"); }
double BrewNote::getProjBoilGrav() const    { return info.value("projBoilGrav"); }
double BrewNote::getProjVolIntoBK() const   { return info.value("projVolIntoBK"); }
double BrewNote::getProjStrikeTemp() const  { return info.value("projStrikeTemp"); }
double BrewNote::getProjMashFinTemp() const { return info.value("projMashFinTemp"); }
double BrewNote::getProjOG() const          { return info.value("projOG"); }
double BrewNote::getProjVolIntoFerm() const { return info.value("projVolIntoFerm"); }
double BrewNote::getProjFG() const          { return info.value("projFG"); }
double BrewNote::getProjEff() const         { return info.value("projEff"); }
double BrewNote::getProjABV() const         { return info.value("projABV"); }
double BrewNote::getProjPoints() const      { return info.value("projPoints"); }
double BrewNote::getProjAtten() const       { return info.value("projAtten"); }
double BrewNote::getBoilOff() const         { return info.value("boilOff"); }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK()
{
   double effIntoBK;
   double maxPoints, actualPoints, sg;

   maxPoints = (getProjPoints() * getProjVolIntoBK());
   sg = getSG();
   actualPoints = (sg - 1) * 1000 * getVolumeIntoBK();

   if (maxPoints <= 0.0)
   {
      Brewtarget::logW(QObject::tr("Avoiding div by 0, maxpoints is %1").arg(maxPoints));
      return 0.0;
   }
   effIntoBK = actualPoints/maxPoints * 100;
   info.insert("effIntoBK", effIntoBK);

   return effIntoBK;
}

// The idea is that based on the preboil gravity, estimate what the actual OG
// will be.
double BrewNote::calculateOG()
{
   double cOG; 
   double points, expectedVol, actualVol;

   points = (getSG()-1) * 1000;
   expectedVol = getProjVolIntoBK() - getBoilOff();
   actualVol   = getVolumeIntoBK();

   if ( expectedVol <= 0.0 )
   {
      Brewtarget::logW(QObject::tr("calculated OG will be off because of bad expected volume into bk %1").arg(expectedVol));
      return 0.0;
   }

   cOG = 1+ ((points * actualVol / expectedVol) / 1000); 

   info.insert("calculatedOG", cOG);

   return cOG;
}

double BrewNote::calculateBrewHouseEff()
{
   double expectedPoints, actualPoints;
   double brewhouseEff;
   
   actualPoints = (getOG()-1.0) * 1000.0 * getVolumeIntoFerm();
   expectedPoints = getProjPoints() * getVolumeIntoBK();

   brewhouseEff = actualPoints/expectedPoints * 100.0;
   info.insert("brewhouseEff", brewhouseEff);

   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewNote::calculateABV()
{
   double atten_pct = getProjAtten();
   double fg = getFG();
   double calculatedABV;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  = 
   // 1 + [(og - 1) * (1.0 - %/100)]
   fg = 1 + ((getOG()-1.0)*(1.0 - atten_pct/100.0));

   calculatedABV = (getOG()-fg)*130;
   info.insert("calculatedABV", calculatedABV);

   return calculatedABV;
}

double BrewNote::actualABV()
{
   double abv;

   abv = (getOG() - getFG()) * 130;
   info.insert("abv", abv);
   
   return abv;
}

// Pay no attention to these.
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
