/*
   usbprog - Change easy the firmware on the usbprog adapter.

   Copyright (C) 2007 Robert Schilling

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "usbprogFrm.h"
#include <wx/arrstr.h>
#include "../lib2/usbprog.h"
#include "../lib2/xmlParser.h"

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
#include "Images/usbprogFrm_WxStaticBitmap1_XPM.xpm"
////Header Include End


// Global Variables:
struct usbprog_context usbprog;
bool internetConnection = true;

//----------------------------------------------------------------------------
// usbprogFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(usbprogFrm,wxFrame)
	////Manual Code Start
	////Manual Code End
	
	EVT_CLOSE(usbprogFrm::OnClose)
	EVT_ACTIVATE(usbprogFrm::usbprogFrmActivate)
	EVT_BUTTON(ID_WXBUTTON6,usbprogFrm::WxButton6Click)
	EVT_BUTTON(ID_WXBUTTON5,usbprogFrm::WxButton5Click)
	EVT_BUTTON(ID_WXBUTTON4,usbprogFrm::WxButton4Click)
	EVT_BUTTON(ID_WXBUTTON2,usbprogFrm::WxButton2Click)
	EVT_BUTTON(ID_WXBUTTON3,usbprogFrm::WxButton3Click)
	EVT_BUTTON(ID_WXBUTTON1,usbprogFrm::WxButton1Click)
	EVT_RADIOBUTTON(ID_WXRADIOBUTTON2,usbprogFrm::WxRadioButton2Click)
	EVT_RADIOBUTTON(ID_WXRADIOBUTTON1,usbprogFrm::WxRadioButton1Click)
END_EVENT_TABLE()
////Event Table End

usbprogFrm::usbprogFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
    usbprog_init(&usbprog);     //Init Usbprog
        
    getUsbDevices();
    
    if(usbprog_online_get_netlist(&usbprog, "http://www.ixbat.de/usbprog/versions.xml")<=0)    //download firmware list
	{
        //Error: No Connection to Internet
         printWxEdit2(usbprog.error_str);
         internetConnection = false;        //Status: No connection to internet
    }
    else
    {
        //Fill Combo Box
        internetConnection = true;          //Status: Connection to internet
        printWxEdit2(usbprog.status_str);
        int firmwareNr = usbprog_online_numberof_firmwares(&usbprog);
        char *versions[firmwareNr];
        usbprog_online_print_netlist(&usbprog, versions, firmwareNr);

        WxComboBox2->Clear();

        for(int i = 0; i < firmwareNr; i++)
        {
            WxComboBox2->Append(wxString(versions[i], wxConvUTF8));
        }
    }
}

usbprogFrm::~usbprogFrm()
{
}

void usbprogFrm::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	SetTitle(wxT("usbprog flash tool"));
	SetIcon(wxNullIcon);
	SetSize(8,8,464,328);
	Center();
	

	WxOpenFileDialog1 =  new wxFileDialog(this, wxT("Choose a file"), wxT("C:\\"), wxT(""), wxT("*.bin"), wxOPEN);

	WxButton6 = new wxButton(this, ID_WXBUTTON6, wxT("(1) Bootoader"), wxPoint(180,141), wxSize(75,25), 0, wxDefaultValidator, wxT("WxButton6"));
	WxButton6->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxStaticText5 = new wxStaticText(this, ID_WXSTATICTEXT5, wxT("Process"), wxPoint(128,179), wxDefaultSize, 0, wxT("WxStaticText5"));
	WxStaticText5->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxStaticText4 = new wxStaticText(this, ID_WXSTATICTEXT4, wxT("Read more: http://www.embedded-projects.net/usbprog"), wxPoint(128,267), wxDefaultSize, 0, wxT("WxStaticText4"));
	WxStaticText4->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	wxBitmap WxStaticBitmap1_BITMAP(usbprogFrm_WxStaticBitmap1_XPM);
	WxStaticBitmap1 = new wxStaticBitmap(this, ID_WXSTATICBITMAP1, WxStaticBitmap1_BITMAP, wxPoint(16,46), wxSize(82,194));
	WxStaticBitmap1->Enable(false);
	WxStaticBitmap1->SetBackgroundColour(wxColour(0,0,153));
	WxStaticBitmap1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxButton5 = new wxButton(this, ID_WXBUTTON5, wxT("Refresh"), wxPoint(383,75), wxSize(51,21), 0, wxDefaultValidator, wxT("WxButton5"));
	WxButton5->Enable(false);
	WxButton5->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxButton4 = new wxButton(this, ID_WXBUTTON4, wxT("Refresh"), wxPoint(383,45), wxSize(51,21), 0, wxDefaultValidator, wxT("WxButton4"));
	WxButton4->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxStaticText3 = new wxStaticText(this, ID_WXSTATICTEXT3, wxT("(C) 2007 by Benedikt Sauter and Robert Schilling"), wxPoint(128,246), wxDefaultSize, 0, wxT("WxStaticText3"));
	WxStaticText3->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxButton2 = new wxButton(this, ID_WXBUTTON2, wxT("Browse"), wxPoint(383,105), wxSize(51,21), 0, wxDefaultValidator, wxT("WxButton2"));
	WxButton2->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxGauge1 = new wxGauge(this, ID_WXGAUGE1, 100, wxPoint(180,178), wxSize(254,16), wxGA_HORIZONTAL, wxDefaultValidator, wxT("WxGauge1"));
	WxGauge1->SetRange(100);
	WxGauge1->SetValue(0);
	WxGauge1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxButton3 = new wxButton(this, ID_WXBUTTON3, wxT("(2) Update"), wxPoint(270,141), wxSize(75,25), 0, wxDefaultValidator, wxT("WxButton3"));
	WxButton3->Enable(false);
	WxButton3->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxButton1 = new wxButton(this, ID_WXBUTTON1, wxT("Quit"), wxPoint(359,141), wxSize(75,25), 0, wxDefaultValidator, wxT("WxButton1"));
	WxButton1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxStaticText2 = new wxStaticText(this, ID_WXSTATICTEXT2, wxT("Status"), wxPoint(128,211), wxDefaultSize, 0, wxT("WxStaticText2"));
	WxStaticText2->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxEdit2 = new wxTextCtrl(this, ID_WXEDIT2, wxT(""), wxPoint(180,209), wxSize(254,19), 0, wxDefaultValidator, wxT("WxEdit2"));
	WxEdit2->Enable(false);
	WxEdit2->SetForegroundColour(wxColour(*wxWHITE));
	WxEdit2->SetBackgroundColour(wxColour(*wxWHITE));
	WxEdit2->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxEdit1 = new wxTextCtrl(this, ID_WXEDIT1, wxT("C:\\"), wxPoint(180,105), wxSize(197,19), 0, wxDefaultValidator, wxT("WxEdit1"));
	WxEdit1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxRadioButton2 = new wxRadioButton(this, ID_WXRADIOBUTTON2, wxT("Local Disk"), wxPoint(102,106), wxSize(69,19), 0, wxDefaultValidator, wxT("WxRadioButton2"));
	WxRadioButton2->SetValue(true);
	WxRadioButton2->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxRadioButton1 = new wxRadioButton(this, ID_WXRADIOBUTTON1, wxT("Online Pool"), wxPoint(102,76), wxSize(75,19), 0, wxDefaultValidator, wxT("WxRadioButton1"));
	WxRadioButton1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxStaticText1 = new wxStaticText(this, ID_WXSTATICTEXT1, wxT("Device"), wxPoint(120,52), wxDefaultSize, 0, wxT("WxStaticText1"));
	WxStaticText1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	wxArrayString arrayStringFor_WxComboBox2;
	WxComboBox2 = new wxComboBox(this, ID_WXCOMBOBOX2, wxT(""), wxPoint(180,75), wxSize(197,21), arrayStringFor_WxComboBox2, wxCB_READONLY, wxDefaultValidator, wxT("WxComboBox2"));
	WxComboBox2->Enable(false);
	WxComboBox2->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	wxArrayString arrayStringFor_WxComboBox1;
	WxComboBox1 = new wxComboBox(this, ID_WXCOMBOBOX1, wxT(""), wxPoint(180,45), wxSize(197,21), arrayStringFor_WxComboBox1, wxCB_READONLY, wxDefaultValidator, wxT("WxComboBox1"));
	WxComboBox1->SetHelpText(wxT("Device"));
	WxComboBox1->SetFont(wxFont(8, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));

	WxStaticBox1 = new wxStaticBox(this, ID_WXSTATICBOX1, wxT("usbprog flash tool v0.2 (GNU/GPL2)"), wxPoint(4,10), wxSize(447,284));
	WxStaticBox1->SetFont(wxFont(9, wxSWISS, wxNORMAL,wxNORMAL, false, wxT("Tahoma")));
	////GUI Items Creation End
	// SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
}

void usbprogFrm::OnClose(wxCloseEvent& event)
{
    //Close
	Destroy();
}

/*
 * usbprogFrmActivate
 */
