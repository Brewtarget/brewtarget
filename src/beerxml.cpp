/*
 * beerxml.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2025
 * - Mik Firestone <mikfire@gmail.com>
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

#include "beerxml.h"

#include <QList>
#include <QDomDocument>
#include <QIODevice>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>
#include <QTextCodec>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QPair>

#include "Algorithms.h"
#include "DatabaseSchema.h"
#include "brewnote.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "instruction.h"
#include "mash.h"
#include "mashstep.h"
#include "misc.h"
#include "recipe.h"
#include "style.h"
#include "water.h"
#include "salt.h"
#include "yeast.h"

#include "TableSchema.h"
BeerXML::BeerXML(DatabaseSchema* tables) : QObject(),
   m_tables(tables)
{
}


QString BeerXML::textFromValue(QVariant value, QString type)
{
   QString retval = value.toString();

   if (type == "boolean" )
      retval = Ingredient::text(value.toBool());
   else if (type == "real")
      retval = Ingredient::text(value.toDouble());
   else if (type == "timestamp")
      retval = Ingredient::text(value.toDate());

   return retval;
}

void BeerXML::toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::BREWNOTETABLE);

   node = doc.createElement("BREWNOTE");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Equipment* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::EQUIPTABLE);

   node = doc.createElement("EQUIPMENT");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::FERMTABLE);

   node = doc.createElement("FERMENTABLE");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Hop* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   node = doc.createElement("HOP");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Instruction* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::INSTRUCTIONTABLE);

   node = doc.createElement("INSTRUCTION");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Mash* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   int i, size;
   TableSchema* tbl = m_tables->table(Brewtarget::MASHTABLE);

   node = doc.createElement("MASH");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }

   tmpElement = doc.createElement("MASH_STEPS");
   QList<MashStep*> mashSteps = a->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
      toXml( mashSteps[i], doc, tmpElement);
   node.appendChild(tmpElement);

   parent.appendChild(node);

}

void BeerXML::toXml( MashStep* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::MASHSTEPTABLE);

   node = doc.createElement("MASH_STEP");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString val;
         // flySparge and batchSparge aren't part of the BeerXML spec.
         // This makes sure we give BeerXML something it understands.
         if ( element == kpropType ) {
            if ( (a->type() == MashStep::flySparge) || (a->type() == MashStep::batchSparge ) ) {
               val = MashStep::types[0];
            }
            else {
               val = a->typeString();
            }
         }
         else {
            val = textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element));
         }
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(val);
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Misc* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);

   node = doc.createElement("MISC");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Recipe* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::RECTABLE);

   int i;

   node = doc.createElement("RECIPE");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }

   Style* style = a->style();
   if( style != nullptr )
      toXml( style, doc, node);

   tmpElement = doc.createElement("HOPS");
   QList<Hop*> hops = a->hops();
   for( i = 0; i < hops.size(); ++i )
      toXml( hops[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("FERMENTABLES");
   QList<Fermentable*> ferms = a->fermentables();
   for( i = 0; i < ferms.size(); ++i )
      toXml( ferms[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("MISCS");
   QList<Misc*> miscs = a->miscs();
   for( i = 0; i < miscs.size(); ++i )
      toXml( miscs[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("YEASTS");
   QList<Yeast*> yeasts = a->yeasts();
   for( i = 0; i < yeasts.size(); ++i )
      toXml( yeasts[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("WATERS");
   QList<Water*> waters = a->waters();
   for( i = 0; i < waters.size(); ++i )
      toXml( waters[i], doc, tmpElement);
   node.appendChild(tmpElement);

   Mash* mash = a->mash();
   if( mash != nullptr )
      toXml( mash, doc, node);

   tmpElement = doc.createElement("INSTRUCTIONS");
   QList<Instruction*> instructions = a->instructions();
   for( i = 0; i < instructions.size(); ++i )
      toXml( instructions[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("BREWNOTES");
   QList<BrewNote*> brewNotes = a->brewNotes();
   for(i=0; i < brewNotes.size(); ++i)
      toXml(brewNotes[i], doc, tmpElement);
   node.appendChild(tmpElement);

   Equipment* equip = a->equipment();
   if( equip )
      toXml( equip, doc, node);

   parent.appendChild(node);
}

void BeerXML::toXml( Style* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::STYLETABLE);

   node = doc.createElement("STYLE");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Water* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::WATERTABLE);

   node = doc.createElement("WATER");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);


}

void BeerXML::toXml( Yeast* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::YEASTTABLE);

   node = doc.createElement("YEAST");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

// fromXml ====================================================================
void BeerXML::fromXml(Ingredient* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::logW( QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();

      if( xmlTagsToProperties.contains(xmlTag) )
      {
         switch( element->metaProperty(xmlTagsToProperties[xmlTag]).type() )
         {
            case QVariant::Bool:
               boolVal = Ingredient::getBool(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = Ingredient::getDouble(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = Ingredient::getInt(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = Ingredient::getDateTime(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = Ingredient::getDate(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateVal);
               break;
               // NOTE: I believe that enum types like Fermentable::Type will go
               // here since Q_ENUMS() converts enums to strings. So, need to make
               // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = Ingredient::getString(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), stringVal);
               break;
            default:
               Brewtarget::logW(QString("%1: don't understand property type.").arg(Q_FUNC_INFO));
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            Brewtarget::logE( QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag));
            return;
         }
      }
   }

}

void BeerXML::fromXml(Ingredient* element, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;
   TableSchema* schema = m_tables->table( element->table() );

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::logW( QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();
      QString pTag = schema->xmlToProperty(xmlTag);

      if( pTag.size() ) {
         switch( element->metaProperty(pTag).type() )
         {
            case QVariant::Bool:
               boolVal = Ingredient::getBool(textNode);
               element->setProperty(pTag.toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = Ingredient::getDouble(textNode);
               element->setProperty(pTag.toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = Ingredient::getInt(textNode);
               element->setProperty(pTag.toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = Ingredient::getDateTime(textNode);
               element->setProperty(pTag.toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = Ingredient::getDate(textNode);
               element->setProperty(pTag.toStdString().c_str(), dateVal);
               break;
               // NOTE: I believe that enum types like Fermentable::Type will go
               // here since Q_ENUMS() converts enums to strings. So, need to make
               // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = Ingredient::getString(textNode);
               element->setProperty(pTag.toStdString().c_str(), stringVal);
               break;
            default:
               Brewtarget::logW(QString("%1: don't understand property type. xmlTag=%2")
                     .arg(Q_FUNC_INFO)
                     .arg(xmlTag));
               break;
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            Brewtarget::logE( QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag));
            return;
         }
      }
   }
}

// Brewnotes can never be created w/ a recipe, so we will always assume the
// calling method has the transactions
BrewNote* BeerXML::brewNoteFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   BrewNote* ret;
   QDateTime theDate;
   Database db = Database::instance();

   n = node.firstChildElement("BREWDATE");
   theDate = QDateTime::fromString(n.firstChild().toText().nodeValue(),Qt::ISODate);

   try {
      ret = new BrewNote(theDate);

      if ( ! ret ) {
         QString error = "Could not create new brewnote.";
         Brewtarget::logE(QString(error));
         QMessageBox::critical(nullptr, tr("Import error."),
               error.append("\nUnable to create brew note."));
         return ret;
      }
      // Need to tell the brewnote not to perform the calculations
      ret->setLoading(true);
      fromXml(ret, node);
      ret->setLoading(false);

      db.insertBrewnote(ret,parent);
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
   }
   return ret;
}

Equipment* BeerXML::equipmentFromXml( QDomNode const& node, Recipe* parent )
{
   // When loading from XML, we need to delay the signals until after
   // everything is done. This should significantly speed up the load times

   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Equipment* ret;
   QString name;
   QList<Equipment*> matching;
   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing an equip by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         // Check to see if there is an equip already in the DB with the same name.
         db.getElementsByName<Equipment>( matching, Brewtarget::EQUIPTABLE, name, db.allEquipments);

         // If we find a match, use it
         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Equipment(name,true);
         }
      }
      else {
         ret = new Equipment(name,true);
      }

      if ( createdNew ) {
         fromXml(ret, node);
         if ( ! ret->isValid() )
            throw QString("There was an error loading equipment profile from XML");

         // If we are importing one of our beerXML files, the utilization is always
         // 0%. We need to fix that.
         if ( ret->hopUtilization_pct() == 0.0 )
            ret->setHopUtilization_pct(100.0);

         db.insertEquipment(ret);
      }

      if( parent ) {
         ret->setDisplay(false);
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      blockSignals(false);
      throw;
   }

   blockSignals(false);


   if ( createdNew ) {
      emit db.changed( db.metaProperty("equipments"), QVariant() );
      emit db.newEquipmentSignal(ret);
   }

   return ret;
}

Fermentable* BeerXML::fermentableFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Fermentable* ret;
   QString name;
   QList<Fermentable*> matching;
   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a ferm by itself, need to do some dupe-checking.
      if ( parent == nullptr )
      {
         // No parent means we handle the transaction
         db.sqlDatabase().transaction();
         // Check to see if we already have a Fermentable with this name
         db.getElementsByName<Fermentable>( matching, 
               Brewtarget::FERMTABLE, 
               name, db.allFermentables);

         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Fermentable(name);
         }
      }
      else {
         ret = new Fermentable(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );
         if ( ! ret->isValid() )
            throw QString("Error reading fermentable from XML");


         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() )
            ret->invalidate();
         else {
            int ndx = Fermentable::types.indexOf( n.firstChild().toText().nodeValue());
            if ( ndx != -1 )
               ret->setType( static_cast<Fermentable::Type>(ndx));
            else
               ret->invalidate();
         }

         if ( ! ret->isValid() ) {
            Brewtarget::logW( QString("BeerXML::fermentableFromXml: Could convert a recognized type") );
         }
         db.insertFermentable(ret);
      }

      if ( parent ) {
         ret->setDisplay(false);
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      if ( parent == nullptr ) {
         db.sqlDatabase().rollback();
      }
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
   }

   if ( parent == nullptr ) {
      db.sqlDatabase().commit();
   }

   blockSignals(false);
   if ( createdNew ) {
      emit db.changed( db.metaProperty("fermentables"), QVariant() );
      emit db.newFermentableSignal(ret);
   }

   return ret;
}

int BeerXML::getQualifiedHopTypeIndex(QString type, Hop* hop)
{
   Database db = Database::instance();
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   if ( Hop::types.indexOf(type) < 0 ) {
      // look for a valid hop type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));
      // Check to see if there is an hop already in the DB with the same name.
      QSqlQuery q(db.sqlDatabase());
      q.prepare(query);
      q.bindValue(":name", hop->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString htype = q.record().value(0).toString();
            q.finish();
            if ( htype != "" &&  Hop::types.indexOf(htype) >= 0 ) {
               return Hop::types.indexOf(htype);
            }
         }
      }
      // out of ideas at this point so default to Both
      return Hop::types.indexOf(QString("Both"));
   }
   else {
      return Hop::types.indexOf(type);
   }
}

int BeerXML::getQualifiedHopUseIndex(QString use, Hop* hop)
{
   Database db = Database::instance();
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   if ( Hop::uses.indexOf(use) < 0 ) {
      // look for a valid hop type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropUse))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));

      QSqlQuery q(db.sqlDatabase());
      q.prepare(query);
      q.bindValue(":name", hop->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString hUse = q.record().value(0).toString();
            q.finish();
            if ( hUse != "" &&  Hop::uses.indexOf(hUse) >= 0 ) {
               return Hop::uses.indexOf(hUse);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Hop::uses.indexOf(QString("Flavor"));
   }
   else {
      return Hop::uses.indexOf(use);
   }
}

Hop* BeerXML::hopFromXml( QDomNode const& node, Recipe* parent )
{
   Database db = Database::instance();
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Hop* ret;
   QString name;
   QList<Hop*> matching;

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a hop by itself, need to do some dupe-checking.
      if( parent == nullptr )
      {
         // as always, start the transaction if no parent
         db.sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         db.getElementsByName<Hop>( matching, 
               Brewtarget::HOPTABLE, 
               name, 
               db.allHops);

         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Hop(name);
         }
      }
      else {
         ret = new Hop(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );
         if ( ! ret->isValid() ) {
            throw QString("Error reading Hop from XML");
         }

         // Handle enums separately.
         n = node.firstChildElement("USE");
         if ( n.firstChild().isNull() )
            ret->invalidate();
         else {
            int ndx = getQualifiedHopUseIndex(n.firstChild().toText().nodeValue(), ret);
            if ( ndx != -1 ) {
               ret->setUse( static_cast<Hop::Use>(ndx));
            }
            else {
               ret->invalidate();
            }
         }

         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = getQualifiedHopTypeIndex(n.firstChild().toText().nodeValue(), ret);
            if ( ndx != -1 ) {
               ret->setType( static_cast<Hop::Type>(ndx) );
            }
            else {
               ret->invalidate();
            }
         }

         n = node.firstChildElement("FORM");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = Hop::forms.indexOf(n.firstChild().toText().nodeValue());
            if ( ndx != -1 ) {
               ret->setForm( static_cast<Hop::Form>(ndx));
            }
            else {
               ret->invalidate();
            }
         }

         if ( ! ret->isValid() ) {
            Brewtarget::logW(QString("BeerXML::hopFromXml: Could convert %1 to a recognized type"));
         }
         db.insertHop(ret);
      }

      if( parent )
         db.addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      blockSignals(false);
      if ( ! parent )
         db.sqlDatabase().rollback();

      abort();
   }

   if ( ! parent ) {
      db.sqlDatabase().commit();
   }

   blockSignals(false);

   if( createdNew ) {
      emit db.changed( db.metaProperty("hops"), QVariant() );
      emit db.newHopSignal(ret);
   }

   return ret;
}

// like brewnotes, we will assume here that the caller has the transaction
// block
Instruction* BeerXML::instructionFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   QString name;
   Instruction* ret;
   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();

   blockSignals(true);
   try {
      ret = new Instruction(name);

      fromXml(ret, node);

      db.insertInstruction(ret, parent);

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e) );
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   emit db.changed( db.metaProperty("instructions"), QVariant() );
   return ret;
}

Mash* BeerXML::mashFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Mash* ret;
   QString name;
   QList<Mash*> matching;
   Database db = Database::instance();

   blockSignals(true);

   // Mashes are weird. We need to know if this is a duplicate, but we need to
   // make a copy of it anyway.
   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();

   try {
      ret = new Mash(name);

      // If the mash has a name
      if ( ! name.isEmpty() ) {
         db.getElementsByName<Mash>( matching, Brewtarget::MASHTABLE, name, db.allMashs );

         // If there are no other matches in the database
         if( matching.isEmpty() ) {
            ret->setDisplay(true);
         }
      }
      // First, get all the standard properties.
      fromXml( ret, node );

      //Need to insert the Mash before the Mash steps to get
      //the ID for foreign key contraint in Maststep table.
      db.sqlDatabase().transaction();
      db.insertMash(ret);

      // Now, get the individual mash steps.
      n = node.firstChildElement("MASH_STEPS");
      if( ! n.isNull() ) {
         // Iterate through all the mash steps.
         for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         {
            MashStep* temp = mashStepFromXml( n, ret );
            if ( ! temp->isValid() ) {
               QString error = QString("Error importing mash step %1. Importing as infusion").arg(temp->name());
               Brewtarget::logE(error);
            }
         }
      }

      if ( parent ) {
         db.addToRecipe( parent, ret, true, false);
      }

   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      blockSignals(false);
      db.sqlDatabase().rollback();
      abort();
   }

   db.sqlDatabase().commit();

   blockSignals(false);

   emit db.changed( db.metaProperty("mashs"), QVariant() );
   emit db.newMashSignal(ret);
   emit ret->mashStepsChanged();

   return ret;
}

// mashsteps don't exist without a mash. It is up to mashFromXml or
// recipeFromXml to deal with the transaction
MashStep* BeerXML::mashStepFromXml( QDomNode const& node, Mash* parent )
{
   QDomNode n;
   QString str;
   Database db = Database::instance();

   try {
      MashStep* ret = new MashStep(true);

      fromXml(ret,node);

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         //Try to make sure incoming format matches
         //e.g. convert INFUSION to Infusion
         str = n.firstChild().toText().nodeValue();
         str = str.toLower();
         str[0] = str.at(0).toTitleCase();
         int ndx =  MashStep::types.indexOf(str);

         if ( ndx != -1 )
            ret->setType( static_cast<MashStep::Type>(ndx) );
         else
            ret->invalidate();
      }

      db.insertMashStep(ret,parent);
      if (! signalsBlocked() )
      {
         emit db.changed( db.metaProperty("mashs"), QVariant() );
         emit parent->mashStepsChanged();
      }
      return ret;
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      abort();
   }

}

int BeerXML::getQualifiedMiscTypeIndex(QString type, Misc* misc)
{
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);
   Database db = Database::instance();

   if ( Misc::types.indexOf(type) < 0 ) {
      // look for a valid mash type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));
      QSqlQuery q(db.sqlDatabase());

      q.prepare(query);
      q.bindValue(":name", misc->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() )
         {
            QString mtype = q.record().value(0).toString();
            q.finish();
            if ( mtype != "" &&  Misc::types.indexOf(mtype) >= 0 ) {
               return Misc::types.indexOf(mtype);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Misc::types.indexOf(QString("Flavor"));
   }
   else {
      return Misc::types.indexOf(type);
   }
}

int BeerXML::getQualifiedMiscUseIndex(QString use, Misc* misc)
{
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);
   Database db = Database::instance();

   if ( Misc::uses.indexOf(use) < 0 ) {
      // look for a valid misc type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:use AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropUse));
      QSqlQuery q(db.sqlDatabase());

      q.prepare(query);
      q.bindValue(":name", misc->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString mUse = q.record().value(0).toString();
            q.finish();
            if ( mUse != "" &&  Misc::uses.indexOf(mUse) >= 0 ) {
               return Misc::uses.indexOf(mUse);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Misc::uses.indexOf(QString("Flavor"));
   }
   else {
      return Misc::uses.indexOf(use);
   }
}

Misc* BeerXML::miscFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Misc* ret;
   QString name;
   QList<Misc*> matching;

   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a misc by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         // Check to see if there is a hop already in the DB with the same name.
         db.sqlDatabase().transaction();

         db.getElementsByName<Misc>( matching, Brewtarget::MISCTABLE, name, db.allMiscs );

         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Misc(name);
         }
      }
      else {
         ret = new Misc(name);
      }

      if ( createdNew ) {

         fromXml( ret, node );

         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         // Assuming these return anything is a bad idea. So far, several other brewing programs are not generating
         // valid XML.
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            ret->setType( static_cast<Misc::Type>(getQualifiedMiscTypeIndex(n.firstChild().toText().nodeValue(), ret)));
         }

         n = node.firstChildElement("USE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            ret->setUse(static_cast<Misc::Use>(getQualifiedMiscUseIndex(n.firstChild().toText().nodeValue(), ret)));
         }

         if ( ! ret->isValid() ) {
            Brewtarget::logW(QString("BeerXML::miscFromXml: Could convert %1 to a recognized type"));
         }
         db.insertMisc(ret);
      }

      if( parent )
         db.addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("miscs"), QVariant() );
      emit db.newMiscSignal(ret);
   }
   return ret;
}

Recipe* BeerXML::recipeFromXml( QDomNode const& node )
{
   QDomNode n;
   blockSignals(true);
   Recipe *ret;
   QString name;
   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {

      // This is all one long, gnarly transaction.
      db.sqlDatabase().transaction();

      // Oh sweet mercy
      ret = new Recipe(name);

      if ( ! ret ) {
         return nullptr;
      }
      // Get standard properties.
      fromXml(ret, node);

      // I need to insert this now, because I need a valid key for adding
      // things to the recipe
      db.insertRecipe(ret);

      // Get style. Note: styleFromXml requires the entire node, not just the
      // firstchild of the node.
      n = node.firstChildElement("STYLE");
      styleFromXml(n, ret);
      if ( ! ret->style()->isValid())
         ret->invalidate();

      // Get equipment. equipmentFromXml requires the entire node, not just the
      // first child
      n = node.firstChildElement("EQUIPMENT");
      equipmentFromXml(n, ret);
      if ( !ret->equipment()->isValid() )
         ret->invalidate();

      // Get hops.
      n = node.firstChildElement("HOPS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Hop* temp = hopFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // Get ferms.
      n = node.firstChildElement("FERMENTABLES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Fermentable* temp = fermentableFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // get mashes. There is only one mash per recipe, so this needs the entire
      // node.
      n = node.firstChildElement("MASH");
      mashFromXml(n, ret);
      if ( ! ret->mash()->isValid() )
         ret->invalidate();

      // Get miscs.
      n = node.firstChildElement("MISCS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Misc* temp = miscFromXml(n, ret);
         if (! temp->isValid())
            ret->invalidate();
      }

      // Get yeasts.
      n = node.firstChildElement("YEASTS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Yeast* temp = yeastFromXml(n, ret);
         if ( !temp->isValid() )
            ret->invalidate();
      }

      // Get waters. Odd. Waters don't invalidate.
      n = node.firstChildElement("WATERS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         waterFromXml(n, ret);

      /* That ends the beerXML defined objects. I'm not going to do the
       * validation for these last two. We write em, and we had better be
       * writing them properly
       */
      // Get instructions.
      n = node.firstChildElement("INSTRUCTIONS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         instructionFromXml(n, ret);

      // Get brew notes
      n = node.firstChildElement("BREWNOTES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         brewNoteFromXml(n, ret);

      // If we get here, commit
      db.sqlDatabase().commit();

      // Recalc everything, just for grins and giggles.
      ret->recalcAll();
      blockSignals(false);
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   emit db.newRecipeSignal(ret);
   return ret;
}

Style* BeerXML::styleFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Style* ret;
   QString name;
   QList<Style*> matching;

   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if ( parent == nullptr ) {
         // No parent means we handle the transaction
         db.sqlDatabase().transaction();
         // Check to see if there is a style already in the DB with the same name.
         db.getElementsByName<Style>( matching, Brewtarget::STYLETABLE, name, db.allStyles );

         // If we found a match, use it.
         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            // We could find no matching style in the db
            ret = new Style(name,true);
         }
      }
      else {
         // If we are inserting this as part of a recipe, we can skip straight
         // to creating a new one
         ret = new Style(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );

         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = Style::m_types.indexOf( n.firstChild().toText().nodeValue());
            if ( ndx != -1 )
               ret->setType(static_cast<Style::Type>(ndx));
            else
               ret->invalidate();
         }

         // If translating the enums craps out, give a warning
         if (! ret->isValid() ) {
            Brewtarget::logW(QString("BeerXML::styleFromXml: Could convert %1 to a recognized type"));
         }
         // we need to poke this into the database
         db.insertStyle(ret);
      }
      if ( parent ) {
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if ( createdNew ) {
      emit db.changed( db.metaProperty("styles"), QVariant() );
      emit db.newStyleSignal(ret);
   }

   return ret;
}

