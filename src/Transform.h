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


#ifndef WITH_GMP
typedef double trx_double_t;
#else
#include <gmpxx.h>
typedef mpf_class trx_double_t;
#endif

namespace ospi {

	class Point
	{
		public:
			trx_double_t x;
			trx_double_t y;

			Point():x(0),y(0){}
			Point(trx_double_t x, trx_double_t y):x(x),y(y){}
			Point& operator=(const Point& o)
			{
				this->x = o.x;
				this->y = o.y;
				return (*this);
			}

			bool IsOrigin() const
			{
				return ( x == trx_double_t(0.0) && y == trx_double_t(0.0));
			}
	};

	class Rectangle : public std::vector<Point>
	{
		protected:
			enum {
				BL = 0,
				TL = 1,
				TR = 2,
				BR = 3
			};


		public:
			Rectangle(){resize(4);}
			Rectangle(const Point& c1, const Point& c2)
			{
				resize(4);
				at(BL) = c1;
				at(TR) = c2;
				normalize();
			}

			Point bottomLeft() const {return at(BL);}
			Point topRight() const {return at(TR);}
			Point topLeft() const {return at(TL);}
			Point bottomRight() const {return at(BR);}

			trx_double_t width() const {return (at(BR).x - at(BL).x);}
			trx_double_t height() const {return(at(TL).y - at(BL).y);}

			void normalize()
			{
				Point c1(at(BL));
				Point c2(at(TR));
				if(c1.x <= c2.x)
				{
					if(c1.y <= c2.y)
					{
						at(BL) = c1;
						at(TR) = c2;
					}
					else
					{
						at(BL) = Point(c1.x, c2.y);
						at(TR) = Point(c2.x, c1.y);
					}
				}
				else
				{
					if(c1.y <= c2.y)
					{
						at(BL) = Point(c2.x, c1.y);
						at(TR) = Point(c1.x, c2.y);
					}
					else
					{
						at(BL) = c2;
						at(TR) = c1;
					}
				}

				at(TL) = Point(at(BL).x, at(TR).y);
				at(BR) = Point(at(TR).x, at(BL).y);
			}

	};
	
	class Transform
	{
		public:
			class MVector : public std::vector<trx_double_t>
			{
				public:
					MVector& operator<< (const trx_double_t& v)
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

					trx_double_t& m(unsigned int row, unsigned int col)
					{
						// idea is to reflect usual notation m11 m12 m21 ...
						return (*this)[row-1][col-1];
					}

					const trx_double_t& m(unsigned int row, unsigned int col) const
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
								trx_double_t sum = 0;
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
						buffer.precision(4);
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
			Transform(trx_double_t a, trx_double_t  b, trx_double_t c,trx_double_t d,trx_double_t e,trx_double_t f)
			{
				m.m(1,1) = a;
				m.m(1,2) = b;
				m.m(2,1) = c;
				m.m(2,2) = d;
				m.m(3,1) = e;
				m.m(3,2) = f;

			}

			Transform& operator=(const Transform& other)
			{
				m = other.m;
				return *this;
			}

			~Transform(){}

			// In recomanded order: translate, rotate, scale [PDFRef1.4v3 p143]
			Transform& translate(trx_double_t dx, trx_double_t dy);
			Transform& rotate(trx_double_t r, const Point& origin = Point());
			Transform& scale(trx_double_t sx, trx_double_t sy, const Point& origin = Point());


			void map(Point& p) const;
			void map(Rectangle& r) const;


			static Transform fromString(const std::string& tm);
			std::string toCMString() const;

		protected:
			Matrix m;

	};
	
} // namespace ospi

#endif // OSPI_TRANSFORM_H
