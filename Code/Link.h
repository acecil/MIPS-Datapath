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
 
#ifndef LINK_H_
#define LINK_H_

#include <vector>
#include <string>
#include <list>
#include <map>
#include <sstream>

#include <wx/string.h>

#include "Coord.h"
#include "Side.h"
#include "Types.h"
#include "Enums.h"

class Component;
class OutputLink;

class Link
{
	public:
		Link(int slot, Component *comp, double x, double y, Side edge, bool output, bool control = false, bool PC = false, bool showText = false, wxString name = _T(""), double textX = 0, double textY = 0);
		virtual ~Link(){};
		int getID(){ return ID; };
		double getX(){ return pos.x; };
		double getY(){ return pos.y; };
		wxString getInfo();
		int getTipPos(Side s);
		void setName(wxString name){ this->name = name; };
		void setTextPos(double x, double y){ this->textPos = Coord(x, y); };
		wxString getName(){ return name; };
		Side getSide(){ return edge; };
		bool isShowText(){ return showText; };
		Coord getTextPos(){ return textPos; };
		void addVertex(double x, double y);
		std::vector<Coord> getVertices(){ return vertices; };
		bool isOutput(){ return output; };
		int getSlot(){ return slot; };
		Component* getComp(){ return comp; };
		void setVertexList(const std::vector<Coord> &vertices);
		int getAbsX();
		int getAbsY();
		virtual bool isActive() = 0;
		bool isActiveValid(Symbol instr = SYM_BAD);
		void setVal(luint value){ this->value = value; };
		virtual luint getVal() = 0;
		void setActive(bool active = true){ this->active = active; };
		virtual void setBits(uint startBit, uint bitLength){};
		bool getIsControl(){ return control; };
		void setControl(bool c){ control = c; };
		bool getIsPC(){ return PC; };
		void setInstrActivity(Symbol inst, bool active);
		virtual bool isConnected()=0;
		void setNegBit16(){ negBit16 = true; };
		bool getNegBit16(){ return negBit16; };
	protected:
		bool active;
		bool negBit16;
		luint value; // Value in link.
		std::map<Symbol, bool> instructionActive;
	private:
		bool output;
		bool showText;
		bool control;
		bool PC;
		Side edge;
		Coord pos;
		Coord textPos;
		wxString name;
		std::vector<Coord> vertices;
		int ID;
		int slot;
		static const int LINK_WIDTH;
		static const int LINK_HEIGHT_TALL;
		static const int LINK_HEIGHT_SHORT;
		static int count;
		Component* comp;
};

class InputLink: public Link
{
	public:
		InputLink(int slot, Component* comp, double x, double y, Side edge, wxString name = _T(""), bool control = false, bool PC = false, bool showText = false, uint startBit = 0, uint bitLength = 32, double textX = 0, double textY = 0);
		void connectOutput(OutputLink* output){ connectedOutput = output; connected = true; };
		OutputLink* getOutput(){ return connectedOutput; };
		virtual bool isConnected(){ return connected; };
		void getValFromOutput();
		virtual bool isActive();
		virtual luint getVal();
		virtual void setBits(uint startBit,uint bitLength){ this->startBit = startBit; this->bitLength = bitLength; };
	private:
		OutputLink* connectedOutput;
		bool connected;
		uint startBit;
		uint bitLength;
		luint extractRequiredBits(luint val);
};

class OutputLink: public Link
{
	public:
		OutputLink(int slot, Component* comp, double x, double y, Side edge, wxString name = _T(""), bool control = false, bool PC = false, bool showText = false, double textX = 0, double textY = 0);
		void connect(Link* link);
		virtual bool isConnected();
		std::vector<Link*> getLinkList(){ return linkList; };
		virtual bool isActive();
		virtual luint getVal();
	private:
		std::vector<Link*> linkList;
};

#endif /*LINK_H_*/