void usbprogFrm::usbprogFrmActivate(wxActivateEvent& event)
{
	// insert your code here
}


/*
 * WxButton3Click
 */
void usbprogFrm::WxButton3Click(wxCommandEvent& event)  //Update Button
{
	// insert your code here
	int firmware = 0, error = 0;
	WxButton3->Enable(false);
	WxGauge1->SetValue(40);

	if(WxRadioButton1->GetValue() == true)     //online
	{
         firmware = WxComboBox2->GetCurrentSelection();  //Get Firmware from ComboBox
         
       	if(firmware == -1)   //No selceted Firmware
    	{
             printWxEdit2("No Firmware selected");
             WxButton6->Enable(true);
             return;
        }
    }

    #ifdef _WIN32 
  
    if(internetConnection == true)
    {  
	
	XMLNode xNode = usbprog.xMainNode.getChildNode("pool");     //Device Check
	int deviceStat = 0;                                       //Device Status Var
      
       for(int i = 0; i < xNode.nChildNode("firmware"); i++)   
        {
            //Compare selected Device with official Device List from the XML File
            char buf[100];
            sprintf(buf, "%s", xNode.getChildNode("firmware",i).getAttribute("label"));
            if(strstr(WxComboBox1->GetValue().c_str(), buf) != NULL || strstr(WxComboBox1->GetValue().c_str(), "Usbprog (Update Mode)") != 0)
                deviceStat = 1;
        }
        
        if(!deviceStat)     //You've selected a wrong device
        {
            printWxEdit2("Wrong Device");
            WxGauge1->SetValue(0);
            WxButton6->Enable(true);
            return;
        }
    }
    #endif
     
    if(usbprog_open(&usbprog, WxComboBox1->GetCurrentSelection()))
         printWxEdit2(usbprog.error_str);
    if(WxRadioButton1->GetValue() == true)     //Online Pool
	{
        
       if((error = usbprog_flash_netfirmware(&usbprog, firmware)) == -1)        //Flash firmware
         printWxEdit2(usbprog.error_str);                                       //If an error occurs print Error String
        WxGauge1->SetValue(60);
    }
    else        //local Firmware
    {
        wxString wxPath = WxEdit1->GetValue();
        char* charPath = (char*)wxPath.c_str(); //converts wxString to charArray
        
        if((error = usbprog_flash_firmware(&usbprog, charPath)) == -1)          //Flash firmware
            printWxEdit2(usbprog.error_str);                                    //If an error occurs print Error String
        WxGauge1->SetValue(60);
    }
    
    usbprog_stop_updatemode(&usbprog);      //Stop Update Mode
    usbprog_close(&usbprog);                //Close
    WxGauge1->SetValue(80);
    
    #ifdef _WIN32
    Sleep(1000);
    #endif
    
    /* Refresh device list */
    getUsbDevices();
    
    if(!error)
        printWxEdit2("Job Done");   //Print Status Message
    else
        printWxEdit2("Error during Flashing");
        
    WxGauge1->SetValue(100);
    WxButton6->Enable(true);
}

