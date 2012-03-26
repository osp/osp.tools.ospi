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


#ifndef OSPI_SIMPLEPLANREADER_H
#define OSPI_SIMPLEPLANREADER_H

#include "PlanReader.h"

#include <boost/shared_ptr.hpp>

namespace ospi {
	
	class SimplePlanReader : public PlanReader
	{
		protected:
			typedef boost::shared_ptr<SourcePage> SourcePagePtr;
			typedef boost::shared_ptr<PoDoFo::PdfMemDocument> DocumentPtr;
			std::vector<SourcePagePtr> spages;
			std::map<std::string, DocumentPtr> documents;

			void readRecord(const std::string& rec);

		public:
			SimplePlanReader(const std::string& plan);

			int Impose();
	};
	
} // namespace ospi

#endif // OSPI_SIMPLEPLANREADER_H
