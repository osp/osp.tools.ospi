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


#ifndef OSPI_SOURCEPAGE_H
#define OSPI_SOURCEPAGE_H

#include "Transform.h"

#include <podofo/podofo.h>

#include <vector>

namespace ospi {
	/**
	  Represents a unit (page) to be laidout on a target page.
	 */
	class SourcePage
	{
		private:
			static const std::vector<PoDoFo::PdfName> resTypes;

		protected:
			typedef std::pair<PoDoFo::PdfName, PoDoFo::PdfObject*> PdfResource;

			// origin
			PoDoFo::PdfDocument * sourceDoc;
			unsigned int sourcePage;
			// handy
			PoDoFo::PdfPage * pCachedPage;

			// transformation to apply to the XObject
			Transform targetTransform;

			// Crop box (unscaled, just inserted as bbox to the XObject dictionary)
			// defaults to the sourcepage Bleed box (following use cases presented in PDFRef1.4v3 p679)
			// keeping the name cropbox for being agnostic
			PoDoFo::PdfRect cropBox;

			// placeholder
			PoDoFo::PdfDocument * targetDoc;

			// target page
			PoDoFo::PdfPage * targetPage;

			// source page instance as xobject in target document
			PoDoFo::PdfXObject * xobj;

			// An identifier in a resource collection
			unsigned int resourceIndex;

			// return a <type, object> pair for a resource name
			PdfResource getNamedResource(const PoDoFo::PdfName &rname) const;

			bool migratable(const PoDoFo::PdfObject *o) const;
			PoDoFo::PdfObject* migrate(PoDoFo::PdfObject* obj);
			void writeResource(const PoDoFo::PdfName &rname, const PdfResource &r);

		public:
			SourcePage(){}
			SourcePage(PoDoFo::PdfDocument * doc, unsigned int pageNumber);
			SourcePage& operator= (const SourcePage& other);

			void setTransform(const Transform& t){targetTransform = t;}
			void setCrop(const PoDoFo::PdfRect& rect){cropBox = rect;}
			void setDoc(PoDoFo::PdfDocument * d){targetDoc = d;}
			void setPage(PoDoFo::PdfPage * p){targetPage = p;}
			void setResource(unsigned int r){resourceIndex = r;}

			Transform getTransform() const {return targetTransform;}
			PoDoFo::PdfRect getCrop(){return cropBox;}
			PoDoFo::PdfDocument * getDoc() const {return targetDoc;}
			PoDoFo::PdfPage * getPage() const {return targetPage;}
			unsigned int getResource() const {return resourceIndex;}

			// parse content stream and get required resources from source document
			void extractResource();

			// layout on target page
			void commit();
	};
	
} // namespace ospi

#endif // OSPI_SOURCEPAGE_H
