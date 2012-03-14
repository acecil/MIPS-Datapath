/*
 *  
 *  MIPS-Datapath - Graphical MIPS CPU Simulator.
 *  Copyright 2012 Andrew Gascoyne-Cecil.
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact: gascoyne+mips@gmail.com
 * 
 */
 
#ifndef SCROLLEDWINDOW_H_
#define SCROLLEDWINDOW_H_

#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/scrolwin.h>

class MyGLCanvas;

class ScrolledWindow : public wxScrolledWindow
{
	public:
		ScrolledWindow(wxWindow *parent,
                     wxWindowID winid = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxScrolledWindowStyle,
                     const wxString& name = wxPanelNameStr);
		void SetCanvas(MyGLCanvas *canvas);
		
		void OnScroll(wxScrollWinEvent &event);
		void OnEraseBackground(wxEraseEvent &event);
	private:
		MyGLCanvas *glCanvas;
		 		
  		DECLARE_EVENT_TABLE()
};

#endif /*SCROLLEDWINDOW_H_*/