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

namespace ospi {
	
	class Transform
	{
		protected:
			double a,b,c,d,e,f;

		public:
			Transform():
				a(1.0), b(0), c(0), d(1.0), e(0), f(0) {}
			Transform(double aa, double ab, double ac, double ad, double ae, double af):
				a(aa), b(ab), c(ac), d(ad), e(ae), f(af) {}
			Transform& operator=(const Transform& other)
			{
				a = other.a;
				b = other.b;
				c = other.c;
				d = other.d;
				e = other.e;
				f = other.f;

				return *this;
			}

			~Transform(){}

			// In recomanded order: translate, rotate, scale [PDFRef1.4v3 p143]
			Transform& translate(double dx, double dy);
			Transform& rotate(double r);
			Transform& scale(double sx, double sy);

			static Transform fromString(const std::string& tm);
			std::string toCMString() const;

	};
	
} // namespace ospi

#endif // OSPI_TRANSFORM_H
