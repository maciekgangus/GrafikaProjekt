#ifndef __GUIMyFrame__
#define __GUIMyFrame__



#include "MyFrame.h"
#include <wx/dcmemory.h>
#include <vector>



class GUIMyFrame : public MyFrame
{
private:
    void OnPaintGraph(wxPaintEvent& event); // Dodano deklaracj� metody
protected:
        void Repaint(wxDC& dc);
        void OnPaint(wxPaintEvent& event);
        void OnOpen(wxCommandEvent& event);
        void OnRotate(wxCommandEvent& event);
        void OnSliderChange(wxCommandEvent& event);
        double CalculateError(const wxImage& original, const wxImage& transformed);
        void OptimizeWeights(double(&weights)[4], double angle);
        void DrawGraph(wxDC& dc);
        void DrawGraphAfterRotation();
        void OnResize(wxSizeEvent& event);
        wxImage InterpolateImage(const wxImage &srcImage, double angle, double (&weights)[4]);
        wxImage Img_Org, Img_Cpy;      // wszystkie zmiany beda wykonywane na tej kopii obrazka
        std::vector <double> errorTab;
public:
		GUIMyFrame(wxWindow* parent );
        wxDECLARE_EVENT_TABLE();


};

#endif
