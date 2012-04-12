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


#ifndef OSPI_READERPYTHON_H
#define OSPI_READERPYTHON_H

#include "PlanReader.h"

#include <boost/python.hpp>

namespace ospi {
	
	class ReaderPython : public PlanReader
	{
		protected:
			const std::string plan;
			const PlanParams params;

			std::vector<SourcePagePtr> spages;
			std::map<std::string, DocumentPtr> sdocuments;
			std::map<std::string, DocumentPtr> tdocuments;

			void readPage(const boost::python::dict& page);
			void readRecord(const boost::python::dict& record, DocumentPtr tdoc, int tpidx);

			std::string parse_python_exception();
		public:
			ReaderPython(const std::string& plan, const PlanParams& params);
			int Impose();
	};

	class ReaderPythonCreator : public PlanReaderFactory::Creator
	{
		public:
			PlanReaderPtr Create(const std::string& plan, const PlanParams& params)
			{
				return PlanReaderPtr(new ReaderPython(plan,params));
			}
	};
	
} // namespace ospi

#endif // OSPI_READERPYTHON_H
