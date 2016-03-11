/*
 * CustomComboBox.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "CustomComboBox.h"
#include <QListView>

#include <QStylePainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>

CustomComboBox::CustomComboBox(QWidget* parent)
   : QComboBox(parent)
{
}

void CustomComboBox::showPopup()
{
   view()->setFixedWidth(300);
   QComboBox::showPopup();
}

void CustomComboBox::paintEvent(QPaintEvent*)
{
   QStylePainter painter(this);
   
   QStyleOptionComboBox opts;
   initStyleOption(&opts);
   //opts.currentText = "Wasup";
   opts.currentText = currentText();

   // Draw combo box frame and shit.
   painter.drawComplexControl(QStyle::CC_ComboBox, opts);
   // Have to draw label separately? Stupid.
   painter.drawControl(QStyle::CE_ComboBoxLabel, opts);
}
