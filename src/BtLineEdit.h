/*
 * BtLineEdit.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#ifndef BTLINEEDIT_H
#define BTLINEEDIT_H


#include <QLineEdit>
#include "unit.h"
#include "UnitSystem.h"
#include "ingredient.h"

class BtGenericEdit;
class BtMassEdit;
class BtVolumeEdit;
class BtTemperatureEdit;
class BtTimeEdit;
class BtDensityEdit;
class BtColorEdit;
class BtMixedEdit;
class BtDiastaticPowerEdit;
class BtStringEdit;

/*!
 * \class BtLineEdit
 * \author Mik Firestone
 *
 * \brief This extends QLineEdit such that the Object handles all the unit
 * transformation we do, instead of each dialog. It makes the code much
 * nicer and prevents more cut'n'paste code.
 *
 */
class BtLineEdit : public QLineEdit
{
   Q_OBJECT
   Q_PROPERTY( int     type              READ type              WRITE setType              STORED false)
   Q_PROPERTY( QString configSection     READ configSection     WRITE setConfigSection     STORED false)
   Q_PROPERTY( QString editField         READ editField         WRITE setEditField         STORED false)
   Q_PROPERTY( QString forcedUnit        READ forcedUnit        WRITE setForcedUnit        STORED false)
   Q_PROPERTY( QString forcedScale       READ forcedScale       WRITE setForcedScale       STORED false)

public:

   /*! \brief Initialize the BtLineEdit with the parent and do some things with the type
   * \param parent - QWidget* to the parent object
   * \param lType - the type of label: none, gravity, mass or volume
   * \return the initialized widget
   * \todo Not sure if I can get the name of the widget being created.
   *       Not sure how to signal the parent to redisplay
   */

   BtLineEdit(QWidget* parent = 0, Unit::UnitType type = Unit::None);
   double toSI(Unit::unitDisplay oldUnit = Unit::noUnit, Unit::unitScale oldScale = Unit::noScale, bool force = false);
   // Use this when you want to do something with the returned QString
   QString displayAmount( double amount, int precision = 3);

   // Use one of these when you just want to set the text
   void    setText( Ingredient* element, int precision=3 );
   void    setText( double amount, int precision = 3);
   void    setText( QString amount, int precision=3 );
   void    setText( QVariant amount, int precision=3 );

   // Too many places still use getDouble, which just hoses me down. We're
   // gonna fix this.
   double  toDouble(bool* ok);

   // By defining the setters/getters, we can remove the need for
   // initializeProperties.
   QString editField() const;
   void setEditField( QString editField );

   QString configSection();
   void setConfigSection( QString configSection );

   int type() const;
   void setType(int type);

   QString forcedUnit() const;
   void setForcedUnit(QString forcedUnit);

   QString forcedScale() const;
   void setForcedScale(QString forcedScale);

public slots:
   void onLineChanged();
   void lineChanged(Unit::unitDisplay oldUnit, Unit::unitScale oldScale);

signals:
   void textModified();

protected:
   QWidget *btParent;
   QString _section, _editField;
   Unit::UnitType _type;
   Unit::unitDisplay _forceUnit;
   Unit::unitScale _forceScale;
   Unit* _units;

};

class BtGenericEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtGenericEdit(QWidget* parent);
};

class BtMassEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtMassEdit(QWidget* parent);
};

class BtVolumeEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtVolumeEdit(QWidget* parent);
};

class BtTemperatureEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtTemperatureEdit(QWidget* parent);
};

class BtTimeEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtTimeEdit(QWidget* parent);
};

class BtDensityEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtDensityEdit(QWidget* parent);
};

class BtColorEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtColorEdit(QWidget* parent);
};

class BtStringEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtStringEdit(QWidget* parent);
};

// mixed objects are a pain.
class BtMixedEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtMixedEdit(QWidget* parent);

public slots:
   void setIsWeight(bool state);

};

class BtDiastaticPowerEdit : public BtLineEdit
{
   Q_OBJECT

public:
   BtDiastaticPowerEdit(QWidget* parent);
};

#endif
