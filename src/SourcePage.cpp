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
#include <boost/lexical_cast.hpp>

#ifndef WITHOUT_BOOST_UUID
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#else
namespace ospi {
// poor man's id
	class PoorManID
	{
		private:
			static unsigned int pID;
		public:
			unsigned int operator()()
			{
				return (++pID);
			}
	};
	unsigned int PoorManID::pID = 0x0;
}
#endif

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <set>
#include <ostream>

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

	SourcePage::SourcePage()
	{
		std::string objname("RPage");
#ifndef WITHOUT_BOOST_UUID
		objname.append( boost::lexical_cast<std::string>(boost::uuids::random_generator()()));
#else
		objname.append( boost::lexical_cast<std::string>(PoorManID()()));
#endif
		rName = objname;

		std::cerr<<"SourcePage: "<< rName << std::endl;
	}

	SourcePage& SourcePage::operator=(const SourcePage& other)
	{
		rName = other.rName;
		sourcePageNr = other.sourcePageNr;
		sourceDoc = other.sourceDoc;
		targetDoc = other.targetDoc;
		targetPageNr = other.targetPageNr;
		targetTransforms = other.targetTransforms;
		xobj = other.xobj;
		return (*this);
	}

	SourcePage::PdfResource SourcePage::getNamedResource(const PoDoFo::PdfName& rname) const
	{
		PoDoFo::PdfPage * sourcePage(sourceDoc->GetPage(sourcePageNr));
		if(!sourcePage)
			throw std::runtime_error("[SourcePage::getNamedResource] No page set");
		// First lookup in page resource dictionary;
		if(sourcePage->GetResources())
		{
			PoDoFo::PdfDictionary&  pres(sourcePage->GetResources()->GetDictionary());
			BOOST_FOREACH(PoDoFo::PdfName t, resTypes)
			{
				if(pres.HasKey(t))
				{

					PoDoFo::PdfObject * o(pres.GetKey(t));
					if(o->IsReference())
						o = sourceDoc->GetObjects()->GetObject(o->GetReference());
					PoDoFo::PdfDictionary& tdict(o->GetDictionary());
					if(tdict.HasKey(rname))
						return PdfResource(t, tdict.GetKey(rname));
				}
			}

		}

		// Then lookup parents resource dictionaries
		PoDoFo::PdfObject *rparent = sourcePage->GetObject()->GetDictionary().GetKey("Parent"); // parent is required for not root objects
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
//		//std::cerr<<errMessage<<std::endl;
	}

	bool SourcePage::migratable(const PoDoFo::PdfObject *o) const
	{
		if(o->IsDictionary() || o->IsArray() || o->IsReference())
			return true;
		return false;
	}

	PoDoFo::PdfObject * SourcePage::migrate(PoDoFo::PdfObject *obj)
	{
		PoDoFo::PdfObject *ret ( 0 );

		if ( obj->IsDictionary() )
		{
			PoDoFo::PdfObject * o(targetDoc->GetObjects()->CreateObject(obj->GetDataTypeString()));
			o->GetDictionary().RemoveKey(PoDoFo::PdfName::KeyType);

			PoDoFo::TKeyMap resmap = obj->GetDictionary().GetKeys();
			for ( PoDoFo::TCIKeyMap itres = resmap.begin(); itres != resmap.end(); ++itres )
			{
				//std::cerr<<"Migrate "<<itres->first.GetName()<<std::endl;
				if(migratable(itres->second))
					o->GetDictionary().AddKey ( itres->first , migrate( itres->second ));
				else
					o->GetDictionary().AddKey (itres->first , itres->second);
			}

			if ( obj->HasStream() )
			{
				PoDoFo::PdfStream * tstream(o->GetStream());
				PoDoFo::PdfStream * sstream(obj->GetStream());
				*tstream = *sstream;
				ret = targetDoc->GetObjects()->CreateObject(o->Reference());
			}
			else
				ret = o;

		}
		else if ( obj->IsArray() )
		{
			PoDoFo::PdfArray carray ( obj->GetArray() );
			PoDoFo::PdfArray narray;
			for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
			{
				if(migratable(&carray[ci]))
				{
					PoDoFo::PdfObject * o(migrate(&carray[ci]));
					narray.push_back(*o);
				}
				else
					narray.push_back(PoDoFo::PdfVariant(carray[ci]));
			}
			ret = targetDoc->GetObjects()->CreateObject(narray);
		}
		else if ( obj->IsReference() )
		{
			ResourceCollection::Resource sres(sourceDoc, obj);
			if(ResourceCollection::Has(targetDoc, sres))
			{
				PoDoFo::PdfObject * o(ResourceCollection::Get(targetDoc, sres));
				ret = o;
			}
			else
			{
				PoDoFo::PdfObject * o (migrate(sourceDoc->GetObjects()->GetObject(obj->GetReference())));
				ResourceCollection::Add(sres, ResourceCollection::Resource(targetDoc, o));
				ret = o;
			}
		}
		else
		{
			ret = new PoDoFo::PdfObject(*obj);
		}
		return ret;
	}

	void SourcePage::writeResource(const PoDoFo::PdfName& rname, const PdfResource &r)
	{
		//std::cerr<<"\n+++++++++++++++++++++++++++++++++++++++++++++++\nwriteResource: " << rname.GetName() <<std::endl;
		if(!xobj)
			throw std::logic_error("[SourcePage::writeResource] No XObject");


		PoDoFo::PdfObject* rdict = NULL;
		PoDoFo::PdfObject * tdict = NULL;

		if(!xobj->GetObject()->GetDictionary().HasKey(PoDoFo::PdfName("Resources")))
		{
//			rdict = new PoDoFo::PdfObject( PoDoFo::PdfDictionary() );
			xobj->GetObject()->GetDictionary().AddKey(PoDoFo::PdfName("Resources"), PoDoFo::PdfObject( PoDoFo::PdfDictionary() ));
		}
		rdict = xobj->GetObject()->GetDictionary().GetKey(PoDoFo::PdfName("Resources"));

		if(!rdict->GetDictionary().HasKey(r.first))
		{
//			tdict = new PoDoFo::PdfObject( PoDoFo::PdfDictionary() );
			rdict->GetDictionary().AddKey(r.first, PoDoFo::PdfObject( PoDoFo::PdfDictionary() ));
		}
		tdict = rdict->GetDictionary().GetKey(r.first);

		PoDoFo::PdfObject * mr(migrate(r.second));
		tdict->GetDictionary().AddKey(rname, mr);

//		tdict->GetDictionary().SetDirty(true);
//		rdict->GetDictionary().SetDirty(true);
//		xobj->GetObject()->GetDictionary().SetDirty(true);

//		std::string dbg;
//		tdict->ToString(dbg);
		//std::cerr<<dbg<<"\n+++++++++++++++++++++++++++++++++++++++++++++++\n"<<std::endl;
	}

	void SourcePage::extractResource()
	{
		PoDoFo::PdfPage * sourcePage(sourceDoc->GetPage(sourcePageNr));
		PoDoFo::PdfObject * content (sourcePage->GetContents());
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
			PoDoFo::PdfArray carray ( sourcePage->GetContents()->GetArray() );
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
							std::string dbg;

//							xobj->GetObject()->ToString(dbg);
//							//std::cerr<<"BEFORE =================================================================================================================="<<std::endl;
//							//std::cerr<<dbg<<std::endl;

							PdfResource res(getNamedResource(curName));
							writeResource(curName, res);

//							xobj->GetObject()->ToString(dbg);
//							//std::cerr<<"AFTER ==================================================================================================================="<<std::endl;
//							//std::cerr<<dbg<<std::endl;

						}
						catch(std::runtime_error & e)
						{
							//std::cerr<<e.what()<<std::endl;
						}

