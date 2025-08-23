/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Fermentation.h is part of Brewtarget, and is copyright the following authors 2023-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_FERMENTATION_H
#define MODEL_FERMENTATION_H
#pragma once

#include <memory>
#include <optional>

#include <QList>
#include <QMetaProperty>
#include <QString>
#include <QVariant>

#include "model/FermentationStep.h"
#include "model/FolderBase.h"
#include "model/NamedEntity.h"
#include "model/StepOwnerBase.h"

class FermentationCatalog;
class FermentationEditor;
class FermentationItemDelegate;
class FermentationTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Fermentation { inline BtStringConst const property{#property}; }
AddPropertyName(description)
AddPropertyName(notes      )
AddPropertyName(primary    )
AddPropertyName(secondary  )
AddPropertyName(tertiary   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \class Fermentation is a collection of steps providing process information for common fermentation procedures.  It is
 *        introduced as part of BeerJSON.  It shares a number of characteristics with \c Mash and \c Boil
 */
class Fermentation : public NamedEntity,
                     public FolderBase<Fermentation>,
                     public StepOwnerBase<Fermentation, FermentationStep> {
   Q_OBJECT
   FOLDER_BASE_DECL(Fermentation)
   STEP_OWNER_COMMON_DECL(Fermentation, fermentation)
   // See model/FolderBase.h for info, getters and setters for these properties
   Q_PROPERTY(QString folderPath        READ folderPath        WRITE setFolderPath)
   // See model/StepOwnerBase.h for info, getters and setters for these properties
   Q_PROPERTY(QList<std::shared_ptr<FermentationStep>> steps   READ steps   WRITE setSteps   STORED false)
   Q_PROPERTY(unsigned int numSteps   READ numSteps   STORED false)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_description();
   static QString localisedName_notes      ();
   static QString localisedName_primary    ();
   static QString localisedName_secondary  ();
   static QString localisedName_tertiary   ();

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass      = FermentationCatalog;
   using EditorClass       = FermentationEditor;
   using ItemDelegateClass = FermentationItemDelegate;
   using TableModelClass   = FermentationTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Fermentation(QString name = "");
   Fermentation(NamedParameterBundle const & namedParameterBundle);
   Fermentation(Fermentation const & other);

   virtual ~Fermentation();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(QString description   READ description   WRITE setDescription)
   Q_PROPERTY(QString notes         READ notes         WRITE setNotes      )

   //! \brief Convenience property for accessing the first fermentation step
   Q_PROPERTY(std::shared_ptr<FermentationStep>   primary     READ primary     WRITE setPrimary     STORED false)
   //! \brief Convenience property for accessing the second fermentation step
   Q_PROPERTY(std::shared_ptr<FermentationStep>   secondary   READ secondary   WRITE setSecondary   STORED false)
   //! \brief Convenience property for accessing the third fermentation step
   Q_PROPERTY(std::shared_ptr<FermentationStep>   tertiary    READ tertiary    WRITE setTertiary    STORED false)

   SUPPORT_NUM_RECIPES_USED_IN

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QString description() const;
   QString notes      () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setDescription(QString const & val);
   void setNotes      (QString const & val);

   /**
    * \brief  A set of convenience functions for accessing the first, second and third steps.  Note that calling
    *         \c setSecondary or \c setTertiary with something other than \c std::nullopt needs to ensure the right
    *         number of prior step(s) exist, if necessary by creating default ones.
    *
    *         We don't put the step name in these getters/setters as it would become unwieldy - eg
    *         \c setSecondaryFermentationStep()
    */
   std::shared_ptr<FermentationStep> primary  () const;
   std::shared_ptr<FermentationStep> secondary() const;
   std::shared_ptr<FermentationStep> tertiary () const;
   void setPrimary  (std::shared_ptr<FermentationStep> val);
   void setSecondary(std::shared_ptr<FermentationStep> val);
   void setTertiary (std::shared_ptr<FermentationStep> val);

public slots:
   void acceptStepChange(QMetaProperty, QVariant);

signals:
   //! Emitted when the number of steps change, or when you should call fermentationSteps() again.
   void stepsChanged();

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   QString m_description;
   QString m_notes      ;
};

BT_DECLARE_METATYPES(Fermentation)

#endif
