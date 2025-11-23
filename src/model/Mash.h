/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Mash.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef MODEL_MASH_H
#define MODEL_MASH_H
#pragma once

#include <memory> // For PImpl

#include <QList>
#include <QMetaProperty>
#include <QSqlRecord>
#include <QString>
#include <QVariant>
#include <QVector>

#include "model/FolderBase.h"
#include "model/NamedEntity.h"
#include "model/MashStep.h"
#include "model/StepOwnerBase.h"

class MashCatalog;
class MashEditor;
class MashItemDelegate;
class MashSortFilterProxyModel;
class MashTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Mash { inline BtStringConst const property{#property}; }
AddPropertyName(equipAdjust              )
AddPropertyName(grainTemp_c              )
AddPropertyName(mashTunSpecificHeat_calGC)
AddPropertyName(mashTunWeight_kg         )
AddPropertyName(notes                    )
AddPropertyName(ph                       )
AddPropertyName(spargeTemp_c             )
AddPropertyName(totalMashWater_l         )
AddPropertyName(totalTime_mins           )
AddPropertyName(tunTemp_c                )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \class Mash
 *
 * \brief Model class for a mash record in the database.
 *
 *        .:TBD:. Mashes have a freestanding existence and can be shared between Recipes but the UI does
 *        not currently enforce them having non-empty names.
 */
class Mash : public NamedEntity,
             public FolderBase<Mash>,
             public StepOwnerBase<Mash, MashStep> {
   Q_OBJECT
   FOLDER_BASE_DECL(Mash)
   STEP_OWNER_COMMON_DECL(Mash, mash)
   // See model/FolderBase.h for info, getters and setters for these properties
   Q_PROPERTY(QString folderPath        READ folderPath        WRITE setFolderPath)
   // See model/StepOwnerBase.h for info, getters and setters for these properties
   Q_PROPERTY(QList<std::shared_ptr<MashStep>> steps   READ steps   WRITE setSteps   STORED false)
   Q_PROPERTY(unsigned int numSteps   READ numSteps   STORED false)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_equipAdjust              ();
   static QString localisedName_grainTemp_c              ();
   static QString localisedName_mashTunSpecificHeat_calGC();
   static QString localisedName_mashTunWeight_kg         ();
   static QString localisedName_notes                    ();
   static QString localisedName_ph                       ();
   static QString localisedName_spargeTemp_c             ();
   static QString localisedName_totalMashWater_l         ();
   static QString localisedName_totalTime_mins           ();
   static QString localisedName_tunTemp_c                ();

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = MashCatalog;
   using EditorClass               = MashEditor;
   using ItemDelegateClass         = MashItemDelegate;
   using SortFilterProxyModelClass = MashSortFilterProxyModel;
   using TableModelClass           = MashTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Mash(QString name = "");
   Mash(NamedParameterBundle const & namedParameterBundle);
   Mash(Mash const & other);

   virtual ~Mash();

   //=================================================== PROPERTIES ====================================================
   //! \brief The initial grain temp in Celsius.
   Q_PROPERTY(double                grainTemp_c               READ grainTemp_c               WRITE setGrainTemp_c  )
   //! \brief The notes.
   Q_PROPERTY(QString               notes                     READ notes                     WRITE setNotes  )

   //! \brief The initial tun temp in Celsius.  ⮜⮜⮜ Optional in BeerXML.  Not part of BeerJSON. ⮞⮞⮞
   Q_PROPERTY(std::optional<double> tunTemp_c                 READ tunTemp_c                 WRITE setTunTemp_c  )
   //! \brief The sparge temp in C.             ⮜⮜⮜ Optional in BeerXML.  Not part of BeerJSON. ⮞⮞⮞
   Q_PROPERTY(std::optional<double> spargeTemp_c              READ spargeTemp_c              WRITE setSpargeTemp_c  )
   //! \brief The pH of the sparge.             ⮜⮜⮜ Optional in BeerXML.  Not part of BeerJSON. ⮞⮞⮞
   Q_PROPERTY(std::optional<double> ph                        READ ph                        WRITE setPh  )
   //! \brief The mass of the tun in kg.        ⮜⮜⮜ Optional in BeerXML.  Not part of BeerJSON. ⮞⮞⮞
   Q_PROPERTY(std::optional<double> mashTunWeight_kg          READ mashTunWeight_kg          WRITE setTunWeight_kg  )
   //! \brief The tun's specific heat in kcal/(g*C).   ⮜⮜⮜ Optional in BeerXML.  Not part of BeerJSON. ⮞⮞⮞
   Q_PROPERTY(std::optional<double> mashTunSpecificHeat_calGC READ mashTunSpecificHeat_calGC WRITE setMashTunSpecificHeat_calGC  )
   //! \brief Whether to adjust strike temperatures to account for the tun.   ⮜⮜⮜ Optional in BeerXML.  Not part of BeerJSON. ⮞⮞⮞
   Q_PROPERTY(bool                  equipAdjust               READ equipAdjust               WRITE setEquipAdjust  )

   //! \brief The total water that went into the mash (ie all the mash water, sparge and strike) in liters. Calculated.
   Q_PROPERTY(double totalMashWater_l   READ totalMashWater_l  STORED false )
   //! \brief The total mash time in minutes. Calculated.
   Q_PROPERTY(double totalTime_mins     READ totalTime_mins    STORED false )

   SUPPORT_NUM_RECIPES_USED_IN

   // ⮜⮜⮜ BeerJSON support does not require any additional properties on this class! ⮞⮞⮞

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double                grainTemp_c              () const;
   QString               notes                    () const;
   std::optional<double> tunTemp_c                () const;
   std::optional<double> spargeTemp_c             () const;
   std::optional<double> ph                       () const;
   std::optional<double> mashTunWeight_kg         () const;
   std::optional<double> mashTunSpecificHeat_calGC() const;
   bool                  equipAdjust              () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setGrainTemp_c              (double                const   val);
   void setNotes                    (QString               const & val);
   void setTunTemp_c                (std::optional<double> const   val);
   void setSpargeTemp_c             (std::optional<double> const   val);
   void setPh                       (std::optional<double> const   val);
   void setTunWeight_kg             (std::optional<double> const   val);
   void setMashTunSpecificHeat_calGC(std::optional<double> const   val);
   void setEquipAdjust              (bool                  const   val);

   // Calculated getters
   double totalMashWater_l() const;
   //! \brief all the infusion water, excluding sparge
   double totalInfusionAmount_l() const;
   //! \brief all the sparge water
   double totalSpargeAmount_l() const;
   double totalTime_mins() const;

   bool hasSparge() const;

public slots:
   void acceptSetMemberChange(QMetaProperty, QVariant);

signals:
   // Emitted when the number of steps change, or when you should call mashSteps() again.
   void ownedItemsChanged();

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   double                m_grainTemp_c              ;
   QString               m_notes                    ;
   std::optional<double> m_tunTemp_c                ;
   std::optional<double> m_spargeTemp_c             ;
   std::optional<double> m_ph                       ;
   std::optional<double> m_mashTunWeight_kg         ;
   std::optional<double> m_mashTunSpecificHeat_calGC;
   bool                  m_equipAdjust              ;
};

BT_DECLARE_METATYPES(Mash)

#endif
