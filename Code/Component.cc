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
 
#include <exception>

#include "Link.h"
#include "Coord.h"
#include "Side.h"
#include "Color.h"

#include "Component.h"

class InvalidInputException : public std::exception
{
	virtual const char* what() const throw()
  	{
    	return "Invalid Input: ComponentID: ";
  	}

} InvalidInputEx;

int Component::count = 0;

uint Component::pipelineCycle = 0;

std::vector<Color> Component::activeLinkColors =
{
	Color(1.0, 0.0, 0.0, 1.0),
	Color(0.0, 1.0, 0.0, 1.0),
	Color(1.0, 0.0, 1.0, 1.0),
	Color(0.0, 0.0, 1.0, 1.0),
	Color(0.0, 1.0, 1.0, 1.0)
};
std::map<configName, Color> Component::colours;
bool Component::dataLinesBold = true;
bool Component::highlightSingleInstruction = true;
int Component::currentInstruction = 0;
const GLfloat Component::DATA_LINE_WIDTH = 2.0;
const GLfloat Component::THIN_DATA_LINE_WIDTH = 1.0;
const double Component::PI = 3.14159265358979323;
		
void Component::setActiveLinkColour(wxColour colour, uint stage)
{
	if(0 <= stage && stage < 5)
	{
		switch(stage)
		{
			case 0:
			setColour(STAGE1_COLOUR, colour);
			break;
			case 1:
			setColour(STAGE2_COLOUR, colour);
			break;
			case 2:
			setColour(STAGE3_COLOUR, colour);
			break;
			case 3:
			setColour(STAGE4_COLOUR, colour);
			break;
			case 4:
			setColour(STAGE5_COLOUR, colour);
			break;			
		}
	}
}

Color Component::getActiveLinkColour(uint stage)
{
	if(0 <= stage && stage < 5)
	{
		switch(stage)
		{
			case 0:
				return getColour(STAGE1_COLOUR);
			case 1:
				return getColour(STAGE2_COLOUR);
			case 2:
				return getColour(STAGE3_COLOUR);
			case 3:
				return getColour(STAGE4_COLOUR);
			case 4:
				return getColour(STAGE5_COLOUR);
		}
	}
	return Color(255, 255, 255, 255);
}

wxColour Component::getwxColour(configName name)
{
	float charM = 255.0;
	Color colour = colours[name];
	return wxColour(int(colour.c[0] * charM), int(colour.c[1] * charM), int(colour.c[2] * charM), 255);
}

void Component::setColour(configName name, wxColour colour)
{
	float charM = 255.0;
	colours[name] = Color(colour.Red() / charM, colour.Green() / charM, colour.Blue() / charM, 1.0);
}

void Component::preStep()
{
	// Darken link color for this component
	if(isActive())
	{
		oldActiveLinkColor = 0.7F;
	}
	if(type != NODE_TYPE)
	{
		for(auto i = linkList.begin(); i != linkList.end(); ++i)
		{
			auto& currLink = i->second;
			if(!(currLink->isOutput()))
			{
				InputLink* iLink = static_cast<InputLink*>(currLink.get());
				iLink->getValFromOutput();
			}
		}	
	}
}

Color Component::scaleColors(Color cMin, Color cMax, double proportion)
{
	double r = (cMax.c[0] - cMin.c[0]) * proportion + cMin.c[0];
	double g = (cMax.c[1] - cMin.c[1]) * proportion + cMin.c[1];
	double b = (cMax.c[2] - cMin.c[2]) * proportion + cMin.c[2];
	double a = (cMax.c[3] - cMin.c[3]) * proportion + cMin.c[3];
	return Color(r, g, b, a);
}

Component::Component(double x, double y, double w, double h, wxString name, ComponentType type, bool control, bool PC, double textPosX, double textPosY)
{
	this->pos.x = x;
	this->pos.y = y;
	this->w = w;
	this->h = h;
	this->name = name;
	this->type = type;
	this->control = control;
	this->PC = PC;

	this->textPos = Coord(textPosX, textPosY);
	activeOutputs = false;
	activeLinkColor = activeLinkColors[0];
	
	oldActiveLinkColor = 1.0;
	preActive = false;
	ID = count++;	
	pipelineStage = 0;
	
	Config &c = Config::Instance();
	setColour(COMPONENT_COLOUR, c.getColour(COMPONENT_COLOUR));
	setColour(PIPEREG_COLOUR, c.getColour(PIPEREG_COLOUR));
	setColour(STAGE1_COLOUR, c.getColour(STAGE1_COLOUR));
	setColour(STAGE2_COLOUR, c.getColour(STAGE2_COLOUR));
	setColour(STAGE3_COLOUR, c.getColour(STAGE3_COLOUR));
	setColour(STAGE4_COLOUR, c.getColour(STAGE4_COLOUR));
	setColour(STAGE5_COLOUR, c.getColour(STAGE5_COLOUR));
	setColour(TEXT_COLOUR, c.getColour(TEXT_COLOUR));
	setColour(BORDER_COLOUR, c.getColour(BORDER_COLOUR));
	setColour(BACKGROUND_COLOUR, c.getColour(BACKGROUND_COLOUR));
	setColour(LINK_COLOUR, c.getColour(LINK_COLOUR));
	setColour(FADE_COLOUR, c.getColour(FADE_COLOUR));
	
	setActiveLinkColour(c.getColour(STAGE1_COLOUR), 0);
	setActiveLinkColour(c.getColour(STAGE2_COLOUR), 1);
	setActiveLinkColour(c.getColour(STAGE3_COLOUR), 2);
	setActiveLinkColour(c.getColour(STAGE4_COLOUR), 3);
	setActiveLinkColour(c.getColour(STAGE5_COLOUR), 4);
	if(!(fillColorMin.init))
	{
		this->fillColorMin = colours[COMPONENT_COLOUR];
	}
}

void Component::setActive(bool active)
{
	preActive = active;
	if(!active)
	{
		oldActiveLinkColor = 1;
	}
}

wxString Component::getMainInfo()
{
	wxString inInfo, outInfo;
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto& currLink = i->second;
		if(!(currLink->isOutput()))
		{
			if(i != linkList.begin())
			{
				inInfo += _T("\n");
			}
			inInfo += getLinkInfo((*i).first, currLink.get());
		}
		else
		{
			outInfo += _T("\n");
			outInfo += getLinkInfo((*i).first, currLink.get());
		}
	}
	return inInfo + outInfo;
}

wxString Component::getLinkInfo(int ID, Link* currLink)
{
	wxString linkInfo;
	if(currLink->getName().IsEmpty())
	{
		linkInfo += wxString::Format(_T("%u"), ID);
	}
	else
	{
		linkInfo += currLink->getName();
	}
	linkInfo += _T(": ");
	// So that instruction is split into fields - does not do anything different yet.
	if(getType() == INSTRUCTION_MEMORY_TYPE && currLink->getName() == _T("Instruction"))
	{
		linkInfo += Maths::convertToBase(currLink->getVal(), false, currLink->getNegBit16());
	}
	else
	{
		linkInfo += Maths::convertToBase(currLink->getVal(), false, currLink->getNegBit16());
	}
	return linkInfo;
}

int Component::findInput(wxPoint mouseLocation)
{
	return findNearestLink(mouseLocation, false);
}

int Component::findOutput(wxPoint mouseLocation)
{
	return findNearestLink(mouseLocation, true);
}

int Component::findNearestLink(wxPoint mouseLocation, bool isOutput)
{
	int nearest = std::numeric_limits<int>::max();
	
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &currLink = i->second;
		bool output = currLink->isOutput() && isOutput;
		bool input = !(currLink->isOutput()) && !(isOutput);
		if(input || output)
		{
			double x = currLink->getX() * this->getW() + this->getX();
			double y = currLink->getY() * this->getH() + this->getY();
			double distX = mouseLocation.x - x;
			double distY = mouseLocation.y - y;
			double dist = sqrt(distX * distX + distY * distY);
			if(dist < nearest)
			{
				nearest = (int)dist;
				this->currentLink = currLink.get();
			}
		}
	}

	return nearest;
}

void Component::setLinkControl(bool c, int l)
{
	linkList[l]->setControl(c); 
}

void Component::position()
{
	glTranslatef(pos.x, pos.y, 0.0);
	glScalef(w, h, 1.0);
}

void Component::drawBackground()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	glColor4fv(colours[BACKGROUND_COLOUR].c);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(-1000, -1000);
	glVertex2f(2000, -1000);
	glVertex2f(-1000, 2000);
	glVertex2f(2000, 2000);
	glEnd();
	glPopMatrix();
	glPopAttrib();
}

