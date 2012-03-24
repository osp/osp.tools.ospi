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


#include "SourcePage.h"
#include "ResourceCollection.h"

#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <set>

namespace ospi {
	
	const std::vector<PoDoFo::PdfName> SourcePage::resTypes = boost::assign::list_of(PoDoFo::PdfName("ExtGState"))
			(PoDoFo::PdfName("ColorSpace"))
			(PoDoFo::PdfName("Pattern"))
			(PoDoFo::PdfName("Shading"))
			(PoDoFo::PdfName("XObject"))
			(PoDoFo::PdfName("Font"))
//			(PoDoFo::PdfName("ProcSet"))
//			(PoDoFo::PdfName("Properties"))
			;

	SourcePage::SourcePage(PoDoFo::PdfDocument* doc, unsigned int pageNumber)
		:
		  sourceDoc(doc),
		  sourcePage(pageNumber),
		  pCachedPage(NULL)
	{
	}

	SourcePage& SourcePage::operator=(const SourcePage& other)
	{
		pCachedPage = NULL;
		sourceDoc = other.sourceDoc;
		sourcePage = other.sourcePage;
		targetDoc = other.targetDoc;
		targetPage = other.targetPage;
		resourceIndex = other.resourceIndex;
		return *this;
	}

	SourcePage::PdfResource SourcePage::getNamedResource(const PoDoFo::PdfName& rname) const
	{
		if(!pCachedPage)
			throw std::runtime_error("[SourcePage::getNamedResource] No page set");
		// First lookup in page resource dictionary;
		if(pCachedPage->GetResources())
		{
			PoDoFo::PdfDictionary&  pres(pCachedPage->GetResources()->GetDictionary());
			BOOST_FOREACH(PoDoFo::PdfName t, resTypes)
			{
				if(pres.HasKey(t))
				{
					PoDoFo::PdfDictionary& tdict(pres.GetKey(t)->GetDictionary());
					if(tdict.HasKey(rname))
						return PdfResource(t, tdict.GetKey(rname));
				}
			}

		}

		// Then lookup parents resource dictionaries
		PoDoFo::PdfObject *rparent = pCachedPage->GetObject()->GetDictionary().GetKey("Parent"); // parent is required for not root objects
		while ( rparent && rparent->IsDictionary() )
		{
			if(rparent->GetDictionary().HasKey("Resources"))
			{
				PoDoFo::PdfDictionary&  pres(rparent->GetDictionary().GetKey("Resources")->GetDictionary());
				BOOST_FOREACH(PoDoFo::PdfName t, resTypes)
				{
					if(pres.HasKey(t))
					{
						PoDoFo::PdfDictionary& tdict(pres.GetKey(t)->GetDictionary());
						if(tdict.HasKey(rname))
							return PdfResource(t, tdict.GetKey(rname));
					}
				}
			}
			if(rparent->GetDictionary().HasKey("Parent"))
				rparent = rparent->GetDictionary().GetKey("Parent");
			else
				rparent = NULL;
		}
		std::string errMessage("[SourcePage::getNamedResource] Missing named resource : /");
		errMessage.append(rname.GetName());
		 throw std::runtime_error(errMessage);
//		std::cerr<<errMessage<<std::endl;
	}

	PoDoFo::PdfObject * SourcePage::migrate(PoDoFo::PdfObject *obj)
	{
		PoDoFo::PdfObject *ret ( 0 );

		if ( obj->IsDictionary() )
		{
			ret = targetDoc->GetObjects()->CreateObject ( *obj );

			PoDoFo::TKeyMap resmap = obj->GetDictionary().GetKeys();
			for ( PoDoFo::TCIKeyMap itres = resmap.begin(); itres != resmap.end(); ++itres )
			{
				PoDoFo::PdfObject *o = itres->second;
				ret->GetDictionary().AddKey ( itres->first , migrate( o ));
			}

			if ( obj->HasStream() )
			{
				* ( ret->GetStream() ) = * ( obj->GetStream() );
			}

		}
		else if ( obj->IsArray() )
		{
			PoDoFo::PdfArray carray ( obj->GetArray() );
			PoDoFo::PdfArray narray;
			for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
			{
				PoDoFo::PdfObject *co ( migrate( &carray[ci] ) );
				narray.push_back ( *co );
			}
			ret = targetDoc->GetObjects()->CreateObject ( narray );
		}
		else if ( obj->IsReference() )
		{
//			PdfObject * o ( migrateResource ( sourceDoc->GetObjects().GetObject ( obj->GetReference() ) ) );
//                        ret  = new PdfObject ( o->Reference() ) ;
			PoDoFo::PdfReference rr( ResourceCollection::ToDoc(targetDoc, ResourceCollection::Resource(sourceDoc, obj)));
			ret = new PoDoFo::PdfObject ( rr );
		}
		else
		{
			ret = new PoDoFo::PdfObject ( *obj );//targetDoc->GetObjects().CreateObject(*obj);
		}
		return ret;
	}

	void SourcePage::writeResource(const PoDoFo::PdfName& rname, const PdfResource &r)
	{
		if(!xobj)
			throw std::logic_error("[SourcePage::writeResource] No XObject");
		PoDoFo::PdfDictionary& rdict(xobj->GetResources()->GetDictionary());
		PoDoFo::PdfObject * tdict = NULL;

		if(!rdict.HasKey(r.first))
		{
			tdict = new PoDoFo::PdfObject( PoDoFo::PdfDictionary() );
			rdict.AddKey(r.first, tdict);
		}
		else
			tdict = rdict.GetKey(r.first);

		tdict->GetDictionary().AddKey(rname, migrate(r.second));
	}