/*
 * WxButton1Click Close Button
 */
void usbprogFrm::WxButton1Click(wxCommandEvent& event)
{
	// insert your code here
	Destroy();     //Close
}


/*
 * WxButton2Click Browse Button
 */
void usbprogFrm::WxButton2Click(wxCommandEvent& event)
{
	// insert your code here
	if(WxOpenFileDialog1->ShowModal() == wxID_OK)      //Get Path from Browse Dialog
    {
        WxEdit1->SetValue(WxOpenFileDialog1->GetPath());    //Refresh Path in Edit field
    }
}

/*
 * WxRadioButton1Click  Browse online
 */
void usbprogFrm::WxRadioButton1Click(wxCommandEvent& event)
{
	// insert your code here
	
   if(WxRadioButton1->GetValue() == true)
	{
        //Activate Online, deactivate local
        WxComboBox2->Enable(true);
        WxEdit1->Enable(false); 
        WxButton2->Enable(false);
        WxButton5->Enable(true);
    }
}

/*
 * WxRadioButton2Click Browse locally
 */
void usbprogFrm::WxRadioButton2Click(wxCommandEvent& event)
{
	// insert your code here
	if(WxRadioButton2->GetValue() == true)
	{
        //Deactivate Online, activate local
        WxComboBox2->Enable(false);
        WxEdit1->Enable(true);  
        WxButton2->Enable(true);
        WxButton5->Enable(false);
    }
}


/*
 * WxButton5Click Refresh Online firmware List
 */