void Component::drawName(bool showControl, bool showPC, double scale)
{
	void *font = GLUT_BITMAP_HELVETICA_10;
	if (scale > 1.8)
	{
		font = GLUT_BITMAP_HELVETICA_18;
	}
	else if (scale > 1.2)
	{
		font = GLUT_BITMAP_HELVETICA_12;
	}

	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	// Translate text to the top of the component.
	glColor4fv(colours[TEXT_COLOUR].c);
	glTranslatef(textPos.x, textPos.y, 0.0);
	glRasterPos2f(0, 0);
	double rpy = 0;
    for(uint i = 0; i < name.length(); i++)
    {
    	if(name[i] == '\n')
    	{
    		glRasterPos2f(0, (--rpy * 3) / getH());
    	}
    	else
    	{
			glutBitmapCharacter(font, name[i]);
    	}
	}
	glPopMatrix();
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &currLink = i->second;
		if(currLink->isShowText())
		{
			if(showControl || !(currLink->getIsControl()))
			{
				if(showPC || !(currLink->getIsPC()))
				{
					glPushMatrix();
					glTranslatef(currLink->getTextPos().x, currLink->getTextPos().y, 0.0);
					glRasterPos2f(0, 0);
					wxString linkName = currLink->getName();
					for(uint j = 0; j < linkName.length(); ++j)
					{
						glutBitmapCharacter(font, linkName[j]);
					}
					glPopMatrix();
				}
			}
		}
	}
	glPopAttrib();
}

void Component::drawConnections(bool showControl, bool showPC, Symbol* instr, bool simpleLayout)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();

	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		if(((*i).second)->isOutput())
		{

			OutputLink* oLink = static_cast<OutputLink*>(i->second.get());
			if(showControl || !(oLink->getIsControl()))
			{
				if(showPC || !(oLink->getIsPC()))
				{
					auto iLinkList = oLink->getLinkList();
					for(auto j = iLinkList.begin(); j != iLinkList.end(); ++j)
					{
						if(!((*j)->isOutput()))
						{
							InputLink* iLink = (InputLink*)(*j);
		
							double x1, y1, x2, y2;
							
							if((oLink->getSide() == LEFT) || (oLink->getSide() == RIGHT))
							{
								// Make sure on curved objects outputs are located precisely horizontally.
								x1 = oLink->getX() * oLink->getComp()->getW() + oLink->getComp()->getX();
							}
							else
							{
								x1 = oLink->getAbsX();
							}
							y1 = oLink->getAbsY();
							x2 = iLink->getAbsX();
							y2 = iLink->getAbsY();
							glPushAttrib(GL_LINE_BIT);
							glLineWidth(THIN_DATA_LINE_WIDTH);
							Component *inputComp = iLink->getComp();
							Component* prevComp = oLink->getComp();
							// Draw connection.
							bool isCInstrValid = (0 <= currentInstruction && currentInstruction < 5);
							if(oLink->isActive())
							{
								uint pStage = ((highlightSingleInstruction && isCInstrValid && !simpleLayout) ? currentInstruction : pipelineStage);
								if(oLink->isActiveValid(instr[pStage]) && (isCInstrValid && (int)pipelineStage <= currentInstruction || !highlightSingleInstruction || simpleLayout))
								{
									uint stage = (10 - pStage - pipelineCycle) % 5;
									Color col = scaleColors(Color(0, 0, 0, 255), getActiveLinkColour(stage), oldActiveLinkColor);
									glColor4fv(col.c);
								}
								else
								{
									glColor4fv(colours[LINK_COLOUR].c); //previously prevComp->linkColor.c
								}
							}
							else
							{
								glColor4fv(colours[LINK_COLOUR].c);
							}
							if((oLink->isActive() && (!highlightSingleInstruction || simpleLayout || (isCInstrValid && (int)pipelineStage <= currentInstruction))) || dataLinesBold)
							{
								if(!(inputComp->getIsControl() || prevComp->getIsControl()))
								{
									if(!(oLink->getIsControl()))
									{
										glLineWidth(DATA_LINE_WIDTH);
									}
								}							
							}

							glBegin(GL_LINE_STRIP);
							if(showControl || !(inputComp->getIsControl()))
							{
								if(showPC || !(inputComp->getIsPC()))
								{
									glVertex2f(x1, y1);
									for(auto &&vertex : iLink->getVertices())
									{
										glVertex2f(vertex.x, vertex.y);
									}
									glVertex2f(x2, y2);
								}
							}
							glEnd();
							glPopAttrib();
						}
					}
				}
			}

		}
	}
	glPopMatrix();
	glPopAttrib();
}

void Component::drawConnectors(bool showControl, bool showPC, Symbol* instr, bool simpleLayout)
{
	bool large;
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_POLYGON_SMOOTH);
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		large = false;
		if(!(((*i).second)->isOutput()))
		{
			InputLink* iLink = static_cast<InputLink*>(i->second.get());
			if(iLink->isConnected())
			{
				OutputLink* oLink = iLink->getOutput();
				Component* prevComp = oLink->getComp();
				Component* inputComp = iLink->getComp();
				if(inputComp->getType() != NODE_TYPE)
				{
					bool isCInstrValid = (0 <= currentInstruction && currentInstruction < 5);
					if(oLink->isActive())
					{

						uint pStage = ((highlightSingleInstruction && isCInstrValid && !simpleLayout) ? currentInstruction : prevComp->getPipelineStage());
						if(oLink->isActiveValid(instr[pStage]) && (isCInstrValid && (int)(prevComp->getPipelineStage()) <= currentInstruction || !highlightSingleInstruction || simpleLayout))
						{
							uint cStage = (10 - pStage - pipelineCycle) % 5;
							float prop = prevComp->getOldActiveLinkColor();
							Color col = scaleColors(Color(0, 0, 0, 255), getActiveLinkColour(cStage), prop);
							glColor4fv(col.c);
						}
						else
						{
							glColor4fv(colours[LINK_COLOUR].c);
						}
					}
					else
					{
						glColor4fv(colours[LINK_COLOUR].c);	
					}
					// Should this connection be large.
					if((oLink->isActive() && (!highlightSingleInstruction || simpleLayout || (isCInstrValid && (int)(prevComp->getPipelineStage()) <= currentInstruction))) || dataLinesBold)
					{
						if(!(inputComp->getIsControl() || prevComp->getIsControl()))
						{
							if(!(oLink->getIsControl()))
							{
								large = true;
							}
						}	
					}
	
					glPushMatrix();
					double x = iLink->getAbsX();
					double y = iLink->getAbsY();
					glTranslatef(x, y, 0);
					if(showControl || !(oLink->getIsControl() || prevComp->getIsControl()))
					{
						if(showPC || !(oLink->getIsPC() || prevComp->getIsPC()))
						{
							drawLinkTriangle(iLink->getSide(), large);
						}
					}
					glPopMatrix();
				}
			}
		}
	}
	glPopAttrib();

}

void Component::drawLinkTriangle(Side edge, bool large)
{
	double height = large ? 1.0 : 0.5;
	switch(edge)
	{
		case TOP:
			glRotatef(270, 0, 0, 1);
			break;
		case BOTTOM:
			glRotatef(90, 0, 0, 1);
			break;
		case LEFT:
		// No rotation. Triangle already pointing in this direction.
			break;
		case RIGHT:
			glRotatef(180, 0, 0, 1);
			break;
	}
	glBegin(GL_TRIANGLES);
		glVertex2f(0, 0);
		glVertex2f(-3, -height);
		glVertex2f(-3, height);
	glEnd();
}

Link* Component::createInput(int slot, double x, double y, Side edge, wxString name, bool control, bool PC, bool showText, double textX, double textY, int startBit, int bitLength)
{
	linkList[slot] = std::unique_ptr<Link>(std::make_unique<InputLink>(slot, this, x, y, edge, name, control, PC, showText, startBit, bitLength, textX, textY));
	return linkList[slot].get();
}

Link* Component::createOutput(int slot, double x, double y, Side edge, wxString name, bool control, bool PC, bool showText, double textX, double textY)
{
	linkList[slot] = std::unique_ptr<Link>(std::make_unique<OutputLink>(slot, this, x, y, edge, name, control, PC, showText, textX, textY));
	return linkList[slot].get();
}

void Component::addLinkVertex(uint linkNum, double x, double y)
{
	linkList[linkNum]->addVertex(x, y);
}

void Component::setInstrActive(uint linkNum, Symbol instr, bool active)
{
	linkList[linkNum]->setInstrActivity(instr, active);
}

