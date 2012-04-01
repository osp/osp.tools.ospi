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

#include "PlanParams.h"

#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

namespace ospi {

	const std::string PlanParams::ParamPlanFile = std::string("plan_file");
	const std::string PlanParams::ParamPlanType = std::string("plan_type");
	
	PlanParams::PlanParams()
	{
	}

	void PlanParams::Add(const std::string &paramstring)
	{
		std::vector<std::string> res;
		boost::algorithm::split( res, paramstring, boost::algorithm::is_any_of("="), boost::algorithm::token_compress_on );
		if(res.size() > 1)
			pData[res.at(0)] = res.at(1);
	}

	void PlanParams::Add(const std::string &key, const std::string &val)
	{
		pData[key] = val;
	}

	bool PlanParams::Has(const std::string &key) const
	{
		if(pData.find(key) != pData.end())
			return true;
		return false;
	}

	bool PlanParams::Has(const std::vector<std::string> &keys) const
	{
		BOOST_FOREACH(const std::string& key , keys)
		{
			if(!Has(key))
				return false;
		}
		return true;
	}

	std::string PlanParams::GetString(const std::string &key) const
	{
		if(Has(key))
			return pData.find(key)->second;
		return std::string();
	}

	void PlanParams::Remove(const std::string &key)
	{
		pData.erase(key);
	}

	
} // namespace ospi
