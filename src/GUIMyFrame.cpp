#include "GUIMyFrame.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include <gsl/gsl_math.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>


double outside(double a, double b, double c, double d, GUIMyFrame* frame, double angle) {

    double wagi[4] = { a, b, c, d };
    wxImage one = frame->rotateIMG(frame->Img_Org, angle, wagi);
    wxImage reverse = frame->ReverseRotate(one, angle, wagi);
    return frame->CalculateError(frame->Img_Org, reverse);
}


struct data {
    size_t n;
    double* y;
    double* sigma;
    GUIMyFrame* frame; // Dodaj wskaźnik do instancji GUIMyFrame
    double angle;
};

 int expb_f(const gsl_vector* x, void* data, gsl_vector* f) {
     struct data* d = (struct data*)data;
     size_t n = d->n;
     double* y = d->y;
     GUIMyFrame* frame = d->frame; // Pobierz wskaźnik do GUIMyFrame
     double angle = d->angle;

    double a = gsl_vector_get(x, 0);
    double b = gsl_vector_get(x, 1);
    double c = gsl_vector_get(x, 2);
    double d1 = gsl_vector_get(x, 3);

    for (size_t i = 0; i < n; i++) {
        double Yi = outside(a,b,c,d1, frame, angle) ; // Przykładowa funkcja zależna od wag
        gsl_vector_set(f, i, (Yi - y[i]));
    }

    return GSL_SUCCESS;
}

 int expb_df(const gsl_vector* x, void* data, gsl_matrix* J) {
    size_t n = ((struct data*)data)->n;
    double a = gsl_vector_get(x, 0);
    double b = gsl_vector_get(x, 1);
    double c = gsl_vector_get(x, 2);
    double d = gsl_vector_get(x, 3);

    for (size_t i = 0; i < n; i++) {
        gsl_matrix_set(J, i, 0, 1.0);
        gsl_matrix_set(J, i, 1, i);
        gsl_matrix_set(J, i, 2, i * i);
        gsl_matrix_set(J, i, 3, i * i * i);
    }

    return GSL_SUCCESS;
}

 int expb_fdf(const gsl_vector* x, void* data, gsl_vector* f, gsl_matrix* J) {
    expb_f(x, data, f);
    expb_df(x, data, J);
    return GSL_SUCCESS;
}


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

void GUIMyFrame::OnSliderChange(wxCommandEvent& event) {
    int angle = AngleInput->GetValue();
    AngleValue->SetLabel(wxString::Format("%d�", angle));
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

    // Pobierz rozmiary panelu
    int panelWidth = MainPanel->GetClientSize().GetWidth();
    int panelHeight = MainPanel->GetClientSize().GetHeight();

    // Zmniejsz rozmiary panelu o 10% marginesu
    int availableWidth = static_cast<int>(panelWidth * 0.9);
    int availableHeight = static_cast<int>(panelHeight * 0.9);

    // Stwórz nowy obraz z białym tłem
    wxImage newImage(availableWidth, availableHeight, true);
    newImage.SetRGB(wxRect(0, 0, availableWidth, availableHeight), 255, 255, 255);

    // Pobierz rozmiary oryginalnego obrazu
    int imgWidth = Img_Org.GetWidth();
    int imgHeight = Img_Org.GetHeight();

    // Oblicz współczynniki proporcji dla szerokości i wysokości
    double widthRatio = static_cast<double>(availableWidth) / (imgWidth * 1.5);
    double heightRatio = static_cast<double>(availableHeight) / (imgHeight * 1.5);
    double scaleFactor = std::min(widthRatio, heightRatio);

    // Oblicz nowe wymiary obrazu
    int newWidth = static_cast<int>(imgWidth * scaleFactor);
    int newHeight = static_cast<int>(imgHeight * scaleFactor);

    // Przeskalowanie obrazu przy użyciu nowych wymiarów
    wxImage scaledImg = Img_Org.Scale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH);

    // Oblicz wymiary nowego obrazu z białym tłem, aby zmieścić obraz po obrocie o 45 stopni
    int diagonal = static_cast<int>(sqrt(newWidth * newWidth + newHeight * newHeight));
    int whiteSpaceWidth = diagonal;
    int whiteSpaceHeight = diagonal;

    // Skaluj przestrzeń białego tła, aby zmieściła się w MainPanel z marginesem
    widthRatio = static_cast<double>(availableWidth) / whiteSpaceWidth;
    heightRatio = static_cast<double>(availableHeight) / whiteSpaceHeight;
    scaleFactor = std::min(widthRatio, heightRatio);

    int finalWidth = static_cast<int>(whiteSpaceWidth * scaleFactor);
    int finalHeight = static_cast<int>(whiteSpaceHeight * scaleFactor);



    // Oblicz pozycję, w której należy umieścić przeskalowany obraz na nowym białym tle
    int posX = (finalWidth - newWidth) / 2;
    int posY = (finalHeight - newHeight) / 2;

    // Skopiuj przeskalowany obraz na nowe białe tło
    newImage.Paste(scaledImg, posX, posY);

    Img_Org = newImage;
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
        DrawGraphAfterRotation();

        OptimizeWeights(weights, angle);
        
        DrawGraphAfterRotation();
        MainPanel->Refresh();
        MainPanel->Update();

    }
}