void Component::connect(int outSlot, Component *comp, int inSlot)
{
	if(comp->getLink(inSlot)->isOutput())
		return;
		
	if(!(linkList[outSlot]->isOutput()))
		return;
		
	OutputLink* oLink = static_cast<OutputLink*>(linkList[outSlot].get());
	oLink->connect(comp->getLink(inSlot));
}

void Component::printLinkData()
{
	std::cout << this->getName() << ": " << std::endl;
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto& aLink = i->second;
		if(aLink->isOutput())
		{
			std::cout << "	" << (*i).first << ": " << aLink->getVal() << std::endl;
		}
	}
//	for(map<int, Link*>::iterator i = linkList.begin(); i != linkList.end(); ++i)
//	{
//		Link* aLink = (*i).second;
//		if(aLink->isOutput())
//		{
//			OutputLink* oLink = (OutputLink*)aLink;
//			vector<Link*> iLinkList = oLink->getLinkList();
//			for(vector<Link*>::iterator j = iLinkList.begin(); j != iLinkList.end(); ++j)
//			{
//				// Input link that this output link links to.
//				Link* iLink = (*j);
//				if(!(iLink->isOutput()))
//				{
//					InputLink* inLink = (InputLink*)iLink;
//					// Component that the iLink is attached to.
//					Component* iComp = inLink->getComp();
//					// Only attempt to print link if the input link has a valid pointer.
//					if(iComp != 0)
//					{
//						// Code line that connects an output to an input.
//						cout << this->getType() << this->getID() << "->connect(";
//						cout << oLink->getSlot() << ", " << iComp->getType() << iComp->getID();
//						cout << ", " << inLink->getSlot() << ");" << endl;
//					}
//				}
//			}
//		}
//		
//		// For all links print code line that adds each vertex.
//		list<Coord*> vertices = aLink->getVertices();
//		for(list<Coord*>::iterator j = vertices.begin(); j != vertices.end(); ++j)
//		{
//			Coord* vertex = (*j);
//			Component* comp = aLink->getComp();
//
//			cout << comp->getType() << comp->getID() << "->addLinkVertex(";
//			cout << aLink->getSlot() << ", ";
//			cout << vertex->x << ", ";
//			cout << vertex->y << ");" << endl;
//		}
//	}
}

Link* Component::getLink(uint slot)
{
	// Return link, but if there is no link, return 0, to prevent 
	// creation of null link.
	if(slot < linkList.size())
	{
		return linkList[slot].get();
	}
	else
	{
		return 0;
	}
}

void Component::drawShadedRectangle()
{
	glBegin(GL_TRIANGLE_STRIP);
		glColor4fv(colours[FADE_COLOUR].c);
		glVertex2f(0.0, 0.0);
		glColor4fv(scaleColors(fillColorMin, colours[FADE_COLOUR], 0.5).c);
		glVertex2f(1.0, 0.0);
		glVertex2f(0.0, 1.0);
		glColor4fv(fillColorMin.c);
		glVertex2f(1.0, 1.0);
	glEnd();
}

void Component::drawRectangle()
{
	glBegin(GL_LINE_LOOP);
		glColor4fv(colours[BORDER_COLOUR].c);
		glVertex2f(0.0, 0.0);
		glVertex2f(1.0, 0.0);
		glVertex2f(1.0, 1.0);
		glVertex2f(0.0, 1.0);
	glEnd();
}

bool Component::allInputsActive()
{
	// Make sure all inputs are active.
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &curLink = i->second;
		if(!(curLink->isOutput()))
		{
			if(!(curLink->isActive()))
			{
				return false;
			}
		}
	}
	return true;
}

void Component::setLinkData(uint linkNum, luint value)
{
	if((luint)linkNum < linkList.size())
	{
		linkList[linkNum]->setVal(value);
	}
}

void Component::setLinkActive(uint linkNum, bool active)
{
	if((luint)linkNum < linkList.size())
	{
		linkList[linkNum]->setActive(active); 
	}
}

void Component::setLinkBits(uint linkNum, uint startBit, uint bitLength)
{
	if((luint)linkNum < linkList.size())
	{
		linkList[linkNum]->setBits(startBit, bitLength);
	}
}

void Component::incrementPipelineCycle()
{ 
	pipelineCycle = (pipelineCycle + 4) % 5; 
	// Increment current instruction for highlighting if it is already valid.
	if(currentInstruction != -1)
	{
		currentInstruction++;
	}
	// If the current instruction goes outside the correct range, make it invalid.
	if(currentInstruction > 4)
	{
		currentInstruction = -1;
	}
}

void Component::setCurrentInstruction(int instr)
{ 
	// If instr is -1 it means no instruction should be highlighted therefore set
	// currentInstruction to -1.
	if(instr == -1)
	{
		currentInstruction = instr;
	}
	else
	{
		// To make sure the correct instruction is highlighted we have to 
		// effectively increment the val inputted by pipelineCycle.
		currentInstruction = (10 - instr - pipelineCycle) % 5; 
	}
}
		
MuxBase::MuxBase(double x, double y, double w, double h, wxString name, ComponentType type, bool PC)
: Component(x, y, w, h, name, type, false, PC, 0.3, 0.75)
{
}

Mux::Mux(double x, double y, double w, double h, wxString name, bool PC)
: MuxBase(x, y, w, h, name, MUX_TYPE, PC)
{
	createOutput(3, 1, 0.5, RIGHT, _T("Output"));
	createInput(2, 0.5, 1, TOP, _T("Control"));
	createInput(0, 0, 0.67, LEFT, _T("Input1"));
	createInput(1, 0, 0.33, LEFT, _T("Input2"));
}

Mux3::Mux3(double x, double y, double w, double h, wxString name)
: MuxBase(x, y, w, h, name, MUX_3_TYPE)
{
	createOutput(4, 1, 0.5, RIGHT, _T("Output"));
	createInput(3, 0.5, 1, TOP, _T("Control"));
	createInput(0, 0, 0.67, LEFT, _T("Input1"));
	createInput(1, 0, 0.5, LEFT, _T("Input2"));
	createInput(2, 0, 0.33, LEFT, _T("Input3"));
}

MainControl::MainControl(int layout, double x, double y, double w, double h, wxString name)
: Control(x, y, w, h, name, MAIN_CONTROL_TYPE, true, false, 0.1, 0.5)
{
	double offset = 0.02;
	createInput(MAINCONTROL_INPUT, 0, 0.5, LEFT, _T("Input"), true, false, false, 0, 0, 26, 6);
	createOutput(MAINCONTROL_REGDST, 0.5, 0.0, BOTTOM, _T("RegDst"), true, false, true, 0, -0.05);
	createOutput(MAINCONTROL_BRANCH, sqrt(0.25 - pow(abs(7 / 8.0 - 0.5), 2)) + 0.5, 7 / 8.0, RIGHT, _T("Branch"), true, true, true, 1.1, 7 / 8.0 + offset);
	createOutput(MAINCONTROL_MEMTOREG, sqrt(0.25 - pow(abs(6 / 8.0 - 0.5), 2)) + 0.5, 6 / 8.0, RIGHT, _T("MemToReg"), true, false, true, 1.1, 6 / 8.0 + offset);
	createOutput(MAINCONTROL_MEMREAD, sqrt(0.25 - pow(abs(5 / 8.0 - 0.5), 2)) + 0.5, 5 / 8.0, RIGHT, _T("MemRead"), true, false, true, 1.1, 5 / 8.0 + offset);
	createOutput(MAINCONTROL_ALUOP, sqrt(0.25 - pow(abs(4 / 8.0 - 0.5), 2)) + 0.5, 4 / 8.0, RIGHT, _T("ALUOp"), true, false, true, 1.1, 4/ 8.0 + offset);
	createOutput(MAINCONTROL_MEMWRITE, sqrt(0.25 - pow(abs(3 / 8.0 - 0.5), 2)) + 0.5, 3 / 8.0, RIGHT, _T("MemWrite"), true, false, true, 1.1, 3 / 8.0 + offset);
	createOutput(MAINCONTROL_ALUSRC, sqrt(0.25 - pow(abs(2 / 8.0 - 0.5), 2)) + 0.5, 2 / 8.0, RIGHT, _T("ALUSrc"), true, false, true, 1.1, 2 / 8.0 + offset);
	createOutput(MAINCONTROL_REGWRITE, sqrt(0.25 - pow(abs(1 / 8.0 - 0.5), 2)) + 0.5, 1 / 8.0, RIGHT, _T("RegWrite"), true, false, true, 1.1, 1 / 8.0 + offset);


	// Setup lookup table for translating opcode to output values:
	lookup[0] = std::make_unique<mainControlLookup>(true, false, false, false, 2, false, false, true); // add, sub, and, or, slt
	lookup[4] = std::make_unique<mainControlLookup>(false, true, false, false, 1, false, false, false); // beq
	lookup[8] = std::make_unique<mainControlLookup>(false, false, false, false, 0, false, true, true); // Add Immediate
	lookup[35] = std::make_unique<mainControlLookup>(false, false, true, true, 0, false, true, true); // lw
	lookup[43] = std::make_unique<mainControlLookup>(false, false, false, false, 0, true, true, false); // sw
}

