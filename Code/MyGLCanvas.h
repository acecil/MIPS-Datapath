/*
 *  
 *  MIPS-Datapath - Graphical MIPS CPU Simulator.
 *  Copyright 2008 Andrew Gascoyne-Cecil.
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
 
#ifndef MYGLCANVAS_H_
#define MYGLCANVAS_H_

#include "Enums.h"
#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/glcanvas.h>
#include <wx/string.h>
#include <wx/tipwin.h>
#include <string>


class MyFrame;
class Model;
class Component;
class Link;

class MyGLCanvas : public wxGLCanvas
{
	typedef enum _Layout Layout;
	public:
		MyGLCanvas(Model* proc,
					wxWindow* parent,
					MyFrame* frame, 
					wxWindowID id = wxID_ANY, 
					const wxPoint& pos = wxDefaultPosition, 
					const wxSize& size = wxDefaultSize, 
					long style=0, 
					const wxString& name = _T("MyGLCanvas"), 					
					int* attribList = 0,
					const wxPalette& palette = wxNullPalette);
		 // constructor
	    void Render();
	private:
	  	void InitGL();                     // function to initialise GL context
 		void OnSize(wxSizeEvent& event);   // callback for when canvas is resized
  		void OnPaint(wxPaintEvent& event); // callback for when canvas is exposed
  		void OnLeftClick(wxMouseEvent& event);
		void OnLeftDClick(wxMouseEvent& event);
  		void OnRightClick(wxMouseEvent& event);
		void OnRightDClick(wxMouseEvent& event);
  		void OnMouseMotion(wxMouseEvent& event);
		void OnMouseWheel(wxMouseEvent& event);
  		wxPoint GetMousePosition();
  		wxPoint convertScreenToMouseCoord(wxPoint pos);
  		bool init;
   		//wxGLContext *context;
  		Model *processor;
  		wxSize size;
		double scale;
  		static const double defaultWidth;
  		static const double defaultHeight;
  		static const GLint canvasWidth;
  		static const GLint canvasHeight;
  		static wxTipWindow* tipWin;
  		wxString popUpString;
  		Component* prevTipComp;
  		Link* prevTipLink;
  		MyFrame* frame;
 		
  		DECLARE_EVENT_TABLE()
};

#endif /*MYGLCANVAS_H_*/
