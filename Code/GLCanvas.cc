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

#include <cmath>

#include "Frame.h"
#include "Model.h"
#include "Component.h"
#include "Link.h"

#include "GLCanvas.h"

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
  EVT_SIZE(GLCanvas::OnSize)
  EVT_PAINT(GLCanvas::OnPaint)
  EVT_LEFT_UP(GLCanvas::OnLeftClick)
  EVT_RIGHT_UP(GLCanvas::OnRightClick)
  EVT_MOTION(GLCanvas::OnMouseMotion)
  EVT_ERASE_BACKGROUND(GLCanvas::OnEraseBackground)
END_EVENT_TABLE()

const double GLCanvas::defaultWidth = 250.0;
const double GLCanvas::defaultHeight = 174.0;
const GLint GLCanvas::canvasWidth = 1150;
const GLint GLCanvas::canvasHeight = 870;
wxTipWindow* GLCanvas::tipWin = NULL;

GLCanvas::GLCanvas(Model* proc, wxWindow* parent, Frame* frame, wxWindowID id, const wxPoint& pos, 
	const wxSize& size, long style, const wxString& name, int* attribList, 
	const wxPalette& palette):
	wxGLCanvas(parent, id, attribList, pos, wxSize(canvasWidth, canvasHeight)),
	glContext(wxGLContext(this))
{
	this->frame = frame;
	init = false;
	processor = proc;
	scale = 1.0;
	
	processor->setup();
}

void GLCanvas::Render()
{
	if( !IsShownOnScreen() )
	{
		/* Cannot render to hidden GL canvas. */
		return;
	}

	SetCurrent(glContext);
  	if (!init) {
    	InitGL();
    	init = true;
  	}
  	 
  	glClear(GL_COLOR_BUFFER_BIT);
   	glViewport(0, 0, size.x, size.y);
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	glOrtho(0, size.x, 0, size.y, -1, 1); 
  	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(canvasWidth / defaultWidth * scale, canvasHeight / defaultHeight * scale, 1.0);
	processor->draw(GetMousePosition());

	glFlush();
  	SwapBuffers();
}

void GLCanvas::SetZoom(int zoom)
{
	scale = std::pow(0.98, static_cast<double>(zoom));
	SetSize(canvasWidth * scale, canvasHeight * scale);
}

wxSize GLCanvas::GetCanvasSize()
{
	return wxSize(canvasWidth, canvasHeight);
}

