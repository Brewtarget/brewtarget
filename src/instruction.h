/*
 * instruction.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _INSTRUCTION_H
#define   _INSTRUCTION_H

// This class is completely outside the BeerXML spec.
class Instruction;

#include <QString>
#include "observable.h"
#include <string>
#include <QVector>
#include <QDomNode>
#include "BeerXMLElement.h"

class Instruction : public Observable, public BeerXMLElement
{
public:
   Instruction();
   Instruction(const QDomNode& instructionNode);
   Instruction( const QString& name,
                const QString& directions,
                bool hasTimer = false,
                const QString& timerVal = "0" );
   virtual ~Instruction() {}

   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   //QString toXml();

   // "set" methods.
   void setName(const QString& n);
   void setDirections(const QString& dir);
   void setHasTimer(bool has);
   void setTimerValue(const QString& timerVal);
   void setCompleted(bool comp);
   void setInterval(double interval);
   void setReagent(const QString& reagent);


   // "get" methods.
   QString getName();
   QString getDirections();
   bool getHasTimer();
   QString getTimerValue();
   bool getCompleted();
   QString getReagent(int i);
   QVector<QString> getReagents();
   double getInterval();

private:
   void setDefaults();

   QString name;
   QString directions;
   bool hasTimer;
   QString timerValue; // hh:mm:ss
   bool completed;
   double interval;     // when the action takes place
   QVector<QString> reagents; // what the action works on
};

#endif   /* _INSTRUCTION_H */

