#include "GUIMyFrame.h"
#include <cmath>
#include <numeric>

wxBEGIN_EVENT_TABLE(GUIMyFrame, wxFrame)
EVT_BUTTON(ID_LoadBitMapButton, GUIMyFrame::OnOpen)
EVT_BUTTON(ID_RotateButton, GUIMyFrame::OnRotate)
EVT_SLIDER(ID_AngleInput, GUIMyFrame::OnSliderChange)
wxEND_EVENT_TABLE()

GUIMyFrame::GUIMyFrame(wxWindow* parent) : MyFrame(parent), errorTab()
{
    MainPanel->Bind(wxEVT_PAINT, &GUIMyFrame::OnPaint, this);
    BottomPanel->Bind(wxEVT_PAINT, &GUIMyFrame::OnPaintGraph, this); // Rejestracja zdarzenia rysowania dla BottomPanel
    BottomPanel->Bind(wxEVT_SIZE, &GUIMyFrame::OnResize, this); // Rejestracja zdarzenia zmiany rozmiaru dla BottomPanel
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

void GUIMyFrame::OnOpen(wxCommandEvent& event)
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

void GUIMyFrame::OnRotate(wxCommandEvent& event)
{
    double angle = AngleInput->GetValue();
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

    if (Img_Org.IsOk())
    {
        Img_Cpy = InterpolateImage(Img_Org, angle, weights);
        
        // Testowanie wag i minimalizacja b��du
        OptimizeWeights(weights, angle);
        MainPanel->Refresh();
        MainPanel->Update();

        DrawGraphAfterRotation();
    }


}

wxImage GUIMyFrame::InterpolateImage(const wxImage& srcImage, double angle, double(&weights)[4]) {
    int width = srcImage.GetWidth();
    int height = srcImage.GetHeight();
    wxImage newImage(width, height);

    double radians = angle * M_PI / 180.0;
    double cosine = cos(radians);
    double sine = sin(radians);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Znalezienie oryginalnych wsp�rz�dnych
            int orig_x = static_cast<int>(cosine * (x - width / 2) + sine * (y - height / 2) + width / 2);
            int orig_y = static_cast<int>(-sine * (x - width / 2) + cosine * (y - height / 2) + height / 2);

            if (orig_x >= 0 && orig_x < width && orig_y >= 0 && orig_y < height) {
                int W[4][2] = {
                    {orig_x, orig_y},
                    {std::min(orig_x + 1, width - 1), orig_y},
                    {orig_x, std::min(orig_y + 1, height - 1)},
                    {std::min(orig_x + 1, width - 1), std::min(orig_y + 1, height - 1)}
                };

                // Obliczenie odleg�o�ci
                double d[4] = {
                    sqrt((x - W[0][0]) * (x - W[0][0]) + (y - W[0][1]) * (y - W[0][1])),
                    sqrt((x - W[1][0]) * (x - W[1][0]) + (y - W[1][1]) * (y - W[1][1])),
                    sqrt((x - W[2][0]) * (x - W[2][0]) + (y - W[2][1]) * (y - W[2][1])),
                    sqrt((x - W[3][0]) * (x - W[3][0]) + (y - W[3][1]) * (y - W[3][1]))
                };

                for (int i = 0; i < 4; i++) {
                    if (d[i] == 0) {
                        d[i] = 1;
                    }
                }

                // Obliczenie wag
                double sum_w = 0;
                for (int i = 0; i < 4; i++) {
                    sum_w += weights[i] / d[i];
                }

                double r = 0, g = 0, b = 0;
                for (int i = 0; i < 4; i++) {
                    r += srcImage.GetRed(W[i][0], W[i][1]) * (weights[i] / d[i]) / sum_w;
                    g += srcImage.GetGreen(W[i][0], W[i][1]) * (weights[i] / d[i]) / sum_w;
                    b += srcImage.GetBlue(W[i][0], W[i][1]) * (weights[i] / d[i]) / sum_w;
                }

                newImage.SetRGB(x, y, static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b));
            }
            else {
                newImage.SetRGB(x, y, 255, 255, 255);
            }
        }
    }
    return newImage;
}

void GUIMyFrame::OnSliderChange(wxCommandEvent& event) {
    int angle = AngleInput->GetValue();
    AngleValue->SetLabel(wxString::Format("%d�", angle));
}

double GUIMyFrame::CalculateError(const wxImage& original, const wxImage& transformed) {
    int width = original.GetWidth();
    int height = original.GetHeight();
    double error = 0.0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r1 = original.GetRed(x, y);
            int g1 = original.GetGreen(x, y);
            int b1 = original.GetBlue(x, y);

            int r2 = transformed.GetRed(x, y);
            int g2 = transformed.GetGreen(x, y);
            int b2 = transformed.GetBlue(x, y);

            error += pow(r1 - r2, 2) + pow(g1 - g2, 2) + pow(b1 - b2, 2);
        }
    }
    return error;
}