//Obliczanie bledu
double GUIMyFrame::CalculateError(const wxImage& original, const wxImage& transformed) {
    int width = original.GetWidth();
    int height = original.GetHeight();
    double error = 0.0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            error += CalculateSingleError(original, transformed, x, y);
        }
    }
    return error / 1000000;
}


//Optymalizacja wag
void GUIMyFrame::OptimizeWeights(double(&weights)[4], double angle) {
    const size_t n = 10;
    double y[n];
    double sigma[n];
   
    // Ustawienia perturbacji
    double perturbation = 0.1; 
    
    // Wielkość perturbacji
    for (int i = 0; i < n; i++) {
        double perturbedWeights[4];
        for (int k = 0; k < 4; k++) {
            // Generowanie punktów początkowych z małą perturbacją
            perturbedWeights[k] = weights[k] + ((rand() % 1000 / 1000.0) * perturbation - (perturbation / 2));
        }
        wxImage basic = rotateIMG(Img_Cpy, angle, perturbedWeights);
        wxImage reverse = ReverseRotate(basic, angle, perturbedWeights);
        y[i] = CalculateError(Img_Org, reverse);
        sigma[i] = 0.1;
    }

    struct data d = { n, y, sigma, this, angle };
    wxLogMessage("Początkowe wagi: [%f, %f, %f, %f]", weights[0], weights[1], weights[2], weights[3]);

    gsl_multifit_function_fdf f;
    f.f = &expb_f;
    f.df = &expb_df;
    f.fdf = &expb_fdf;
    f.n = n;
    f.p = 4;
    f.params = &d;
    
    gsl_vector* x = gsl_vector_alloc(4);
    gsl_vector_set(x, 0, weights[0]);
    gsl_vector_set(x, 1, weights[1]);
    gsl_vector_set(x, 2, weights[2]);
    gsl_vector_set(x, 3, weights[3]);
    
    const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc(T, n, 4);
    int status = gsl_multifit_fdfsolver_set(s, &f, x);
    if (status != GSL_SUCCESS) {
        wxLogError("Błąd inicjalizacji solvera GSL: %s", gsl_strerror(status));
        gsl_multifit_fdfsolver_free(s);
        gsl_vector_free(x);
        return;
    }
    errorTab.clear();
    int iter = 0;
    do {
        iter++;
        status = gsl_multifit_fdfsolver_iterate(s);
        errorTab.push_back(gsl_vector_get(s->x, 0) + gsl_vector_get(s->x, 1) + gsl_vector_get(s->x, 2) + gsl_vector_get(s->x, 3));

        status = gsl_multifit_test_delta(s->dx, s->x, 1e-4, 1e-4);
        wxLogError("Liczba iteracji: %d", iter);
    } while (iter < 1000);

    if (status != GSL_SUCCESS && status != GSL_CONTINUE) {
        wxLogError("Solver nie zbiega się: %s", gsl_strerror(status));
        
    }

    for (int i = 0; i < 4; i++) {
        weights[i] = gsl_vector_get(s->x, i);
    }
    Img_Cpy = rotateIMG(Img_Org, angle, weights);
    Img_Cpy = ReverseRotate(Img_Cpy, angle, weights);

    gsl_multifit_fdfsolver_free(s);
    gsl_vector_free(x);
    
}


//Rysowanie wykresy funkcji
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
    MainPanel->Refresh();
    event.Skip(); // Pozwól na dalsze przetwarzanie zdarzenia
}