//						//std::cerr<< res.first.GetName()<< " => " <<var.GetName().GetName()<<std::endl;
					}
				}
			}
			var.Clear();
		}

		delete tokenizer;

	}

	void SourcePage::commit()
	{
		PoDoFo::PdfPage * sourcePage(sourceDoc->GetPage(sourcePageNr));
		PoDoFo::PdfPage * targetPage(targetDoc->GetPage(targetPageNr));
		if(!sourceDoc)
			throw std::runtime_error("[SourcePage::commit] No source document");
		else if(!sourcePage)
			throw std::runtime_error("[SourcePage::commit] No source page");
		else if(!targetDoc)
			throw std::runtime_error("[SourcePage::commit] No target document");
		else if(!targetPage)
			throw std::runtime_error("[SourcePage::commit] No target page");

		xobj = new PoDoFo::PdfXObject ( sourcePage->GetMediaBox(), targetDoc );
		PoDoFo::PdfVariant bbox;
		if(cropBox.GetBottom() != 0
				|| cropBox.GetHeight() != 0
				|| cropBox.GetLeft() != 0
				|| cropBox.GetWidth() != 0)
		{
			cropBox.ToVariant(bbox);
		}
		else
			sourcePage->GetBleedBox().ToVariant(bbox);

		xobj->GetObject()->GetDictionary().AddKey(PoDoFo::PdfName("BBox"), PoDoFo::PdfObject(bbox));

		PoDoFo::PdfMemoryOutputStream outMemStream ( 1 );
		if ( sourcePage->GetContents()->HasStream() )
		{
			sourcePage->GetContents()->GetStream()->GetFilteredCopy ( &outMemStream );
		}
		else if ( sourcePage->GetContents()->IsArray() )
		{
			PoDoFo::PdfArray carray ( sourcePage->GetContents()->GetArray() );
			for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
			{
				if ( carray[ci].HasStream() )
				{
					carray[ci].GetStream()->GetFilteredCopy ( &outMemStream );
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
			if ( sourcePage->GetObject()->GetDictionary().HasKey ( keyname ) )
			{
				xobj->GetObject()->GetDictionary().AddKey ( keyname, migrate( sourcePage->GetObject()->GetDictionary().GetKey ( keyname ) ) );
			}
		}

		outMemStream.Close();

		boost::shared_ptr<const char> spBuffer(outMemStream.TakeBuffer(), PoDoFo::podofo_free);
		PoDoFo::PdfMemoryInputStream inStream( spBuffer.get(), outMemStream.GetLength() );
		xobj->GetContents()->GetStream()->Set( &inStream );
		extractResource();

		// Now put this object in target doc

		std::ostringstream buffer;
		BOOST_FOREACH(Transform& trx, targetTransforms)
		{
			buffer << "q\n";
			buffer << trx.toCMString()<<"\n";
			buffer << "/" << rName << " Do\n";
			buffer << "Q\n";
		}
		std::string bufStr = buffer.str();
		PoDoFo::PdfObject* pageObject(targetPage->GetContentsForAppending());
		PoDoFo::PdfStream * contentstream(pageObject->GetStream());
		contentstream->BeginAppend(false);
		contentstream->Append(bufStr.data(), bufStr.size() );
		contentstream->EndAppend();
		if(!targetPage->GetResources()->GetDictionary().HasKey(PoDoFo::PdfName("XObject")))
		{
			PoDoFo::PdfObject to = PoDoFo::PdfObject(PoDoFo::PdfDictionary());
			targetPage->GetResources()->GetDictionary().AddKey(PoDoFo::PdfName("XObject"), to);
		}
		targetPage->GetResources()->GetDictionary().GetKey(PoDoFo::PdfName("XObject"))->GetDictionary().AddKey(PoDoFo::PdfName(rName), xobj->GetObjectReference());

//		std::cerr<<"Appended xobject: /"<< objname << "\tT: "<< targetTransform.toCMString() <<std::endl;

	}
	
} // namespace ospi
