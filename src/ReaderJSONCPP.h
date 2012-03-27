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


#ifndef OSPI_READERJSONCPP_H
#define OSPI_READERJSONCPP_H

#include "PlanReader.h"
#include "json/json.h"

namespace ospi {
	
	class ReaderJSONCPP : public PlanReader
	{
		protected:

			std::string planPath;
			PlanParams params;

			std::vector<SourcePagePtr> spages;
			std::map<std::string, DocumentPtr> sdocuments;
			std::map<std::string, DocumentPtr> tdocuments;

			static const std::string K_Plan;
			static const std::string K_TargetWidth;
			static const std::string K_TargetHeight;
			static const std::string K_Slots;
			static const std::string K_SlotWidth;
			static const std::string K_SlotHeight;
			static const std::string K_SlotLeft;
			static const std::string K_SlotTop;
			static const std::string K_SlotFile;
			static const std::string K_SlotPage;
			static const std::string K_CropWidth;
			static const std::string K_CropHeight;
			static const std::string K_CropLeft;
			static const std::string K_CropTop;
			static const std::string K_Rotation;

			void readRecord(const Json::Value& rec);
		public:
			ReaderJSONCPP(const std::string& plan, const PlanParams& params);
			int Impose();
	};

	class ReaderJSONCPPCreator : public PlanReaderFactory::Creator
	{
		public:
			PlanReaderPtr Create(const std::string& plan, const PlanParams& params)
			{
				return PlanReaderPtr(new ReaderJSONCPP(plan,params));
			}
	};
} // namespace ospi

#endif // OSPI_READERJSONCPP_H
