
#pragma once

#include <wx/wx.h>


class MyFrame : public wxFrame
{
	private:
           
	protected:



	public:
        wxPanel *MainPanel;
        wxPanel *SidePanel;
        wxPanel *BottomPanel;
        wxButton *LoadBitMapButton;
        wxStaticBitmap* ImageDisplay;
        wxSlider* AngleInput;
        wxButton* RotateButton;
        wxStaticText* AngleValue;
        wxTextCtrl* weight1;
        wxTextCtrl* weight2;
        wxTextCtrl* weight3;
        wxTextCtrl* weight4;
    enum {
        ID_LoadBitMapButton = 1,
        ID_RotateButton = 2,
        ID_AngleInput = 3
    };







		MyFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Projekcik", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1000,1000 ), long style = wxDEFAULT_FRAME_STYLE|wxBORDER_NONE|wxTAB_TRAVERSAL );
		~MyFrame();

};

