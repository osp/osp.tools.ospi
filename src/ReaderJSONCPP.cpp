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

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#ifndef WITHOUT_BOOST_FS3
#define BOOST_FILESYSTEM_VERSION 3
namespace filesystem = boost::filesystem3;
#else
namespace filesystem = boost::filesystem;
#endif




#include <boost/algorithm/string.hpp>

//#define WITH_CURL
#ifdef WITH_CURL
extern "C"{
int ospi_curl_get_file(const char * url, const char * filename);
}
#endif


namespace ospi {

	const std::string ReaderJSONCPP::K_OutputFile = std::string("output_file_name");
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
	const std::string ReaderJSONCPP::K_PageDimMod = std::string("page_dim_mod");
	const std::string ReaderJSONCPP::K_CropDimMod = std::string("crop_dim_mod");
	const std::string ReaderJSONCPP::K_SlotDimMod = std::string("slot_dim_mod");
	const std::string ReaderJSONCPP::V_DimModAbsolute = std::string("absolute");
	const std::string ReaderJSONCPP::V_DimModRelative = std::string("relative");
	const std::string ReaderJSONCPP::V_DimModPercent = std::string("percent");


	bool ReaderJSONCPP::SourcePage_Key::operator< (const ReaderJSONCPP::SourcePage_Key& o) const
	{
		if(doc < o.doc)
			return true;
		else if(doc == o.doc)
		{
			if(pnumber < o.pnumber)
				return true;
			else if(pnumber == o.pnumber)
			{
				if(bbox.ToString() < o.bbox.ToString())
					return true;
				else
					return false;
			}
			return false;
		}
		return false;
	}
	
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
		{
			Rectangle r;
			templatePage(page, r);
			tpagewidth = r.width();
			tpageheight = r.height();
			std::cerr<<"Computed target page: w = "<<tpagewidth<<" ; h = "<<tpageheight<<std::endl;
//			throw std::runtime_error("Invalid target page geometry (JSONCPP)");
		}

		tdocument->CreatePage(PoDoFo::PdfRect(0,0,tpagewidth,tpageheight));