MainControlPipelined::MainControlPipelined(int layout, double x, double y, double w, double h, wxString name)
: Control(x, y, w, h, name, MAIN_CONTROL_TYPE, true, false, 0.1, 0.5)
{
	createInput(MAINCONTROLPIPELINED_INPUT, 0, 0.5, LEFT, _T("Input"));
	createOutput(MAINCONTROLPIPELINED_WB, sqrt(0.25 - pow(abs(0.5 - MAINCONTROLPIPELINED_WB / 4.0), 2)) + 0.5, 1.0 - MAINCONTROLPIPELINED_WB / 4.0, RIGHT, _T("WB"));
	createOutput(MAINCONTROLPIPELINED_MEM, sqrt(0.25 - pow(abs(0.5 - MAINCONTROLPIPELINED_MEM / 4.0), 2)) + 0.5, 1.0 - MAINCONTROLPIPELINED_MEM / 4.0, RIGHT, _T("MEM"));
	createOutput(MAINCONTROLPIPELINED_EX, sqrt(0.25 - pow(abs(0.5 - MAINCONTROLPIPELINED_EX / 4.0), 2)) + 0.5, 1.0 - MAINCONTROLPIPELINED_EX / 4.0, RIGHT, _T("EX"));


	// Setup lookup table for translating opcode to output values:
	lookup[0] = std::make_unique<mainControlLookup>(true, false, false, false, 2, false, false, true); // add, sub, and, or, slt
	lookup[4] = std::make_unique<mainControlLookup>(false, true, false, false, 1, false, false, false); // beq
	lookup[8] = std::make_unique<mainControlLookup>(false, false, false, false, 0, false, true, true); // Add Immediate
	lookup[35] = std::make_unique<mainControlLookup>(false, false, true, true, 0, false, true, true); // lw
	lookup[43] = std::make_unique<mainControlLookup>(false, false, false, false, 0, true, true, false); // sw
}

ALUControl::ALUControl(double x, double y, double w, double h, wxString name)
: Control(x, y, w, h, name, ALU_CONTROL_TYPE, true, false, 0.05, 0.6)
{
	createInput(0, 0.03, 0.67, LEFT, _T("ALUOp"));
	createInput(1, 0.03, 0.33, LEFT, _T("Function"));
	createOutput(2, 0.5, 1.0, TOP, _T("Output"));
	funcLookup[32] = 2; // Add instruction.
	funcLookup[34] = 6; // Sub instruction.
	funcLookup[36] = 0; // And instruction.
	funcLookup[37] = 1; // Or instruction.
	funcLookup[42] = 7; // slt instruction.
}

SignExtend::SignExtend(double x, double y, double w, double h, wxString name)
: Control(x, y, w, h, name, SIGN_EXTEND_TYPE, false, false, 0.05, 0.6)
{
	Link* ln = createInput(0, 0, 0.5, LEFT, _T("16"), false, false, true, -0.4, 0.55);
	ln->setNegBit16();
	createOutput(1, 1, 0.5, RIGHT, _T("32"), false, false, true, 1.1, 0.55);
}

ShiftLeft2::ShiftLeft2(double x, double y, double w, double h, wxString name, bool PC)
: Control(x, y, w, h, name, SHIFT_LEFT_2_TYPE, false, PC, 0.25, 0.7)
{
	createInput(0, 0, 0.5, LEFT, _T("Input"));
	createOutput(1, 1, 0.5, RIGHT, _T("Output"));
}

luint Memory::getData(luint address)
{ 
	return data[address]; 
}

InstructionMemory::InstructionMemory(double x, double y, double w, double h, wxString name)
: Memory(x, y, w, h, name, INSTRUCTION_MEMORY_TYPE, 0.3, 0.85)
{
	createInput(0, 0, 0.5, LEFT, _T("Address"), false, false, true, 0.02, 0.48);
	createOutput(1, 1, 0.5, RIGHT, _T("Instruction"), false, false, true, 0.55, 0.48);
	outputAddress = 0;
}

Registers::Registers(Layout layout, double x, double y, double w, double h, wxString name)
: Memory(x, y, w, h, name, REGISTERS_TYPE, 0.6, 0.48)
{
	createInput(0, 0.5, 1.0, TOP, _T("RegWrite"));
	createInput(1, 0, 0.8, LEFT, _T("Write Data"), false, false, true, 0.02, 0.78);
	createInput(2, 0, 0.61, LEFT, _T("Read Reg 1"), false, false, true, 0.02, 0.58, 21, 5);
	createInput(3, 0, 0.4, LEFT, _T("Read Reg 2"), false, false, true, 0.02, 0.38, 16, 5);
	createInput(4, 0, 0.2, LEFT, _T("Write Reg"), false, false, true, 0.02, 0.18);
	createOutput(5, 1, 0.8, RIGHT, _T("Read Data 1"), false, false, true, 0.5, 0.78);
	createOutput(6, 1, 0.2, RIGHT, _T("Read Data 2"), false, false, true, 0.5, 0.18);
	writeToReg = 0;
	if(layout == LAYOUT_SIMPLE)
	{
		writeDelay = Component::simpleClockSteps;
	}
	else
	{
		writeDelay = Component::pipelineClockSteps;
	}
}

void Registers::reset()
{
	writeToReg = 0;
	data.clear();
}

void DataMemory::reset()
{
	writeToMem = 0;
	data.clear();
}

DataMemory::DataMemory(Layout layout, double x, double y, double w, double h, wxString name)
: Memory(x, y, w, h, name, DATA_MEMORY_TYPE, 0.35, 0.55)
{
	createInput(0, 0.2, 1.0, TOP, _T("MemWrite"));
	createInput(1, 0.8, 1.0, TOP, _T("MemRead"));
	createInput(2, 0, 0.8, LEFT, _T("Address"), false, false, true, 0.02, 0.78);
	createInput(3, 0, 0.2, LEFT, _T("Write Data"), false, false, true, 0.02, 0.18);
	createOutput(4, 1, 0.8, RIGHT, _T("Read Data"), false, false, true, 0.55, 0.78);
	writeToMem = 0;
	
	// Delays setup to prevent junking good data.
	if(layout == LAYOUT_SIMPLE)
	{
		writeDelay = Component::simpleClockSteps;
		writeWait = 5;
	}
	else
	{
		writeDelay = Component::pipelineClockSteps;
		writeWait = 2;
	}
}

ALU::ALU(double x, double y, double w, double h, wxString name, bool PC)
: Component(x, y, w, h, name, ALU_TYPE, false, PC, 0.51, 0.48)
{
	createInput(0, 0, 0.8, LEFT, _T("Input1"));
	createInput(1, 0, 0.2, LEFT, _T("Input2"));
	createInput(2, 0.5, 0.167, BOTTOM, _T("Control"));
	createOutput(3, 1, 0.5, RIGHT, _T("Result"));
	createOutput(4, 1, 0.4, RIGHT, _T("Zero"), true);
}

AndGate::AndGate(double x, double y, double w, double h, wxString name, bool PC)
: Component(x, y, w, h, name, AND_GATE_TYPE, true, PC)
{
	createInput(0, 0, 0.8, LEFT, _T("Input1"));
	createInput(1, 0, 0.2, LEFT, _T("Input2"));
	createOutput(2, 1, 0.5, RIGHT, _T("Output"), true);
}

PC::PC(int layout, double x, double y, double w, double h, wxString name)
: Component(x, y, w, h, name, PC_TYPE, false, false, 0.1, 0.48)
{
	createInput(0, 0, 0.5, LEFT, _T("Input"));
	createOutput(1, 1, 0.5, RIGHT, _T("Output"));
	// Set initial program counter position to 0.
	setLinkData(1, 0);
	// Set delay (Point at which new instruction is read)
	switch(layout)
	{
	case LAYOUT_SIMPLE:
		delay = Component::simpleClockSteps;
		updateStep = 0;
		storeStep = 0;
		break;
	case LAYOUT_PIPELINE:
	case LAYOUT_FORWARDING:
		updateStep = 1;
		storeStep = 2;
		delay = Component::pipelineClockSteps;
		break;
	}

	delayRemaining = delay;
}

