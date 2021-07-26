/*
 * BtLabel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "BtLabel.h"

#include <QSettings>
#include <QDebug>

#include "brewtarget.h"
#include "model/Style.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"

/*!
 * \brief Initialize the BtLabel with the parent and do some things with the type
 *
 * \param parent - QWidget* to the parent object
 * \param lType - the type of label: none, gravity, mass or volume
 * \return the initialized widget
 * \todo Not sure if I can get the name of the widget being created.
 *       Not sure how to signal the parent to redisplay
 */

BtLabel::BtLabel(QWidget *parent, LabelType lType) :
   QLabel(parent)
{
   whatAmI = lType;
   btParent = parent;
   _menu = 0;

   connect(this, &QWidget::customContextMenuRequested, this, &BtLabel::popContextMenu);
   return;
}

void BtLabel::initializeSection()
{
   QWidget* mybuddy;

   if ( ! _section.isEmpty() )
      return;

   // as much as I dislike it, dynamic properties can't be referenced on
   // initialization.
   mybuddy = buddy();
   // If the label has the configSection defined, use it
   // otherwise, if the paired field has a configSection, use it
   // otherwise, if the parent object has a configSection, use it
   // if all else fails, get the parent's object name
   if ( property("configSection").isValid() )
      _section = property("configSection").toString();
   else if ( mybuddy && mybuddy->property("configSection").isValid() )
      _section = mybuddy->property("configSection").toString();
   else if ( btParent->property("configSection").isValid() )
      _section = btParent->property("configSection").toString();
   else
   {
      qDebug() << "this failed" << this;
      _section = btParent->objectName();
   }
}

void BtLabel::initializeProperty()
{
   QWidget* mybuddy;

   if ( ! propertyName.isEmpty() )
      return;

   mybuddy = buddy();
   if ( property("editField").isValid() )
      propertyName = property("editField").toString();
   else if ( mybuddy && mybuddy->property("editField").isValid() )
      propertyName = mybuddy->property("editField").toString();
   else
      qDebug() << "That failed miserably";
}

void BtLabel::initializeMenu()
{
   Unit::unitDisplay unit;
   Unit::unitScale scale;

   if ( _menu )
      return;

   unit  = static_cast<Unit::unitDisplay>(PersistentSettings::value(propertyName, Unit::noUnit, _section, PersistentSettings::UNIT).toInt());
   scale = static_cast<Unit::unitScale>(PersistentSettings::value(propertyName, Unit::noScale, _section, PersistentSettings::SCALE).toInt());

   switch( whatAmI )
   {
      case COLOR:
         _menu = Brewtarget::setupColorMenu(btParent,unit);
         break;
      case DENSITY:
         _menu = Brewtarget::setupDensityMenu(btParent,unit);
         break;
      case MASS:
         _menu = Brewtarget::setupMassMenu(btParent,unit,scale);
         break;
      case MIXED:
         // This looks weird, but it works.
         _menu = Brewtarget::setupVolumeMenu(btParent,unit,scale,false); // no scale menu
         break;
      case TEMPERATURE:
         _menu = Brewtarget::setupTemperatureMenu(btParent,unit);
         break;
      case VOLUME:
         _menu = Brewtarget::setupVolumeMenu(btParent,unit,scale);
         break;
      case TIME:
         _menu = Brewtarget::setupTimeMenu(btParent,scale); //scale menu only
         break;
      case DATE:
         _menu = Brewtarget::setupDateMenu(btParent,unit); // unit only
         break;
      case DIASTATIC_POWER:
         _menu = Brewtarget::setupDiastaticPowerMenu(btParent,unit);
         break;
      default:
         return;
   }
}

void BtLabel::popContextMenu(const QPoint& point)
{
   QObject* calledBy = sender();
   QWidget* widgie;
   QAction *invoked;

   if ( calledBy == 0 )
      return;

   widgie = qobject_cast<QWidget*>(calledBy);
   if ( widgie == 0 )
      return;

   initializeProperty();
   initializeSection();
   initializeMenu();

   invoked = _menu->exec(widgie->mapToGlobal(point));
   Unit::unitDisplay unit = static_cast<Unit::unitDisplay>(PersistentSettings::value(propertyName, Unit::noUnit, _section, PersistentSettings::UNIT).toInt());
   Unit::unitScale scale  = static_cast<Unit::unitScale>(PersistentSettings::value(propertyName, Unit::noUnit, _section, PersistentSettings::SCALE).toInt());

   if ( invoked == 0 )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == _menu ) {
      PersistentSettings::insert(propertyName, invoked->data(), _section, PersistentSettings::UNIT);
      // reset the scale if required
      if (PersistentSettings::contains(propertyName, _section, PersistentSettings::SCALE) ) {
         PersistentSettings::insert(propertyName, Unit::noScale, _section, PersistentSettings::SCALE);
      }
   } else {
      PersistentSettings::insert(propertyName, invoked->data(), _section, PersistentSettings::SCALE);
   }

   // To make this all work, I need to set ogMin and ogMax when og is set.
   if ( propertyName == "og" ) {
      PersistentSettings::insert(PropertyNames::Style::ogMin, invoked->data(),_section, PersistentSettings::UNIT);
      PersistentSettings::insert(PropertyNames::Style::ogMax, invoked->data(),_section, PersistentSettings::UNIT);
   } else if ( propertyName == "fg" ) {
      PersistentSettings::insert(PropertyNames::Style::fgMin, invoked->data(),_section, PersistentSettings::UNIT);
      PersistentSettings::insert(PropertyNames::Style::fgMax, invoked->data(),_section, PersistentSettings::UNIT);
   } else if ( propertyName == "color_srm" ) {
      PersistentSettings::insert(PropertyNames::Style::colorMin_srm, invoked->data(),_section, PersistentSettings::UNIT);
      PersistentSettings::insert(PropertyNames::Style::colorMax_srm, invoked->data(),_section, PersistentSettings::UNIT);
   }

   // Hmm. For the color fields, I want to include the ecb or srm in the label
   // text here.
   if ( whatAmI == COLOR ) {
      Unit::unitDisplay disp = (Unit::unitDisplay)invoked->data().toInt();
      setText( tr("Color (%1)").arg(Brewtarget::colorUnitName(disp)));
   }

   // Remember, we need the original unit, not the new one.

   emit labelChanged(unit,scale);

}

BtColorLabel::BtColorLabel(QWidget *parent)
   : BtLabel(parent,COLOR)
{
}

BtDateLabel::BtDateLabel(QWidget *parent)
   : BtLabel(parent,DATE)
{
}

BtDensityLabel::BtDensityLabel(QWidget *parent)
   : BtLabel(parent,DENSITY)
{
}

BtMassLabel::BtMassLabel(QWidget *parent)
   : BtLabel(parent,MASS)
{
}

BtMixedLabel::BtMixedLabel(QWidget *parent)
   : BtLabel(parent,MIXED)
{
}

BtTemperatureLabel::BtTemperatureLabel(QWidget *parent)
   : BtLabel(parent,TEMPERATURE)
{
}

BtTimeLabel::BtTimeLabel(QWidget *parent)
   : BtLabel(parent,TIME)
{
}

BtVolumeLabel::BtVolumeLabel(QWidget *parent)
   : BtLabel(parent,VOLUME)
{
}

BtDiastaticPowerLabel::BtDiastaticPowerLabel(QWidget *parent)
   : BtLabel(parent,DIASTATIC_POWER)
{
}
