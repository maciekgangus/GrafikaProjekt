#ifndef __GUIMyFrame__
#define __GUIMyFrame__



#include "MyFrame.h"
#include <wx/dcmemory.h>



class GUIMyFrame : public MyFrame
{
protected:
        void Repaint(wxDC& dc);
        void OnPaint(wxPaintEvent& event);
        void OnOpen(wxCommandEvent& event);
        void OnRotate(wxCommandEvent& event);
        void OnSliderChange(wxCommandEvent& event);
        wxImage InterpolateImage(const wxImage &srcImage, double angle, double (&weights)[4]);
        wxImage Img_Org, Img_Cpy;      // wszystkie zmiany beda wykonywane na tej kopii obrazka


public:
		GUIMyFrame(wxWindow* parent );
        wxDECLARE_EVENT_TABLE();


};

#endif
