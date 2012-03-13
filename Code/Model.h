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
 
#ifndef MODEL_H_
#define MODEL_H_

#include "Enums.h"
#include "Types.h"
#include "Config.h"
#include <list>
#include <iostream>
#include <wx/gdicmn.h>
#include <wx/utils.h>
#include <wx/string.h>
#include <wx/colour.h>
#include <limits>
#include <cmath>
#include <map>

class Component;
class Link;
class Scanner;
class Parser;
class Memory;
class PipelineRegister;
class PC;
class AndGate;
class OutputLink;
class InputLink;
struct Coord;

class Model
{
	public:
		Model();
		~Model();
		void resetColours();
		void getError(int num, wxString& error);
		int getNumberOfErrors();
		void getFileContents(wxString & str);
		luint getParserMemData(luint address);
		void setParserMemData(luint address, luint data);
		void parse(wxString str);
		void getInstructionString(luint address, wxString& instr, bool & err);
		void setup();
		void resetup();
		void resetup(Layout newLayout);
		void loadFile(wxString file);
		void saveFile(wxString file);
		void step();
		void reset();
		void draw(wxPoint mousePos);
//		void printAllData();
//		bool isCreatingConnection(){ return creatingConnection; };
//		void findInput(wxPoint mouseLocation);
//		void findOutput(wxPoint mouseLocation);
//		void cancelCreatingConnection();
//		void startMovingComponent(wxPoint mousePos);
//		void moveComponent(wxPoint mousePos);
//		void finishMovingComponent(wxPoint mousePos);
//		bool isMovingComponent(){ return movingComponent; };
		void addVertex(wxPoint mousePos);
		Component* findComponent(wxPoint mousePos);
		Link* findLink(wxPoint mousePos);
		luint getMemoryData(uint mem, luint address);
		void setMemoryData(uint mem, luint address, luint data);
		void getFieldedInstruction(luint address, wxString & str);
		void getParsedInstructions();
		luint getCurrentInstruction();
		void setHighlightInstruction(luint instr, bool valid = true);
		wxColour getColourForInstruction(uint loc);
		static const luint MAX_MEM_VAL = (((luint)1)<<32) - 1;
		static const uint MAX_INSTRUCTIONS = 512;
		static const uint MAX_REGISTERS = 32;
		static const uint MAX_DATA = 512;
		static void setBool(configName name, bool newBool){ bools[name] = newBool; };
		static bool getBool(configName name){ return bools[name]; };
	private:
	  	std::list<Component*> components;
	  	std::map<uint, Memory*> memories;
	  	PipelineRegister* buffers[4];
	  	PC *programCounter;
	  	AndGate* branchCheckGate;
	  	bool creatingConnection;
	  	bool movingComponent;
	  	Component* comp;
	  	wxRealPoint movingDifference;
	  	OutputLink* oLink;
	  	InputLink* iLink;
	  	std::list<Coord*> vertices;
	  	Layout layout;
	  	Scanner *smz;
	  	Parser *pmz;
	  	bool validInstructions;
	  	Symbol currInstr[5];
	  	luint currAddr[5];
	  	uint lastAddrLoc;
	  	static std::map<configName, bool> bools;
};

#endif /*MODEL_H_*/
