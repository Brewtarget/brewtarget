/*
 * RefractoDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com) and Eric Tamme,  2009-2010.
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

#include <QString>
#include "RefractoDialog.h"
#include "Algorithms.h"
#include "brewtarget.h"
#include <cmath>
#include <QMessageBox>

RefractoDialog::RefractoDialog(QWidget* parent) : QDialog(parent)
{
   setupUi(this);

   connect( pushButton_calculate, SIGNAL( clicked() ), this, SLOT( calculate() ) );
}

RefractoDialog::~RefractoDialog()
{
}

void RefractoDialog::calculate()
{
   bool haveCP = true;
   bool haveOP = true;
   bool haveOG = true;

   double originalPlato = lineEdit_op->text().toDouble(&haveOP);
   double inputOG = lineEdit_inputOG->text().toDouble(&haveOG);
   double currentPlato = lineEdit_cp->text().toDouble(&haveCP);
   double ri = 0;
   double og = 0;
   double sg = 0;
   double re = 0;
   double abv = 0;
   double abw = 0;

   clearOutputFields();

   // Abort if we don't have the current plato.
   if( ! haveCP )
      return;

   ri = Algorithms::Instance().refractiveIndex(currentPlato);
   lineEdit_ri->setText(Brewtarget::displayAmount(ri));

   if( ! haveOG && haveOP )
   {
         inputOG = Algorithms::Instance().PlatoToSG_20C20C( originalPlato );
         lineEdit_inputOG->setText(Brewtarget::displayAmount(inputOG));
   }
   else if( ! haveOP && haveOG )
   {
      originalPlato = Algorithms::Instance().SG_20C20C_toPlato( inputOG );
      lineEdit_op->setText(Brewtarget::displayAmount(originalPlato));
   }
   else if( ! haveOP && ! haveOG )
      return; // Can't do much if we don't have OG or OP.

   og = Algorithms::Instance().PlatoToSG_20C20C( originalPlato );
   sg = Algorithms::Instance().sgByStartingPlato( originalPlato, currentPlato );
   re = Algorithms::Instance().realExtract( sg, currentPlato );
   abv = Algorithms::Instance().getABVBySGPlato( sg, currentPlato );
   abw = Algorithms::Instance().getABWBySGPlato( sg, currentPlato );

   // Warn the user if the inputOG and calculated og don't match.
   if( abs(og - inputOG) > 0.002 )
      QMessageBox::warning(this, tr("OG Mismatch"),
                           tr("Based on the given original plato, the OG should be %1, but you have entered %2. Continuing with the calculated OG.").arg(og,0,'f',3).arg(inputOG,0,'f',3));

   lineEdit_og->setText(Brewtarget::displayAmount(og));
   lineEdit_sg->setText(Brewtarget::displayAmount(sg));
   lineEdit_re->setText(Brewtarget::displayAmount(re));
   lineEdit_abv->setText(Brewtarget::displayAmount(abv));
   lineEdit_abw->setText(Brewtarget::displayAmount(abw));
}

void RefractoDialog::clearOutputFields()
{
   lineEdit_ri->clear();
   lineEdit_og->clear();
   lineEdit_sg->clear();
   lineEdit_re->clear();
   lineEdit_abv->clear();
   lineEdit_abw->clear();
}