void PC::reset()
{
	delayRemaining = delay;
}

luint PC::getOutput()
{
	return linkList[1]->getVal();
}

Node::Node(double x, double y, double w, double h, wxString name)
: Component(x, y, w, h, name, NODE_TYPE)
{
	createInput(0, 0.0, 0, LEFT);
	createInput(1, 0, 0.0, TOP);
	createInput(2, 0, 0.1, BOTTOM);
	createInput(3, 0.1, 0, RIGHT);
	createOutput(4, 0.0, 0, LEFT);
	createOutput(5, 0, 0.1, TOP);
	createOutput(6, 0, 0.0, BOTTOM);
	createOutput(7, 0.1, 0, RIGHT);
}

PipelineRegister::PipelineRegister(double x, double y, double w, double h, wxString name, ComponentType type, double textPosX, double textPosY)
: Component(x, y, w, h, name, type, false, false, textPosX, textPosY)
{
	fillColorMin = colours[PIPEREG_COLOUR];
	setDelay(Component::pipelineClockSteps); // Sets the wait time for all registers so that the relevant pipeline stage
				// settles to the correct values before doing our propogation.
	resetDelayRemaining();
}

wxString PipelineRegister::getMainInfo(wxPoint mousePos)
{
	wxString info;
	bool findInputs = true;
	
	if(mousePos.x < (this->getX() + (this->getW() / 2)))
	{
		// left hand side so inputs
		info += _T("Inputs\n");
		findInputs = true;	
	}
	else
	{ 
		// right hand side so outputs
		info += _T("Outputs\n");
		findInputs = false;
	}
	
	// Get puts in descending order of y pos.
	std::map<double, Link*> yposList;
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &ln = i->second;
		if((findInputs && !ln->isOutput()) || (!findInputs && ln->isOutput()))
		{
			yposList[1.0 - ln->getY()] = ln.get();
		}
	}
	for(auto i = yposList.begin(); i != yposList.end(); ++i)
	{
		Link* ln = (*i).second;
		info += Maths::convertToBase(ln->getVal()) + _T("\n");
	}
	info.RemoveLast();
	return info;
}

// Set all outputs of a pipeline register to 0.
void PipelineRegister::reset()
{
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &link= i->second;
		if(link->isOutput())
		{
			link->setVal(0);
		}
	}
}

IFIDReg::IFIDReg(Layout layout, double x, double y, double w, double h, wxString name)
: PipelineRegister(x, y, w, h, name, IFID_REG_TYPE, 0.04, 1.005)
{
	if(layout == LAYOUT_FORWARDING)
	{
		createInput(0, 0, 0.5, LEFT, _T("Instruction"));
		createInput(1, 0, 0.91, LEFT, _T("PC"));
		createOutput(2, 1, 0.5, RIGHT);
		createOutput(3, 1, 0.91, RIGHT, _T(""), false, true); 
	}
	else if(layout == LAYOUT_PIPELINE)
	{
		createInput(0, 0, 0.495, LEFT);
		createInput(1, 0, 0.855, LEFT);
		createOutput(2, 1, 0.495, RIGHT);
		createOutput(3, 1, 0.855, RIGHT, _T(""), false, true); 
	}
}

IDEXReg::IDEXReg(Layout layout, double x, double y, double w, double h, wxString name)
: PipelineRegister(x, y, w, h, name, IDEX_REG_TYPE, -0.08, 1.005)
{
	createInput(3, 0, 0.397, LEFT);
	createInput(4, 0, 0.5, LEFT);
	createOutput(12, 1, 0.397, RIGHT);
	createOutput(13, 1, 0.5, RIGHT);
	if(layout == LAYOUT_FORWARDING)
	{
		createInput(0, 0, 0.04, LEFT);
		createInput(1, 0, 0.08, LEFT);
		createInput(2, 0, 0.2, LEFT);
		createInput(5, 0, 0.78, LEFT);
		createInput(6, 0, 0.86, LEFT);
		createInput(7, 0, 0.925, LEFT);
		createInput(8, 0, 0.985, LEFT);
		createInput(20, 0, 0.11, LEFT);
		createOutput(9, 1, 0.04, RIGHT);
		createOutput(10, 1, 0.08, RIGHT);
		createOutput(11, 1, 0.2, RIGHT);	
		createOutput(21, 1, 0.11, RIGHT);
		createOutput(14, 1, 0.78, RIGHT);
		createOutput(15, 1, 0.852, RIGHT, _T(""), true);
		createOutput(16, 1, 0.86, RIGHT, _T(""), true);
		createOutput(17, 1, 0.87, RIGHT, _T(""), true);
		createOutput(18, 1, 0.925, RIGHT, _T(""), true);
		createOutput(19, 1, 0.985, RIGHT, _T(""), true);
	}
	else if(layout == LAYOUT_PIPELINE)
	{
		createInput(0, 0, 0.05, LEFT);
		createInput(1, 0, 0.095, LEFT);
		createInput(2, 0, 0.195, LEFT);
		createInput(5, 0, 0.74, LEFT);
		createInput(6, 0, 0.84, LEFT);
		createInput(7, 0, 0.9, LEFT);
		createInput(8, 0, 0.96, LEFT);
		createOutput(9, 1, 0.05, RIGHT);
		createOutput(10, 1, 0.095, RIGHT);
		createOutput(11, 1, 0.195, RIGHT);	
		createOutput(14, 1, 0.74, RIGHT);
		createOutput(15, 1, 0.84, RIGHT, _T(""), true);
		createOutput(16, 1, 0.895, RIGHT, _T(""), true);
		createOutput(17, 1, 0.9, RIGHT, _T(""), true);
		createOutput(18, 1, 0.91, RIGHT, _T(""), true);
		createOutput(19, 1, 0.96, RIGHT, _T(""), true);
	}
	
	this->layout = layout;
}

EXMEMReg::EXMEMReg(Layout layout, double x, double y, double w, double h, wxString name)
: PipelineRegister(x, y, w, h, name, EXMEM_REG_TYPE, -0.35, 1.005)
{
	if(layout == LAYOUT_FORWARDING)
	{
		createInput(0, 0, 0.060, LEFT);
		createInput(1, 0, 0.315, LEFT);
		createInput(2, 0, 0.41, LEFT);
		createInput(3, 0, 0.425, LEFT);
		createInput(4, 0, 0.725, LEFT);
		createInput(5, 0, 0.925, LEFT);
		createInput(6, 0, 0.985, LEFT);
		createOutput(7, 1, 0.060, RIGHT);
		createOutput(8, 1, 0.315, RIGHT);
		createOutput(9, 1, 0.41, RIGHT);
		createOutput(10, 1, 0.425, RIGHT);
		createOutput(11, 1, 0.725, RIGHT);
		createOutput(12, 1, 0.92, RIGHT, _T(""), true);
		createOutput(13, 1, 0.925, RIGHT, _T(""), true);
		createOutput(14, 1, 0.932, RIGHT, _T(""), true);
		createOutput(15, 1, 0.985, RIGHT, _T(""), true);
	}
	else if(layout == LAYOUT_PIPELINE)
	{
		createInput(0, 0, 0.075, LEFT);
		createInput(1, 0, 0.32, LEFT);
		createInput(2, 0, 0.41, LEFT);
		createInput(3, 0, 0.425, LEFT);
		createInput(4, 0, 0.68, LEFT);
		createInput(5, 0, 0.91, LEFT);
		createInput(6, 0, 0.96, LEFT);
		createOutput(7, 1, 0.075, RIGHT);
		createOutput(8, 1, 0.32, RIGHT);
		createOutput(9, 1, 0.41, RIGHT);
		createOutput(10, 1, 0.425, RIGHT);
		createOutput(11, 1, 0.68, RIGHT);
		createOutput(12, 1, 0.902, RIGHT, _T(""), true);
		createOutput(13, 1, 0.91, RIGHT, _T(""), true);
		createOutput(14, 1, 0.92, RIGHT, _T(""), true);
		createOutput(15, 1, 0.96, RIGHT, _T(""), true);
	}	
}