Water* BeerXML::waterFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Water* ret;
   QString name;
   QList<Water*> matching;
   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         db.sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         db.getElementsByName<Water>( matching, Brewtarget::WATERTABLE, name, db.allWaters );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
         else
            ret = new Water(name);
      }
      else
         ret = new Water(name);

      if ( createdNew ) {
         fromXml( ret, node );
         db.insertWater(ret);
         if( parent ) {
            db.addToRecipe( parent, ret, false );
         }
      }
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( parent == nullptr )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("waters"), QVariant() );
      emit db.newWaterSignal(ret);
   }

   return ret;
}

Yeast* BeerXML::yeastFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Yeast* ret;
   QString name;
   QList<Yeast*> matching;
   Database db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a yeast by itself, need to do some dupe-checking.
      if ( parent == nullptr ) {
         // start the transaction, just in case
         db.sqlDatabase().transaction();
         // Check to see if there is a yeast already in the DB with the same name.
         db.getElementsByName<Yeast>( matching, Brewtarget::YEASTTABLE, name, db.allYeasts );

         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Yeast(name);
         }
      }
      else {
         ret = new Yeast(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );

         // Handle type enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            Brewtarget::logE(
                  QString("Could not find TYPE in %1.  Please select an appropriate value once the yeast is imported").arg(name)
                  );
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::types.indexOf( tname );
            if ( ndx != -1) {
               ret->setType( static_cast<Yeast::Type>(ndx) );
            }
            else {
               ret->setType(static_cast<Yeast::Type>(0));
               Brewtarget::logE(
                     QString("Could not translate the type %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name));
            }
         }
         // Handle form enums separately.
         n = node.firstChildElement("FORM");
         if ( n.firstChild().isNull() ) {
            Brewtarget::logE(
                  QString("Could not find FORM in %1.  Please select an appropriate value once the yeast is imported").arg(name)
                  );
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::forms.indexOf( tname );
            if ( ndx != -1 ) {
               ret->setForm( static_cast<Yeast::Form>(ndx) );
            }
            else {
               ret->setForm( static_cast<Yeast::Form>(0) );
               Brewtarget::logE(
                     QString("Could not translate the form %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name));
            }
         }
         // Handle flocc enums separately.
         n = node.firstChildElement("FLOCCULATION");
         if ( n.firstChild().isNull() ) {
            Brewtarget::logE(
                  QString("Could not find FLOCCULATION in %1.  Please select an appropriate value once the yeast is imported").arg(name)
                  );
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::flocculations.indexOf( tname );
            if (ndx != -1) {
               ret->setFlocculation( static_cast<Yeast::Flocculation>(ndx) );
            }
            else {
               ret->setFlocculation( static_cast<Yeast::Flocculation>(0) );
               Brewtarget::logE(
                     QString("Could not translate the flocculation %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name));
            }
         }

         db.insertYeast(ret);
      }

      if( parent ) {
         // we are in a transaction boundary, so tell addToRecipe not to start
         // another
         db.addToRecipe( parent, ret, false );
         parent->recalcOgFg();
         parent->recalcABV_pct();
      }
   }
   catch (QString e) {
      Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   db.sqlDatabase().commit();
   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("yeasts"), QVariant() );
      emit db.newYeastSignal(ret);
   }

   return ret;
}

