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
#include "PlanReader.h"


int main(int ac, char ** av)
{
//	ospi::ResourceCollection rc;
//	PoDoFo::PdfMemDocument targetDoc;

//	for(int d(1); d < ac; ++d)
//	{
//		PoDoFo::PdfMemDocument doc(av[d]);
//		int pc(doc.GetPageCount());
//		for(int i(0); i < pc; ++i)
//		{
//			PoDoFo::PdfPage* p(doc.GetPage(i));
//			std::cerr<<"Page ("<<d<<")"<<(i+1)<<std::endl;
//			ospi::SourcePage sp(&doc, i);
//			sp.setDoc(&targetDoc);
//			sp.setPage(targetDoc.CreatePage(p->GetMediaBox()));

//			sp.commit();
//		}
//	}
//	targetDoc.SetWriteMode(PoDoFo::ePdfWriteMode_Clean);
//	targetDoc.Write("test.pdf");

	std::string plan(av[1]);
	std::string reader(av[2]);

	ospi::PlanReaderFactory::Impose(reader, plan);

	return 0;
}

