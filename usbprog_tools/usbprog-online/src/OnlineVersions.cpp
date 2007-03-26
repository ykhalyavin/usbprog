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
// Class: OnlineVersions
//
// Created by: Alois Flammensböck <flammensboeck@softsprings.org>
// Created on: Fri Mar 16 12:57:01 2007
//

#include "OnlineVersions.h"
#include <wx/protocol/http.h>
#include "CsvReader.h"
#include <wx/log.h>
#include <wx/wfstream.h>
#include <stdexcept>

OnlineVersions::OnlineVersions()
{
	// TODO: put constructor code here
}


OnlineVersions::~OnlineVersions()
{
	ClearVersions();
}

void OnlineVersions::Update(wxString url)
{
	ClearVersions();
	
	wxLogInfo(_T("Downloading %s"),url.c_str());
		//remove "http://" from Url;
	if (url.Left(7).CmpNoCase(_T("http://"))==0 ){
		url = url.Remove(0,7);
	}
	
	int fileLocationPos = url.Find('/'); 
	wxString servername;
	wxString fileLocation;
	//get the servername
	if (fileLocationPos > 0) {
		servername = url.Left(fileLocationPos);
		fileLocation = url.Right(url.Len()-fileLocationPos);
	}else{
		servername = url;
		fileLocation = _T("");
	}
	
	//find out port if available
	int portPos = servername.Find(':');
	unsigned long port;
	if (portPos > 0){
		if(!servername.Right(servername.Len()-portPos-1).ToULong(&port)){
			throw("Could not cast port");
		}
		servername = servername.Left(portPos);
	}else{
		port =80;
	}
	
	wxHTTP get;
    wxString res= _T("");


    get.SetHeader( _T("User-Agent"), _T("usbprog-online") );
    
	//Set Timeout to 10 seconds
	get.SetTimeout(10); 
    
    if (get.Connect(servername, port)){
    

    	// just grab the root document. index.html, default.asp, etc. the server will determine what it sould be.
    	wxInputStream* resStream = get.GetInputStream( fileLocation );
 		wxProtocolError error = get.GetError() ;
 		if (error == wxPROTO_NOERR){
 		
			CsvReader reader(*resStream);
	
			int index=0;
			while (reader.ReadNextRecord()){
				if (reader.count()==4){
					VersionInfo *info = new VersionInfo;
					versions.push_back(info);
					info->Name = reader[0].Remove(0,1);
					info->Version = reader[1];
					info->URL = reader[2];
					info->Description = reader[3];
		
				}
			}
		}else{
			wxString errorMessage;
			switch  (error) {
				case wxPROTO_NETERR:  
					errorMessage = _T("A generic network error occurred.");
					break;
				case wxPROTO_PROTERR:  
					errorMessage = _T("An error occurred during negotiation.");
					break;
				case wxPROTO_CONNERR:  
					errorMessage = _T("The client failed to connect the server.");
					break;
				case wxPROTO_INVVAL:
					 errorMessage = _T("Invalid value.");
					break;
				case wxPROTO_NOFILE: 
					errorMessage = _T("The remote file doesn't exist.");
					break;
				case wxPROTO_ABRT:
					errorMessage = _T("Last action aborted.");
					break;
				case wxPROTO_RCNCT:  
					errorMessage = _T("An error occurred during reconnection.");
					break;
				errorMessage = _T("Unknown Error"); 
			}
			wxLogError(_T("Could not get %s, %s"),fileLocation.c_str(),errorMessage.c_str());
		}
	} else{
		wxLogError(_T("Could not connect to Server %s"),servername.c_str());
	}
	
}

void OnlineVersions::ClearVersions()
{
	for (int i=0;i < versions.size();i++){
		delete versions[i];
	}
	versions.clear();
}

void OnlineVersions::DownloadOnlineVersion(int versionID,  wxString fileName)
{

	if (versionID < versions.size()){
		wxString url = versions[versionID]->URL;
		wxLogInfo(_T("Downloading %s"),url.c_str());
		//remove "http://" from Url;
		if (url.Left(7).CmpNoCase(_T("http://"))==0 ){
			url = url.Remove(0,7);
		}
		
		wxLogDebug(_T("URL: %s"),url.c_str());
		
		int fileLocationPos = url.Find('/'); 
		wxString servername;
		wxString fileLocation;
		//get the servername
		if (fileLocationPos > 0) {
			servername = url.Left(fileLocationPos);
			fileLocation = url.Right(url.Len()-fileLocationPos);
		}else{
			servername = url;
			fileLocation = _T("");
		}
		
		
		wxLogDebug(_T("Extracted Servername: %s"),servername.c_str());
		
		//find out port if available
		int portPos = servername.Find(':');
		unsigned long port;
		if (portPos > 0){
			if(!servername.Right(servername.Len()-portPos-1).ToULong(&port)){
				throw("Could not cast port");
			}
			servername = servername.Left(portPos);
		}else{
			port =80;
		}
		
		wxHTTP get;
    	wxString res= _T("");
	
	
    	get.SetHeader( _T("User-Agent"), _T("usbprog-online") );
		
		//Set Timeout to 10 seconds
		get.SetTimeout(10); 
    	
		wxLogDebug(_T("Try to connect to %s Port %d"),servername.c_str(),port);
    	
    	if (get.Connect(servername, port)){
    		
    		wxInputStream* resStream = get.GetInputStream( fileLocation );
 			wxProtocolError error = get.GetError() ;
 			if (error == wxPROTO_NOERR){
 			
				wxLogDebug(_T("Download successful"),url.c_str());
 				
 				wxFFileOutputStream fileStream(fileName);
 				fileStream.Write(*resStream);
 				
 				
 				
			}else{
				wxString errorMessage;
				switch  (error) {
					case wxPROTO_NETERR:  
						errorMessage = _T("A generic network error occurred.");
						break;
					case wxPROTO_PROTERR:  
						errorMessage = _T("An error occurred during negotiation.");
						break;
					case wxPROTO_CONNERR:  
						errorMessage = _T("The client failed to connect the server.");
						break;
					case wxPROTO_INVVAL:
						 errorMessage = _T("Invalid value.");
						break;
					case wxPROTO_NOFILE: 
						errorMessage = _T("The remote file doesn't exist.");
						break;
					case wxPROTO_ABRT:
						errorMessage = _T("Last action aborted.");
						break;
					case wxPROTO_RCNCT:  
						errorMessage = _T("An error occurred during reconnection.");
						break;
					errorMessage = _T("Unknown Error"); 
				}
				wxLogError(_T("Could not get %s, %s"),fileLocation.c_str(),errorMessage.c_str());
			}
		} else{
			wxLogError(_T("Could not connect to Server %s"),servername.c_str());
		}
	}else {
			wxLogError(_T("Invalid versionID %d"),versionID);
	}
}
