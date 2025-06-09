/*======================================================================================================================
 * widgets/SmartValueDisplay.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
 =====================================================================================================================*/
#include "widgets/SmartValueDisplay.h"

#include "widgets/SmartLabel.h"

SmartValueDisplay::SmartValueDisplay(QWidget *parent) :
   QLabel(parent),
   SmartField{} {
   return;
}

SmartValueDisplay::~SmartValueDisplay() = default;

QString SmartValueDisplay::getRawText() const {
   return this->text();
}

void SmartValueDisplay::setRawText(QString const & text) {
   this->QLabel::setText(text);
   return;
}

void SmartValueDisplay::connectSmartLabelSignal(SmartLabel & smartLabel) {
   connect(&smartLabel, &SmartLabel::changedSystemOfMeasurementOrScale, this, &SmartValueDisplay::displayChanged);
   return;
}

void SmartValueDisplay::doPostInitWork() {
   return;
}

void SmartValueDisplay::displayChanged(SmartAmounts::ScaleInfo previousScaleInfo) {
   this->correctEnteredText(previousScaleInfo);
   return;
}
