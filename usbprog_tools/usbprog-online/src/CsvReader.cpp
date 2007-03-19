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
//
// Created by: Alois Flammensböck <alois@softsprings.org>
// Created on: Fri Mar 16 07:49:46 2007
//

#include "CsvReader.h"
#include <wx/tokenzr.h>

CsvReader::CsvReader(wxInputStream& stream )
{
	this->stream = &stream;
	
	this->textStream = new wxTextInputStream(stream);
	this->textStream->SetStringSeparators(_T(";"));
}


CsvReader::~CsvReader()
{
	delete this->textStream;
}

bool CsvReader::ReadNextRecord()
{
	if ( !(stream->Eof())){
		record.clear();
		wxString line = textStream->ReadLine();
		wxStringTokenizer tkz(line, wxT(";"));
		while ( tkz.HasMoreTokens() )
		{
			record.push_back(tkz.GetNextToken());
		}
		return true;
	}else{
		return false;
	}
}

