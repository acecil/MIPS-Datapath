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

#include "ScrolledWindow.h"

#include "GLCanvas.h"

BEGIN_EVENT_TABLE(ScrolledWindow, wxScrolledWindow)
	EVT_SCROLL(ScrolledWindow::OnScroll)
	EVT_ERASE_BACKGROUND(ScrolledWindow::OnEraseBackground)
END_EVENT_TABLE()

ScrolledWindow::ScrolledWindow(wxWindow *parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: wxScrolledWindow(parent, winid, pos, size, style, name),
	glCanvas(NULL)
{
	/* Nothing else to do. */
}

void ScrolledWindow::SetCanvas(GLCanvas *canvas)
{
	glCanvas = canvas;
	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(glCanvas);
	SetSizer(sizer);
}

void ScrolledWindow::OnScroll(wxScrollEvent &event)
{
	glCanvas->Render();
}

void ScrolledWindow::OnEraseBackground(wxEraseEvent &event)
{
	/* Do nothing to prevent flicker. */
}
