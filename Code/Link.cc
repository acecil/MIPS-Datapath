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

#include "Component.h"

#include "Link.h"

int Link::count = 0;
const int Link::LINK_WIDTH = 4;
const int Link::LINK_HEIGHT_SHORT = 2;
const int Link::LINK_HEIGHT_TALL = 4;

Link::Link(int slot, Component* comp, double x, double y, Side edge, bool output, bool control, bool PC, bool showText, wxString name, double textX, double textY)
{
	this->pos.x = x;
	this->pos.y = y;
	this->textPos = Coord(textX, textY);
	this->edge = edge;
	ID = count;
	count++;
	this->output = output;
	this->control = control;
	this->PC = PC;
	this->comp = comp;
	this->slot = slot;
	this->value = 0;
	this->showText = showText;
	this->name = name;
	active = false;
	negBit16 = false;
	instructionActive[SYM_BAD] = false; // So when no instr specified this test is effectively ignored.
	instructionActive[SYM_ADD] = false;
	instructionActive[SYM_ADDI] = false;
	instructionActive[SYM_NOOP] = false;
	instructionActive[SYM_LW] = false;
	instructionActive[SYM_SW] = false;
	instructionActive[SYM_AND] = false;
	instructionActive[SYM_OR] = false;
	instructionActive[SYM_SUB] = false;
	instructionActive[SYM_SLT] = false;
	instructionActive[SYM_BEQ] = false;
	
}

InputLink::InputLink(int slot, Component* comp, double x, double y, Side edge, wxString name, bool control, bool PC, bool showText, uint startBit, uint bitLength, double textX, double textY)
: Link(slot, comp, x, y, edge, false, control, PC, showText, name, textX, textY)
{
	connected = false;
	this->startBit = startBit;
	this->bitLength = bitLength;
}

OutputLink::OutputLink(int slot, Component* comp, double x, double y, Side edge, wxString name, bool control, bool PC, bool showText, double textX, double textY)
: Link(slot, comp, x, y, edge, true, control, PC, showText, name, textX, textY)
{
}

void Link::addVertex(double x, double y)
{ 
	this->vertices.push_back(Coord(x, y)); 
}

void Link::setVertexList(const std::vector<Coord> &newVertices)
{
	vertices.clear();
	vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
	if(!vertices.empty())
	{
		Coord& backVertex = vertices.back();
		if(edge == TOP || edge == BOTTOM)
		{
			backVertex.x = getAbsX();
		}
		else
		{
			backVertex.y = getAbsY();
		}
	}
}

int Link::getAbsX()
{
	return (int)(getX() * getComp()->getW() + getComp()->getX());
}

int Link::getAbsY()
{
	return (int)(getY() * getComp()->getH() + getComp()->getY());
}

wxString Link::getInfo()
{
	return Maths::convertToBase(getVal(), false, negBit16);
}
		
int Link::getTipPos(Side s)
{
	int left, right, top, bottom;
	int LINK_HEIGHT = isOutput() ? LINK_HEIGHT_SHORT : LINK_HEIGHT_TALL;
	left = LINK_WIDTH / 2;
	right = LINK_WIDTH / 2;
	top = LINK_HEIGHT / 2;
	bottom = LINK_HEIGHT / 2;
	if(getSide() == LEFT)
	{
		left = LINK_WIDTH;
		right = 0;
	}
	else if(getSide() == RIGHT)
	{
		left = 0;
		right = LINK_WIDTH;
	}
	else if(getSide() == TOP)
	{
		top = LINK_HEIGHT;
		bottom = 0;
	}
	else if(getSide() == BOTTOM)
	{
		top = 0;
		bottom = LINK_HEIGHT;
	}
	switch(s)
	{
		case LEFT:
			return getAbsX() - left;
		case RIGHT:
			return getAbsX() + right;
		case TOP:
			return getAbsY() + top;
		case BOTTOM:
			return getAbsY() - bottom;
		default:
			return 0;
	}
}
		
bool InputLink::isActive()
{
	if(isConnected())
	{
		return getOutput()->isActive();
	}
	else
	{
		return active;
	}
}

luint InputLink::getVal()
{
	// Get value from input.
	return extractRequiredBits(value);
}

void InputLink::getValFromOutput()
{
	if(connected)
	{
		setVal(connectedOutput->getVal()); 
	}
}

luint InputLink::extractRequiredBits(luint val)
{
	// Construct bit mask to extract the portion of the value required.
	luint bitMask = (((luint)(1)<<(bitLength)) - 1)<<startBit;
	luint maskedVal = val & bitMask;
	luint result = maskedVal>>startBit;

	return result;
}

luint OutputLink::getVal()
{
	if(getComp()->getType() == NODE_TYPE)
	{
		return getComp()->getVal();
	}
	else
	{
		return value;
	}
}

bool OutputLink::isConnected()
{
	return (linkList.size() > 0);
}

void Link::setInstrActivity(Symbol inst, bool active)
{
	if(inst == SYM_MAX)
	{
		instructionActive[SYM_ADD] = true;
		instructionActive[SYM_ADDI] = true;
		instructionActive[SYM_LW] = true;
		instructionActive[SYM_SW] = true;
		instructionActive[SYM_AND] = true;
		instructionActive[SYM_OR] = true;
		instructionActive[SYM_SUB] = true;
		instructionActive[SYM_SLT] = true;
		instructionActive[SYM_BEQ] = true;
	}
	else if(inst == SYM_EOF)
	{
		instructionActive[SYM_ADD] = true;
		instructionActive[SYM_AND] = true;
		instructionActive[SYM_OR] = true;
		instructionActive[SYM_SUB] = true;
		instructionActive[SYM_SLT] = true;
	}
	else
	{
		instructionActive[inst] = active;
	}
}

bool OutputLink::isActive()
{ 
	return getComp()->isActive();
}

bool Link::isActiveValid(Symbol instr)
{
	return instructionActive[instr];
}

void OutputLink::connect(Link* link)
{
	linkList.push_back(link);
	((InputLink*)link)->connectOutput(this);
}
