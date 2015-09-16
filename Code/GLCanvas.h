/*
*  
*  MIPS-Datapath - Graphical MIPS CPU Simulator.
*  Copyright 2008, 2012 Andrew Gascoyne-Cecil.
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

#ifndef GL_CANVAS_H_
#define GL_CANVAS_H_

#include <string>

#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/glcanvas.h>
#include <wx/string.h>
#include <wx/tipwin.h>

#include "Enums.h"

class Frame;
class Model;
class Component;
class Link;

class GLCanvas : public wxGLCanvas
{
	typedef enum _Layout Layout;
public:
	GLCanvas(Model* proc,
		wxWindow* parent,
		Frame* frame, 
		wxWindowID id = wxID_ANY, 
		const wxPoint& pos = wxDefaultPosition, 
		long style=0, 
		const wxString& name = _T("GLCanvas"), 					
		int* attribList = 0,
		const wxPalette& palette = wxNullPalette);
	// constructor
	void Render();
	void SetZoom(int zoom);
	wxSize GetCanvasSize();
	double GetScale() { return scale; }
private:
	void InitGL();                     // function to initialise GL context
	void OnPaint(wxPaintEvent& event); // callback for when canvas is exposed
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftClick(wxMouseEvent& event);
	void OnRightClick(wxMouseEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
	void OnWheel(wxMouseEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	wxPoint GetMousePosition() const;
	wxPoint convertMouseToScreenCoord(wxPoint pos) const;
	wxPoint convertScreenToMouseCoord(wxPoint pos) const;
	bool init;
	Model *processor;
	static const double defaultScale;
	double scale;
	wxPoint offset;
	wxPoint currPos;
	wxPoint downPos;
	wxTipWindow* tipWin;
	wxString popUpString;
	Component* prevTipComp;
	Link* prevTipLink;
	Frame* frame;
	wxGLContext glContext;

	DECLARE_EVENT_TABLE()
};

#endif /*GL_CANVAS_H_*/
