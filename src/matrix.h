/*
 * matrix.h is part of Brewtarget, and is Copyright the following
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

#ifndef _MATRIX_H
#define _MATRIX_H

#include <iostream>
#include <QVector>
#include <cmath>
#include <exception>

#define EPSILON 0.00001

//======================Class Defns.=============================
class Matrix;
class DimensionException;
class IncomputableException;

std::ostream& operator<<( std::ostream &os, const Matrix &rhs );

//======================Class: Matrix=============================
class Matrix
{
   friend std::ostream& operator<<( std::ostream &os, const Matrix &rhs );

   public:
      ~Matrix(); // Destructor
      Matrix( unsigned int rows, unsigned int cols ); // Constructor
      Matrix( const QVector<Matrix> &colVec ); // Constructor
      Matrix( const Matrix &m, unsigned int colStart, unsigned int colEnd ); // Constructor
      Matrix( const Matrix &rhs ); // Copy constructor
      
      static Matrix getIdentity( unsigned int n ); // Gets n x n identity matrix.
      
      Matrix& operator=( const Matrix &rhs );
      Matrix& operator+=( const Matrix &rhs );
      Matrix& operator-=( const Matrix &rhs );
      const Matrix operator+( const Matrix &other ) const;
      const Matrix operator-( const Matrix &other ) const;
      const Matrix operator*( const Matrix &rhs ) const;
      Matrix getRow( unsigned int row ) const;
      Matrix getCol( unsigned int col ) const;
      unsigned int getRows() const;
      unsigned int getCols() const;
      inline double getVal( unsigned int row, unsigned int col ) const;
      inline void setVal( unsigned int row, unsigned int col, double val );
      void setRow( unsigned int row, QVector<double> vec );
      void setCol( unsigned int col, QVector<double> vec );
      Matrix inverse() const;
      bool hasInverse() const;
      
      void rref();
      bool hasNonZeroDiags() const;
      void swapRows( unsigned int row1, unsigned int row2 );
      void appendCols( const Matrix& other );
      
   private:
      unsigned int _rows;
      unsigned int _cols;
      double *_data;
};

//======================Class: DimensionException=============================
class DimensionException: public std::exception
{
   virtual const char* what() const throw()
   {
      return "Dimensions of argument were not expected.";
   }
   
   public:
      DimensionException(unsigned int argRows, unsigned int argCols, bool rowsMatter, bool colsMatter )
      {
         _argRows = argRows;
         _argCols = argCols;
         _rowsMatter = rowsMatter;
         _colsMatter = colsMatter;
      }
      
      bool colsMatter(){ return _colsMatter; }
      bool rowsMatter(){ return _rowsMatter; }
      unsigned int getArgRows(){ return _argRows; }
      unsigned int getArgCols(){ return _argCols; }
      
   private:
      unsigned int _argRows;
      unsigned int _argCols;
      bool _rowsMatter;
      bool _colsMatter;
};

//======================Class: IncomputableException=============================
class IncomputableException: public std::exception
{
   virtual const char* what() const throw()
   {
      return "Could not compute what you asked.";
   }
};

#endif

