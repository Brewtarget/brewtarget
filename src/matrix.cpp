/*
 * matrix.cpp is part of Brewtarget, and is Copyright the following
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

#include <iostream>
#include <QVector>
#include <cmath>
#include "matrix.h"

Matrix::~Matrix()
{
   delete [] _data;
}

Matrix::Matrix( unsigned int rows, unsigned int cols )
{
   _rows = rows;
   _cols = cols;
   _data = new double[ rows * cols ];
}

Matrix::Matrix( const QVector<Matrix> &colVec )
{
   unsigned int i, j;
   
   _cols = colVec.size();
   if( _cols == 0 )
   {
      _rows = 0;
      return;
   }
   
   _rows =  colVec[0]._rows;
   
   _data = new double[ _cols * _rows ];
   
   for( j = 0; static_cast<int>(j) < colVec.size(); ++j )
   {
      if( colVec[j]._rows != _rows )
      {
         std::cerr << "Matrix: dimension error in initialization\n";
         throw DimensionException( colVec[j]._rows, 0, true, false );
      }
      
      for( i = 0; i < _rows; ++i )
         setVal( i, j, colVec[j].getVal(i, 0) );
   }
}

Matrix::Matrix( const Matrix &m, unsigned int colStart, unsigned int colEnd )
{
   unsigned int i, j, k;
   
   _rows = m._rows;
   _cols = colEnd - colStart + 1;
   _data = new double[ _rows * _cols ];
   
   for( i = 0; i < _rows; ++i )
      for( j=0, k=colStart; k <= colEnd; ++j,++k )
         setVal( i, j, m.getVal(i, k) );
}

Matrix::Matrix( const Matrix &rhs )
{
   _rows = rhs._rows;
   _cols = rhs._cols;
   
   unsigned int numElts = _rows*_cols;
   unsigned int i;
   
   _data = new double[ _rows*_cols ];
   for( i = 0; i < numElts; ++i )
      _data[i] = rhs._data[i];
}

Matrix& Matrix::operator=( const Matrix &rhs )
{
   if( this == &rhs )
      return *this;
      
   _rows = rhs._rows;
   _cols = rhs._cols;
   
   unsigned int numElts = _rows*_cols;
   unsigned int i;
   
   if( ! _data )
      delete [] _data;
   _data = new double[ _rows*_cols ];
   for( i = 0; i < numElts; ++i )
      _data[i] = rhs._data[i];
      
   return *this;
}

std::ostream& operator<<( std::ostream &os, const Matrix &rhs )
{
   unsigned int i;
   unsigned int j;
   double *ptr = rhs._data;
   
   for( i = 0; i < rhs._rows; ++i )
   {
      os << "[ ";
      for( j = 0; j < rhs._cols - 1; ++j )
      {
         os << *ptr << ", ";
         ptr++;
      }
      
      os << *ptr << "]\n";
      ptr++;
   }
   
   return os;
}

Matrix& Matrix::operator+=( const Matrix &rhs )
{
   unsigned int numElts = _rows * _cols;
   unsigned int i;
   
   if( !(_rows == rhs._rows && _cols == rhs._cols) )
   {
      std::cerr << "Matrix: dimension error with +=\n";
      throw DimensionException( rhs._rows, rhs._cols, true, true);
   }
   
   for( i = 0; i < numElts; ++i )
      _data[i] += rhs._data[i];
      
   return *this;
}

Matrix& Matrix::operator-=( const Matrix &rhs )
{
   unsigned int numElts = _rows * _cols;
   unsigned int i;
   
   if( !(_rows == rhs._rows && _cols == rhs._cols) )
   {
      std::cerr << "Matrix: dimension error with -=\n";
      throw DimensionException( rhs._rows, rhs._cols, true, true);
   }
   
   for( i = 0; i < numElts; ++i )
      _data[i] -= rhs._data[i];
      
   return *this;
}

const Matrix Matrix::operator*( const Matrix &rhs ) const
{
   unsigned int i, j, k;
   double tmp;
   
   if( rhs._rows != _cols )
   {
      std::cerr << "Matrix: dimension error with *\n";
      throw DimensionException( rhs._rows, 0, true, false );
   }
   
   Matrix ret( _rows, rhs._cols );
   
   for( i = 0; i < ret._rows; ++i )
   {
      for( j = 0; j < ret._cols; ++j )
      {
         tmp = 0.0;
         for( k = 0; k < _cols; ++k )
            tmp += getVal( i, k ) * rhs.getVal( k, j );
            
         ret.setVal( i, j, tmp );
      }
   }
   
   return ret;
}

const Matrix Matrix::operator+( const Matrix &other ) const
{
   Matrix result( *this );
   result += other;
   return result;
}

const Matrix Matrix::operator-( const Matrix &other ) const
{
   Matrix result( *this );
   result -= other;
   return result;
}

Matrix Matrix::getRow( unsigned int row ) const
{
   unsigned int i;
   
   if( row >= _rows )
   {
      std::cerr << "Matrix: dimension error in getRow()\n";
      throw DimensionException( _rows, 0, true, false );
   }
   
   Matrix ret( 1, _cols );
   
   for( i = 0; i < _cols; ++i )
      ret.setVal( 0, i, getVal( row, i) );
      
   return ret;
}

Matrix Matrix::getCol( unsigned int col ) const
{
   unsigned int i;
   
   if( col >= _cols )
   {
      std::cerr << "Matrix: dimension error in getCol()\n";
      throw DimensionException( 0, _cols, false, true );
   }
   
   Matrix ret( _rows, 1 );
   
   for( i = 0; i < _rows; ++i )
      ret.setVal( i, 0, getVal(i, col) );
   
   return ret;
}

inline double Matrix::getVal( unsigned int row, unsigned int col ) const
{
   if( _cols*row + col < _rows*_cols )
      return _data[ _cols*row + col ];
   else
   {
      std::cerr << "Matrix: invalid access at _data[" << row << "][" << col << "]\n";
      throw DimensionException( _rows, _cols, true, true );
   }
}

inline void Matrix::setVal( unsigned int row, unsigned int col, double val )
{
   if( _cols*row + col < _rows*_cols )
      _data[ _cols*row + col ] = val;
   else
   {
      std::cerr << "Matrix: invalid access at _data[" << row << "][" << col << "]\n";
      throw DimensionException( _rows, _cols, true, true );
   }

   return;
}

void Matrix::swapRows( unsigned int row1, unsigned int row2 )
{
   unsigned int j;
   double tmp;
   
   if( row1 >= _rows || row2 >= _rows )
   {
      std::cerr << "Matrix: swapRows(): can't swap row " << row1 << " and row " << row2;
      throw DimensionException( _rows, 0, true, false );
   }
   
   for( j = 0; j < _cols; ++j )
   {
      tmp = getVal( row1, j );
      setVal( row1, j, getVal( row2, j ) );
      setVal( row2, j, tmp );
   }
}

void Matrix::rref()
{
   unsigned int i,j,k,l,m;
   double pivot, mult;

   k = 0;
   for( i = 0; i < _rows && k < _cols; ++i )
   {
      // If this row's kth column is zero...
      if( qAbs( getVal( i, k ) ) < EPSILON )
      {
         // Search for nonzero entry in this column (after the ith row).
         for( l = i+1; l < _rows; ++l )
            if( qAbs( getVal( l, k ) ) >= EPSILON )
               break;

         // Make sure we didn't fall off the edge
         if( l == _rows )
         {
            ++k; // Give up on this k and try the next one.
            --i; // Need to subtract so next iteration continues on this row.
            continue;
         }

         // Otherwise, swap rows.
         swapRows( i, l );
      }

      // Normalize the row so that a[i][k] = 1.
      pivot = getVal( i, k );
      for( j = k; j < _cols; ++j )
         setVal( i, j, getVal(i, j) / pivot );

      // Search for rows to add to.
      for( l = 0; l < _rows; ++l )
      {
         // Don't want to do this to our own row.
         if( l == i )
            continue;

         if( qAbs( getVal( l, k ) ) >= EPSILON )
         {
            mult = getVal(l,k);
            for( m = 0; m < _cols; ++m )
               setVal( l, m, getVal(l,m) - mult*getVal(i,m) );
         }
      }

      ++k;
   }
}

// Returns true if the matrix has non-zero
// entries on the diagonals.
bool Matrix::hasNonZeroDiags() const
{
   unsigned int i;
   bool ret = true;
   
   for( i = 0; i < _rows && i < _cols; ++i )
      ret = ret & ( qAbs(getVal(i,i)) >= EPSILON );
      
   return ret;
}

unsigned int Matrix::getRows() const
{
   return _rows;
}

unsigned int Matrix::getCols() const
{
   return _cols;
}

void Matrix::setRow( unsigned int row, QVector<double> vec )
{
   unsigned int j;
   
   if( vec.size() != static_cast<int>(_cols) )
   {
      std::cerr << "Matrix: setRow(): dimension error\n";
      throw DimensionException( 0, _cols, false, true );
   }
   
   for( j = 0; j < _cols; ++j )
      setVal( row, j, vec[j] );
}

void Matrix::setCol( unsigned int col, QVector<double> vec )
{
   unsigned int i;
   
   if( vec.size() != static_cast<int>(_rows) )
   {
      std::cerr << "Matrix: setCol(): dimension error\n";
      throw DimensionException( _rows, 0, true, false );
   }
   
   for( i = 0; i < _rows; ++i )
      setVal( i, col, vec[i] );
}

bool Matrix::hasInverse() const
{
   if( _rows != _cols )
      return false;
      
   Matrix m( *this );
   m.rref();
   return m.hasNonZeroDiags();
}

Matrix Matrix::getIdentity( unsigned int n )
{
   unsigned int i;
   Matrix m( n, n );
   
   for( i = 0; i < n; ++i )
      m.setVal( i, i, 1.0 );
 
   return m;
}

void Matrix::appendCols( const Matrix& other )
{
   unsigned int i, j, k;
   double *oldData = _data;
   unsigned int oldCols;
   
   if( _rows != other._rows )
   {
      std::cerr << "Matrix: appendCols(): dimension error\n";
      throw DimensionException( other._rows, 0, true, false );
   }
   
   _data = new double[ _rows * (_cols + other._cols) ];
   oldCols = _cols;
   _cols += other._cols;
   
   
   // Put in the old values, and copy in the new
   for( i = 0; i < _rows; ++i )
   {
      // Old values
      for( j = 0; j < oldCols; ++j )
         setVal(i, j, oldData[oldCols*i + j]);
      // Appended values
      for( j = oldCols, k=0; j < _cols; ++j, ++k )
         setVal( i, j, other.getVal(i, k) );
   }
   
   if( ! oldData )
      delete [] oldData;
}

Matrix Matrix::inverse() const
{
   if( _rows != _cols )
   {
      std::cerr << "Matrix: inverse(): must be square";
      throw DimensionException( _rows, _cols, true, true );
   }
   
   Matrix m( *this );
   
   m.appendCols(getIdentity(_rows));
   m.rref();

   if( ! m.hasNonZeroDiags() )
   {
      std::cerr << "Matrix: inverse(): did not have an inverse";
      throw IncomputableException();
   }
   
   Matrix inv( m, _cols, m._cols - 1);
   return inv;
}

