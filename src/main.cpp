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
#include <boost/algorithm/string.hpp>

#include "ResourceCollection.h"
#include "SourcePage.h"
#include "PlanReader.h"


int main(int ac, char ** av)
{
	PoDoFo::PdfError::EnableDebug ( false );
	PoDoFo::PdfError::EnableLogging ( false );

	if(ac == 1)
	{
		std::cerr<<"Usage: "<<av[0]<< " " << ospi::PlanParams::ParamPlanFile << "=your_plan_file ["
			<< ospi::PlanParams::ParamPlanType << "=simple|json] [option_key=option_value ...]"<<std::endl;
		return 1;
	}

	ospi::PlanParams params;
	for(int i(1); i < ac; ++i)
	{
		params.Add(std::string(av[i]));
	}

	if(!params.Has(ospi::PlanParams::ParamPlanFile) && !params.Has(ospi::PlanParams::ParamPlanData))
	{
		std::cerr<<"You must provide an imposition plan file path argument, or plan data."<<std::endl;
		return 2;
	}
	if(!params.Has(ospi::PlanParams::ParamPlanType))
	{
		if(params.Has(ospi::PlanParams::ParamPlanFile))
		{
			// try to guess the type by the extension of the plan file
			std::vector<std::string> res;
			std::string planfile(params.GetString(ospi::PlanParams::ParamPlanFile));
			boost::algorithm::split( res, planfile , boost::algorithm::is_any_of("."), boost::algorithm::token_compress_on );
			if(res.size() == 1) // no extension, we assume it's a simple plan
				params.Add(ospi::PlanParams::ParamPlanType, std::string("simple"));
			else
			{
				std::string extension(res.back());
				if(extension == std::string("json"))
					params.Add(ospi::PlanParams::ParamPlanType, std::string("json"));
				else if(extension == std::string("py"))
					params.Add(ospi::PlanParams::ParamPlanType, std::string("python"));
				// else if ...
				else
				{
					std::cerr<<"Unknown extension and no plan type provided, stop here."<<std::endl;
					return 3;
				}
			}
		}
		else
		{
			std::cerr<<"Cannot guess type of the plan data, please provide plan data type"<<std::endl;
			return 3;
		}

	}

	if(params.Has(ospi::PlanParams::ParamPlanData))
		ospi::PlanReaderFactory::Impose(params.GetString(ospi::PlanParams::ParamPlanType), params.GetString(ospi::PlanParams::ParamPlanData), params, true);
	else
		ospi::PlanReaderFactory::Impose(params.GetString(ospi::PlanParams::ParamPlanType), params.GetString(ospi::PlanParams::ParamPlanFile), params, false);

	return 0;
}

