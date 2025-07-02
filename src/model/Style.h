/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Style.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
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
#ifndef MODEL_STYLE_H
#define MODEL_STYLE_H
#pragma once

#include <QString>
#include <QStringList>
#include <QSqlRecord>

#include "model/FolderBase.h"
#include "model/NamedEntity.h"
#include "utils/EnumStringMapping.h"

class StyleCatalog;
class StyleEditor;
class StyleItemDelegate;
class StyleSortFilterProxyModel;
class StyleTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Style { inline BtStringConst const property{#property}; }
AddPropertyName(abvMax_pct       )
AddPropertyName(abvMin_pct       )
AddPropertyName(appearance       )
AddPropertyName(aroma            )
AddPropertyName(carbMax_vol      )
AddPropertyName(carbMin_vol      )
AddPropertyName(category         )
AddPropertyName(categoryNumber   )
AddPropertyName(colorMax_srm     )
AddPropertyName(colorMin_srm     )
AddPropertyName(examples         )
AddPropertyName(fgMax            )
AddPropertyName(fgMin            )
AddPropertyName(flavor           )
AddPropertyName(ibuMax           )
AddPropertyName(ibuMin           )
AddPropertyName(ingredients      )
AddPropertyName(mouthfeel        )
AddPropertyName(notes            )
AddPropertyName(ogMax            )
AddPropertyName(ogMin            )
AddPropertyName(overallImpression)
AddPropertyName(styleGuide       )
AddPropertyName(styleLetter      )
AddPropertyName(type             )
AddPropertyName(typeString       )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Style
 *
 * \brief Model for style records in the database.
 */
class Style : public NamedEntity,
              public FolderBase<Style> {
   Q_OBJECT
   FOLDER_BASE_DECL(Style)
   // See model/FolderBase.h for info, getters and setters for these properties
   Q_PROPERTY(QString folderPath        READ folderPath        WRITE setFolderPath)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief The type of beverage.
    *
    *        Note that this has changed a fair bit from BeerXML to BeerJSON.  We map as follows:
    *
    *                Ideal Mapping                               Bidirectional Mapping
    *                -------------                               ---------------------
    *           BeerXML         BeerJSON                        BeerXML         BeerJSON
    *           -------         --------                        -------         --------
    *           Lager    ────>  beer                            Lager   <────>  other
    *           Ale     <────>  beer                            Ale     <────>  beer
    *           Wheat    ────>  beer                            Wheat   <────>  kombucha
    *           Cider   <────>  cider                           Cider   <────>  cider
    *           Mead    <────>  mead                            Mead    <────>  mead
    *           Mixed   <────>  other                           Mixed   <────>  soda
    *           Mixed   <────   kombucha                        Mixed   <────   wine
    *           Mixed   <────   soda
    *           Mixed   <────   wine
    *
    *        The bidirectional mapping is a compromise to avoid adding complexity to reading & writing BeerXML files.
    */
   enum class Type {
      Beer    ,
      Cider   ,
      Mead    ,
      Kombucha,
      Soda    ,
      Wine    ,
      Other   ,
   };
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Style::Type and string values suitable for serialisation in DB, BeerJSON, etc (but
    *        \b not BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c Style::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = StyleCatalog;
   using EditorClass               = StyleEditor;
   using ItemDelegateClass         = StyleItemDelegate;
   using SortFilterProxyModelClass = StyleSortFilterProxyModel;
   using TableModelClass           = StyleTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Style(QString name = "");
   Style(NamedParameterBundle const & namedParameterBundle);
   Style(Style const & other);

   virtual ~Style();

   //=================================================== PROPERTIES ====================================================
   //! \brief The category.
   Q_PROPERTY(QString category                       READ category           WRITE setCategory         )
   //! \brief The category number.
   Q_PROPERTY(QString categoryNumber                 READ categoryNumber     WRITE setCategoryNumber   )
   //! \brief The style letter
   Q_PROPERTY(QString styleLetter                    READ styleLetter        WRITE setStyleLetter      )
   //! \brief Which style guide the description belongs to.
   Q_PROPERTY(QString styleGuide                     READ styleGuide         WRITE setStyleGuide       )
   //! \brief The \c Type.
   Q_PROPERTY(Type type                              READ type               WRITE setType             )
   //! \brief The minimum og.
   Q_PROPERTY(double ogMin                           READ ogMin              WRITE setOgMin            )
   //! \brief The maximum og.
   Q_PROPERTY(double ogMax                           READ ogMax              WRITE setOgMax            )
   //! \brief The minimum fg.
   Q_PROPERTY(double fgMin                           READ fgMin              WRITE setFgMin            )
   //! \brief The maximum fg.
   Q_PROPERTY(double fgMax                           READ fgMax              WRITE setFgMax            )
   //! \brief The minimum ibus.
   Q_PROPERTY(double ibuMin                          READ ibuMin             WRITE setIbuMin           )
   //! \brief The maximum ibus.
   Q_PROPERTY(double ibuMax                          READ ibuMax             WRITE setIbuMax           )
   //! \brief The minimum color in SRM.
   Q_PROPERTY(double colorMin_srm                    READ colorMin_srm       WRITE setColorMin_srm     )
   //! \brief The maximum color in SRM.
   Q_PROPERTY(double colorMax_srm                    READ colorMax_srm       WRITE setColorMax_srm     )
   //! \brief The mininum carbonation in volumes at STP.   ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> carbMin_vol      READ carbMin_vol        WRITE setCarbMin_vol      )
   //! \brief The maximum carbonation in volumes at STP.   ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> carbMax_vol      READ carbMax_vol        WRITE setCarbMax_vol      )
   //! \brief The minimum ABV in percent.                  ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> abvMin_pct       READ abvMin_pct         WRITE setAbvMin_pct       )
   //! \brief The maximum ABV in percent.                  ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> abvMax_pct       READ abvMax_pct         WRITE setAbvMax_pct       )
   //! \brief The notes.
   Q_PROPERTY(QString notes                          READ notes              WRITE setNotes            )
   //! \brief The ingredients.
   Q_PROPERTY(QString ingredients                    READ ingredients        WRITE setIngredients      )
   //! \brief The commercial examples.
   Q_PROPERTY(QString examples                       READ examples           WRITE setExamples         )
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(QString aroma                          READ aroma              WRITE setAroma            )
   Q_PROPERTY(QString appearance                     READ appearance         WRITE setAppearance       )
   Q_PROPERTY(QString flavor                         READ flavor             WRITE setFlavor           )
   Q_PROPERTY(QString mouthfeel                      READ mouthfeel          WRITE setMouthfeel        )
   Q_PROPERTY(QString overallImpression              READ overallImpression  WRITE setOverallImpression)

   SUPPORT_NUM_RECIPES_USED_IN

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QString               category         () const;
   QString               categoryNumber   () const;
   QString               styleLetter      () const;
   QString               styleGuide       () const;
   Type                  type             () const;
   double                ogMin            () const;
   double                ogMax            () const;
   double                fgMin            () const;
   double                fgMax            () const;
   double                ibuMin           () const;
   double                ibuMax           () const;
   double                colorMin_srm     () const;
   double                colorMax_srm     () const;
   std::optional<double> carbMin_vol      () const;
   std::optional<double> carbMax_vol      () const;
   std::optional<double> abvMin_pct       () const;
   std::optional<double> abvMax_pct       () const;
   QString               notes            () const;
   QString               ingredients      () const;
   QString               examples         () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               aroma            () const;
   QString               appearance       () const;
   QString               flavor           () const;
   QString               mouthfeel        () const;
   QString               overallImpression() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setCategory         (QString               const & val);
   void setCategoryNumber   (QString               const & val);
   void setStyleLetter      (QString               const & val);
   void setStyleGuide       (QString               const & val);
   void setType             (Type                  const   val);
   void setOgMin            (double                const   val);
   void setOgMax            (double                const   val);
   void setFgMin            (double                const   val);
   void setFgMax            (double                const   val);
   void setIbuMin           (double                const   val);
   void setIbuMax           (double                const   val);
   void setColorMin_srm     (double                const   val);
   void setColorMax_srm     (double                const   val);
   void setCarbMin_vol      (std::optional<double> const   val);
   void setCarbMax_vol      (std::optional<double> const   val);
   void setAbvMin_pct       (std::optional<double> const   val);
   void setAbvMax_pct       (std::optional<double> const   val);
   void setNotes            (QString               const & val);
   void setIngredients      (QString               const & val);
   void setExamples         (QString               const & val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setAroma            (QString               const & val);
   void setAppearance       (QString               const & val);
   void setFlavor           (QString               const & val);
   void setMouthfeel        (QString               const & val);
   void setOverallImpression(QString               const & val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   QString               m_category         ;
   QString               m_categoryNumber   ;
   QString               m_styleLetter      ;
   QString               m_styleGuide       ;
   Type                  m_type             ;
   double                m_ogMin            ;
   double                m_ogMax            ;
   double                m_fgMin            ;
   double                m_fgMax            ;
   double                m_ibuMin           ;
   double                m_ibuMax           ;
   double                m_colorMin_srm     ;
   double                m_colorMax_srm     ;
   std::optional<double> m_carbMin_vol      ;
   std::optional<double> m_carbMax_vol      ;
   std::optional<double> m_abvMin_pct       ;
   std::optional<double> m_abvMax_pct       ;
   QString               m_notes            ;
   QString               m_ingredients      ;
   QString               m_examples         ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               m_aroma            ;
   QString               m_appearance       ;
   QString               m_flavor           ;
   QString               m_mouthfeel        ;
   QString               m_overallImpression;
};

BT_DECLARE_METATYPES(Style)

#endif
