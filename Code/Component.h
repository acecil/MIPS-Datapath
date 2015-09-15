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
 
#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <GL/glut.h>
#include <wx/msw/winundef.h>

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <list>
#include <iostream>
#include <limits>
#include <exception>

#include <wx/wx.h>
#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/colour.h>

#include "Types.h"
#include "Enums.h"
#include "Maths.h"
#include "Config.h"
#include "Side.h"
#include "Color.h"
#include "Coord.h"
#include "Link.h"

class Link;
class InputLink;
class OutputLink;

enum ComponentType
{
	MUX_TYPE,
	MUX_3_TYPE,
	MAIN_CONTROL_TYPE,
	ALU_CONTROL_TYPE,
	SIGN_EXTEND_TYPE,
	SHIFT_LEFT_2_TYPE,
	INSTRUCTION_MEMORY_TYPE, 
	REGISTERS_TYPE,
	DATA_MEMORY_TYPE,
	ALU_TYPE,
	AND_GATE_TYPE,
	PC_TYPE,
	NODE_TYPE,
	IFID_REG_TYPE,
	IDEX_REG_TYPE,
	EXMEM_REG_TYPE,
	MEMWB_REG_TYPE,
	FORWARDING_TYPE,
};

class Component
{
	public:
		Component(){};
		Component(double x, double y, double w, double h, wxString name, ComponentType type, bool control = false, bool PC = false, double textPosX = 0, double textPosY = 1);
		virtual ~Component(){};
		virtual void reset(){};
		virtual void resetColour(){ fillColorMin = colours[COMPONENT_COLOUR]; };
		// Drawing functions:
		static void drawBackground();
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale) = 0;
		void drawName(bool showControl, bool showPC, double scale);
		void drawConnections(bool showControl, bool showPC, Symbol* instr, bool simpleLayout);
		virtual void drawConnectors(bool showControl, bool showPC, Symbol* instr, bool simpleLayout);
		void drawLinkTriangle(Side edge, bool large);
		void position();
		// Setup functions:
		Link* createInput(int slot, double x, double y, Side edge, wxString name = _T(""),  
						  bool control = false, bool PC = false, bool showText = false, double textX = 0, double textY = 0,
						  int startBit = 0, int bitLength = 32);
		Link* createOutput(int slot, double x, double y, Side edge, wxString name = _T(""), 
						   bool control = false, bool PC = false, bool showText = false, double textX = 0, double textY = 0);
		void addLinkVertex(uint linkNum, double x, double y);
		void connect(int thisSlot, Component *comp, int otherSlot);
		// Run functions:
		virtual void preStep();
		virtual void step(){};
		virtual bool allInputsActive();
		// Accessor functions:
		int getX(){ return int(pos.x); };
		int getY(){ return int(pos.y); };
		virtual int getW(){ return int(w); };
		virtual int getH(){ return int(h); };
		wxString getName(){ return name; };
		wxString getMainInfo();
		ComponentType getType(){ return type; };
		int getID(){ return ID; };
		virtual bool isActive(){ return activeOutputs; };
		virtual luint getVal(){ return 0; };
		Link* getLink(uint slot);
		void setX(double x){ pos.x = x; };
		void setY(double y){ pos.y = y; };
		void setLinkData(uint linkNum, luint value);
		void setLinkActive(uint linkNum, bool active = true);
		void setLinkBits(uint linkNum, uint startBit, uint bitLength);
		void setActive(bool active = true);
		void setInstrActive(uint linkNum, Symbol instr, bool active);
		virtual void confirmActive(){ activeOutputs = preActive; };
		// Functions used to build layout:
		Link* getCurrentLink(){ return currentLink; };
		void printLinkData();
		int findInput(wxPoint mouseLocation);
		int findOutput(wxPoint mouseLocation);
		int findNearestLink(wxPoint mouseLocation, bool isOutput);
		// Useful numbers:
		static const int NUM_CIRCLE_LINES = 20;
		static const double PI;
		Color activeLinkColor;
		virtual bool getIsControl(){ return control; };
		void setLinkControl(bool c, int l);
		virtual bool getIsPC(){ return PC; };
		static const int simpleClockSteps = 7;
		static const int pipelineClockSteps = 6;
		static void setCurrentInstruction(int instr);
		void setPipelineStage(uint stage){ pipelineStage = stage; };
		uint getPipelineStage(){ return pipelineStage; };
		static void incrementPipelineCycle();
		static void resetPipelineCycle(){ pipelineCycle = 0; };
		static void setActiveLinkColour(wxColour colour, uint stage);
		static Color getActiveLinkColour(uint stage);
		static void setColour(configName name, wxColour colour);
		static Color getColour(configName name){ return colours[name]; };
		static wxColour getwxColour(configName name);
		float getOldActiveLinkColor(){ return oldActiveLinkColor; };
		void resetOldActiveLinkColor(){ oldActiveLinkColor = 1.0; };
		static bool getAreDataLinesBold(){ return dataLinesBold; };
		static void setAreDataLinesBold(bool val = true){ dataLinesBold = val; };
		static bool getHighlightSingleInstruction(){ return highlightSingleInstruction; };
		static void setHighlightSingleInstruction(bool val = true){ highlightSingleInstruction = val; };
	protected:
		void drawShadedRectangle();
		void drawRectangle();
		static std::map<configName, Color> colours;
		static std::vector<Color> activeLinkColors;
		float oldActiveLinkColor;
		Color fillColorMin;
		static Color scaleColors(Color c1, Color c2, double proportion);
		std::map<int, std::unique_ptr<Link>> linkList;
		uint pipelineStage; // What pipiline stage is this component in?
		static uint pipelineCycle; // What pipeline stage is the program in?
		static bool dataLinesBold;
		static int currentInstruction;
		static bool highlightSingleInstruction;
	private:
		wxString getLinkInfo(int ID, Link* currLink);
		void * getFont();
		Link* currentLink; // Current link, used when setting up layouts.
		wxString name; 
		ComponentType type;
		Coord pos;          // The x, y coordinates of the bottom left corner of the component.
		Coord textPos;		// The x, y coordinates of where to place the name of the component.
		double w;			// The width of the component.
		double h; 			// The height of the component.
		int ID;				// A unique ID for the component. Used for auto generation of code.
		bool preActive;     // The component should be shown as activeOutputs next step.
		bool activeOutputs; // The component has active outputs.
		static int count;   // Count of the number of components created.
		bool control;		// Is this component a control type of component.
		bool PC;			// Is this component involved in dealing with the PC.
		static const GLfloat DATA_LINE_WIDTH;
		static const GLfloat THIN_DATA_LINE_WIDTH;
};

