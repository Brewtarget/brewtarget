/*
 * ConverterTool.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "ConverterTool.h"
#include "unit.h"

ConverterTool::ConverterTool(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   
   connect( pushButton_convert, SIGNAL(clicked()), this, SLOT(convert()) );
}

void ConverterTool::convert()
{
   outputLineEdit->setText(Unit::convert(inputLineEdit->text(), outputUnitsLineEdit->text()));
}