		const Json::Value slots_(page[K_Slots]);
		std::map<SourcePage_Key,SourcePagePtr> pDict;
		for (unsigned int index(0); index < slots_.size(); ++index )
		{
			Json::Value rec(slots_[index]);
			readSlot(rec, tpidx, pDict);
		}

	}

	void ReaderJSONCPP::templatePage(const Json::Value &page, Rectangle &rect)
	{
		const Json::Value slots_(page[K_Slots]);
		for (unsigned int index(0); index < slots_.size(); ++index )
		{
			Json::Value slot(slots_[index]);
			std::string sdoc;
			unsigned int spagenumber;
			double left, top, width, height;
			double cleft, ctop, cwidth, cheight, cbottom;
			double rotation(slot.get(K_Rotation, 0).asDouble());
			std::string slotDimMod(slot.get(K_SlotDimMod, V_DimModAbsolute).asString());
			std::string cropDimMod(slot.get(K_CropDimMod, V_DimModAbsolute).asString());

			sdoc = slot.get(K_SlotFile, sdoc).asString();
			spagenumber = slot.get(K_SlotPage, 1).asInt() - 1;
			DocumentPtr sdocptr(new PoDoFo::PdfMemDocument(sdoc.c_str()));
			PoDoFo::PdfPage * sourcepage(sdocptr->GetPage(std::min(int(spagenumber), sdocptr->GetPageCount() - 1)));

			PoDoFo::PdfRect srect(sourcepage->GetMediaBox());
			PoDoFo::PdfRect crect(sourcepage->GetBleedBox());

			if(cropDimMod == V_DimModAbsolute)
			{
				cleft = slot.get(K_CropLeft,crect.GetLeft()).asDouble();
				ctop = slot.get(K_CropTop, srect.GetHeight() -(crect.GetBottom() + crect.GetHeight())).asDouble();
				cwidth = slot.get(K_CropWidth,crect.GetWidth()).asDouble();
				cheight = slot.get(K_CropHeight, crect.GetHeight()).asDouble();
			}
			else if(cropDimMod == V_DimModRelative)
			{
				cleft = crect.GetLeft() + slot.get(K_CropLeft, 0).asDouble();
				ctop = (srect.GetHeight() - (crect.GetBottom() + crect.GetHeight())) + slot.get(K_CropTop, 0).asDouble();
				cwidth = crect.GetWidth() + slot.get(K_CropWidth,0).asDouble();
				cheight = crect.GetHeight() + slot.get(K_CropHeight, 0).asDouble();
			}
			else if(cropDimMod == V_DimModPercent)
			{
				cleft = srect.GetLeft() + (srect.GetWidth() * (slot.get(K_CropLeft,100.0).asDouble() / 100.0));
				ctop = (srect.GetBottom() + srect.GetHeight()) - (srect.GetHeight() * (slot.get(K_CropTop, 100.0).asDouble() / 100.0));
				cwidth =  srect.GetWidth() * (slot.get(K_CropWidth,100.0).asDouble() / 100.0);
				cheight = srect.GetHeight() * (slot.get(K_CropHeight, 100.0).asDouble() / 100.0);
			}

			left = slot.get(K_SlotLeft, 0).asDouble();
			top = slot.get(K_SlotTop, 0).asDouble();
			if(slotDimMod == V_DimModAbsolute)
			{
				width = slot.get(K_SlotWidth,cwidth).asDouble();
				height = slot.get(K_SlotHeight, cheight).asDouble();
			}
			else if(slotDimMod == V_DimModRelative)
			{
				width = cwidth + slot.get(K_SlotWidth,0).asDouble();
				height = cheight + slot.get(K_SlotHeight,0).asDouble();
			}
			else if(slotDimMod == V_DimModPercent)
			{
				width = cwidth *( slot.get(K_SlotWidth,100.0).asDouble() / 100.0);
				height = cheight *( slot.get(K_SlotHeight,100.0).asDouble() / 100.0);
			}

			if(rotation == 1 || rotation == 3)
			{
				double tmpW = width;
				width = height;
				height = tmpW;
			}

			Rectangle slotRect(Point(left, top), Point(left+width, top+height));
			rect.united(slotRect);

		}
	}

	void ReaderJSONCPP::readSlot(const Json::Value &slot, unsigned int tpidx, std::map<ReaderJSONCPP::SourcePage_Key,SourcePagePtr>& pDict)
	{
		// here we are!
		std::string sdoc;
		unsigned int spagenumber;
		double left, top, width, height, bottom;
		double cleft, ctop, cwidth, cheight, cbottom;
		double rotation;
		PoDoFo::PdfPage * tpage(tdocument->GetPage(tpidx));
		std::string slotDimMod(slot.get(K_SlotDimMod, V_DimModAbsolute).asString());
		std::string cropDimMod(slot.get(K_CropDimMod, V_DimModAbsolute).asString());

		// IMPORTANT: the slot is already rotated. It does mean that width and height of the source document have to be compared to de-rotated slot dimensions

		// Source PDF file
		sdoc = slot.get(K_SlotFile, sdoc).asString();
//		bool fileIsRemote(false);
		if(sdoc.empty())
		{
			Json::Value remote(slot.get(K_SlotRemoteFile, sdoc));
			std::string rsdocurl(remote.get(K_SlotRemoteFileURL, std::string()).asString());
			std::vector<std::string> urlvec;
			boost::algorithm::split( urlvec, rsdocurl, boost::algorithm::is_any_of("/"), boost::algorithm::token_compress_on );
			bool first(true);
			BOOST_FOREACH(const std::string& urlpart, urlvec)
			{
				if(first)
					first = false;
				else
					sdoc.append(urlpart);
			}

//			sdoc = urlvec.back();
//			fileIsRemote = true;

		}

		// Source page
		spagenumber = slot.get(K_SlotPage, 1).asInt() - 1; // to discuss with json providers whether we go natural counting or not.

		std::string origineName(sdoc);
		origineName.append("#");
		origineName.append(boost::lexical_cast<std::string>(spagenumber));

		filesystem::path fp(sdoc.c_str());
		if(!filesystem::exists(fp) /*|| fileIsRemote*/)
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

		if(sdocuments.find(sdoc) == sdocuments.end())
		{
			DocumentPtr d(new PoDoFo::PdfMemDocument(sdoc.c_str()));
			sdocuments[sdoc] = d;
		}
		DocumentPtr sdocptr(sdocuments[sdoc]);

		if(sdocptr->GetPageCount() <= spagenumber)
		{
			spagenumber = sdocptr->GetPageCount() - 1;
			std::cerr<<"Try to get non-existing page, take the last page of the document"<<std::endl;
		}

		PoDoFo::PdfPage * sourcepage(sdocptr->GetPage(spagenumber));

		// GEOMETRY

		// NOTE: rotation point is the center of the cropped page
		rotation = slot.get(K_Rotation, 0).asDouble();

		PoDoFo::PdfRect targetPageRect(tpage->GetMediaBox());

		PoDoFo::PdfRect srect(sourcepage->GetMediaBox());
		PoDoFo::PdfRect crect(sourcepage->GetBleedBox());


		if(cropDimMod == V_DimModAbsolute)
		{
			cleft = slot.get(K_CropLeft,crect.GetLeft()).asDouble();
			ctop = slot.get(K_CropTop, srect.GetHeight() -(crect.GetBottom() + crect.GetHeight())).asDouble();
			cwidth = slot.get(K_CropWidth,crect.GetWidth()).asDouble();
			cheight = slot.get(K_CropHeight, crect.GetHeight()).asDouble();
		}
		else if(cropDimMod == V_DimModRelative)
		{
			cleft = crect.GetLeft() + slot.get(K_CropLeft, 0).asDouble();
			ctop = (srect.GetHeight() - (crect.GetBottom() + crect.GetHeight())) + slot.get(K_CropTop, 0).asDouble();
			cwidth = crect.GetWidth() + slot.get(K_CropWidth,0).asDouble();
			cheight = crect.GetHeight() + slot.get(K_CropHeight, 0).asDouble();
		}
		else if(cropDimMod == V_DimModPercent)
		{
			// here it makes it too much convoluted to use the given BleedBox, let's assume the effect is to crop to some percentage of the MediaBox
			cleft = srect.GetLeft() + (srect.GetWidth() * (slot.get(K_CropLeft,100.0).asDouble() / 100.0));
			ctop = (srect.GetBottom() + srect.GetHeight()) - (srect.GetHeight() * (slot.get(K_CropTop, 100.0).asDouble() / 100.0));
			cwidth =  srect.GetWidth() * (slot.get(K_CropWidth,100.0).asDouble() / 100.0);
			cheight = srect.GetHeight() * (slot.get(K_CropHeight, 100.0).asDouble() / 100.0);
		}
//		cbottom = srect.GetHeight() - (ctop + cheight);

		left = slot.get(K_SlotLeft, 0).asDouble();
		top = slot.get(K_SlotTop, 0).asDouble();
		if(rotation == 1 || rotation == 3)
		{
			if(slotDimMod == V_DimModAbsolute)
			{
				height = slot.get(K_SlotWidth, cwidth).asDouble();
				width = slot.get(K_SlotHeight, cheight).asDouble();
			}
			else if(slotDimMod == V_DimModRelative)
			{
				height = cwidth + slot.get(K_SlotWidth, 0).asDouble();
				width = cheight + slot.get(K_SlotHeight, 0).asDouble();
			}
			else if(slotDimMod == V_DimModPercent)
			{
				height = cwidth * (slot.get(K_SlotWidth, 100.0).asDouble() / 100.0);
				width = cheight * (slot.get(K_SlotHeight, 100.0).asDouble() / 100.0);
			}
		}
		else
		{
			if(slotDimMod == V_DimModAbsolute)
			{
				width = slot.get(K_SlotWidth, cwidth).asDouble();
				height = slot.get(K_SlotHeight, cheight).asDouble();
			}
			else if(slotDimMod == V_DimModRelative)
			{
				width = cwidth + slot.get(K_SlotWidth, 0).asDouble();
				height = cheight + slot.get(K_SlotHeight, 0).asDouble();
			}
			else if(slotDimMod == V_DimModPercent)
			{
				width = cwidth * (slot.get(K_SlotWidth, 100.0).asDouble() / 100.0);
				height = cheight * (slot.get(K_SlotHeight, 100.0).asDouble() / 100.0);
			}
		}
		bottom = targetPageRect.GetHeight() - (top + height);
		ospi::Point slotCenter(left + (width / 2.0), (targetPageRect.GetHeight() - top) - (height / 2.0));


		PoDoFo::PdfRect crop(cleft, srect.GetHeight() -( ctop + cheight), cwidth, cheight);

		trx_double_t rotate(-rotation * 90.0);
		trx_double_t scaleX(width / cwidth);
		trx_double_t scaleY(height / cheight);
		if(rotation == 1 || rotation == 3)
		{
			scaleX = width / cheight;
			scaleY = height / cwidth;
		}

		ospi::Point cropCenter((cleft + (cwidth / 2.0)) * 1.0, ((srect.GetHeight() - ctop) - (cheight / 2.0)) * 1.0);

		trx_double_t transX(slotCenter.x - cropCenter.x);
		trx_double_t transY(slotCenter.y - cropCenter.y);

		Transform t;
		t.translate(transX, transY);
		t.rotate(rotate, slotCenter);
		t.scale(scaleX, scaleY, slotCenter);


		SourcePage_Key spk(sdoc, spagenumber, crop);
		SourcePagePtr sp;
		if(pDict.find(spk) == pDict.end())
		{
			sp = SourcePagePtr(new SourcePage);
			sp->setSourceDoc(sdocptr.get());
			sp->setSourcePage(spagenumber);
			sp->setTargetDoc(tdocument.get());
			sp->setTargetPage(tpidx);
			sp->setCrop(crop);
			pDict[spk] = sp;
		}
		else
			sp = pDict.find(spk)->second;

		sp->addTransform(t);

		if(std::find(spages.begin(), spages.end(), sp) == spages.end())
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
		outputName = root.get(K_OutputFile,outputName).asString();
		if(outputName.empty())
		{
			if(params.Has(K_OutputFile))
			{
				outputName = params.Get(K_OutputFile, std::string());
			}
			if(outputName.empty())
				throw std::runtime_error("Can't get output_file_name (JSONCPP)");
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
		tdocument->Write(outputName.c_str());

	}
	
} // namespace ospi