	void SourcePage::extractResource()
	{
		PoDoFo::PdfObject * content (pCachedPage->GetContents());
		PoDoFo::PdfMemoryOutputStream bufferStream ( 1 );

		if(content->HasStream())
		{
			const PoDoFo::PdfStream * const stream = content->GetStream();
			try
			{
				stream->GetFilteredCopy ( &bufferStream );
			}
			catch ( PoDoFo::PdfError & e )
			{
				return;
			}
		}
		else if(content->IsArray())
		{
			PoDoFo::PdfArray carray ( pCachedPage->GetContents()->GetArray() );
			for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
			{
				if ( carray[ci].HasStream() )
				{
					try
					{
						carray[ci].GetStream()->GetFilteredCopy ( &bufferStream );
					}
					catch ( PoDoFo::PdfError & e )
					{
						return;
					}

				}
				else if ( carray[ci].IsReference() )
				{
					PoDoFo::PdfObject *co = sourceDoc->GetObjects()->GetObject ( carray[ci].GetReference() );

					while ( co != NULL )
					{
						if ( co->IsReference() )
						{
							co = sourceDoc->GetObjects()->GetObject ( co->GetReference() );
						}
						else if ( co->HasStream() )
						{
							try
							{
								co->GetStream()->GetFilteredCopy ( &bufferStream );
							}
							catch ( PoDoFo::PdfError & e )
							{
								return;
							}
							break;
						}
					}

				}

			}
		}

		bufferStream.Close();

		boost::shared_ptr<const char> spBuffer(bufferStream.TakeBuffer(), PoDoFo::podofo_free);
		PoDoFo::PdfContentsTokenizer * tokenizer = new PoDoFo::PdfContentsTokenizer ( spBuffer.get(), bufferStream.GetLength() );
		const char*      pszToken = NULL;
		PoDoFo::PdfVariant       var;
		PoDoFo::EPdfContentsType eType;
		std::set<PoDoFo::PdfName> foundNames;
		PoDoFo::PdfName curName;
		while ( tokenizer->ReadNext ( eType, pszToken, var ) )
		{
			if ( eType == PoDoFo::ePdfContentsType_Variant )
			{
				if(var.IsName())
				{
					curName = var.GetName();
					if(foundNames.find(curName) == foundNames.end())
					{
						foundNames.insert(curName);
						try
						{
							PdfResource res(getNamedResource(curName));
							writeResource(curName, res);

						}
						catch(std::runtime_error & e)
						{
							std::cerr<<e.what()<<std::endl;
						}

//						std::cerr<< res.first.GetName()<< " => " <<var.GetName().GetName()<<std::endl;
					}
				}
			}
			var.Clear();
		}

		delete tokenizer;

	}

	void SourcePage::commit()
	{
		if(!sourceDoc)
			throw std::runtime_error("[SourcePage::extractResource] No document set");
		int sdgpc(sourceDoc->GetPageCount());
		if((sourcePage < 0) ||  (sourcePage > sdgpc))
			throw std::runtime_error("[SourcePage::extractResource] Invalid page number");

		pCachedPage = sourceDoc->GetPage(sourcePage);

		xobj = new PoDoFo::PdfXObject ( pCachedPage->GetMediaBox(), targetDoc );
		PoDoFo::PdfMemoryOutputStream outMemStream ( 1 );
		if ( pCachedPage->GetContents()->HasStream() )
		{
			pCachedPage->GetContents()->GetStream()->GetFilteredCopy ( &outMemStream );
		}
		else if ( pCachedPage->GetContents()->IsArray() )
		{
			PoDoFo::PdfArray carray ( pCachedPage->GetContents()->GetArray() );
			for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
			{
				if ( carray[ci].HasStream() )
				{
					carray[ci].GetStream()->GetFilteredCopy ( &outMemStream );
				}
				else if ( carray[ci].IsReference() )
				{
					PoDoFo::PdfObject *co = sourceDoc->GetObjects().GetObject ( carray[ci].GetReference() );

					while ( co != NULL )
					{
						if ( co->IsReference() )
						{
							co = sourceDoc->GetObjects().GetObject ( co->GetReference() );
						}
						else if ( co->HasStream() )
						{
							co->GetStream()->GetFilteredCopy ( &outMemStream );
							break;
						}
					}

				}

			}
		}

		std::vector<std::string> pageKeys;
		std::vector<std::string>::const_iterator itKey;
		pageKeys.push_back ( "Group" );
		for ( itKey = pageKeys.begin(); itKey != pageKeys.end(); ++itKey )
		{
			PoDoFo::PdfName keyname ( *itKey );
			if ( page->GetObject()->GetDictionary().HasKey ( keyname ) )
			{
				xobj->GetObject()->GetDictionary().AddKey ( keyname, migrateResource ( page->GetObject()->GetDictionary().GetKey ( keyname ) ) );
			}
		}

		outMemStream.Close();

		boost::shared_ptr<const char> spBuffer(outMemStream.TakeBuffer(), PoDoFo::podofo_free);
		PoDoFo::PdfMemoryInputStream inStream( spBuffer.get(), outMemStream.GetLength() );
		xobj->GetContents()->GetStream()->Set( &inStream );
		extractResource();

		// Now put this object in target doc
		PoDoFo::PdfPage * newpage;
		if(targetDoc->GetPageCount() < targetPage)
		{
			for(int i(targetDoc->GetPageCount()); i < targetPage; i++)
			{
				newpage = targetDoc->CreatePage ( PdfRect ( 0.0, 0.0, destWidth, destHeight ) );
				++lastPlate;
			}
		}
	}
	
} // namespace ospi
