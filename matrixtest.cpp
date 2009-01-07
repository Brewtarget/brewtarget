/*
 * matrixtest.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <iostream>
#include "matrix.h"

int main()
{
   Matrix m( 2, 2 );
   vector<double> row;
   
   row.push_back(1);
   row.push_back(2);
   m.setRow(0, row);
   
   row[0] = 4;
   row[1] = 5;
   m.setRow(1, row);
   
   cout << m << '\n';
   cout << m.inverse() << '\n';
   
   Matrix inv = m.inverse();
   Matrix id = m * inv;
   
   cout << id;
   
   return 0;
}

