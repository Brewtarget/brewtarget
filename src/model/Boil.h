/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Boil.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_BOIL_H
#define MODEL_BOIL_H
#pragma once

#include <memory>
#include <optional>

#include <QList>
#include <QString>

#include "model/BoilStep.h"
#include "model/FolderBase.h"
#include "model/NamedEntity.h"
#include "model/StepOwnerBase.h"

class BoilCatalog;
class BoilEditor;
class BoilItemDelegate;
class BoilSortFilterProxyModel;
class BoilTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Boil { inline BtStringConst const property{#property}; }
AddPropertyName(description  )
AddPropertyName(notes        )
AddPropertyName(preBoilSize_l)
AddPropertyName(boilTime_mins)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \class Boil is a collection of steps providing process information for common boil procedures.  It is introduced as
 *             part of BeerJSON.  It shares a number of characteristics with \c Mash.
 *
 *             A \c Boil with no \c BoilSteps is the same as a standard single step boil.
 *
 *             Note that, although it seems like rather a lot of pain to move this info out of the \c Recipe object,
 *             there is a long-term merit in it for two reasons:
 *               - A boil profile will likely be shared across a lot of recipes;
 *               - Some "raw ale" recipes do not have a boil (see eg https://byo.com/article/raw-ale/)
 *             We don't yet implement/support either of these features, it should be easier to introduce them in future
 *             with this boil-as-a-separate-object structure.
 *
 *             Additionally, there is a short-term benefit, which is that we can share a lot of the logic between
 *             MashStep and BoilStep, which saves us duplicating code.
 *
 *             Our \c Boil class maps closely to a BeerJSON "boil procedure", with the exception that, in BeerJSON "a
 *             boil procedure with no steps is the same as a standard single step boil."  We treat steps as required
 *             (for consistency with \c Mash::mashSteps and \c Fermentation::fermentationSteps) and map "no list of boil
 *             steps" to "empty list of boil steps".
 */
class Boil : public NamedEntity,
             public FolderBase<Boil>,
             public StepOwnerBase<Boil, BoilStep> {
   Q_OBJECT

   FOLDER_BASE_DECL(Boil)
   STEP_OWNER_COMMON_DECL(Boil, boil)
   // See model/FolderBase.h for info, getters and setters for these properties
   Q_PROPERTY(QString folderPath        READ folderPath        WRITE setFolderPath)
   // See model/StepOwnerBase.h for info, getters and setters for these properties
   Q_PROPERTY(QList<std::shared_ptr<BoilStep>> steps   READ steps   WRITE setSteps   STORED false)
   Q_PROPERTY(unsigned int numSteps   READ numSteps   STORED false)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_description  ();
   static QString localisedName_notes        ();
   static QString localisedName_preBoilSize_l();
   static QString localisedName_boilTime_mins();

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = BoilCatalog;
   using EditorClass               = BoilEditor;
   using ItemDelegateClass         = BoilItemDelegate;
   using SortFilterProxyModelClass = BoilSortFilterProxyModel;
   using TableModelClass           = BoilTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Boil(QString name = "");
   Boil(NamedParameterBundle const & namedParameterBundle);
   Boil(Boil const & other);

   virtual ~Boil();

   /**
    * \brief In some parts of the code, we need to know if a particular part of the recipe counts as part of the boil
    *        proper.  We allow the user to specify the boiling point of water that they want to use for their brewing,
    *        in case they are brewing at high altitude.
    *
    *        According to https://en.wikipedia.org/wiki/High-altitude_cooking, the boiling point of water is a bit over
    *        84.5°C at 4500 meters altitude, which is already higher than the vast majority of settlements, per
    *        https://en.wikipedia.org/wiki/List_of_highest_settlements.  If you were at 5100 meters, the height of the
    *        highest human settlement in the world, then the boiling point of water would be about 82.5°C.
    *
    *        Per https://en.wikipedia.org/wiki/Lautering, mashout temperature is 77°C.
    *
    *        So, we take 81°C as a sensible dividing line.  If the wort is not above this temperature, we can't be in
    *        the boil proper (though we might be ramping up to, or down from, the boil).
    *
    *        Put another way, we're assuming 81°C is higher than any mash would end and lower than the temperature of
    *        any boil.
    */
   static constexpr double minimumBoilTemperature_c{81.0};

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(QString description   READ description   WRITE setDescription)
   Q_PROPERTY(QString notes         READ notes         WRITE setNotes      )

   /**
    * \brief This is optional because it's optional in BeerJSON.  The equivalent field in BeerXML (BOIL_SIZE on RECIPE)
    *        is a required field.
    *
    *        Note however, that Recipe::batchSize_l is a required field in both BeerJSON and BeerXML, so callers should
    *        fall back to that as a "better than nothing" value when this one is \c std::nullopt.
    */
   Q_PROPERTY(std::optional<double> preBoilSize_l          READ preBoilSize_l   WRITE setPreBoilSize_l )

   /**
    * \brief In BeerXML, this is "The total time to boil the wort in minutes".  I think this would mostly be understood
    *        as the length of the "boil proper" so, in our new model, it is the length of the step(s) for which
    *        \c Step::startTemp_c and \c Step::endTemp_c are above \c minimumBoilTemperature_c.  In other words, it is
    *        a convenience way of accessing the length of the boil excluding any ramp-up or cool-down steps.
    *
    *        TBD: It's possible we should make this optional if we desire to support "no boil" recipes in future.
    */
   Q_PROPERTY(double                boilTime_mins          READ boilTime_mins   WRITE setBoilTime_mins STORED false)

   SUPPORT_NUM_RECIPES_USED_IN

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QString               description  () const;
   QString               notes        () const;
   std::optional<double> preBoilSize_l() const;
   double                boilTime_mins() const;

   /**
    * \brief If there is a post-boil cooling step, and if it has a duration, this will return it, otherwise returns
    *        \c std::nullopt
    */
   std::optional<double> coolTime_mins() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setDescription  (QString               const & val);
   void setNotes        (QString               const & val);
   void setPreBoilSize_l(std::optional<double> const   val);
   void setBoilTime_mins(double                const   val);

   //============================================= OTHER MEMBER FUNCTIONS ==============================================

   /**
    * \brief Ensures that the boil has a "standard" structure of 3 steps:
    *          - Pre-boil ramping up from mash temperature
    *          - Boil proper at or around 100°C
    *          - Post-boil cooling to 30°C (or primary fermentation temperature if known)
    *
    *        This is useful when reading BeerXML files which mark hop additions as "First Wort" or "Aroma", meaning
    *        during the pre-boil or post-boil respectively.
    */
   void ensureStandardProfile();

public slots:
   void acceptStepChange(QMetaProperty, QVariant);

signals:
   // Emitted when the number of steps change, or when you should call boilSteps() again.
   void stepsChanged();

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   QString               m_description  ;
   QString               m_notes        ;
   std::optional<double> m_preBoilSize_l;
};

BT_DECLARE_METATYPES(Boil)

#endif
