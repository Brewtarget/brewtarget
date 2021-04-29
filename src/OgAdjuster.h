/*
 * OgAdjuster.h is part of Brewtarget, and is Copyright the following
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
#ifndef OGADJUSTER_H
#define OGADJUSTER_H

#include <QDialog>
#include "ui_ogAdjuster.h"
#include "model/Recipe.h"

/*!
 * \class OgAdjuster
 * \author Philip G. Lee
 *
 * \brief View/controller dialog that helps you correct your OG on brew day.
 */
class OgAdjuster : public QDialog, public Ui::ogAdjuster
{
   Q_OBJECT

public:
   OgAdjuster( QWidget* parent = 0 );
   //! Set the recipe whose OG to correct.
   void setRecipe( Recipe* rec );

public slots:
   void calculate();

private:
   Recipe* recObs;
};

#endif
