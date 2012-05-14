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
#include "ReaderJSONCPP.h"
#ifdef WITH_PYTHONREADER
#include "ReaderPython.h"
#endif

#include <stdexcept>

namespace ospi {

	PlanReaderFactory * PlanReaderFactory::instance = NULL;
	PlanReaderFactory::PlanReaderFactory()
	{
		creators[std::string("simple")] = CreatorPtr(new SimplePlanReaderCreator);
		creators[std::string("json")] = CreatorPtr(new ReaderJSONCPPCreator);
#ifdef WITH_PYTHONREADER
		creators[std::string("python")] = CreatorPtr(new ReaderPythonCreator);
#endif
	}

	int PlanReaderFactory::Impose(const std::string &readerTS, const std::string &plan, const PlanParams &params, bool isData)
	{
		if(instance == NULL)
			instance = new PlanReaderFactory;
		if(instance->creators.find(readerTS) == instance->creators.end())
		{
			std::string exceptMessage("Cannot find reader of type ");
			throw std::runtime_error(exceptMessage.append(readerTS));
		}

		PlanReaderPtr preader(instance->creators[readerTS]->Create(plan, params, isData));
		preader->Impose();
	}
	
} // namespace ospi
