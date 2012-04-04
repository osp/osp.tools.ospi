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
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/foreach.hpp>

#define OSPI_MAX_RECORD_SIZE 1024

namespace ospi {

	const std::string SimplePlanReader::InputKW = std::string("INPUT");
	const std::string SimplePlanReader::OutputKW = std::string("OUTPUT");

	SimplePlanReader::SimplePlanReader(const std::string& plan, const PlanParams& params)
		:planPath(plan), params(params)
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

		stream >> sdoc >> tdoc >> spagenumber >> tpagenumber >> tpagewidth >> tpageheight >> a >> b >> c >> d >> e >> f;
		if(params.Has(InputKW))
		{
			if(sdoc == InputKW)
				sdoc = params.Get(InputKW, std::string());
		}
		if(params.Has(OutputKW))
		{
			if(tdoc == OutputKW)
				tdoc = params.Get(OutputKW, std::string());
		}
		if(sdocuments.find(sdoc) == sdocuments.end())
		{
			PoDoFo::PdfMemDocument * d(new PoDoFo::PdfMemDocument(sdoc.c_str()));
			sdocuments[sdoc] = DocumentPtr(d);
		}
		DocumentPtr sdocptr(sdocuments[sdoc]);
		SourcePagePtr sp(new SourcePage(sdocptr.get(), spagenumber));

		if(tdocuments.find(tdoc) == tdocuments.end())
		{
			PoDoFo::PdfMemDocument * d(new PoDoFo::PdfMemDocument);
			tdocuments[tdoc] = DocumentPtr(d);
		}
		DocumentPtr tdocptr(tdocuments[tdoc]);
		if(tdocptr->GetPageCount() <= tpagenumber)
		{
			if(tdocptr->GetPageCount() == tpagenumber)
				tdocptr->CreatePage(PoDoFo::PdfRect(0,0,tpagewidth,tpageheight));
			else
				throw std::logic_error("Would need to create empty pages without knowing their geometry");
		}
		PoDoFo::PdfPage * tpage(tdocptr->GetPage(tpagenumber));
		sp->setDoc(tdocptr.get());
		sp->addTransform(Transform(a,b,c,d,e,f));
		sp->setPage(tpage);

		spages.push_back(sp);
	}

	int SimplePlanReader::Impose()
	{
		std::ifstream in ( planPath.c_str(), std::ifstream::in );
		if ( !in.good() )
			throw std::runtime_error ( "Failed to open plan file" );
		char cbuffer[OSPI_MAX_RECORD_SIZE];
		int blen (0);
		do
		{
			in.getline ( cbuffer, OSPI_MAX_RECORD_SIZE );
			blen = in.gcount() ;
			std::string buffer ( cbuffer, blen );
			readRecord(buffer);
		}
		while(!in.eof());

		// Results
		std::cerr<<"Documents: "<<tdocuments.size()<<std::endl;
		std::cerr<<"Pages:" <<spages.size()<<std::endl;
		BOOST_FOREACH(SourcePagePtr spp, spages)
		{
			spp->commit();
		}
		BOOST_FOREACH(DocInfoKey k, tdocuments)
		{
			k.second->SetWriteMode(PoDoFo::ePdfWriteMode_Clean);
			k.second->Write(k.first.c_str());
		}

	}
	
} // namespace ospi
