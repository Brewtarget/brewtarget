/*
 * OptionDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include "OptionDialog.h"
#include "brewtarget.h"

OptionDialog::OptionDialog(QWidget* parent)
{
   setupUi(this);
   
   if( parent != 0 )
   {
      setWindowIcon(parent->windowIcon());
   }

   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveAndClose() ) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( cancel() ) );
}

void OptionDialog::show()
{
   showChanges();
   setVisible(true);
}

void OptionDialog::saveAndClose()
{
   Brewtarget::englishUnits = (checkBox_USUnits->checkState() == Qt::Checked);

   if( Brewtarget::mainWindow != 0 )
      Brewtarget::mainWindow->showChanges(); // Make sure the main window updates.

   setVisible(false);
}

void OptionDialog::cancel()
{
   setVisible(false);
}

void OptionDialog::showChanges()
{
   checkBox_USUnits->setCheckState( Brewtarget::englishUnits ? Qt::Checked : Qt::Unchecked );
}