MEMWBReg::MEMWBReg(Layout layout, double x, double y, double w, double h, wxString name)
: PipelineRegister(x, y, w, h, name, MEMWB_REG_TYPE, -0.35, 1.005)
{
	if(layout == LAYOUT_FORWARDING)
	{
		createInput(0, 0, 0.070, LEFT);
		createInput(1, 0, 0.495, LEFT);
		createOutput(4, 1, 0.070, RIGHT);
		createOutput(5, 1, 0.495, RIGHT);
	}
	else if(layout == LAYOUT_PIPELINE)
	{
		createInput(0, 0, 0.086, LEFT);
		createInput(1, 0, 0.495, LEFT);
		createOutput(4, 1, 0.086, RIGHT);
		createOutput(5, 1, 0.495, RIGHT);
	}
	createInput(2, 0, 0.605, LEFT);
	createInput(3, 0, 0.96, LEFT);
		
	createOutput(6, 1, 0.605, RIGHT);
	createOutput(7, 1, 0.952, RIGHT, _T(""), true);
	createOutput(8, 1, 0.96, RIGHT, _T(""), true);
}

Forwarding::Forwarding(double x, double y, double w, double h, wxString name)
: Component(x, y, w, h, name, FORWARDING_TYPE, true, false, 0.3, 0.60)
{
	setDelay(Component::pipelineClockSteps); // Sets the wait time for all registers so that the relevant pipeline stage
				// settles to the correct values before doing our propogation.
	resetDelayRemaining();
	createInput(FORWARDING_READREG1, 0, 0.75, LEFT, _T("Read Reg 1"));
	createInput(FORWARDING_READREG2, 0, 0.25, LEFT, _T("Read Reg 2"));
	createInput(FORWARDING_EXMEMREG, 1, 0.75, RIGHT, _T("EX/MEM Write Reg"));
	createInput(FORWARDING_MEMWBREG, 1, 0.25, RIGHT, _T("MEM/WB Write Reg"));
	createOutput(FORWARDING_MUXA, 0.75, 1, TOP, _T("Forward A"));
	createOutput(FORWARDING_MUXB, 0.25, 1, TOP, _T("Forward B"));
	
}

void PC::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	position(); 
	drawShadedRectangle();
	drawRectangle();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void AndGate::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);	
	glPushMatrix();
	position(); 
	drawShadedRectangle();
	glScalef(0.67F, 1.0F, 1.0F);
	// Hide shaded areas outside shape.
	glBegin(GL_TRIANGLE_STRIP);
		glColor4fv(colours[BACKGROUND_COLOUR].c);
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			if(i < NUM_CIRCLE_LINES / 2)
			{
				glVertex2f(1.5, 1);
			}
			else
			{
				glVertex2f(1.5, 0);
			}
			glVertex2f(1.0 + 0.5 * sin(PI * i / NUM_CIRCLE_LINES), 0.5 + 0.5 * cos(PI * i / NUM_CIRCLE_LINES));
		}
	glEnd();
	// Draw border.
	glBegin(GL_LINE_LOOP);
		glColor4fv(colours[BORDER_COLOUR].c);
		glVertex2f(0.0, 0.0);
		glVertex2f(0.0, 1.0);
		glVertex2f(1.0, 1.0);
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			glVertex2f(1.0 + 0.5 * sin(PI * i / NUM_CIRCLE_LINES), 0.5 + 0.5 * cos(PI * i / NUM_CIRCLE_LINES));
		}
		glVertex2f(1.0, 0.0);
	glEnd();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void ALU::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);	
	glPushMatrix();
	position(); 
	glBegin(GL_TRIANGLE_STRIP);
		glColor4fv(colours[FADE_COLOUR].c);
  		glVertex2f(0.0F, 0.0F); 
  		glColor4fv(scaleColors(fillColorMin, colours[FADE_COLOUR], 0.67).c);
  		glVertex2f(0.0F, 0.33F);
  		glColor4fv(scaleColors(fillColorMin, colours[FADE_COLOUR], 0.33).c);
  		glVertex2f(1.0F, 0.33F);	
  		glVertex2f(0.45F, 0.5F);
  		glColor4fv(fillColorMin.c);
  		glVertex2f(1.0F, 0.67F);
  		glColor4fv(scaleColors(fillColorMin, colours[FADE_COLOUR], 0.33).c);
  		glVertex2f(0.0F, 0.67F);
  		glColor4fv(fillColorMin.c);
  		glVertex2f(0.0F, 1.0F); 			
  	glEnd();
  	glBegin(GL_LINE_LOOP);
		glColor4fv(colours[BORDER_COLOUR].c);
  		glVertex2f(0.0F, 0.0F); 
  		glVertex2f(1.0F, 0.33F);
  		glVertex2f(1.0F, 0.67F);
  		glVertex2f(0.0F, 1.0F);
  		glVertex2f(0.0F, 0.67F);
  		glVertex2f(0.45F, 0.5F);
  		glVertex2f(0.0F, 0.33F);	
	glEnd();
  	drawName(showControl, showPC, scale);
  	glPopMatrix();
  	glPopAttrib();
}

void Memory::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	position(); 
	drawShadedRectangle();
	drawRectangle();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void Control::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	position(); 
	glPushMatrix();
	glTranslatef(0.5, 0.5, 0.0);
	glScalef(0.5, 0.5, 1.0);
	glBegin(GL_TRIANGLE_STRIP);
		for(int i = 0; i <= NUM_CIRCLE_LINES / 2; i++)
		{
			double proportion = (cos((i * 2 * PI / NUM_CIRCLE_LINES) + (3 * PI / 4)) + 1) / 2;
			glColor4fv(scaleColors(fillColorMin, colours[FADE_COLOUR], proportion).c);
			glVertex2f(sin((2 * PI * i) / NUM_CIRCLE_LINES) , cos((2 * PI * i) / NUM_CIRCLE_LINES));
			int j = NUM_CIRCLE_LINES - i;
			proportion = (cos((j * 2 * PI / NUM_CIRCLE_LINES) + (3 * PI / 4)) + 1) / 2;
			glColor4fv(scaleColors(fillColorMin, colours[FADE_COLOUR], proportion).c);
			glVertex2f(sin((2 * PI * j) / NUM_CIRCLE_LINES) , cos((2 * PI * j) / NUM_CIRCLE_LINES));
		
		}
	glEnd();
	glBegin(GL_LINE_LOOP);
		glColor4fv(colours[BORDER_COLOUR].c);
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			glVertex2f(sin((2 * PI * i) / NUM_CIRCLE_LINES) , cos((2 * PI * i) / NUM_CIRCLE_LINES));
		}
	glEnd();
	glPopMatrix();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void MuxBase::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	position(); 
	// Fill rectangle with fade.
	drawShadedRectangle();
	// Hide areas outside shape.
	glBegin(GL_TRIANGLE_STRIP);
		glColor4fv(colours[BACKGROUND_COLOUR].c);
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			if(i < NUM_CIRCLE_LINES / 2)
			{
				glVertex2f(0, 0);
			}
			else
			{
				glVertex2f(1, 0);
			}
			glVertex2f(0.5 - 0.5 * cos(PI * i / NUM_CIRCLE_LINES), 0.125 - 0.125 * sin(PI * i / NUM_CIRCLE_LINES));
		}
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			if(i < NUM_CIRCLE_LINES / 2)
			{
				glVertex2f(1, 1);
			}
			else
			{
				glVertex2f(0, 1);
			}
			glVertex2f(0.5 + 0.5 * cos(PI * i / NUM_CIRCLE_LINES), 0.875 + 0.125 * sin(PI * i / NUM_CIRCLE_LINES));
		}
	glEnd();
	// Draw border.
	glBegin(GL_LINE_LOOP);
		glColor4fv(colours[BORDER_COLOUR].c);
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			glVertex2f(0.5 - 0.5 * cos(PI * i / NUM_CIRCLE_LINES), 0.125 - 0.125 * sin(PI * i / NUM_CIRCLE_LINES));
		}
		for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
		{
			glVertex2f(0.5 + 0.5 * cos(PI * i / NUM_CIRCLE_LINES), 0.875 + 0.125 * sin(PI * i / NUM_CIRCLE_LINES));
		}
	glEnd();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void PipelineRegister::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	position(); 
	drawShadedRectangle();
	drawRectangle();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void Forwarding::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	position(); 
	drawShadedRectangle();
	drawRectangle();
	drawName(showControl, showPC, scale);
	glPopMatrix();
	glPopAttrib();
}