void GUIMyFrame::OptimizeWeights(double(&weights)[4], double angle) {
    const double learningRate = 0.01;
    const int maxIterations = 100;
    const double tolerance = 1e-6;

    double bestWeights[4];
    std::copy(std::begin(weights), std::end(weights), std::begin(bestWeights));
    double bestError = std::numeric_limits<double>::max();

    //Wyczyszczenie wektora bledow
    errorTab.clear();

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        wxImage transformed = InterpolateImage(Img_Org, angle, bestWeights);
        double error = CalculateError(Img_Org, transformed);

        //Zapisanie bledow do tablicy
        errorTab.push_back(error / 10000000);

        if (error < bestError) {
            bestError = error;
            std::copy(std::begin(bestWeights), std::end(bestWeights), std::begin(weights));
        }

        double gradients[4];
        for (int i = 0; i < 4; ++i) {
            bestWeights[i] += tolerance;
            wxImage transformedPlus = InterpolateImage(Img_Org, angle, bestWeights);
            double errorPlus = CalculateError(Img_Org, transformedPlus);

            gradients[i] = (errorPlus - error) / tolerance;
            bestWeights[i] -= tolerance;
        }

        for (int i = 0; i < 4; ++i) {
            bestWeights[i] -= learningRate * gradients[i];
        }

        if (sqrt(std::inner_product(std::begin(gradients), std::end(gradients), std::begin(gradients), 0.0)) < tolerance) {
            break;
        }
    }
}
void GUIMyFrame::DrawGraph(wxDC& dc) {
    wxSize size = BottomPanel->GetClientSize();
    int width = size.GetWidth();
    int height = size.GetHeight();

    // Ustawienie osi
    int originX = 50;
    int originY = height - 50;
    int chartHeight = originY - 10;

    // Zakresy osi
    double xMax = 100.0; // Oś X kończy się na 100
    double yMax = 100.0; // Zakres osi Y




    if (errorTab.empty()) {
        return;
    }

    dc.SetPen(*wxBLACK_PEN);
    dc.DrawLine(originX, 10, originX, height - 40); // Oś Y
    dc.DrawLine(50, originY, width - 10, originY); // Oś X

    //Rysowanie oznaczen osi X
    for (int x = 0; x <= 100; x += 10) {
        int drawX = originX + static_cast<int>(x * (width - originX) / xMax) - (x / 2);
        dc.DrawLine(drawX, originY - 5, drawX, originY + 5); // Małe pionowe kreski
        dc.DrawText(wxString::Format("%d", x), drawX - 5, originY + 10); // Wartości osi X
    }

    //Maksymalna wartosc bledu
    yMax = *std::max_element(errorTab.begin(), errorTab.end());
    if (yMax == 0) yMax = 1;

    for (int i = 0; i <= 10; ++i) {
        double value = (i * yMax / 10); // Wartość na osi Y
        int drawY = originY - static_cast<int>((value / yMax) * (height - 60));
        dc.DrawLine(originX - 5, drawY, originX + 5, drawY); // Małe poziome kreski
        dc.DrawText(wxString::Format("%.1f", value), originX - 40, drawY - 5); // Wartości osi Y
    }

    // Rysowanie funkcji błędów
    dc.SetPen(*wxBLACK_PEN);
    int drawXLast = originX;
    int drawYLast = originY - static_cast<int>((errorTab[0] / yMax) * (height - 60));;
    for (size_t i = 0; i < errorTab.size() && i < 100; ++i) {
        int drawX = originX + static_cast<int>(i * ((width - originX) / xMax) - (i / 2));
        int drawY = originY - static_cast<int>((errorTab[i] / yMax) * (height - 60)); // Przeskalowanie punktów
        
        if (drawX < width && drawY < height && drawY >= 0) {
            //dc.DrawPoint(drawX, drawY);
            dc.DrawLine(drawXLast, drawYLast, drawX, drawY);
        }
        drawXLast = drawX;
        drawYLast = drawY;
    }
}

void GUIMyFrame::OnPaintGraph(wxPaintEvent& event) {
    wxPaintDC dc(BottomPanel);
    DrawGraph(dc);
}

void GUIMyFrame::DrawGraphAfterRotation() {
    if (BottomPanel) {
        wxClientDC dc(BottomPanel);
        DrawGraph(dc); // Użyj wxClientDC zamiast wxPaintDC
    }
}

void GUIMyFrame::OnResize(wxSizeEvent& event) {
    BottomPanel->Refresh();
    event.Skip(); // Pozwól na dalsze przetwarzanie zdarzenia
}