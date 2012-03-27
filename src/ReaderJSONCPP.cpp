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

#include <stdexcept>

#include <boost/foreach.hpp>


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
		double rotation;


		sdoc = slot.get(K_SlotFile, sdoc).asString();
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
		top = slot.get(K_SlotTop, srect.GetHeight()).asDouble();
		width = slot.get(K_SlotWidth,srect.GetWidth()).asDouble();
		height = slot.get(K_SlotHeight, srect.GetHeight()).asDouble();
		rotation = slot.get(K_Rotation, 0).asDouble();

		Transform t;
		t.translate(left, top - height);
		t.rotate(rotation * 90.0);
		t.scale(width / srect.GetWidth(), height / srect.GetHeight());

		sp->setTransform(t);
		spages.push_back(sp);

	}

	int ReaderJSONCPP::Impose()
	{
		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( planPath, root );
		if ( !parsingSuccessful )
		{
			throw std::runtime_error("Cant parse plan file (JSONCPP)");
		}

		std::string outputName;
		outputName = root.get(K_OutputRoot,outputName).asString();
		if(outputName.empty())
			throw std::runtime_error("Can't get output_root_name (JSONCPP)");

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
