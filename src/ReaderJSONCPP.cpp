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


#include "ReaderJSONCPP.h"

#include <fstream>
#include <stdexcept>
#include <cmath>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

//#define WITH_CURL
#ifdef WITH_CURL
extern "C"{
int ospi_curl_get_file(const char * url, const char * filename);
}
#endif


namespace ospi {

	const std::string ReaderJSONCPP::K_OutputRoot = std::string("output_root_name");
	const std::string ReaderJSONCPP::K_Plan = std::string("plan");
	const std::string ReaderJSONCPP::K_TargetWidth = std::string("page_width");
	const std::string ReaderJSONCPP::K_TargetHeight = std::string("page_height");
	const std::string ReaderJSONCPP::K_Slots = std::string("slots");
	const std::string ReaderJSONCPP::K_SlotWidth = std::string("width");
	const std::string ReaderJSONCPP::K_SlotHeight = std::string("height");
	const std::string ReaderJSONCPP::K_SlotLeft = std::string("left");
	const std::string ReaderJSONCPP::K_SlotTop = std::string("top");
	const std::string ReaderJSONCPP::K_SlotFile = std::string("file");
	const std::string ReaderJSONCPP::K_SlotRemoteFile = std::string("remote_file");
	const std::string ReaderJSONCPP::K_SlotRemoteFileURL = std::string("url");
	const std::string ReaderJSONCPP::K_SlotRemoteFileType = std::string("type");
	const std::string ReaderJSONCPP::K_SlotPage = std::string("page");
	const std::string ReaderJSONCPP::K_CropWidth = std::string("crop_width");
	const std::string ReaderJSONCPP::K_CropHeight = std::string("crop_height");
	const std::string ReaderJSONCPP::K_CropLeft = std::string("crop_left");
	const std::string ReaderJSONCPP::K_CropTop = std::string("crop_top");
	const std::string ReaderJSONCPP::K_Rotation = std::string("rotation");
	
	ReaderJSONCPP::ReaderJSONCPP(const std::string& plan, const PlanParams& params)
		:planPath(plan), params(params)
	{

	}

	void ReaderJSONCPP::readPage(const Json::Value &page, unsigned int tpidx)
	{
		double tpagewidth(0);
		double tpageheight(0);
		tpagewidth = page.get(K_TargetWidth, tpagewidth).asDouble();
		tpageheight = page.get(K_TargetHeight, tpageheight).asDouble();
		if(tpagewidth <= 0.0 || tpageheight <= 0.0)
			throw std::runtime_error("Invalid target page geometry (JSONCPP)");

		PoDoFo::PdfPage * pp(tdocument->CreatePage(PoDoFo::PdfRect(0,0,tpagewidth,tpageheight)));
		pp->GetContentsForAppending();

		const Json::Value slots_(page[K_Slots]);
		for (unsigned int index(0); index < slots_.size(); ++index )
		{
			Json::Value rec(slots_[index]);
			readSlot(rec, pp);
		}

	}

