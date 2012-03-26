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


#include "SimplePlanReader.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <exception>

#define OSPI_MAX_RECORD_SIZE 1024

namespace ospi {
	
	SimplePlanReader::SimplePlanReader(const std::string &plan)
		:PlanReader(plan)
	{
	}

	void SimplePlanReader::readRecord(const std::string &rec)
	{
		std::stringstream stream(std::stringstream::in | std::stringstream::out);
		stream << rec;
		std::string sdoc;
		std::string tdoc;
		unsigned int spagenumber;
		unsigned int tpagenumber;
		double tpagewidth, tpageheight;
		double a,b,c,d,e,f;

		stream >> sdoc >> tdoc >> spagenumber >> tpagenumber >> a >> b >> c >> d >> e >> f;
		if(sdocuments.find(sdoc) == sdocuments.end())
		{
			PoDoFo::PdfMemDocument * d(new PoDoFo::PdfMemDocument(sdoc.c_str()));
			sdocuments[sdoc] = DocumentPtr(d);
		}
		DocumentPtr sdocptr(sdocuments[sdoc]);
		SourcePagePtr sp(new SourcePage(sdocptr, spagenumber));

	}

	int SimplePlanReader::Impose()
	{
		std::ifstream in ( plan.c_str(), std::ifstream::in );
		if ( !in.good() )
			throw std::runtime_error ( "Failed to open plan file" );
		char cbuffer[MAX_RECORD_SIZE];
		int blen (0);
		do
		{
			in.getline ( cbuffer, MAX_RECORD_SIZE );
			blen = in.gcount() ;
			std::string buffer ( cbuffer, blen );
			readRecord(buffer);
		}
		while(!in.eof());
	}
	
} // namespace ospi
