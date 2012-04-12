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


#include "ReaderPython.h"

#include "Python.h"

#include <boost/foreach.hpp>



namespace ospi {
	
	ReaderPython::ReaderPython(const std::string& plan, const PlanParams& params)
		:plan(plan), params(params)
	{
		Py_Initialize();
	}

	void ReaderPython::readRecord(const boost::python::dict &record, DocumentPtr tdoc, int tpidx)
	{
		std::string sdoc = boost::python::extract<std::string>(record["source_document"]);
		int spagenumber = boost::python::extract<int>(record["source_page"]);
		boost::python::dict bbox = boost::python::extract<boost::python::dict>(record["crop_box"]);
		boost::python::list tr = boost::python::extract<boost::python::list>(record["translate"]);
		boost::python::list sc = boost::python::extract<boost::python::list>(record["scale"]);
		double rot = boost::python::extract<double>(record["rotate"]);

		if(sdocuments.find(sdoc) == sdocuments.end())
		{
			PoDoFo::PdfMemDocument * d(new PoDoFo::PdfMemDocument(sdoc.c_str()));
			sdocuments[sdoc] = DocumentPtr(d);
		}
		DocumentPtr sdocptr(sdocuments[sdoc]);

		double tx = boost::python::extract<double>(tr[0]);
		double ty = boost::python::extract<double>(tr[1]);
		double sx = boost::python::extract<double>(sc[0]);
		double sy = boost::python::extract<double>(sc[1]);
		Transform t;
		t.translate(tx, ty);
		t.rotate(rot);
		t.scale(sx, sy);


		SourcePagePtr sp(new SourcePage);
		sp->setTargetDoc(tdoc.get());
		sp->setSourceDoc(sdocptr.get());
		sp->setTargetPage(tpidx);
		sp->setSourcePage(spagenumber);
		sp->addTransform(t);
		sp->setCrop(PoDoFo::PdfRect(boost::python::extract<double>(bbox["bottom"]),
					    boost::python::extract<double>(bbox["left"]),
					    boost::python::extract<double>(bbox["width"]),
					    boost::python::extract<double>(bbox["height"])));

		spages.push_back(sp);

	}

	void ReaderPython::readPage(const boost::python::dict &page)
	{
		std::string tdoc;
		double tpagewidth;
		double tpageheight;
		boost::python::list elems;

		tdoc = boost::python::extract<std::string>(page["target_document"]);
		tpagewidth = boost::python::extract<double>(page["target_page_width"]);
		tpageheight = boost::python::extract<double>(page["target_page_height"]);
		elems = boost::python::extract<boost::python::list>(page["pages"]);


		if(tdocuments.find(tdoc) == tdocuments.end())
		{
			PoDoFo::PdfMemDocument * d(new PoDoFo::PdfMemDocument);
			tdocuments[tdoc] = DocumentPtr(d);
		}
		DocumentPtr tdocptr(tdocuments[tdoc]);
		tdocptr->CreatePage(PoDoFo::PdfRect(0,0,tpagewidth,tpageheight));
		int tpidx(tdocptr->GetPageCount() - 1);

		boost::python::ssize_t eCount = boost::python::len(elems);
		for(boost::python::ssize_t i(0); i < eCount; i++)
		{
			boost::python::object elemObject = elems[i];
			boost::python::dict elem = boost::python::extract<boost::python::dict>(elemObject);
			readRecord(elem, tdocptr, tpidx);
		}

	}

	int ReaderPython::Impose()
	{

		boost::python::object main_module = boost::python::import("__main__");
		boost::python::object main_namespace = main_module.attr("__dict__");

		boost::python::exec_file(plan.c_str(), main_namespace);
		boost::python::list imposition = boost::python::extract<boost::python::list>(main_namespace["imposition_plan"]);
		boost::python::ssize_t pCount = boost::python::len(imposition);
		for(boost::python::ssize_t i(0); i < pCount; i++)
		{
			boost::python::object pageObject = imposition[i];
			boost::python::dict page = boost::python::extract<boost::python::dict>(pageObject);
			try
			{
				readPage(page);
			}
			catch(boost::python::error_already_set const &)
			{
				std::string perror_str = parse_python_exception();
				std::cerr << "Error in Python: " << perror_str << std::endl;
			}
		}

		BOOST_FOREACH(SourcePagePtr spp, spages)
		{
			spp->commit();
		}
		BOOST_FOREACH(DocInfoKey k, tdocuments)
		{
			k.second->SetWriteMode(PoDoFo::ePdfWriteMode_Clean);
			k.second->Write(k.first.c_str());
		}

	}


	// from https://github.com/william76/boost-python-tutorial/blob/master/part2/handle_error.cpp
	// no licence attached, assumed it's free software or open source from its hosting on the open source side of github
	// TODO rewrite that, i hate to copy & paste code - pm
	std::string ReaderPython::parse_python_exception()
	{
		PyObject *type_ptr = NULL, *value_ptr = NULL, *traceback_ptr = NULL;
		// Fetch the exception info from the Python C API
		PyErr_Fetch(&type_ptr, &value_ptr, &traceback_ptr);

		// Fallback error
		std::string ret("Unfetchable Python error");
		// If the fetch got a type pointer, parse the type into the exception string
		if(type_ptr != NULL){
			boost::python::handle<> h_type(type_ptr);
			boost::python::str type_pstr(h_type);
			// Extract the string from the boost::python object
			boost::python::extract<std::string> e_type_pstr(type_pstr);
			// If a valid string extraction is available, use it
			//  otherwise use fallback
			if(e_type_pstr.check())
				ret = e_type_pstr();
			else
				ret = "Unknown exception type";
		}
		// Do the same for the exception value (the stringification of the exception)
		if(value_ptr != NULL){
			boost::python::handle<> h_val(value_ptr);
			boost::python::str a(h_val);
			boost::python::extract<std::string> returned(a);
			if(returned.check())
				ret +=  ": " + returned();
			else
				ret += std::string(": Unparseable Python error: ");
		}
		// Parse lines from the traceback using the Python traceback module
		if(traceback_ptr != NULL){
			boost::python::handle<> h_tb(traceback_ptr);
			// Load the traceback module and the format_tb function
			boost::python::object tb(boost::python::import("traceback"));
			boost::python::object fmt_tb(tb.attr("format_tb"));
			// Call format_tb to get a list of traceback strings
			boost::python::object tb_list(fmt_tb(h_tb));
			// Join the traceback strings into a single string
			boost::python::object tb_str(boost::python::str("\n").join(tb_list));
			// Extract the string, check the extraction, and fallback in necessary
			boost::python::extract<std::string> returned(tb_str);
			if(returned.check())
				ret += ": " + returned();
			else
				ret += std::string(": Unparseable Python traceback");
		}
		return ret;
	}
	
} // namespace ospi
