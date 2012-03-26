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


namespace ospi {

	/**
	  A reader for basic, verbose, imposition plan format.
	  each line of the plan file represents a record of the form:
	  source_document target_doc  source_page_number  target_page_number  target_page_width target_page_height a b c d e f;

	  note: a b c d e f is the transformation matrix to apply to the page
	  */
	class SimplePlanReader : public PlanReader
	{
		protected:
			typedef boost::shared_ptr<SourcePage> SourcePagePtr;
			typedef boost::shared_ptr<PoDoFo::PdfMemDocument> DocumentPtr;
			typedef std::pair<std::string, DocumentPtr> DocInfoKey;

			std::string planPath;
			PlanParams params;

			std::vector<SourcePagePtr> spages;
			std::map<std::string, DocumentPtr> sdocuments;
			std::map<std::string, DocumentPtr> tdocuments;

			static const std::string InputKW;
			static const std::string OutputKW;

			void readRecord(const std::string& rec);

		public:
			SimplePlanReader(const std::string& plan, const PlanParams& params);

			int Impose();
	};

	class SimplePlanReaderCreator : public PlanReaderFactory::Creator
	{
		public:
			PlanReaderPtr Create(const std::string& plan, const PlanParams& params)
			{
				return PlanReaderPtr(new SimplePlanReader(plan,params));
			}
	};
	
} // namespace ospi

#endif // OSPI_SIMPLEPLANREADER_H
