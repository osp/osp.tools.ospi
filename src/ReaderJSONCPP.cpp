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


namespace ospi {

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

	void ReaderJSONCPP::readRecord(const Json::Value &rec)
	{
		std::string sdoc;
		std::string tdoc;
		unsigned int spagenumber(0);
		unsigned int tpagenumber(0);
		double tpagewidth(0), tpageheight(0);
		double a(1),b(0),c(0),d(1),e(0),f(0);
		rec.get()
	}

	int ReaderJSONCPP::Impose()
	{
		Json::Value root;   // will contains the root value after parsing.
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( planPath, root );
		if ( !parsingSuccessful )
		{
			throw std::runtime_error("Cant parse plan file (JSONCPP)");
		}
		const Json::Value plan = root[K_Plan];
		for (unsigned int index(0); index < plan.size(); ++index )
		{
			Json::Value rec(plan[index]);
			readRecord(rec);
		}
	}
	
} // namespace ospi
