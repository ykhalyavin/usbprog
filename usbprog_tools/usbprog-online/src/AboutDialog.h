
#ifndef AboutDialogGenerated_H
#define AboutDialogGenerated_H

 #include "AboutDialogGenerated.h"
 
class AboutDialog: public AboutDialogGenerated
{
	private:
	wxBitmap* hardware;
	wxBitmap* softspringsLogo;
	
	protected:
  		virtual void OnBtnLnkEmbeddedProjectsClick( wxCommandEvent& event );
  		virtual void OnBtnLnkSoftspringsClick( wxCommandEvent& event );
		void ExecuteURL(const wxString &link);
	public:
		AboutDialog( wxWindow* parent, int id = wxID_ANY, wxString title = wxEmptyString, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 400,500 ), int style = wxDEFAULT_DIALOG_STYLE );
	
};

#endif // AboutDialogGenerated_H
