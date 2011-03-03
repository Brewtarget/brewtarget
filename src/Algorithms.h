/*
 * Algorithms.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com) and Eric Tamme,  2009-2011.
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

#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#define ROOT_PRECISION 0.0000001

#include <QList>

class Algorithms
{
public:
	static Algorithms& Instance() {
	  static Algorithms algorithmsSingleton;
	  return algorithmsSingleton;
	}

   // Need the following to be cross-platform compatible.
   bool isnan(double d);
   double round(double d);

   /**
    * A duplicate-removing function for SORTED lists.
    * equals(T,T) is a function that should return true iff the
    * two arguments are considered equal in the same sense that
    * the list is sorted in.
    * Why isn't this already a member of QList?
    */
   template<class T> void unDup( QList<T>& list, bool (*equals)(T,T) )
   { // Must be implemented in this header file apparently :(
      int i = 0;

      while( i < list.size() - 1 )
      {
         if( equals(list.at(i), list.at(i+1)) )
            list.removeAt(i+1); // Removes the next item.
         else
            i++; // If the adjacent items are unequal, we can advance.
      }
   }

	// Polynomial evaluation
	double intPow( double base, unsigned int pow );
	double polyEval( double* poly, unsigned int order, double x );
	// Root finding by the secant method. Returns HUGE_VAL on failure.
	double rootFind( double* poly, unsigned int order, double x0, double x1 );

	//===================Beer-related stuff=====================
	double SG_20C20C_toPlato( double sg );
	double PlatoToSG_20C20C( double plato );
	double getWaterDensity_kgL( double celsius );
	double hydrometer15CCorrection( double celsius );

   // Estimates plato from kg of dissolved sucrose (sugar_kg) and
   // the total wort volume wort_l.
   double getPlato( double sugar_kg, double wort_l );
   // Converts FG to plato, given the OG.
   double ogFgToPlato( double og, double fg );
	// Gets ABV by using current gravity reading and brix reading.
	double getABVBySGPlato( double sg, double plato );
	// Gets ABW from current gravity and plato.
	double getABWBySGPlato( double sg, double plato );
	// Gives you the SG from the starting plato and current plato.
	double sgByStartingPlato( double startingPlato, double currentPlato );
	// Returns the refractive index from plato.
	double refractiveIndex( double plato );
	// Corrects the apparent extract 'plato' to the real extract using current gravity 'sg'.
	double realExtract( double sg, double plato );

private:
	// This is the cubic fit to get Plato from specific gravity, measured at 20C
	// relative to density of water at 20C.
	// P = -616.868 + 1111.14(SG) - 630.272(SG)^2 + 135.997(SG)^3
	double PlatoFromSG_20C20C[4];
	unsigned int PlatoFromSG_20C20C_order;
	// Water density polynomial, given in kg/L as a function of degrees C.
	// 1.80544064e-8*x^3 - 6.268385468e-6*x^2 + 3.113930471e-5*x + 0.999924134
	double waterDensityPoly_C[6];
	int waterDensityPoly_C_order;
	double hydroCorrection15CPoly[4];
	int hydroCorrection15CPoly_order;
   double sucroseDensity_kgL;

	Algorithms(); 								// ctor hidden
	Algorithms(Algorithms const&); 				// copy ctor hidden
	Algorithms& operator=(Algorithms const&); 	// assign op. hidden
	// dtor must be defined in header or linking explodes on undefined reference to ~Algorithms()
	~Algorithms(){}								// dtor hidden
};


#endif /* ALGORITHMS_H_ */