	void ReaderJSONCPP::readSlot(const Json::Value &slot, PoDoFo::PdfPage * tpage)
	{
		// here we are!
		std::string sdoc;
		unsigned int spagenumber;
		double left, top, width, height;
		double cleft, ctop, cwidth, cheight;
		double rotation;


		sdoc = slot.get(K_SlotFile, sdoc).asString();
		if(sdoc.empty())
		{
			Json::Value remote(slot.get(K_SlotRemoteFile, sdoc));
			std::string rsdocurl(remote.get(K_SlotRemoteFileURL, std::string()).asString());
			std::vector<std::string> urlvec;
			boost::algorithm::split( urlvec, rsdocurl, boost::algorithm::is_any_of("/"), boost::algorithm::token_compress_on );
			sdoc = urlvec.back();

		}
		boost::filesystem3::path fp(sdoc.c_str());
		if(!boost::filesystem3::exists(fp))
		{
#ifdef WITH_CURL
			// try to get it from internet
			Json::Value remote(slot.get(K_SlotRemoteFile, sdoc));
			std::string rsdocurl(remote.get(K_SlotRemoteFileURL, std::string()).asString());
			std::string rsdoctype(remote.get(K_SlotRemoteFileType, std::string("pdf")).asString());
			if(rsdoctype != std::string("pdf"))
				throw std::runtime_error("type of remote file is not PDF (JSONCPP)");
			if(ospi_curl_get_file(rsdocurl.c_str(), sdoc.c_str()) > 0)
				throw std::runtime_error("Can't fetch file from internet (JSONCPP/CURL)");
#else
			throw std::runtime_error("Can't find source PDF file (JSONCPP/CURL)");
#endif
		}


		spagenumber = slot.get(K_SlotPage, 1).asInt() - 1; // to discuss with json providers whether we go natural counting or not.
		if(sdocuments.find(sdoc) == sdocuments.end())
		{
			PoDoFo::PdfMemDocument * d(new PoDoFo::PdfMemDocument(sdoc.c_str()));
			sdocuments[sdoc] = DocumentPtr(d);
		}
		DocumentPtr sdocptr(sdocuments[sdoc]);
		SourcePagePtr sp(new SourcePage(sdocptr.get(), spagenumber));
		sp->setPage(tpage);
		sp->setDoc(tdocument.get());

		PoDoFo::PdfPage * sourcepage(sdocptr->GetPage(spagenumber));

		PoDoFo::PdfRect srect(sourcepage->GetMediaBox());
		left = slot.get(K_SlotLeft, 0).asDouble();
		top = slot.get(K_SlotTop, 0).asDouble();
		width = slot.get(K_SlotWidth,srect.GetWidth()).asDouble();
		height = slot.get(K_SlotHeight, srect.GetHeight()).asDouble();
		rotation = slot.get(K_Rotation, 0).asDouble();

		Transform t;
		std::cerr<<"========="<<std::endl;

		std::cerr<<"translate = "<<left<<" "<< (tpage->GetMediaBox().GetHeight() - (top + height)) <<std::endl;
		t.translate(left, tpage->GetMediaBox().GetHeight() - (top + height));
		std::cerr<<t.toCMString()<<std::endl;

		std::cerr<<"rotate = "<<(abs((rotation * 90.0) - 360.0))<<std::endl;
		t.rotate(abs((rotation * 90.0) - 360.0), ospi::Point(left, top));
		std::cerr<<t.toCMString()<<std::endl;

		t.scale(width / srect.GetWidth(), height / srect.GetHeight());
		std::cerr<<"scale = "<< (width / srect.GetWidth())<<" "<< (height / srect.GetHeight())<<std::endl;
		std::cerr<<t.toCMString()<<std::endl;

		sp->setTransform(t);

		PoDoFo::PdfRect crect(sourcepage->GetBleedBox());
		cleft = slot.get(K_CropLeft, 0).asDouble();
		ctop = slot.get(K_CropTop, 0).asDouble();
		cwidth = slot.get(K_CropWidth,crect.GetWidth()).asDouble();
		cheight = slot.get(K_CropHeight, crect.GetHeight()).asDouble();

		PoDoFo::PdfRect crop(cleft, srect.GetHeight() -( ctop + cheight), cwidth, cheight);
		sp->setCrop(crop);

		spages.push_back(sp);
	}

	int ReaderJSONCPP::Impose()
	{
		std::ifstream in ( planPath.c_str(), std::ifstream::in );
		if ( !in.good() )
			throw std::runtime_error ( "Failed to open plan file" );

		// How that's ugly
		in.seekg (0, std::ios::end);
		int length = in.tellg();
		in.seekg (0, std::ios::beg);
		boost::shared_ptr<char> buffer(new char [length]);
		in.read(buffer.get(), length);
		std::string jsondata(buffer.get(), length);

		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( jsondata, root );
		if ( !parsingSuccessful )
		{
			throw std::runtime_error(reader.getFormattedErrorMessages());
		}

		std::string outputName;
		outputName = root.get(K_OutputRoot,outputName).asString();
		if(outputName.empty())
		{
			if(params.Has(K_OutputRoot))
			{
				outputName = params.Get(K_OutputRoot, std::string());
			}
			if(outputName.empty())
				throw std::runtime_error("Can't get output_root_name (JSONCPP)");
		}

		PoDoFo::PdfMemDocument * tdoc(new PoDoFo::PdfMemDocument);
		tdocument = DocumentPtr(tdoc);


		const Json::Value plan(root[K_Plan]);
		for (unsigned int index(0); index < plan.size(); ++index )
		{
			Json::Value rec(plan[index]);
			readPage(rec, index);
		}

		BOOST_FOREACH(SourcePagePtr spp, spages)
		{
			spp->commit();
		}

		tdocument->SetWriteMode(PoDoFo::ePdfWriteMode_Clean);
		tdocument->Write(outputName.append(".pdf").c_str());

	}
	
} // namespace ospi
