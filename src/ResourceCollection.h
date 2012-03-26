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


#ifndef OSPI_RESOURCECOLLECTION_H
#define OSPI_RESOURCECOLLECTION_H

#include <podofo/podofo.h>

#include <map>
#include <vector>


namespace ospi {
	
	/**
	  An intermediate interface to manage resource streams
	  */
	class ResourceCollection
	{
		public:
			typedef std::pair<PoDoFo::PdfDocument*, PoDoFo::PdfObject*> Resource;
			typedef std::vector<PoDoFo::PdfObject*> ObjVect;
			typedef std::map<PoDoFo::PdfDocument*, ObjVect> ResDict;

			class ResourceKey : public Resource
			{
				public:
					ResourceKey(const Resource& r);
					bool operator==(const ResourceKey& rk) const;
					bool operator<(const ResourceKey& rk) const;
					bool operator>(const ResourceKey& rk) const;
			};

			typedef std::map<ResourceKey, PoDoFo::PdfObject*> ControlMap;

		protected:
			ResourceCollection();
			static ResourceCollection* instance;
			static ResourceCollection* that();

//			ResDict pResDict;
			std::map<PoDoFo::PdfDocument*, ControlMap> tDocReg;

//			bool resExist(const Resource& r);

		public:
			/*
			  Insert an object to the collection.
			  will possibly return an identifier for this resource
			*/
			static unsigned int Add(const Resource& res, const Resource &tres);
			static bool Has(PoDoFo::PdfDocument *tdoc, const Resource& res);
			static PoDoFo::PdfObject *Get(PoDoFo::PdfDocument *tdoc, const Resource &res);

			static void BeginDoc(PoDoFo::PdfDocument* tdoc);
//			static void EndDoc(PoDoFo::PdfDocument* tdoc);
//			static const PoDoFo::PdfReference &ToDoc(PoDoFo::PdfDocument* tdoc, const Resource& res);
	};
	
} // namespace ospi

#endif // OSPI_RESOURCECOLLECTION_H
