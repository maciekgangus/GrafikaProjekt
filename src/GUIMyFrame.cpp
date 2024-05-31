#include "GUIMyFrame.h"


wxBEGIN_EVENT_TABLE(GUIMyFrame, wxFrame)
                EVT_BUTTON(ID_LoadBitMapButton, GUIMyFrame::OnOpen)
                EVT_BUTTON(ID_RotateButton, GUIMyFrame::OnRotate)
                EVT_SLIDER(ID_AngleInput, GUIMyFrame::OnSliderChange)
wxEND_EVENT_TABLE()

GUIMyFrame::GUIMyFrame(wxWindow* parent ) : MyFrame(parent)
{
    MainPanel->Bind(wxEVT_PAINT, &GUIMyFrame::OnPaint, this);
}




void GUIMyFrame::Repaint(wxDC& dc)
{
    if (Img_Cpy.IsOk()) {
        wxBitmap bitmap(Img_Cpy);
        int x = (MainPanel->GetClientSize().GetWidth() - bitmap.GetWidth()) / 2;
        int y = (MainPanel->GetClientSize().GetHeight() - bitmap.GetHeight()) / 2;
        dc.DrawBitmap(bitmap, x, y, false);
    }
}

void GUIMyFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(MainPanel);
    Repaint(dc);
}

void GUIMyFrame::OnOpen(wxCommandEvent &event)
{
    wxFileDialog openFileDialog(this, "Open Image file", "", "",
                                "Image files (*.bmp;*.jpg;*.png)|*.bmp;*.jpg;*.png", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    Img_Org.LoadFile(openFileDialog.GetPath(), wxBITMAP_TYPE_ANY);
    if (!Img_Org.IsOk()) {
        wxMessageBox("Failed to load image.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    wxLogMessage("Image loaded successfully.");

    Img_Org = Img_Org.Scale(MainPanel->GetClientSize().GetWidth() * 0.9,
                            MainPanel->GetClientSize().GetHeight() * 0.9, wxIMAGE_QUALITY_HIGH);
    Img_Cpy = Img_Org;
    MainPanel->Refresh();
    MainPanel->Update();
}

void GUIMyFrame::OnRotate(wxCommandEvent &event)
{
    double angle;
    angle = AngleInput->GetValue();
    double weights[4];
    wxString value;

    value = weight1->GetValue();
    if (!value.ToDouble(&weights[0])) {
        wxLogError("Invalid input for weight1");
    }

    value = weight2->GetValue();
    if (!value.ToDouble(&weights[1])) {
        wxLogError("Invalid input for weight2");
    }

    value = weight3->GetValue();
    if (!value.ToDouble(&weights[2])) {
        wxLogError("Invalid input for weight3");
    }

    value = weight4->GetValue();
    if (!value.ToDouble(&weights[3])) {
        wxLogError("Invalid input for weight4");
    }

    // Tutaj zaimplementuj funkcję obracania obrazu i interpolacji
    if(Img_Org.IsOk())
    {
        Img_Cpy = InterpolateImage(Img_Org, angle, weights);

    }
    MainPanel->Refresh();  // Odśwież panel, aby wywołać OnPaint
    MainPanel->Update();
}
wxImage GUIMyFrame::InterpolateImage(const wxImage& srcImage, double angle, double (&weights)[4]) {
    // Implementacja szczegółowa obrotu i interpolacji obrazu
    wxImage newImage = srcImage; // Placeholder for actual implementation
    return newImage;
}

void GUIMyFrame::OnSliderChange(wxCommandEvent& event) {
    int angle = AngleInput->GetValue();
    AngleValue->SetLabel(wxString::Format("%d°", angle));

}
