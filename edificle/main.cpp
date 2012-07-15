/**
edificle application object source
Copyright (C) 2012, Leszek Godlewski <lg@inequation.org>

\author Leszek Godlewski
*/

#include "main.h"
#include "MainFrame.h"

IMPLEMENT_APP(EdificleApp)

bool EdificleApp::OnInit() {
	if ( !wxApp::OnInit() )
        return false;

    wxFrame *frame = new MainFrame(NULL,
		wxID_ANY,
		wxT("wxAUI Sample Application"),
		wxDefaultPosition,
		wxSize(800, 600));
    frame->Show(true);

    return true;
}
