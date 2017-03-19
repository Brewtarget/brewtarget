/*
 * instruction.h is part of Brewtarget, and is Copyright the following
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

#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

// This class is completely outside the BeerXML spec.

#include <QString>
#include <QVector>
#include <QDomNode>
#include "BeerXMLElement.h"

/*!
 * \class Instruction
 * \author Philip G. Lee
 *
 * \brief Model class for an instruction record in the database.
 */
class Instruction : public BeerXMLElement
{
   Q_OBJECT
   Q_CLASSINFO("signal", "instructions")
   Q_CLASSINFO("prefix", "instruction")
   friend class Database;
public:
   
   virtual ~Instruction() {}

   Q_PROPERTY( QString directions READ directions WRITE setDirections /*NOTIFY changed*/ /*changedDirections*/ )
   Q_PROPERTY( bool hasTimer READ hasTimer WRITE setHasTimer /*NOTIFY changed*/ /*changedHasTimer*/ )
   Q_PROPERTY( QString timerValue READ timerValue WRITE setTimerValue /*NOTIFY changed*/ /*changedTimerValue*/ )
   Q_PROPERTY( bool completed READ completed WRITE setCompleted /*NOTIFY changed*/ /*changedCompleted*/ )
   Q_PROPERTY( double interval READ interval WRITE setInterval /*NOTIFY changed*/ /*changedInterval*/ )
   Q_PROPERTY( QList<QString> reagents READ reagents /*WRITE*/ /*NOTIFY changed*/ /*changedReagents*/ )
   
   Q_PROPERTY( int instructionNumber READ instructionNumber /*WRITE*/ /*NOTIFY changed*/ STORED false )
   
   // "set" methods.
   void setDirections(const QString& dir);
   void setHasTimer(bool has);
   void setTimerValue(const QString& timerVal);
   void setCompleted(bool comp);
   void setInterval(double interval);
   void addReagent(const QString& reagent);

   // "get" methods.
   QString directions();
   bool hasTimer();
   QString timerValue();
   bool completed();
   //! This is a non-stored temporary in-memory set.
   QList<QString> reagents();
   double interval();

   int instructionNumber() const;

   static QString classNameStr();

signals:
   /*
   void changedName(QString);
   void changedDirections(QString);
   void changedHasTimer(bool);
   void changedTimerValue(QString);
   void changedCompleted(bool);
   void changedInterval(double);
   void changedReagents(QVector<QString>);
   */

private:
   //! Only database gets to construct instances.
   Instruction(Brewtarget::DBTable table, int key);
   Instruction( Instruction const& other );
   /*
   Instruction( const QString& name,
                const QString& directions,
                bool hasTimer = false,
                const QString& timerVal = "0" );
   */
   QList<QString> _reagents;
   
   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
};

Q_DECLARE_METATYPE( QList<Instruction*> )

//! \brief Compares Instruction pointers by Instruction::instructionNumber().
inline bool insPtrLtByNumber( Instruction* lhs, Instruction* rhs)
{
   return lhs->instructionNumber() < rhs->instructionNumber();
}

#endif   /* _INSTRUCTION_H */