//Obraca w prawo
wxImage GUIMyFrame::rotateIMG(const wxBitmap& srcImage, double angle, double(&weights)[4])
{
    int width = srcImage.GetWidth();
    int height = srcImage.GetHeight();
    wxImage srcImg = srcImage.ConvertToImage();
    wxImage newImage(width, height);

    double radians = angle * M_PI / 180.0;
    double cosine = cos(radians);
    double sine = sin(radians);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            // Find the original coordinates
            int orig_x = static_cast<int>(cosine * (x - width / 2) + sine * (y - height / 2) + width / 2);
            int orig_y = static_cast<int>(-sine * (x - width / 2) + cosine * (y - height / 2) + height / 2);

            if (orig_x >= 0 && orig_x < width && orig_y >= 0 && orig_y < height) {
                //Oryginalne punkty
                int W[4][2] = {
                    {orig_x, orig_y},
                    {std::min(orig_x + 1, width - 1), orig_y},
                    {orig_x, std::min(orig_y + 1, height - 1)},
                    {std::min(orig_x + 1, width - 1), std::min(orig_y + 1, height - 1)}
                };

                // Calculate distances
                double d[4] = {
                    sqrt((x - W[0][0]) * (x - W[0][0]) + (y - W[0][1]) * (y - W[0][1])),
                    sqrt((x - W[1][0]) * (x - W[1][0]) + (y - W[1][1]) * (y - W[1][1])),
                    sqrt((x - W[2][0]) * (x - W[2][0]) + (y - W[2][1]) * (y - W[2][1])),
                    sqrt((x - W[3][0]) * (x - W[3][0]) + (y - W[3][1]) * (y - W[3][1]))
                };
                //Zapobieganie dzieleniu przez 0
                for (int i = 0; i < 4; i++) {
                    if (d[i] == 0) {
                        d[i] = 1;
                    }
                }

                for (int i = 0; i < 4; i++)
                    for (int j = 1; j < 3; j++)
                        if (d[j - 1] > d[j]) {
                            std::swap(d[j - 1], d[j]);
                        }



                double r = 0, g = 0, b = 0;
                for (int i = 0; i < 4; i++) {
                    r += srcImg.GetRed(W[i][0], W[i][1]) * (weights[i] / d[i]) * d[0];
                    g += srcImg.GetGreen(W[i][0], W[i][1]) * (weights[i] / d[i]) * d[0];
                    b += srcImg.GetBlue(W[i][0], W[i][1]) * (weights[i] / d[i]) * d[0];
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
//Obraca spowrotem
wxImage GUIMyFrame::ReverseRotate(const wxImage& srcImage, double angle, double(&weights)[4]) {
    int width = srcImage.GetWidth();
    int height = srcImage.GetHeight();
    wxImage newImage(width, height);

    double radians = -angle * M_PI / 180.0; // Reverse angle
    double cosine = cos(radians);
    double sine = sin(radians);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            // Find the original coordinates
            int orig_x = static_cast<int>(cosine * (x - width / 2) + sine * (y - height / 2) + width / 2);
            int orig_y = static_cast<int>(-sine * (x - width / 2) + cosine * (y - height / 2) + height / 2);

            if (orig_x >= 0 && orig_x < width && orig_y >= 0 && orig_y < height) {
                //Oryginalne punkty
                int W[4][2] = {
                    {orig_x, orig_y},
                    {std::min(orig_x + 1, width - 1), orig_y},
                    {orig_x, std::min(orig_y + 1, height - 1)},
                    {std::min(orig_x + 1, width - 1), std::min(orig_y + 1, height - 1)}
                };

                // Calculate distances
                double d[4] = {
                    sqrt((x - W[0][0]) * (x - W[0][0]) + (y - W[0][1]) * (y - W[0][1])),
                    sqrt((x - W[1][0]) * (x - W[1][0]) + (y - W[1][1]) * (y - W[1][1])),
                    sqrt((x - W[2][0]) * (x - W[2][0]) + (y - W[2][1]) * (y - W[2][1])),
                    sqrt((x - W[3][0]) * (x - W[3][0]) + (y - W[3][1]) * (y - W[3][1]))
                };
                //Zapobieganie dzieleniu przez 0
                for (int i = 0; i < 4; i++) {
                    if (d[i] == 0) {
                        d[i] = 1;
                    }
                }

                for (int i = 0; i < 4; i++)
                    for (int j = 1; j < 3; j++)
                        if (d[j - 1] > d[j]) {
                            std::swap(d[j - 1], d[j]);
                        }

                double r = 0, g = 0, b = 0;
                for (int i = 0; i < 4; i++) {
                    r += srcImage.GetRed(W[i][0], W[i][1]) * (weights[i] / d[i]) * d[0];
                    g += srcImage.GetGreen(W[i][0], W[i][1]) * (weights[i] / d[i]) * d[0];
                    b += srcImage.GetBlue(W[i][0], W[i][1]) * (weights[i] / d[i]) * d[0];
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



double GUIMyFrame::CalculateSingleError(const wxImage& basicImg, const wxImage& rotatedImg, int _x, int _y) {
    double singleError, r, g, b;
    r = abs(basicImg.GetRed(_x, _y) - rotatedImg.GetRed(_x, _y));
    g = abs(basicImg.GetGreen(_x, _y) - rotatedImg.GetGreen(_x, _y));
    b = abs(basicImg.GetBlue(_x, _y) - rotatedImg.GetBlue(_x, _y));
    return r + g + b;
}