class MuxBase: public Component
{
	public:
		MuxBase(double x, double y, double w, double h, wxString name, ComponentType type, bool PC = false);
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
	private:
};

class Mux: public MuxBase
{
	public:
		Mux(double x, double y, double w = 5, double h = 15, wxString name = _T("Mux"), bool PC = false);
		virtual void step();
	private:
};

class Mux3: public MuxBase
{
	public:
		Mux3(double x, double y, double w = 5, double h = 15, wxString name = _T("Mux3"));
	private:
		virtual void step();
};

class Control: public Component
{
	public:
		Control(double x, double y, double w, double h, wxString name, ComponentType type, bool control = true, bool PC = false, double textPosX = 0, double textPosY = 1)
		: Component(x, y, w, h, name, type, control, PC, textPosX, textPosY){};
	private:
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		
};

class mainControlLookup
{
	public:
		mainControlLookup(){};
		mainControlLookup(bool RegDst, bool Branch, bool MemToReg, bool MemRead, int ALUOp, 
						  bool MemWrite, bool ALUSrc, bool RegWrite)
						{ this->RegDst = RegDst; this->Branch = Branch; this->MemToReg = MemToReg;
						  this->MemRead = MemRead; this->ALUOp = ALUOp; this->MemWrite = MemWrite;
						  this->ALUSrc = ALUSrc; this->RegWrite = RegWrite; };
		bool getRegDst(){ return RegDst; };
		bool getBranch(){ return Branch; };
		bool getMemToReg(){ return MemToReg; };
		bool getMemRead(){ return MemRead; };
		int getALUOp(){ return ALUOp; };
		bool getMemWrite(){ return MemWrite; };
		bool getALUSrc(){ return ALUSrc; };
		bool getRegWrite(){ return RegWrite; };
		int getWB(){ return RegWrite*2 + MemToReg; };
		int getMEM(){ return MemRead*4 + MemWrite*2 + Branch; };
		int getEX(){ return RegDst*8 + ALUOp*2 + ALUSrc; };
	private:
		bool RegDst;
		bool Branch;
		bool MemToReg;
		bool MemRead;
		int ALUOp;
		bool MemWrite;
		bool ALUSrc;
		bool RegWrite;
};

class MainControl: public Control
{
	public:
		MainControl(int layout, double x, double y, double w = 10, double h = 15, wxString name = _T("Control"));
		virtual void step();
	private:
		std::map<uint, std::unique_ptr<mainControlLookup>> lookup;
		
};

class MainControlPipelined: public Control
{
	public:
		MainControlPipelined(int layout, double x, double y, double w = 10, double h = 15, wxString name = _T("Control"));
		virtual void step();
	private:
		std::map<uint, std::unique_ptr<mainControlLookup>> lookup;
};

class ALUControl: public Control
{
	public:
		ALUControl(double x, double y, double w = 10, double h = 15, wxString name = _T("ALU Control"));
		virtual void step();
	private:
		std::map<uint, uint> funcLookup;
};

class SignExtend: public Control
{
	public:
		SignExtend(double x, double y, double w = 10, double h = 15, wxString name = _T("Sign Extend"));
		virtual void step();
	private:
};

