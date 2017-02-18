/*
 * style.h is part of Brewtarget, and is Copyright the following
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

#ifndef _STYLE_H
#define _STYLE_H

#include <QString>
#include <QStringList>
#include "BeerXMLElement.h"

// Forward declarations.
class Style;
bool operator<(Style &s1, Style &s2);
bool operator==(Style &s1, Style &s2);

/*!
 * \class Style
 * \author Philip G. Lee
 *
 * \brief Model for style records in the database.
 */
class Style : public BeerXMLElement
{
   Q_OBJECT
   Q_CLASSINFO("signal", "styles")
   Q_CLASSINFO("prefix", "style")
   
   friend class Database;
public:

   virtual ~Style() {}

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
   
   void setCategory( const QString& var );
   void setCategoryNumber( const QString& var );
   void setStyleLetter( const QString& var );
   void setStyleGuide( const QString& var );
   void setType( Type t );
   void setOgMin( double var );
   void setOgMax( double var );
   void setFgMin( double var );
   void setFgMax( double var );
   void setIbuMin( double var );
   void setIbuMax( double var );
   void setColorMin_srm( double var );
   void setColorMax_srm( double var );
   void setCarbMin_vol( double var );
   void setCarbMax_vol( double var );
   void setAbvMin_pct( double var );
   void setAbvMax_pct( double var );
   void setNotes( const QString& var );
   void setProfile( const QString& var );
   void setIngredients( const QString& var );
   void setExamples( const QString& var );

   QString category() const;
   QString categoryNumber() const;
   QString styleLetter() const;
   QString styleGuide() const;
   const Type type() const;
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

   static QString classNameStr();

signals:
   //! \brief Emitted when \c name() changes.
   void changedName(QString);

   /*
   void changedCategory(QString);
   void changedCategoryNumber(QString);
   void changedStyleLetter(QString);
   void changedStyleGuide(QString);
   void changedType(Type);
   void changedOgMin(double);
   void changedOgMax(double);
   void changedFgMin(double);
   void changedFgMax(double);
   void changedIbuMin(double);
   void changedIbuMax(double);
   void changedColorMin_srm(double);
   void changedColorMax_srm(double);
   void changedCarbMin_vol(double);
   void changedCarbMax_vol(double);
   void changedAbvMin_pct(double);
   void changedAbvMax_pct(double);
   void changedNotes(QString);
   void changedProfile(QString);
   void changedIngredients(QString);
   void changedExamples(QString);
   */

private:
   Style(Brewtarget::DBTable table, int key);
   Style( Style const& other );
   
   bool isValidType( const QString &str );
   static QStringList types;
   
   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
};

Q_DECLARE_METATYPE( Style* )

inline bool StylePtrLt( Style* lhs, Style* rhs)
{
   return *lhs < *rhs;
}

inline bool StylePtrEq( Style* lhs, Style* rhs)
{
   return *lhs == *rhs;
}

struct Style_ptr_cmp
{
   bool operator()( Style* lhs, Style* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Style_ptr_equals
{
   bool operator()( Style* lhs, Style* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif //_STYLE_H