void GLCanvas::InitGL()
{
  	SetCurrent(glContext);
  	glDrawBuffer(GL_BACK);
  	glClearColor(1.0, 1.0, 1.0, 0.0);
   	glViewport(0, 0, size.x, size.y);
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	glOrtho(0, size.x, 0, size.y, -1, 1); 
  	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  	// To antialias connectors...
  	glShadeModel(GL_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLCanvas::OnPaint(wxPaintEvent& event)
{		
	wxPaintDC dc(this); // required for correct refreshing under MS windows
	Render();
}

void GLCanvas::OnSize(wxSizeEvent& event)
{
  	size = this->GetSize();
	event.Skip();
}

wxPoint GLCanvas::GetMousePosition()
{
	wxPoint mousePos = wxGetMousePosition();
	wxPoint scaledPos;
	scaledPos.x = (int)((mousePos.x - GetScreenPosition().x) * defaultWidth / canvasWidth / scale);
	scaledPos.y = (int)((size.y - mousePos.y + GetScreenPosition().y) * defaultHeight / canvasHeight / scale);
	return scaledPos;
}

wxPoint GLCanvas::convertScreenToMouseCoord(wxPoint pos)
{
	wxPoint scaledPos;
	scaledPos.x = (int)(pos.x * canvasWidth / defaultWidth * scale + GetScreenPosition().x);
	scaledPos.y = (int)(size.y - (pos.y * canvasHeight / defaultHeight * scale) + GetScreenPosition().y);
	return scaledPos;
}

void GLCanvas::OnLeftClick(wxMouseEvent& event)
{
	processor->step();
	frame->updateDataList();
 	Render();
}

void GLCanvas::OnRightClick(wxMouseEvent& event)
{
	frame->showHideLeftPanel(true, true);
	Render();
}

void GLCanvas::OnMouseMotion(wxMouseEvent& event)
{
	popUpString.Clear();
	if(Model::getBool(SHOW_POPUPS))
	{
		// Display popup if the mouse is over a component.
		// Set bounding rectangle to hide popup once the mouse leave the bounds of that component.
		Component *comp = processor->findComponent(GetMousePosition());
		// Find link if no component found.
		Link *ln = (comp == 0) ? processor->findLink(GetMousePosition()) : 0;
		if(Model::getBool(SHOW_CONTROL_LINES) || !(((comp == 0) ? false : comp->getIsControl()) || ((ln == 0) ? false : ln->getIsControl())))
		{
			if(Model::getBool(SHOW_PC_LINES) || !(((comp == 0) ? false : comp->getIsPC()) || ((ln == 0) ? false : ln->getIsPC())))
			{
				if(ln != 0 && comp == 0 && ln != prevTipLink)
				{
					Component* lnComp = ln->getComp();
					if(lnComp->getType() == IFID_REG_TYPE || lnComp->getType() == IDEX_REG_TYPE || lnComp->getType() == EXMEM_REG_TYPE || lnComp->getType() == MEMWB_REG_TYPE)
					{
						// maybe put something here
					}
					else
					{
						popUpString += ln->getName() + _T(": ");
					}
					if(lnComp->getType() == INSTRUCTION_MEMORY_TYPE && ln->getSlot() == 1)
					{
						InstructionMemory* instrMem = (InstructionMemory*)(lnComp);
						// This is the instruction output of the instruction memory;
						if(processor->getBool(SHOW_INSTRUCTION_FIELDS))
						{
							wxString val;
							processor->getFieldedInstruction(instrMem->getOutputAddress(), val);
							popUpString += val;
						}
						else
						{
							popUpString += Maths::convertToBase(instrMem->getLink(1)->getVal(), true);
						}
					}
					else
					{
						popUpString += ln->getInfo();
					}
					// Translate the component coordinate system to screen coordinates.
					wxPoint linkPos = wxPoint(ln->getTipPos(LEFT), ln->getTipPos(BOTTOM));
					wxPoint pos = convertScreenToMouseCoord(linkPos);
					wxPoint topLinkPos = wxPoint(ln->getTipPos(RIGHT), ln->getTipPos(TOP));
					wxPoint topPos = convertScreenToMouseCoord(topLinkPos);
					tipWin = NULL;
					if(tipWin)
					{
						tipWin->SetTipWindowPtr(NULL);
						tipWin->Close();
					}
					tipWin = new wxTipWindow(this, popUpString, 100, &tipWin, new wxRect(pos.x, topPos.y, topPos.x - pos.x, pos.y - topPos.y));

					prevTipComp = 0;
					prevTipLink = ln;	
				}
				// Only update tooltip if the component found is different from the previous.
				else if(comp != 0 && comp != prevTipComp)
				{
					wxPoint compPos, topCompPos;
					if(comp->getType() == IFID_REG_TYPE || comp->getType() == IDEX_REG_TYPE || comp->getType() == EXMEM_REG_TYPE || comp->getType() == MEMWB_REG_TYPE)
					{
						PipelineRegister* buffer = (PipelineRegister*)comp;
						popUpString = buffer->getMainInfo(GetMousePosition());
						if(GetMousePosition().x < (comp->getX() + (comp->getW() / 2)))
						{
							// left half
							compPos = wxPoint(comp->getX(), comp->getY());
							topCompPos = wxPoint(comp->getX() + (comp->getW() / 2), comp->getY() + comp->getH());
						}
						else
						{
							// right half
							compPos = wxPoint(comp->getX() + (comp->getW() / 2), comp->getY());
							topCompPos = wxPoint(comp->getX() + comp->getW(), comp->getY() + comp->getH());
						}
					}
					else 
					{
						if(comp->getType() == INSTRUCTION_MEMORY_TYPE)
						{
							InstructionMemory* instrMem = (InstructionMemory*)comp;
							popUpString += _T("Address: ") + Maths::convertToBase(instrMem->getLink(0)->getVal()) + _T("\n");
							popUpString += _T("Instruction: ");
							if(processor->getBool(SHOW_INSTRUCTION_FIELDS))
							{
								wxString val;
								processor->getFieldedInstruction(instrMem->getOutputAddress(), val);
								popUpString += 	val;
							}
							else
							{
								popUpString += Maths::convertToBase(instrMem->getLink(1)->getVal(), true);
							}
						}
						else
						{
							popUpString = comp->getMainInfo();
						}
						// Translate the component coordinate system to screen coordinates.
						compPos = wxPoint(comp->getX(), comp->getY());
						topCompPos = wxPoint(comp->getX() + comp->getW(), comp->getY() + comp->getH());
					}
					wxPoint pos = convertScreenToMouseCoord(compPos);
					wxPoint topPos = convertScreenToMouseCoord(topCompPos);
					tipWin = new wxTipWindow(this, popUpString, 500, NULL, new wxRect(pos.x, topPos.y, topPos.x - pos.x, pos.y - topPos.y));
					prevTipComp = comp;
					prevTipLink = 0;
				}
				else
				{
					// Makes sure that when the mouse moves from component to blank and then back to same 
					// component a change in component is registered causing a tooltip to be displayed.
					prevTipComp = 0;
					prevTipLink = 0;
				}
			}
		}
	}
	event.Skip();
}

void GLCanvas::OnEraseBackground(wxEraseEvent &event)
{
	/* Do nothing to prevent flicker. */
}

