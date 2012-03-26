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


#include "ResourceCollection.h"

#include <algorithm>

namespace ospi {

	ResourceCollection* ResourceCollection::instance = 0;

	ResourceCollection::ResourceKey::ResourceKey(const Resource &r):
		Resource(r)
	{
	}

	bool ResourceCollection::ResourceKey::operator ==(const ResourceCollection::ResourceKey& rk) const
	{
		if(first == rk.first && second == rk.second)
			return true;
		return false;
	}

	bool ResourceCollection::ResourceKey::operator <(const ResourceCollection::ResourceKey& rk) const
	{
		if(first < rk.first)
			return true;
		else if(first == rk.first)
			return second < rk.second;
		return false;
	}

	bool ResourceCollection::ResourceKey::operator >(const ResourceCollection::ResourceKey& rk) const
	{
		if(first > rk.first)
			return true;
		else if(first == rk.first)
			return second > rk.second;
		return false;
	}
	
	ResourceCollection::ResourceCollection()
	{
	}

	ResourceCollection* ResourceCollection::that()
	{
		if(instance == 0)
			instance = new ResourceCollection;
		return instance;
	}

//	bool ResourceCollection::resExist(const Resource &r)
//	{
//		if(pResDict.find(r.first) == pResDict.end())
//			return false;
//		const ObjVect& ov(pResDict.find(r.first)->second);
//		if(std::find(ov.begin(), ov.end(), r.second) == ov.end())
//			return false;

//		return true;
//	}

	unsigned int ResourceCollection::Add(const Resource &sres, const Resource &tres)
	{
		ResourceKey key(sres);
		ControlMap& cm(that()->tDocReg[tres.first]);
		if(cm.find(key) == cm.end())
		{
			cm[key] = tres.second;
		}

		return 1;
	}

	bool ResourceCollection::Has(PoDoFo::PdfDocument *tdoc, const Resource& res)
	{
		ResourceKey key(res);
		ControlMap& cm(that()->tDocReg[tdoc]);
		return (cm.find(key) != cm.end());
	}

	PoDoFo::PdfObject * ResourceCollection::Get(PoDoFo::PdfDocument *tdoc, const Resource& res)
	{
		ResourceKey key(res);
		ControlMap& cm(that()->tDocReg[tdoc]);
		if(cm.find(key) != cm.end())
			return cm[key];
		throw PoDoFo::ePdfError_NoObject;
	}

	void ResourceCollection::BeginDoc(PoDoFo::PdfDocument *tdoc)
	{
		that()->tDocReg[tdoc].clear();
	}

//	const PoDoFo::PdfReference& ResourceCollection::ToDoc(PoDoFo::PdfDocument *tdoc, const Resource &res)
//	{
//		ResourceKey key(res);
//		ControlMap& cm(that()->tDocReg[tdoc]);

//		// If the object referenced by the res key has already been inserted in the target document, just return a reference to it;
//		if(cm.find(key) != cm.end())
//		{
//			PoDoFo::PdfObject * o = cm[key];
//			return o->Reference();
//		}

//		// otherwise, actually recreate this object in target document
//		PoDoFo::PdfObject * obj = tdoc->GetObjects()->CreateObject(*res.second);
//		cm[key] = obj;
//		return obj->Reference();
//	}


	
} // namespace ospi
