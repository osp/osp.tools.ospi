/***************************************************************************
 *   Copyright (C) 2012 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef OSPI_TRANSFORM_H
#define OSPI_TRANSFORM_H

#include <string>
#include <vector>
#include <sstream>

namespace ospi {

	class Point{
		public:
			double x;
			double y;

			Point():x(0),y(0){}
			Point(double x, double y):x(x),y(y){}

			bool IsOrigin() const
			{
				return ( x == 0.0 && y == 0.0);
			}
	};
	
	class Transform
	{
		public:
			class MVector : public std::vector<double>
			{
				public:
					MVector& operator<< (const double& v)
					{
						this->push_back(v);
						return (*this);
					}
			};
			class Matrix : public std::vector<MVector>
			{
				public:
					Matrix()
					{
						MVector m1;
						MVector m2;
						MVector m3;

						m1 << 1 << 0 << 0;
						m2 << 0 << 1 << 0;
						m3 << 0 << 0 << 1;

						(*this) << m1 << m2 << m3;
					}

					double& m(unsigned int row, unsigned int col)
					{
						// idea is to reflect usual notation m11 m12 m21 ...
						return (*this)[row-1][col-1];
					}

					const double& m(unsigned int row, unsigned int col) const
					{
						return this->at(row-1).at(col-1);
					}

					Matrix& operator<< (const MVector& v)
					{
						this->push_back(v);
						return (*this);
					}

					const Matrix& operator *= (const Matrix &o)
					{
						Matrix product;

						for (int x(1); x<4; ++x)
							for (int y(1); y<4; ++y)
							{
								double sum = 0;
								for (int z(1); z<4; ++z)
									sum += m(x,z) * o.m(z,y);
								product.m(x,y) = sum;
							}

						(*this) = product;
						return (*this);
					}

					std::string toString() const
					{
						std::ostringstream buffer;
						buffer.precision(5);
						buffer << std::fixed
						       << m(1,1) << ' '
						       << m(1,2) << ' '
						       << m(2,1) << ' '
						       << m(2,2) << ' '
						       << m(3,1) << ' '
						       << m(3,2);
						return buffer.str();
					}
			};

			Transform(){}
			Transform(double a, double  b, double c,double d,double e,double f)
			{
				m.m(1,1) = a;
				m.m(1,2) = b;
				m.m(2,1) = c;
				m.m(2,2) = d;
				m.m(3,1) = e;
				m.m(3,2) = f	;

			}

			Transform& operator=(const Transform& other)
			{
				m = other.m;
				return *this;
			}

			~Transform(){}

			// In recomanded order: translate, rotate, scale [PDFRef1.4v3 p143]
			Transform& translate(double dx, double dy);
			Transform& rotate(double r, const Point& origin = Point());
			Transform& scale(double sx, double sy);


			static Transform fromString(const std::string& tm);
			std::string toCMString() const;

		protected:
			Matrix m;

	};
	
} // namespace ospi

#endif // OSPI_TRANSFORM_H
