/*
 * beerxml.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
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

#ifndef _BEERXML_H
#define _BEERXML_H

class BeerXML;

#include <memory> // For PImpl

#include <QDomDocument>
#include <QDomNode>
#include <QList>
#include <QHash>
#include <QFile>
#include <QString>
#include <QVariant>
#include <QMetaProperty>
#include <QUndoStack>
#include <QObject>
#include <QPair>
#include <QDebug>
#include <QRegExp>
#include <QMap>

#include "ingredient.h"
#include "brewtarget.h"
#include "database.h"
#include "TableSchema.h"

class BrewNote;
class Equipment;
class Fermentable;
class Hop;
class Instruction;
class Mash;
class MashStep;
class Misc;
class Recipe;
class Style;
class Water;
class Yeast;

/*!
 * \class BeerXML
 * \author Mik Firestone
 *
 * \brief Handles all translations to and from BeerXML
 *
 */
class BeerXML : public QObject
{
   Q_OBJECT

   friend class Database;
public:

   virtual ~BeerXML();

   // Export to BeerXML =======================================================
   void toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Equipment* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Hop* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Instruction* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Mash* a, QDomDocument& doc, QDomNode& parent );
   void toXml( MashStep* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Misc* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Recipe* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Style* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Water* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Yeast* a, QDomDocument& doc, QDomNode& parent );
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   /*! Populates the \b element with properties. This must be a class that
    *  simple properties only (no subelements).
    * \param element is the element you want to populate.
    * \param xmlTagsToProperties is a hash from xml tags to meta property names.
    * \param elementNode is the root node of the element we are reading from.
    */
   void fromXml(Ingredient* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode);
   void fromXml(Ingredient* element, QDomNode const& elementNode);

   /*! Import ingredients, recipes, etc from BeerXML documents.
    * \param filename
    * \param errorMessage
    * \return true if succeeded, false otherwise
    */
   bool importFromXML(QString const & filename, QString & errorMessage);

   // Import from BeerXML =====================================================
   BrewNote*    brewNoteFromXml(    QDomNode const& node, Recipe* parent );
   Equipment*   equipmentFromXml(   QDomNode const& node, Recipe* parent = nullptr );
   Fermentable* fermentableFromXml( QDomNode const& node, Recipe* parent = nullptr );
   Hop*         hopFromXml(         QDomNode const& node, Recipe* parent = nullptr );
   Instruction* instructionFromXml( QDomNode const& node, Recipe* parent );
   Mash*        mashFromXml(        QDomNode const& node, Recipe* parent = nullptr );
   MashStep*    mashStepFromXml(    QDomNode const& node, Mash* parent );
   Misc*        miscFromXml(        QDomNode const& node, Recipe* parent = nullptr );
   Style*       styleFromXml(       QDomNode const& node, Recipe* parent = nullptr );
   Water*       waterFromXml(       QDomNode const& node, Recipe* parent = nullptr );
   Yeast*       yeastFromXml(       QDomNode const& node, Recipe* parent = nullptr );
   Recipe*      recipeFromXml(      QDomNode const& node);
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   DatabaseSchema* m_tables;

   /**
    * Private constructor means our friend class (Database) can construct us
    */
   BeerXML(DatabaseSchema* tables);

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   BeerXML(BeerXML const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   BeerXML& operator=(BeerXML const&) = delete;

   QString textFromValue(QVariant value, QString type);
   int getQualifiedHopTypeIndex(QString type, Hop* hop);
   int getQualifiedHopUseIndex(QString use, Hop* hop);

   int getQualifiedMiscTypeIndex(QString type, Misc* misc);
   int getQualifiedMiscUseIndex(QString use, Misc* misc);

};

#endif