void usbprogFrm::WxButton5Click(wxCommandEvent& event)
{
	// insert your code here

	if(usbprog_online_get_netlist(&usbprog, "http://www.ixbat.de/usbprog/versions.xml")<=0)    //download firmware list
	{
        //No Connection to Internet
        printWxEdit2(usbprog.error_str);
    }
    else
    {
        printWxEdit2(usbprog.status_str);
        int firmwareNr = usbprog_online_numberof_firmwares(&usbprog);   //Get number of firmwares online
        
        if(firmwareNr == -1)
        {
            //Error during download the XML File
            printWxEdit2(usbprog.error_str);
            return;
        }
            
        char *versions[firmwareNr];
        usbprog_online_print_netlist(&usbprog, versions, firmwareNr);   //Get Names of fimwares and save in buffer
        
        WxComboBox2->Clear();
        
        for(int i = 0; i < firmwareNr; i++)
        {
            //Print Firmwares in Combo Box
            WxComboBox2->Append(wxString(versions[i], wxConvUTF8));
        }
    }
}


/*
 * WxButton4Click Refresh device List
 */
void usbprogFrm::WxButton4Click(wxCommandEvent& event)
{
    getUsbDevices();
}


/*
 * printWxEdit prints text on WxEdit2 Text field
 */

void usbprogFrm::printWxEdit2(char * text)
{
	/* TODO (#1#): Implement usbprogFrm::printWxEdit2() */
	
	WxEdit2->Clear();
	WxEdit2->WriteText(wxString(text, wxConvUTF8));
}


/*
 * WxButton6Click Start Update Mode
 */
void usbprogFrm::WxButton6Click(wxCommandEvent& event)
{
    WxGauge1->SetValue(20);
    WxButton6->Enable(false);

    printWxEdit2("Starting");
    int device = WxComboBox1->GetCurrentSelection();
    
  	if(device == -1)   //No Device selceted
	{
        printWxEdit2("No Device Selected");
        WxButton6->Enable(true);
        return;
    }
           
    if(usbprog.devList[device]->descriptor.bcdDevice == 0)
    {
        printWxEdit2("Device already in update mode");
    }
    else
    {
        usbprog_update_mode_device(&usbprog, device);   //Set the usbprog in update mode
        getUsbDevices();
    }
    WxGauge1->SetValue(40);
    WxButton3->Enable(true);

}

void usbprogFrm::getUsbDevices(void)
{
    WxComboBox1->Clear();
    struct usb_bus *busses;
    char vendor[255];
    char product[255];
    char serial[255];
    int vendorlen=0, productlen=0, seriallen=0;
    int i=0;
    struct usb_device *dev;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    busses = usb_get_busses();
    struct usb_bus *bus;

      for (bus = busses; bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
	        #ifndef _WIN32
            if(dev->descriptor.bDeviceClass==0x09) // hub devices
                continue;
            #endif

            if(dev->descriptor.bDescriptorType !=1)
                continue;

            vendor[0]=0x00; product[0]=0x00;serial[0]=0x00;

            if(dev->descriptor.idVendor==0x1781 && dev->descriptor.idProduct==0x0c62)
            {
                usb_dev_handle * tmp_handle = usb_open(dev);
                if(usb_get_string_simple(tmp_handle, 1, vendor, 255) <= 0 && usb_get_string_simple(tmp_handle, 2, product, 255) <= 0)
                {
                    sprintf(vendor,"usbprog");
                    sprintf(product,"update mode");
                    vendorlen = strlen(vendor);
                    productlen = strlen(product);
                }
                else
                {
                    vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
                    productlen = usb_get_string_simple(tmp_handle, 2, product, 255);
                    seriallen = usb_get_string_simple(tmp_handle, 3, serial, 255);
                }
                usb_close(tmp_handle);
            }
            else
            {
                usb_dev_handle * tmp_handle = usb_open(dev);
                vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
                productlen = usb_get_string_simple(tmp_handle, 2, product, 255);
                seriallen = usb_get_string_simple(tmp_handle, 3, serial, 255);
                usb_close(tmp_handle);

                if(vendorlen<=0) sprintf(vendor,"unkown vendor");
                if(productlen<=0) sprintf(product,"unkown product");
                if(seriallen<=0) sprintf(serial,"none");
            }

            if(vendorlen<=0 && productlen<=0)
                continue;

	       char * complete = (char*)malloc(sizeof(char)*(strlen(vendor)+strlen(product)+strlen(serial)+30));
	       sprintf(complete,"%s",product);
	               
            if(strcmp(complete, "update mode") == 0)
                WxComboBox1->Append(wxString("Usbprog (Update Mode)", wxConvUTF8));
            else
                WxComboBox1->Append(wxString(complete, wxConvUTF8));
	       //usb_close(tmp_handle);

	       usbprog.devList[i] = dev;
	       i++;
        }
    }
    char status[40];
    sprintf(status, "Found %d devices on USB Bus", i);    //Print Status
    printWxEdit2(status);
}
    
