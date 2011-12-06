/*
 * mash.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <iostream>
#include <string>
#include <QVector>
#include "mash.h"
#include "mashstep.h"
#include "ui_mainWindow.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>

bool operator<(Mash &m1, Mash &m2)
{
   return m1.name() < m2.name();
}

bool operator==(Mash &m1, Mash &m2)
{
   return m1.name() == m2.name();
}

/*
void Mash::setDefaults()
{
   name = "";
   grainTemp_c = 21.0;
   mashSteps = QVector<MashStep *>();
   notes = "";
   tunTemp_c = 21.0;
   spargeTemp_c = 74.0; // 74C is recommended in John Palmer's How to Brew.
   ph = 7.0;
   tunWeight_kg = 0.0;
   tunSpecificHeat_calGC = 0.0;
   equipAdjust = true;
}
*/

Mash::Mash()
   : BeerXMLElement()
{
}

/*
void Mash::fromNode(const QDomNode& mashNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = mashNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() )
         continue;
      
      property = node.nodeName();
      if( child.isText() )
         textNode = child.toText();
      value = textNode.nodeValue();
      
      if( property == "NAME" )
      {
         name = value;
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("YEAST says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "GRAIN_TEMP" )
      {
         setGrainTemp_c(getDouble(textNode));
      }
      else if( property == "MASH_STEPS" )
      {
         QDomNode step;
         
         for( step = child; ! step.isNull(); step = step.nextSibling() )
            addMashStep(new MashStep(step));
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "TUN_TEMP" )
      {
         setTunTemp_c(getDouble(textNode));
      }
      else if( property == "SPARGE_TEMP" )
      {
         setSpargeTemp_c(getDouble(textNode));
      }
      else if( property == "PH" )
      {
         setPh(getDouble(textNode));
      }
      else if( property == "TUN_WEIGHT" )
      {
         setTunWeight_kg(getDouble(textNode));
      }
      else if( property == "TUN_SPECIFIC_HEAT" )
      {
         setTunSpecificHeat_calGC(getDouble(textNode));
      }
      else if( property == "EQUIP_ADJUST" )
      {
         setEquipAdjust(getBool(textNode));
      }
      else
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported MASH property: %1. Line %2").arg(property).arg(node.lineNumber()) );
      }
   }
}
*/

void Mash::setName( const QString& var )
{
   set("name", "name", var);
}

void Mash::setGrainTemp_c( double var )
{
   set("grainTemp_c", "grain_temp", var);
}

void Mash::setNotes( const QString& var )
{
   set("notes", "notes", var);
}

void Mash::setTunTemp_c( double var )
{
   set("tunTemp_c", "tun_temp", var);
}

void Mash::setSpargeTemp_c( double var )
{
   set("spargeTemp_c", sparge_temp, var);
}

void Mash::setPh( double var )
{
   if( var < 0.0 || var > 14.0 )
   {
      Brewtarget::logW( QString("Mash: 0 < pH < 14: %1").arg(var) );
      return;
   }
   else
   {
      set("ph", "ph", var);
   }
}

void Mash::setTunWeight_kg( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Mash: tun weight < 0: %1").arg(var) );
      return;
   }
   else
   {
      set("tunWeight_kg", "tun_weight", var);
   }
}

void Mash::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Mash: sp heat < 0: %1").arg(var) );
      return;
   }
   else
   {
      set("tunSpecificHeat_calGC", "tun_specific_heat", var);
   }
}

void Mash::setEquipAdjust( bool var )
{
   set("equipAdjust", "equip_adjust", var);
}

void Mash::addMashStep(MashStep* step)
{
   if( step == 0 )
      return;
   
   mashSteps.push_back(step);
   addObserved(step);
   hasChanged();
}

void Mash::removeMashStep(MashStep* step)
{
   if( step == 0 )
      return;

   QVector<MashStep*>::iterator it;
   for( it = mashSteps.begin(); it != mashSteps.end(); it++ )
   {
      if(*it == step )
      {
         mashSteps.erase(it);
         removeObserved(*it);
         hasChanged();
         return;
      }
   }
}

void Mash::removeAllMashSteps()
{
   MashStep* step;

   while( mashSteps.size() > 0 )
   {
      step = mashSteps.back();
      mashSteps.pop_back(); // Remove last element.
      delete step; // Delete storage.
   }

   hasChanged();
}

//============================="GET" METHODS====================================
QString Mash::getName() const
{
   return name;
}

double Mash::getGrainTemp_c() const
{
   return grainTemp_c;
}

unsigned int Mash::getNumMashSteps() const
{
   return mashSteps.size();
}

MashStep* Mash::getMashStep( unsigned int i )
{
  if( i >= static_cast<unsigned int>(mashSteps.size()) )
      return 0;
   else
      return mashSteps[i];
}

QString Mash::getNotes() const
{
   return notes;
}

double Mash::getTunTemp_c() const
{
   return tunTemp_c;
}

double Mash::getSpargeTemp_c() const
{
   return spargeTemp_c;
}

double Mash::getPh() const
{
   return ph;;
}

double Mash::getTunWeight_kg() const
{
   return tunWeight_kg;
}

double Mash::getTunSpecificHeat_calGC() const
{
   return tunSpecificHeat_calGC;
}

bool Mash::getEquipAdjust() const
{
   return equipAdjust;
}

// === other methods ===
double Mash::totalMashWater_l() const
{
   unsigned int i, size;
   double waterAdded_l = 0.0;
   MashStep* step;
   
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
   {
      step = mashSteps[i];
      
      if( step->getType() == MashStep::TYPEINFUSION )
      waterAdded_l += step->getInfuseAmount_l();
   }
   
   return waterAdded_l;
}

double Mash::getTotalTime()
{
   unsigned int i;
   double totalTime = 0.0;
   MashStep* mstep;

   for( i = 0; i < getNumMashSteps(); ++i )
   {
      mstep = getMashStep(i);
      totalTime += mstep->getStepTime_min();
   }
   return totalTime;
}

void Mash::notify(Observable *notifier, QVariant info)
{
   unsigned int i, size;
   size = mashSteps.size();
   
   for( i = 0; i < size; ++i )
   {
      if( mashSteps[i] == notifier )
      {
         hasChanged(QVariant(i)); // Mash notifies its observers of which mashStep changed.
         return;
      }
   }
}

void Mash::swapSteps( unsigned int i, unsigned int j )
{
   if( i < 0 || j < 0 || static_cast<int>(i) >= mashSteps.size() || static_cast<int>(j) >= mashSteps.size() )
      return; // Bad indices.

   MashStep* tmp = mashSteps[i];
   mashSteps[i] = mashSteps[j];
   mashSteps[j] = tmp;

   hasChanged();
}
