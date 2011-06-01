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
#include <QDateTime>
#include <algorithm>
#include <QRegExp>
#include "brewnote.h"
#include "brewtarget.h"
#include "Algorithms.h"

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

void BrewNote::setDefaults(Recipe* parent)
{
   Mash* mash;
   MashStep* temp;
   Yeast *yeast;
   Equipment* equip;
   int numYeast = parent->getNumYeasts();
   double atten_pct = -1.0;

   int i;

   brewDate = QDateTime::currentDateTime();
   fermentDate = brewDate.addDays(7);
 
   setInfo("SG",parent->getBoilGrav());
   setInfo("projBoilGrav",parent->getBoilGrav());

   setInfo("volumeIntoBK",parent->getBoilSize_l());
   setInfo("projVolIntoBK",parent->getBoilSize_l());

   mash = parent->getMash();

   if ( mash ) 
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
      if ( mash )
         temp = mash->getMashStep(i-2);
      if ( temp )
      {
         setInfo("mashFinTemp",temp->getEndTemp_c());
         setInfo("projMashFinTemp",temp->getEndTemp_c());
      }
   }

   setInfo("OG",parent->getOg());
   setInfo("projOG",parent->getOg());

   setInfo("postBoilVolume",parent->estimatePostBoilVolume_l());
   setInfo("volumeIntoFerm",parent->estimateFinalVolume_l());
   setInfo("projVolIntoFerm",parent->estimateFinalVolume_l());

   setInfo("pitchTemp",parent->getPrimaryTemp_c());

   setInfo("FG",parent->getFg());
   setInfo("projFG",parent->getFg());

   setInfo("finalVolume",parent->estimateFinalVolume_l());

   setInfo("projEff",parent->getEfficiency_pct());
   setInfo("projPoints",parent->getPoints(parent->getBoilSize_l()));
   setInfo("projABV", parent->getABV_pct());

   for( i = 0; i < numYeast; ++i )
   {
      yeast = parent->getYeast(i);
      if ( yeast->getAttenuation_pct() > atten_pct )
         atten_pct = yeast->getAttenuation_pct();
   }

   if ( numYeast == 0 || atten_pct < 0.0 )
      atten_pct = 75;
   setInfo("projAtten", atten_pct);

   equip = parent->getEquipment();
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
BrewNote::BrewNote(Recipe* parent)
{
   setDefaults(parent);
   rec = parent;
}

BrewNote::BrewNote(BrewNote& other)
   : Observable()
{
   rec         = other.rec;

   brewDate    = other.getBrewDate();
   fermentDate = other.getFermentDate();

   setInfo("SG",other.getSG());
   setInfo("volumeIntoBK",other.getVolumeIntoBK_l());
   setInfo("strikeTemp",other.getStrikeTemp_c());
   setInfo("mashFinTemp",other.getMashFinTemp_c());

   setInfo("OG",other.getOG());
   setInfo("volumeIntoFerm",other.getVolumeIntoFerm_l());
   setInfo("pitchTemp",other.getPitchTemp_c());

   setInfo("FG",other.getFG());
   setInfo("finalVolume",other.getFinalVolume_l());

   setInfo("effIntoBK",other.calculateEffIntoBK_pct());
   setInfo("calculatedOG",other.calculateOG());
   setInfo("brewhouseEff",other.calculateBrewHouseEff_pct());
   setInfo("calculatedABV",other.calculateABV_pct());
   setInfo("abv",other.actualABV_pct());

   setInfo("projBoilGrav",other.getProjBoilGrav());
   setInfo("projStrikeTemp",other.getProjStrikeTemp_c());
   setInfo("projMashFinTemp",other.getProjMashFinTemp_c());
   setInfo("projVolIntoBK",other.getProjVolIntoBK_l());
   setInfo("projOG",other.getProjOG());
   setInfo("projVolIntoFerm",other.getProjVolIntoFerm_l());
   setInfo("projFG",other.getProjFG());
   setInfo("projABV",other.getProjABV_pct());
   setInfo("projPoints",other.getProjPoints());
}

BrewNote::BrewNote(Recipe* parent, const QDomNode& bnoteNode)
{
   fromNode(bnoteNode);
   rec = parent;
}

// Setters
void BrewNote::setParent(Recipe* parent)
{
   rec = parent;
}

void BrewNote::setBrewDate(QString date)
{
   QDateTime temp;
   
   if ( date != "" ) 
   {
      temp = QDateTime::fromString(date,Qt::ISODate);
      if ( temp.isValid() ) 
         brewDate = temp;
      else
      {
         Brewtarget::logW(QObject::tr("Invalid date string %1, defaulting to today").arg(date));
         brewDate = QDateTime::currentDateTime();
      }
   }
   else
      brewDate = QDateTime::currentDateTime();

   hasChanged();
}

