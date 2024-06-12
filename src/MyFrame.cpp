#include "MyFrame.h"

MyFrame::MyFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style)
{
    // Tworzenie paneli
    MainPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    MainPanel->SetBackgroundColour(wxColour(100, 200, 100));
    SidePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    SidePanel->SetBackgroundColour(wxColour(100, 200, 100));
    BottomPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    BottomPanel->SetBackgroundColour(wxColour(100, 200, 100));

    // Główne sizer'y
    wxBoxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* leftSideSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* rightSideSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* controlsSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* weightsSizer = new wxFlexGridSizer(2, 4, 5, 5); // 2 rows, 4 columns, 5px gap

    // Dodanie paneli do odpowiednich sizerów
    leftSideSizer->Add(MainPanel, 4, wxEXPAND | wxALL, 10);
    leftSideSizer->Add(BottomPanel, 2, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 10);

    rightSideSizer->Add(SidePanel, 1, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 10);

    // Dodanie przycisków i kontrolek do SidePanel
    LoadBitMapButton = new wxButton(SidePanel, ID_LoadBitMapButton, "Upload bitmap");
    controlsSizer->Add(LoadBitMapButton, 0, wxEXPAND | wxALL, 5);

    AngleValue = new wxStaticText(SidePanel, wxID_ANY, "0°");
    controlsSizer->Add(AngleValue, 0, wxALIGN_CENTER | wxALL, 5);

    AngleInput = new wxSlider(SidePanel, ID_AngleInput, 0, 0, 360);
    controlsSizer->Add(new wxStaticText(SidePanel, wxID_ANY, "Slect angle: "), 0, wxALL, 5);
    controlsSizer->Add(AngleInput, 0, wxEXPAND | wxALL, 5);

    RotateButton = new wxButton(SidePanel, ID_RotateButton, "Rotate");
    controlsSizer->Add(RotateButton, 0, wxEXPAND | wxALL, 5);

    // Dodanie tekstu opisowego nad polami tekstowymi
    controlsSizer->Add(new wxStaticText(SidePanel, wxID_ANY, "Podaj wagi:"), 0, wxALL, 5);

    // Dodanie pól tekstowych na wagi
    weight1 = new wxTextCtrl(SidePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    weight2 = new wxTextCtrl(SidePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    weight3 = new wxTextCtrl(SidePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    weight4 = new wxTextCtrl(SidePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);

    // Dodanie etykiet i pól tekstowych do weightsSizer
    weightsSizer->Add(new wxStaticText(SidePanel, wxID_ANY, "W1:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    weightsSizer->Add(weight1, 0, wxEXPAND | wxALL, 5);
    weightsSizer->Add(new wxStaticText(SidePanel, wxID_ANY, "W2:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    weightsSizer->Add(weight2, 0, wxEXPAND | wxALL, 5);
    weightsSizer->Add(new wxStaticText(SidePanel, wxID_ANY, "W3:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    weightsSizer->Add(weight3, 0, wxEXPAND | wxALL, 5);
    weightsSizer->Add(new wxStaticText(SidePanel, wxID_ANY, "W4:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    weightsSizer->Add(weight4, 0, wxEXPAND | wxALL, 5);

    // Konfiguracja proporcji kolumn w weightsSizer
    weightsSizer->AddGrowableCol(1, 1);
    weightsSizer->AddGrowableCol(3, 1);

    controlsSizer->Add(weightsSizer, 0, wxEXPAND | wxALL, 5);

    // Ustawienie sizer'a dla SidePanel
    SidePanel->SetSizer(controlsSizer);

    // Dodanie sizer'ów do MainSizer
    MainSizer->Add(leftSideSizer, 3, wxEXPAND);
    MainSizer->Add(rightSideSizer, 0, wxEXPAND);

    // Ustawienie sizer'a dla ramki
    this->SetSizerAndFit(MainSizer);
    this->SetMinSize(wxSize(800, 800));
}

MyFrame::~MyFrame()
{
}