void Node::draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale)
{
	uint numInputs = 1;
	uint numOutputs = 2;
	if(!(showControl && showPC))
	{
		numInputs = 0;
		numOutputs = 0;
		bool sControl, sPC;
		OutputLink* oLink;
		InputLink* iLink;
		Component* nComp;
		for(auto i = linkList.begin(); i != linkList.end(); ++i)
		{
			auto &link= i->second;
			if(link->isOutput())
			{
				oLink = static_cast<OutputLink*>(link.get());
				
				std::vector<Link*> oLinkList = oLink->getLinkList();
				for(auto j = oLinkList.begin(); j != oLinkList.end(); ++j)
				{
					if(!((*j)->isOutput()))
					{
						iLink = (InputLink*)(*j);
						nComp = iLink->getComp();
						sControl =  showControl || !(nComp->getIsControl() || oLink->getIsControl());
						sPC = showPC || !(nComp->getIsPC() || oLink->getIsPC());
						if(sPC && sControl)
						{
							numOutputs++;
						}
					}
				}
			}
			else
			{
				iLink = static_cast<InputLink*>(link.get());
				
				if(iLink->isConnected())
				{
					oLink = iLink->getOutput();
					
					nComp = oLink->getComp();
					sControl =  showControl || !(nComp->getIsControl());
					sPC = showPC || !(nComp->getIsPC());
					if(sPC && sControl)
					{
						numInputs++;
					}
				}
			}
		}
	}
	if(numInputs > 0 && numOutputs > 1)
	{
		double scaleFactor;
		glPushAttrib(GL_COLOR_BUFFER_BIT);
		glPushMatrix();
		position();
		bool isCInstrValid = (0 <= currentInstruction && currentInstruction < 5);
		if(isActive())
		{
			uint pStage = ((highlightSingleInstruction && isCInstrValid && !simpleLayout) ? currentInstruction : pipelineStage);
			if(isActiveValid(currInstr[pStage]) && (isCInstrValid && (int)pipelineStage <= currentInstruction || !highlightSingleInstruction || simpleLayout))
			{
				uint stage = (10 - pStage - pipelineCycle) % 5;
				Color col = scaleColors(Color(0, 0, 0, 255), getActiveLinkColour(stage), oldActiveLinkColor);
				glColor4fv(col.c);
			}
			else
			{
				glColor4fv(colours[LINK_COLOUR].c);
			}
		}
		else 
		{
			glColor4fv(colours[LINK_COLOUR].c);
		}
		if((isActive() && (!highlightSingleInstruction || simpleLayout || (isCInstrValid && (int)pipelineStage <= currentInstruction))) || dataLinesBold)
		{
			scaleFactor = 1.3;
		}
		else
		{
			scaleFactor = 1.0;
		}
		glScalef(0.5 * scaleFactor, 0.5 * scaleFactor, 1.0);
		glBegin(GL_POLYGON);
			for(int i = 0; i <= NUM_CIRCLE_LINES; i++)
			{
				glVertex2f(sin((2 * PI * i) / NUM_CIRCLE_LINES) / 2, cos((2 * PI * i) / NUM_CIRCLE_LINES) / 2);
			}
		glEnd();
		glPopMatrix();
		glPopAttrib();
	}
}

bool Node::isActive()
{
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &link= i->second;
		if(!(link->isOutput()))
		{
			InputLink* iLink = static_cast<InputLink*>(link.get());
			
			if(iLink->isConnected())
			{
				OutputLink* oLink = iLink->getOutput();
				
				Component* comp = oLink->getComp();
				
				// Only check for isActive if the sub component is not this component.
				// Prevents infinite recursion.
				if(comp->getID() != this->getID())
				{
					return comp->isActive();
				}
				else
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool Node::isActiveValid(Symbol instr)
{
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &link= i->second;
		if(!(link->isOutput()))
		{
			InputLink* iLink = static_cast<InputLink*>(link.get());
			
			if(iLink->isConnected())
			{
				OutputLink* oLink = iLink->getOutput();
				
				return oLink->isActiveValid(instr);
			}
		}
	}
	return false;
}

luint Node::getVal()
{
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &link= i->second;
		if(!(link->isOutput()))
		{
			InputLink* iLink = static_cast<InputLink*>(link.get());
			
			if(iLink->isConnected())
			{
				OutputLink* oLink = iLink->getOutput();
				
				return oLink->getVal();
			}
		}
	}
	return 0;
}

bool Node::getIsControl()
{
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &link= i->second;
		if(!(link->isOutput()))
		{
			InputLink* iLink = static_cast<InputLink*>(link.get());
			
			if(iLink->isConnected())
			{
				OutputLink* oLink = iLink->getOutput();
				Component* comp = oLink->getComp();
				
				return comp->getIsControl();
			}
		}
	}
	return false;
}

bool Node::getIsPC()
{
	for(auto i = linkList.begin(); i != linkList.end(); ++i)
	{
		auto &link= i->second;
		if(!(link->isOutput()))
		{
			InputLink* iLink = static_cast<InputLink*>(link.get());
			
			if(iLink->isConnected())
			{
				OutputLink* oLink = iLink->getOutput();
				Component* comp = oLink->getComp();
				
				return comp->getIsPC();
			}
		}
	}
	return false;	
}

