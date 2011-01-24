/*
 * equipment.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
#include "equipment.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "HeatCalculations.h"

bool operator<(Equipment &e1, Equipment &e2)
{
   return e1.name < e2.name;
}

bool operator==(Equipment &e1, Equipment &e2)
{
   return e1.name == e2.name;
}

void Equipment::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement equipNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   equipNode = doc.createElement("EQUIPMENT");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name);
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOIL_SIZE");
   tmpText = doc.createTextNode(text(boilSize_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BATCH_SIZE");
   tmpText = doc.createTextNode(text(batchSize_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_VOLUME");
   tmpText = doc.createTextNode(text(tunVolume_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_WEIGHT");
   tmpText = doc.createTextNode(text(tunWeight_kg));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_SPECIFIC_HEAT");
   tmpText = doc.createTextNode(text(tunSpecificHeat_calGC));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TOP_UP_WATER");
   tmpText = doc.createTextNode(text(topUpWater_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TRUB_CHILLER_LOSS");
   tmpText = doc.createTextNode(text(trubChillerLoss_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("EVAP_RATE");
   tmpText = doc.createTextNode(text(evapRate_pctHr));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("REAL_EVAP_RATE");
   tmpText = doc.createTextNode(text(evapRate_lHr));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOIL_TIME");
   tmpText = doc.createTextNode(text(boilTime_min));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CALC_BOIL_VOLUME");
   tmpText = doc.createTextNode(text(calcBoilVolume));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("LAUTER_DEADSPACE");
   tmpText = doc.createTextNode(text(lauterDeadspace_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TOP_UP_KETTLE");
   tmpText = doc.createTextNode(text(topUpKettle_l));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HOP_UTILIZATION");
   tmpText = doc.createTextNode(text(hopUtilization_pct));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes);
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   // My extensions below
   tmpNode = doc.createElement("ABSORPTION");
   tmpText = doc.createTextNode(text(absorption_LKg));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   parent.appendChild(equipNode);
}

//=============================CONSTRUCTORS=====================================

void Equipment::setDefaults()
{
   name = "";
   boilSize_l = 0.0;
   batchSize_l = 0.0;
   tunVolume_l = 0.0;
   tunWeight_kg = 0.0;
   tunSpecificHeat_calGC = 0.0;
   topUpWater_l = 0.0;
   trubChillerLoss_l = 0.0;
   evapRate_pctHr = 0.0;
   evapRate_lHr = 0.0;
   boilTime_min = 0.0;
   calcBoilVolume = false;
   lauterDeadspace_l = 0.0;
   topUpKettle_l = 0.0;
   hopUtilization_pct = 0.0;
   notes = "";
   absorption_LKg = HeatCalculations::absorption_LKg;
}

Equipment::Equipment()
{
   setDefaults();
}

Equipment::Equipment(const QDomNode& equipmentNode)
{
   fromNode(equipmentNode);
}

void Equipment::fromNode(const QDomNode& equipmentNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   bool hasRealEvapRate = false;
   
   setDefaults();
   
   for( node = equipmentNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;
      
      property = node.nodeName();
      textNode = child.toText();
      value = textNode.nodeValue();
      
      if( property == "NAME" )
      {
         name = value;
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("EQUIPMENT says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "BOIL_SIZE" )
      {
         setBoilSize_l(getDouble(textNode));
      }
      else if( property == "BATCH_SIZE" )
      {
         setBatchSize_l(getDouble(textNode));
      }
      else if( property == "TUN_VOLUME" )
      {
         setTunVolume_l(getDouble(textNode));
      }
      else if( property == "TUN_WEIGHT" )
      {
         setTunWeight_kg(getDouble(textNode));
      }
      else if( property == "TUN_SPECIFIC_HEAT" )
      {
         setTunSpecificHeat_calGC(getDouble(textNode));
      }
      else if( property == "TOP_UP_WATER" )
      {
         setTopUpWater_l(getDouble(textNode));
      }
      else if( property == "TRUB_CHILLER_LOSS" )
      {
         setTrubChillerLoss_l(getDouble(textNode));
      }
      else if( property == "EVAP_RATE" && ! hasRealEvapRate )
      {
         setEvapRate_pctHr(getDouble(textNode));
      }
      else if( property == "REAL_EVAP_RATE" )
      {
         setEvapRate_lHr(getDouble(textNode));
         hasRealEvapRate = true;
      }
      else if( property == "BOIL_TIME" )
      {
         setBoilTime_min(getDouble(textNode));
      }
      else if( property == "CALC_BOIL_VOLUME" )
      {
         setCalcBoilVolume(getBool(textNode));
      }
      else if( property == "LAUTER_DEADSPACE" )
      {
         setLauterDeadspace_l(getDouble(textNode));
      }
      else if( property == "TOP_UP_KETTLE" )
      {
         setTopUpKettle_l(getDouble(textNode));
      }
      else if( property == "HOP_UTILIZATION" )
      {
         setHopUtilization_pct(getDouble(textNode));
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "ABSORPTION" ) // My extension.
      {
         setGrainAbsorption_LKg( getDouble(textNode) );
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported EQUIPMENT property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
   
   // Estimate the actual evaporation rate if we didn't get one.
   if( ! hasRealEvapRate )
      setEvapRate_lHr( evapRate_pctHr/(double)100 * boilSize_l );
}

//============================"SET" METHODS=====================================

void Equipment::setName( const QString &var )
{
   name = QString(var);
   hasChanged();
}

void Equipment::setBoilSize_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil size negative: %1").arg(var) );
      boilSize_l = 0;
      hasChanged();
   }
   else
   {
      boilSize_l = var;
      hasChanged();
   }
}

void Equipment::setBatchSize_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: batch size negative: %1").arg(var) );
      batchSize_l = 0;
      hasChanged();
      doCalculations();
   }
   else
   {
      batchSize_l = var;
      hasChanged();
      doCalculations();
   }
}

void Equipment::setTunVolume_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun volume negative: %1").arg(var) );
      tunVolume_l = 0;
      hasChanged();
   }
   else
   {
      tunVolume_l = var;
      hasChanged();
   }
}

void Equipment::setTunWeight_kg( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun weight negative: %1").arg(var) );
      tunWeight_kg = 0;
      hasChanged();
   }
   else
   {
      tunWeight_kg = var;
      hasChanged();
   }
}

void Equipment::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun sp heat negative: %1").arg(var) );
      tunSpecificHeat_calGC = 0;
      hasChanged();
   }
   else
   {
      tunSpecificHeat_calGC = var;
      hasChanged();
   }
}

void Equipment::setTopUpWater_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: top up water negative: %1").arg(var) );
      topUpWater_l = 0;
      hasChanged();
      doCalculations();
   }
   else
   {
      topUpWater_l = var;
      hasChanged();
      doCalculations();
   }
}

void Equipment::setTrubChillerLoss_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: trub chiller loss negative: %1").arg(var) );
      trubChillerLoss_l = 0;
      hasChanged();
      doCalculations();
   }
   else
   {
      trubChillerLoss_l = var;
      hasChanged();
      doCalculations();
   }
}

void Equipment::setEvapRate_pctHr( double var )
{
   if( var < 0.0 || var > 100.0)
   {
      Brewtarget::logW( QString("Equipment: 0 < evap rate < 100: %1").arg(var) );
      evapRate_pctHr = 0;
      hasChanged();
      doCalculations();
   }
   else
   {
      evapRate_pctHr = var;
      hasChanged();
      doCalculations();
   }
}

void Equipment::setEvapRate_lHr( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: evap rate negative: %1").arg(var) );
      evapRate_lHr = 0;
      hasChanged();
      doCalculations();
   }
   else
   {
      evapRate_lHr = var;
      hasChanged();
      doCalculations();
   }
}

void Equipment::setBoilTime_min( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil time negative: %1").arg(var) );
      boilTime_min = 0;
      hasChanged();
      doCalculations();
   }
   else
   {
      boilTime_min = var;
      hasChanged();
      doCalculations();
   }
}

void Equipment::setCalcBoilVolume( bool var )
{
   calcBoilVolume = var;
   hasChanged();
   if( var )
      doCalculations();
}

void Equipment::setLauterDeadspace_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: deadspace negative: %1").arg(var) );
      lauterDeadspace_l = 0;
      hasChanged();
   }
   else
   {
      lauterDeadspace_l = var;
      hasChanged();
   }
}

void Equipment::setTopUpKettle_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: top up kettle negative: %1").arg(var) );
      topUpKettle_l = 0;
      hasChanged();
   }
   else
   {
      topUpKettle_l = var;
      hasChanged();
   }
}

void Equipment::setHopUtilization_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
   {
      Brewtarget::logW( QString("Equipment: 0 < hop utilization < 100: %1").arg(var) );
      hopUtilization_pct = 20;
      hasChanged();
   }
   else
   {
      hopUtilization_pct = var;
      hasChanged();
   }
}

void Equipment::setNotes( const QString &var )
{
   notes = QString(var);
   hasChanged();
}

void Equipment::setGrainAbsorption_LKg(double var)
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: absorption < 0: %1").arg(var) );
      absorption_LKg = HeatCalculations::absorption_LKg;
   }
   else
   {
      absorption_LKg = var;
   }

   hasChanged();
}

//============================"GET" METHODS=====================================

QString Equipment::getName() const
{
   return name;
}

double Equipment::getBoilSize_l() const
{
   return boilSize_l;
}

double Equipment::getBatchSize_l() const
{
   return batchSize_l;
}

double Equipment::getTunVolume_l() const
{
   return tunVolume_l;
}

double Equipment::getTunWeight_kg() const
{
   return tunWeight_kg;
}

double Equipment::getTunSpecificHeat_calGC() const
{
   return tunSpecificHeat_calGC;
}

double Equipment::getTopUpWater_l() const
{
   return topUpWater_l;
}

double Equipment::getTrubChillerLoss_l() const
{
   return trubChillerLoss_l;
}

double Equipment::getEvapRate_pctHr() const
{
   return evapRate_pctHr;
}

double Equipment::getEvapRate_lHr() const
{
   return evapRate_lHr;
}

double Equipment::getBoilTime_min() const
{
   return boilTime_min;
}

bool Equipment::getCalcBoilVolume() const
{
   return calcBoilVolume;
}

double Equipment::getLauterDeadspace_l() const
{
   return lauterDeadspace_l;
}

double Equipment::getTopUpKettle_l() const
{
   return topUpKettle_l;
}

double Equipment::getHopUtilization_pct() const
{
   return hopUtilization_pct;
}

QString Equipment::getNotes() const
{
   return notes;
}

double Equipment::getGrainAbsorption_LKg()
{
   return absorption_LKg;
}

//TODO: take a look at evapRate_pctHr.
void Equipment::doCalculations()
{
   // Only do the calculation if we're asked to.
   if( ! calcBoilVolume )
      return;
   
   /* The equation given the BeerXML 1.0 spec was way wrong. */
   /*
   boilSize_l =
      (batchSize_l - topUpWater_l + trubChillerLoss_l)
      / (1 - (boilTime_min/(double)60) * (evapRate_pctHr/(double)100) );
   */

   boilSize_l = batchSize_l - topUpWater_l + trubChillerLoss_l + (boilTime_min/(double)60)*evapRate_lHr;

   hasChanged();
}

double Equipment::wortEndOfBoil_l( double kettleWort_l ) const
{
   //return kettleWort_l * (1 - (boilTime_min/(double)60) * (evapRate_pctHr/(double)100) );

   return kettleWort_l - (boilTime_min/(double)60)*evapRate_lHr;
}
