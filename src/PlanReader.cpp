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


#include "PlanReader.h"

#include "SimplePlanReader.h"

namespace ospi {
	
	PlanReader::PlanReader(const std::string& plan)
		: planPath(plan)
	{
	}

	PlanReaderFactory * PlanReaderFactory::instance = NULL;
	PlanReaderFactory::PlanReaderFactory()
	{
		creators[std::string("simple")] = CreatorPtr(new SimplePlanReaderCreator);
	}

	int PlanReaderFactory::Impose(const std::string &readerTS, const std::string &plan)
	{
		if(instance == NULL)
			instance = new PlanReaderFactory;
		if(instance->creators.find(readerTS) == instance->creators.end())
			return -1;

		PlanReaderPtr preader(instance->creators[readerTS]->Create(plan));
		preader->Impose();
	}
	
} // namespace ospi