void Mux::step()
{
	(static_cast<InputLink*>(linkList[0].get()))->getValFromOutput();
	// Set output based on inputs.
	if(linkList[2]->getVal())
	{
		// Control line is true (1).
		linkList[3]->setVal(linkList[1]->getVal());
	}
	else
	{
		// Control line is false (0).
		linkList[3]->setVal(linkList[0]->getVal());		
	}
	
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void Mux3::step()
{
	// Get control input.
	int control = linkList[3]->getVal();
	
	// Check control input is valid.
	try
	{
		if(0 <= control || control <= 2)
		{
			// Set output by choosing input decided by control.
			linkList[4]->setVal(linkList[control]->getVal());
		}
		else
		{
			// Bad input.
			throw InvalidInputEx;		
		}
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << this->getID() << std::endl;
	}
	
	if(!isActive() && linkList[0]->isActive() && linkList[3]->isActive())
	{
		this->setActive();
	}
}

void ALU::step()
{
	luint result;
	luint input1 = linkList[0]->getVal();
	luint input2 = linkList[1]->getVal();
	long long int input1N, input2N;
	bool neg1, neg2;
	// Switch on ALU operation input.
	switch(linkList[2]->getVal())
	{
		case 0: // And
			result = input1 & input2;
			break;
		case 1: // Or
			result = input1 | input2;
			break;
		case 2: // Add
			// 2's complement addition
			result = (input1 +  input2) % ((luint)(1)<<32); 
			break;
		case 6: // Subtract
			// 2's complement subtraction
			result = (input1 - input2) % ((luint)(1)<<32); 
			break;
		case 7: // Set on less than 
			// 2's complement subtraction
			neg1 = !!(input1>>31); // is input1 negative
			neg2 = !!(input2>>31); // is input2 negative
			input1N = neg1 ? ((luint)(1)<<32) - input1 : input1; // positive version of input1;
			input2N = neg2 ? ((luint)(1)<<32) - input2 : input2; // positive version of input1;
			if((neg1 ? -input1N : input1N) < (neg2 ? -input2N : input2N))
			{
				result = 1;
			}
			else
			{
				result = 0;
			}
			break;
		default: // Error
			// Do nothing.
			result = linkList[3]->getVal();
			// Implement exception here?
			break;
	}
	linkList[3]->setVal(result);
	linkList[4]->setVal(result == 0); // Set this output high if result is zero.
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void MainControl::step()
{
	auto& result = lookup[linkList[MAINCONTROL_INPUT]->getVal()];
	
	linkList[MAINCONTROL_REGDST]->setVal(result->getRegDst());
	linkList[MAINCONTROL_BRANCH]->setVal(result->getBranch());
	linkList[MAINCONTROL_MEMTOREG]->setVal(result->getMemToReg());
	linkList[MAINCONTROL_MEMREAD]->setVal(result->getMemRead());
	linkList[MAINCONTROL_ALUOP]->setVal(result->getALUOp());
	linkList[MAINCONTROL_MEMWRITE]->setVal(result->getMemWrite());
	linkList[MAINCONTROL_ALUSRC]->setVal(result->getALUSrc());
	linkList[MAINCONTROL_REGWRITE]->setVal(result->getRegWrite());
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void MainControlPipelined::step()
{
	auto& result = lookup[linkList[MAINCONTROLPIPELINED_INPUT]->getVal()];

	linkList[MAINCONTROLPIPELINED_WB]->setVal(result->getWB());
	linkList[MAINCONTROLPIPELINED_MEM]->setVal(result->getMEM());
	linkList[MAINCONTROLPIPELINED_EX]->setVal(result->getEX());
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void ALUControl::step()
{
	uint ALUOp = linkList[0]->getVal();
	uint function = linkList[1]->getVal();
	uint result;
	try
	{
		switch(ALUOp)
		{
			case 0:
				result = 2; // instruction is lw or sw, or addi so add.
				break;
			case 1:
				result = 6; // instruction is beq so sub.
				break;
			case 2:
				result = funcLookup[function]; // instruction is R-Type so output is determined purely by function code.
				break;
			default:
				// Bad Input.
				throw InvalidInputEx;
				break;
		}
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << this->getID() << std::endl;
		result = 0;
	}
	
	linkList[2]->setVal(result);
	
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void DataMemory::step()
{
	luint memWrite = linkList[0]->getVal();
	luint memRead = linkList[1]->getVal();
	luint address = linkList[2]->getVal();
	luint writeData = linkList[3]->getVal();
	
	(++writeToMem) %= writeDelay;
	if(memWrite && writeToMem >= writeWait)
	{
		data[address] = writeData;
	}
	if(memRead)
	{
		luint readData = data[address];
		linkList[4]->setVal(readData);
	}
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}	
}

bool Registers::allInputsActive()
{
	if(!(linkList[0]->isActive()))
	{
		return false;
	}
	if(!(linkList[2]->isActive()))
	{
		return false;
	}
	if(!(linkList[3]->isActive()))
	{
		return false;
	}
	if(!(linkList[4]->isActive()))
	{
		return false;
	}
	
	return true;
}

void Registers::step()
{
	//cout << "Data in reg 7: " << data[7] << endl;
	// Read inputs.
	luint readReg1 = linkList[2]->getVal();
	luint readReg2 = linkList[3]->getVal();
	luint regWrite = linkList[0]->getVal();
	luint readData1 = data[readReg1];
	luint readData2 = data[readReg2];
	//cout << readReg1 << endl;
	//cout << readReg2 << endl;
	// Set read data outputs.
	linkList[5]->setVal(readData1);
	linkList[6]->setVal(readData2);
	// If register writing should occur, write to correct register.
	if(writeToReg >= 2)
	{
		luint writeData = linkList[1]->getVal();
		luint writeReg = linkList[4]->getVal();
		if(writeReg != 0)
		{
			data[writeReg] = writeData;
		}
	}
	// Delay writing to register by one step to prevent junking good register data!
	if(regWrite)
	{
		(++writeToReg) %= writeDelay; // Has to equal steps per clock cycle.
	}
	// Set active if the read registers are active.
	if(!isActive() && linkList[2]->isActive() && linkList[3]->isActive())
	{
		this->setActive();
	}
}

void InstructionMemory::step()
{
	outputAddress = linkList[0]->getVal();
	linkList[1]->setVal(data[outputAddress]);
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void AndGate::step()
{
	luint input1 = linkList[0]->getVal();
	luint input2 = linkList[1]->getVal();
	luint output = input1 & input2;
	linkList[2]->setVal(output);
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

luint AndGate::getOutput()
{ 
	return linkList[2]->getVal(); 
}

void SignExtend::step()
{
	luint result;
	luint input = linkList[0]->getVal();
	if((input>>15) % 2)
	{
		// MSB is high, extend number with 16 1's.
		result = ((((luint)(1)<<16)-1)<<16) + input; 
	}
	else
	{
		// MSB is low, extend number with 16 0's (ie. do nothing).
		result = input;
	}
	linkList[1]->setVal(result);
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void ShiftLeft2::step()
{
	luint input = linkList[0]->getVal();
	luint result = (input<<2) % ((luint)(1)<<32);
	linkList[1]->setVal(result);
	if(!isActive() && allInputsActive())
	{
		this->setActive();
	}
}

void Forwarding::step()
{
	decrementDelayRemaining();
			
	luint readReg1 = linkList[FORWARDING_READREG1]->getVal();
	luint readReg2 = linkList[FORWARDING_READREG2]->getVal();
	luint exMemReg = linkList[FORWARDING_EXMEMREG]->getVal();
	luint memWBReg = linkList[FORWARDING_MEMWBREG]->getVal();
	int muxA = 0;
	int muxB = 0;
	
	if(readReg1 == exMemReg)
	{
		muxA = 2;
	}
	else if(readReg1 == memWBReg)
	{
		muxA = 1;
	}
	
	if(readReg2 == exMemReg)
	{
		muxB = 2;
	}
	else if(readReg2 ==  memWBReg)
	{
		muxB = 1;	
	}

	// Now, catch if read registers are either 0, as this register cannot be written to
	// so in this case do not forward.
	if(readReg1 == 0)
	{
		muxA = 0;
	}
	if(readReg2 == 0)
	{
		muxB = 0;
	}
	
	linkList[FORWARDING_MUXA]->setVal(muxA);
	linkList[FORWARDING_MUXB]->setVal(muxB);
	if(getDelayRemaining() == 0)
	{
		resetDelayRemaining();	
	}
			
	if(!isActive() && linkList[FORWARDING_READREG1]->isActive() && linkList[FORWARDING_READREG2])
	{
		this->setActive();
	}
}

void PC::step()
{
	delayRemaining--;
	if(delayRemaining == delay)
	{
		this->setActive();
	}
	if(delayRemaining == storeStep) 
	{
		tempAddr = linkList[0]->getVal();
	}
	if(delayRemaining == updateStep) // Needs to be 0 for simple layout and 1 for other!
	{
		linkList[1]->setVal(tempAddr);
	}
	if(delayRemaining == 0)
	{
		delayRemaining = delay;
	}
}

void IFIDReg::step()
{
	decrementDelayRemaining();
	if(getDelayRemaining() == 1)
	{
		linkList[2]->setVal(linkList[0]->getVal());
		linkList[3]->setVal(linkList[1]->getVal());
	}
	if(getDelayRemaining() == 0)
	{
		resetDelayRemaining();
	}
}

void IDEXReg::step()
{
	decrementDelayRemaining();
	if(getDelayRemaining() == 1)
	{
		// Extract the three Execution control lines
		int EX = linkList[6]->getVal();
		bool RegDst = !!(EX >> 3);
		int ALUOp = (EX >> 1) - (RegDst << 2);
		bool ALUSrc = !!(EX % 2);
		
		linkList[9]->setVal(linkList[0]->getVal());
		linkList[10]->setVal(linkList[1]->getVal());
		linkList[11]->setVal(linkList[2]->getVal());
		linkList[12]->setVal(linkList[3]->getVal());
		linkList[13]->setVal(linkList[4]->getVal());
		linkList[14]->setVal(linkList[5]->getVal());
		linkList[15]->setVal(ALUSrc);
		linkList[16]->setVal(ALUOp);
		linkList[17]->setVal(RegDst);
		linkList[18]->setVal(linkList[7]->getVal());
		linkList[19]->setVal(linkList[8]->getVal());
		// If layout type is forwarding ... 
		if(layout == LAYOUT_FORWARDING)
		{
			linkList[21]->setVal(linkList[20]->getVal());
		}
	}
	if(getDelayRemaining() == 0)
	{
		resetDelayRemaining();
	}
}

void EXMEMReg::step()
{
	decrementDelayRemaining();
	if(getDelayRemaining() == 1)
	{
		int MEM = linkList[5]->getVal();
		bool MemRead = !!(MEM >> 2);
		bool MemWrite = !!((MEM >> 1) - (MemRead << 1));
		bool Branch = !!(MEM % 2);
		
		linkList[7]->setVal(linkList[0]->getVal());
		linkList[8]->setVal(linkList[1]->getVal());
		linkList[9]->setVal(linkList[2]->getVal());
		linkList[10]->setVal(linkList[3]->getVal());
		linkList[11]->setVal(linkList[4]->getVal());
		linkList[12]->setVal(Branch);
		linkList[13]->setVal(MemWrite);
		linkList[14]->setVal(MemRead);
		linkList[15]->setVal(linkList[6]->getVal());
	}
	if(getDelayRemaining() == 0)
	{
		resetDelayRemaining();
	}
}

void MEMWBReg::step()
{
	decrementDelayRemaining();
	if(getDelayRemaining() == 1)
	{
		int WB = linkList[3]->getVal();
		bool RegWrite = !!(WB >> 1);
		bool MemToReg = !!(WB % 2);
		
		linkList[4]->setVal(linkList[0]->getVal());
		linkList[5]->setVal(linkList[1]->getVal());
		linkList[6]->setVal(linkList[2]->getVal());
		linkList[7]->setVal(MemToReg);
		linkList[8]->setVal(RegWrite);
	}
	if(getDelayRemaining() == 0)
	{
		resetDelayRemaining();
	}
}

void Registers::maintainReg0()
{
	// Keep register 0 with the value 0;
	data[0] = 0;
}
