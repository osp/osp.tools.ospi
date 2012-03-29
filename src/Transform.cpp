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


#include "Transform.h"

#include <iostream>
#include <sstream>
#include <cmath>

namespace ospi {

	Transform Transform::fromString(const std::string &tm)
	{
		// TODO - if found useful
		return Transform();
	}

	std::string Transform::toCMString() const
	{
		std::ostringstream buffer;
		buffer.precision(5);
		buffer << std::fixed
		       << m.m(1,1) << ' '
		       << m.m(1,2) << ' '
		       << m.m(2,1) << ' '
		       << m.m(2,2) << ' '
		       << m.m(3,1) << ' '
		       << m.m(3,2) << ' '
		       << "cm";
		return buffer.str();
	}

	Transform& Transform::translate(double dx, double dy)
	{
		Matrix transMat;
		transMat.m(3,1) = dx;
		transMat.m(3,2) = dy;
		m *= transMat;
		return (*this);
	}

	Transform& Transform::rotate(double r)
	{
		double rGrad(r * 3.14159 / 180.0);
		double cosR = cos(rGrad);
		double sinR = sin(rGrad);
		Matrix rotMat;
		rotMat.m(1,1) = cosR;
		rotMat.m(1,2) = -sinR;
		rotMat.m(2,1) = sinR;
		rotMat.m(2,2) = cosR;
		m *= rotMat;
		return (*this);

	}

	Transform& Transform::scale(double sx, double sy)
	{
		Matrix scaleMat;
		scaleMat.m(1,1) = sx;
		scaleMat.m(2,2) = sy;
		m *= scaleMat;
		return (*this);
	}

	
} // namespace ospi