void BrewNote::setFermentDate(QString date)
{

   fermentDate = QDateTime::fromString(date, Qt::ISODate);
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
/*
   there is no UnitSystem for gravity, so we need to translate here.  
   If the user has appended a "P" (case insensitive, with or without a space),
   the gravity reading will be assumed to be in plato/brix.
   If there is no unit, and the value is < 2.0, it will be assumed to be a
   specific gravity.
   If there is no unit and the value is greater than 2.0, it will be assumed
   to be a plato/brix
*/
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

void BrewNote::setSG(QString var)              { setInfo("SG", translateSG(var)); }
void BrewNote::setVolumeIntoBK_l(double var)    { setInfo("volumeIntoBK", var); }
void BrewNote::setStrikeTemp_c(double var)      { setInfo("strikeTemp", var); }
void BrewNote::setMashFinTemp_c(double var)     { setInfo("mashFinTemp", var); }
void BrewNote::setOG(QString var)              { setInfo("OG", translateSG(var)); }
void BrewNote::setPostBoilVolume_l(double var)  { setInfo("postBoilVolume", var); }
void BrewNote::setVolumeIntoFerm_l(double var)  { setInfo("volumeIntoFerm", var); }
void BrewNote::setPitchTemp_c(double var)       { setInfo("pitchTemp", var); }
void BrewNote::setFG(QString var)              { setInfo("FG", translateSG(var)); }
void BrewNote::setFinalVolume_l(double var)     { setInfo("finalVolume", var); }
void BrewNote::setProjBoilGrav(double var)      { setInfo("projBoilGrav", var); }
void BrewNote::setProjVolIntoBK_l(double var)   { setInfo("projVolIntoBK", var); }
void BrewNote::setProjStrikeTemp_c(double var)  { setInfo("projStrikeTemp", var); }
void BrewNote::setProjMashFinTemp_c(double var) { setInfo("projMashFinTemp", var); }
void BrewNote::setProjOG(double var)            { setInfo("projOG", var); }
void BrewNote::setProjVolIntoFerm_l(double var) { setInfo("projVolIntoFerm", var); }
void BrewNote::setProjFG(double var)          { setInfo("projFG", var); }
void BrewNote::setProjEff_pct(double var)         { setInfo("projEff", var); }
void BrewNote::setProjABV_pct(double var)         { setInfo("projABV", var); }
void BrewNote::setProjPoints(double var)      { setInfo("projPoints",var); }
void BrewNote::setProjAtten(double var)       { setInfo("projAtten", var); }
void BrewNote::setBoilOff_l(double var)         { setInfo("boilOff", var); }

// Getters
Recipe* BrewNote::getParent()          const { return rec; }
QDateTime BrewNote::getBrewDate()      const { return brewDate; }
QString BrewNote::getBrewDate_str()    const { return brewDate.toString(Qt::ISODate); }
QString BrewNote::getBrewDate_short()  const { return brewDate.toString("yyyy-MM-dd"); }
QDateTime BrewNote::getFermentDate()   const { return fermentDate; }
QString BrewNote::getFermentDate_str() const { return fermentDate.toString(Qt::ISODate); }
QString BrewNote::getFermentDate_short() const { return fermentDate.toString("yyyy-MM-dd"); }
QString BrewNote::getNotes()           const { return notes; }

double BrewNote::getSG() const              { return info.value("SG"); }
double BrewNote::getVolumeIntoBK_l() const    { return info.value("volumeIntoBK"); }
double BrewNote::getStrikeTemp_c() const      { return info.value("strikeTemp"); }
double BrewNote::getMashFinTemp_c() const     { return info.value("mashFinTemp"); }
double BrewNote::getOG() const              { return info.value("OG"); }
double BrewNote::getVolumeIntoFerm_l() const  { return info.value("volumeIntoFerm"); }
double BrewNote::getPostBoilVolume_l() const  { return info.value("postBoilVolume"); }
double BrewNote::getPitchTemp_c() const       { return info.value("pitchTemp"); }
double BrewNote::getFG() const              { return info.value("FG"); }
double BrewNote::getFinalVolume_l() const     { return info.value("finalVolume"); }
double BrewNote::getProjBoilGrav() const    { return info.value("projBoilGrav"); }
double BrewNote::getProjVolIntoBK_l() const   { return info.value("projVolIntoBK"); }
double BrewNote::getProjStrikeTemp_c() const  { return info.value("projStrikeTemp"); }
double BrewNote::getProjMashFinTemp_c() const { return info.value("projMashFinTemp"); }
double BrewNote::getProjOG() const          { return info.value("projOG"); }
double BrewNote::getProjVolIntoFerm_l() const { return info.value("projVolIntoFerm"); }
double BrewNote::getProjFG() const          { return info.value("projFG"); }
double BrewNote::getProjEff_pct() const         { return info.value("projEff"); }
double BrewNote::getProjABV_pct() const         { return info.value("projABV"); }
double BrewNote::getProjPoints() const      { return info.value("projPoints"); }
double BrewNote::getProjAtten() const       { return info.value("projAtten"); }
double BrewNote::getBoilOff_l() const         { return info.value("boilOff"); }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK_pct()
{
   double effIntoBK;
   double maxPoints, actualPoints, sg;

   maxPoints = (getProjPoints() * getProjVolIntoBK_l());
   sg = getSG();
   actualPoints = (sg - 1) * 1000 * getVolumeIntoBK_l();

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
   expectedVol = getProjVolIntoBK_l() - getBoilOff_l();
   actualVol   = getVolumeIntoBK_l();

   if ( expectedVol <= 0.0 )
   {
      Brewtarget::logW(QObject::tr("calculated OG will be off because of bad expected volume into bk %1").arg(expectedVol));
      return 0.0;
   }

   cOG = 1+ ((points * actualVol / expectedVol) / 1000); 

   info.insert("calculatedOG", cOG);

   return cOG;
}

double BrewNote::calculateBrewHouseEff_pct()
{
   double expectedPoints, actualPoints;
   double brewhouseEff;
   
   actualPoints = (getOG()-1.0) * 1000.0 * getVolumeIntoFerm_l();
   expectedPoints = getProjPoints() * getVolumeIntoBK_l();

   brewhouseEff = actualPoints/expectedPoints * 100.0;
   info.insert("brewhouseEff", brewhouseEff);

   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewNote::calculateABV_pct()
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

double BrewNote::actualABV_pct()
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
