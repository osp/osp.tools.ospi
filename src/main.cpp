/***************************************************************************
 *   Copyright (C) 2012 by Pierre Marchand   *
 *   pierremarc@oep-h.com   *
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

#include <podofo/podofo.h>
#include <iostream>
#include <map>
#include <boost/foreach.hpp>

#include "ResourceCollection.h"
#include "SourcePage.h"

typedef std::pair<PoDoFo::PdfName, PoDoFo::PdfObject*> dictP;


void a(PoDoFo::PdfDocument *doc, PoDoFo::PdfObject* o, int indent)
{
	if(o->IsDictionary())
	{
		std::cerr<<std::string(indent, '\t')<<"<<"<<std::endl;
		BOOST_FOREACH( dictP nv , o->GetDictionary().GetKeys())
		{
			std::cerr<<std::string(indent, '\t')<<nv.first.GetName()<<": "<<std::endl;
			a(doc, nv.second, indent + 1);
		}
		std::cerr<<std::string(indent, '\t')<<">>"<<std::endl;
	}
	else if(o->IsArray())
	{
		std::cerr<<std::string(indent, '\t')<<"["<<std::endl;
		BOOST_FOREACH( PoDoFo::PdfObject oo , o->GetArray())
		{
			a(doc, &oo, indent + 1);
		}
		std::cerr<<std::string(indent, '\t')<<"]"<<std::endl;
	}
	else if(o->IsName())
	{
		std::cerr<<std::string(indent, '\t')<<"/"<<o->GetName().GetName()<<std::endl;
	}
	else if(o->IsReference())
	{
		std::cerr<<std::string(indent, '\t')<<o->GetReference().ToString()<<std::endl;
		PoDoFo::PdfObject * r(doc->GetObjects()->GetObject(o->GetReference()));
		a(doc, r, indent + 1);
	}
	else
	{
		std::string s;
		o->ToString(s);
		std::cerr<<std::string(indent, '\t')<<"("<< o->GetDataTypeString() <<") "<< s << std::endl;
	}
}

int main(int ac, char ** av)
{
//	ospi::ResourceCollection rc;
	PoDoFo::PdfMemDocument doc(av[1]);
	int pc(doc.GetPageCount());


	for(int i(0); i < pc; ++i)
	{
//		PoDoFo::PdfPage* p(doc.GetPage(i));
//		PoDoFo::PdfObject* r(p->GetResources());
		std::cerr<<"Page "<<(i+1)<<"___________________________________________________________________________"<<std::endl;
		ospi::SourcePage sp(&doc, i);
		sp.extractResource();
//		a(&doc, r, 1);

	}
	return 0;
}