class ShiftLeft2: public Control
{
	public:
		ShiftLeft2(double x, double y, double w = 10, double h = 15, wxString name = _T("Shift Left 2"), bool PC = false);
		virtual void step();
	private:
};

class Memory: public Component
{
	public:
		Memory(double x, double y, double w, double h, wxString name, ComponentType type, double textPosX = 0, double textPosY = 1)
		: Component(x, y, w, h, name, type, false, false, textPosX, textPosY){};
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		luint getData(luint address);
		void setData(luint address, luint val){ data[address] = val; };
	protected:
		std::map<luint, luint> data;
	private:
};

class InstructionMemory: public Memory
{
	public:
		InstructionMemory(double x, double y, double w = 25, double h = 25, wxString name = _T("Instruction Memory"));
		virtual void step();
		luint getOutputAddress(){ return outputAddress; };
	private:
		luint outputAddress;
};

class Registers: public Memory
{
	public:
		Registers(Layout layout, double x, double y, double w = 25, double h = 25, wxString name = _T("Registers"));
		virtual void step();
		virtual bool allInputsActive();
		virtual void reset();
	private:
		int writeToReg;
		void maintainReg0();
		int writeDelay;
};

class DataMemory: public Memory
{
	public:
		DataMemory(Layout layout, double x, double y, double w = 25, double h = 25, wxString name = _T("Data Memory"));
		virtual void step();
		virtual void reset();
	private:
		int writeToMem;
		int writeDelay;
		int writeWait;
};

class ALU: public Component
{
	public:
		ALU(double x, double y, double w = 10, double h = 25, wxString name = _T("ALU"), bool PC = false);
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		virtual void step();
	private:
};

class AndGate: public Component
{
	public:
		AndGate(double x, double y, double w = 8, double h = 5, wxString name = _T("AndGate"), bool PC = false);
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		virtual void step();
		luint getOutput();
	private:
};

class PC: public Component
{
	public:
		PC(int layout, double x, double y, double w = 5, double h = 8, wxString name = _T("PC"));
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		virtual void step();
		virtual void reset();
		luint getOutput();
	private:	
		uint delayRemaining;
		uint delay;
		luint tempAddr;
		uint updateStep;
		uint storeStep;
};

class PipelineRegister: public Component
{
	public:
		PipelineRegister(double x, double y, double w, double h, wxString name, ComponentType type, double textPosX = 0.0, double textPosY = 1.0);
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		virtual void reset();
		virtual void resetColour(){ fillColorMin = colours[PIPEREG_COLOUR]; };
		wxString getMainInfo(wxPoint mousePos);
	protected:
		void setDelay(int delay){ this->delay = delay; };
		void decrementDelayRemaining(){ delayRemaining--; };
		int getDelayRemaining(){ return delayRemaining; };
		void resetDelayRemaining(){ delayRemaining = delay; };
	private:
		uint delay;
		uint delayRemaining;
};

class IFIDReg: public PipelineRegister
{
	public:
		IFIDReg(Layout layout, double x, double y, double w = 4, double h = 120, wxString name = _T("IF/ID"));
	private:
		virtual void step();
};

class IDEXReg: public PipelineRegister
{
	public:
		IDEXReg(Layout layout, double x, double y, double w = 4, double h = 120, wxString name = _T("ID/EX"));
	private:
		virtual void step();
		Layout layout;
};

class EXMEMReg: public PipelineRegister
{
	public:
		EXMEMReg(Layout layout, double x, double y, double w = 4, double h = 120, wxString name = _T("EX/MEM"));
	private:
		virtual void step();
};

class MEMWBReg: public PipelineRegister
{
	public:
		MEMWBReg(Layout layout, double x, double y, double w = 4, double h = 120, wxString name = _T("MEM/WB"));
	private:
		virtual void step();
};

class Forwarding: public Component
{
	public:
		Forwarding(double x, double y, double w = 30, double h = 10, wxString name = _T("Forwarding"));
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
	private:
		void setDelay(int delay){ this->delay = delay; };
		void decrementDelayRemaining(){ delayRemaining--; };
		int getDelayRemaining(){ return delayRemaining; };
		void resetDelayRemaining(){ delayRemaining = delay; };
		uint delay;
		uint delayRemaining;
		virtual void step();
};

class Node: public Component
{
	public:
		Node(double x, double y, double w = 2, double h = 2, wxString name = _T(""));
		virtual void draw(bool showControl, bool showPC, Symbol* currInstr, bool simpleLayout, double scale);
		virtual void drawConnectors(bool showControl, bool showPC){};
		virtual bool isActive();
		virtual luint getVal();
		virtual bool getIsControl();
		virtual bool getIsPC();
		virtual int getW(){ return 0; };
		virtual int getH(){ return 0; };
	private:
		bool isActiveValid(Symbol instr);
};

#endif /*COMPONENT_H_*/
