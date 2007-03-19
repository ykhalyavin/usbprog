/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: CsvReader
// Created by: Alois Flammensböck <alois@softsprings.org>
// Created on: Fri Mar 16 07:49:46 2007
//

#ifndef _CSVREADER_H_
#define _CSVREADER_H_
#include <wx/stream.h>
#include <vector>
#include <wx/txtstrm.h>

class CsvReader
{
		std::vector<wxString> record;
		wxTextInputStream *textStream;
		wxInputStream *stream;
		
	public:
		CsvReader(wxInputStream& stream);
		 ~CsvReader();
	
		// CsvReader interface
		bool ReadNextRecord();
		
		int count(){return record.size();}
		
		
		wxString operator[](unsigned int position){return this->record[position];}
		
	
	protected:
		// CsvReader variables
	
		// TODO: add member variables...
	
};


#endif	//_CSVREADER_H_

