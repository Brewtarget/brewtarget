/*
 * model/Style.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef MODEL_STYLE_H
#define MODEL_STYLE_H
#pragma once

#include <QString>
#include <QStringList>
#include <QSqlRecord>

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Style { BtStringConst const property{#property}; }
AddPropertyName(abvMax_pct)
AddPropertyName(abvMin_pct)
AddPropertyName(carbMax_vol)
AddPropertyName(carbMin_vol)
AddPropertyName(category)
AddPropertyName(categoryNumber)
AddPropertyName(colorMax_srm)
AddPropertyName(colorMin_srm)
AddPropertyName(examples)
AddPropertyName(fgMax)
AddPropertyName(fgMin)
AddPropertyName(ibuMax)
AddPropertyName(ibuMin)
AddPropertyName(ingredients)
AddPropertyName(notes)
AddPropertyName(ogMax)
AddPropertyName(ogMin)
AddPropertyName(profile)
AddPropertyName(styleGuide)
AddPropertyName(styleLetter)
AddPropertyName(type)
AddPropertyName(typeString)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Style
 *
 * \brief Model for style records in the database.
 */
class Style : public NamedEntity {
   Q_OBJECT
   Q_CLASSINFO("signal", "styles")


   friend class StyleEditor;
public:
   Style(QString t_name = "");
   Style(NamedParameterBundle const & namedParameterBundle);
   Style(Style const & other);

   virtual ~Style() = default;

   //! \brief The type of beverage.
   enum Type {Lager, Ale, Mead, Wheat, Mixed, Cider};
   Q_ENUMS( Type )

   //! \brief The category.
   Q_PROPERTY( QString category READ category WRITE setCategory /*NOTIFY changed*/ /*changedCategory*/ )
   //! \brief The category number.
   Q_PROPERTY( QString categoryNumber READ categoryNumber WRITE setCategoryNumber /*NOTIFY changed*/ /*changedCategoryNumber*/ )
   //! \brief The style letter
   Q_PROPERTY( QString styleLetter READ styleLetter WRITE setStyleLetter /*NOTIFY changed*/ /*changedStyleLetter*/ )
   //! \brief Which style guide the description belongs to.
   Q_PROPERTY( QString styleGuide READ styleGuide WRITE setStyleGuide /*NOTIFY changed*/ /*changedStyleGuide*/ )
   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The untranslated \c Type string.
   Q_PROPERTY( QString typeString READ typeString /* WRITE setUse NOTIFY changed*/ /*changedUse*/ )
   //! \brief The minimum og.
   Q_PROPERTY( double ogMin READ ogMin WRITE setOgMin /*NOTIFY changed*/ /*changedOgMin*/ )
   //! \brief The maximum og.
   Q_PROPERTY( double ogMax READ ogMax WRITE setOgMax /*NOTIFY changed*/ /*changedOgMax*/ )
   //! \brief The minimum fg.
   Q_PROPERTY( double fgMin READ fgMin WRITE setFgMin /*NOTIFY changed*/ /*changedFgMin*/ )
   //! \brief The maximum fg.
   Q_PROPERTY( double fgMax READ fgMax WRITE setFgMax /*NOTIFY changed*/ /*changedFgMax*/ )
   //! \brief The minimum ibus.
   Q_PROPERTY( double ibuMin READ ibuMin WRITE setIbuMin /*NOTIFY changed*/ /*changedIbuMin*/ )
   //! \brief The maximum ibus.
   Q_PROPERTY( double ibuMax READ ibuMax WRITE setIbuMax /*NOTIFY changed*/ /*changedIbuMax*/ )
   //! \brief The minimum color in SRM.
   Q_PROPERTY( double colorMin_srm READ colorMin_srm WRITE setColorMin_srm /*NOTIFY changed*/ /*changedColorMin_srm*/ )
   //! \brief The maximum color in SRM.
   Q_PROPERTY( double colorMax_srm READ colorMax_srm WRITE setColorMax_srm /*NOTIFY changed*/ /*changedColorMax_srm*/ )
   //! \brief The mininum carbonation in volumes at STP.
   Q_PROPERTY( double carbMin_vol READ carbMin_vol WRITE setCarbMin_vol /*NOTIFY changed*/ /*changedCarbMin_vol*/ )
   //! \brief The maximum carbonation in volumes at STP.
   Q_PROPERTY( double carbMax_vol READ carbMax_vol WRITE setCarbMax_vol /*NOTIFY changed*/ /*changedCarbMax_vol*/ )
   //! \brief The minimum ABV in percent.
   Q_PROPERTY( double abvMin_pct READ abvMin_pct WRITE setAbvMin_pct /*NOTIFY changed*/ /*changedAbvMin_pct*/ )
   //! \brief The maximum ABV in percent.
   Q_PROPERTY( double abvMax_pct READ abvMax_pct WRITE setAbvMax_pct /*NOTIFY changed*/ /*changedAbvMax_pct*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The profile.
   Q_PROPERTY( QString profile READ profile WRITE setProfile /*NOTIFY changed*/ /*changedProfile*/ )
   //! \brief The ingredients.
   Q_PROPERTY( QString ingredients READ ingredients WRITE setIngredients /*NOTIFY changed*/ /*changedIngredients*/ )
   //! \brief The commercial examples.
   Q_PROPERTY( QString examples READ examples WRITE setExamples /*NOTIFY changed*/ /*changedExamples*/ )

   void setCategory( const QString& var);
   void setCategoryNumber( const QString& var);
   void setStyleLetter( const QString& var);
   void setStyleGuide( const QString& var);
   void setType( Type t);
   void setOgMin( double var);
   void setOgMax( double var);
   void setFgMin( double var);
   void setFgMax( double var);
   void setIbuMin( double var);
   void setIbuMax( double var);
   void setColorMin_srm( double var);
   void setColorMax_srm( double var);
   void setCarbMin_vol( double var);
   void setCarbMax_vol( double var);
   void setAbvMin_pct( double var);
   void setAbvMax_pct( double var);
   void setNotes( const QString& var);
   void setProfile( const QString& var);
   void setIngredients( const QString& var);
   void setExamples( const QString& var);

   QString category() const;
   QString categoryNumber() const;
   QString styleLetter() const;
   QString styleGuide() const;
   Type type() const;
   const QString typeString() const;
   double ogMin() const;
   double ogMax() const;
   double fgMin() const;
   double fgMax() const;
   double ibuMin() const;
   double ibuMax() const;
   double colorMin_srm() const;
   double colorMax_srm() const;
   double carbMin_vol() const;
   double carbMax_vol() const;
   double abvMin_pct() const;
   double abvMax_pct() const;
   QString notes() const;
   QString profile() const;
   QString ingredients() const;
   QString examples() const;

   virtual Recipe * getOwningRecipe();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   QString m_category;
   QString m_categoryNumber;
   QString m_styleLetter;
   QString m_styleGuide;
   Type m_type;
   double m_ogMin;
   double m_ogMax;
   double m_fgMin;
   double m_fgMax;
   double m_ibuMin;
   double m_ibuMax;
   double m_colorMin_srm;
   double m_colorMax_srm;
   double m_carbMin_vol;
   double m_carbMax_vol;
   double m_abvMin_pct;
   double m_abvMax_pct;
   QString m_notes;
   QString m_profile;
   QString m_ingredients;
   QString m_examples;
};

Q_DECLARE_METATYPE( Style* )

#endif
