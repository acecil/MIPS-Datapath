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
#include "Scanner.h"
#include "Parser.h"
#include "Config.h"
#include "Link.h"

#include "Model.h"

std::map<configName, bool> Model::bools;

Model::Model()
	: smz(std::make_unique<Scanner>(_T(""))), pmz(std::make_unique<Parser>(smz.get()))
{
	creatingConnection = false;
	layout = LAYOUT_SIMPLE;
	validInstructions = false;
}

void Model::resetColours()
{
	for(auto &&i : components)
	{
		if(i->getType() != NODE_TYPE)
		{
	 		i->resetColour();
	 	}
	}
}

luint Model::getCurrentInstruction()
{
	return programCounter->getOutput();
}

void Model::getError(int num, wxString& error)
{
	pmz->getError(num, error);
}

int Model::getNumberOfErrors()
{
	return pmz->getNumberOfErrors();
}

void Model::getInstructionString(luint address, wxString& instr, bool & err)
{
	pmz->getInstructionString(address, instr, err);
}

wxColour Model::getColourForInstruction(uint loc)
{
	if(layout == LAYOUT_SIMPLE)
	{
		if(programCounter->getOutput() == loc)
		{
			return Component::getwxColour(STAGE1_COLOUR);
		}
	}
	else
	{
		if(loc == currAddr[0])
		{
			return Component::getwxColour(STAGE1_COLOUR);
		}
		else if(loc == currAddr[1])
		{
			return Component::getwxColour(STAGE2_COLOUR);
		}
		else if(loc == currAddr[2])
		{
			return Component::getwxColour(STAGE3_COLOUR);
		}
		else if(loc == currAddr[3])
		{
			return Component::getwxColour(STAGE4_COLOUR);
		}
		else if(loc == currAddr[4])
		{
			return Component::getwxColour(STAGE5_COLOUR);
		}
	}
	return wxColour(255, 255, 255);
}

void Model::getFileContents(wxString & str)
{
	str.Clear();
	bool err;
	wxString instr;
	for(uint i = 0; i < MAX_DATA; i += 4)
	{
		getInstructionString(i, instr, err);
		str += instr;
	}
}

void Model::resetup()
{
	resetup(layout);
}

void Model::resetup(Layout newLayout)
{
	components.clear();
	Component::resetPipelineCycle();
	layout = newLayout;
	setup();
	getParsedInstructions();
	for(int i = 0; i < 5; i++)
	{
		currInstr[i] = SYM_BAD;
		currAddr[i] = 0;
	}
	luint addr = programCounter->getOutput();
	currInstr[0] = pmz->getInstructionSymbol(addr);
	currAddr[0] = addr;
	lastAddrLoc = 0;
	if(layout != LAYOUT_SIMPLE)
	{
	 	// With non simple layout set all components to active.
	 	for(auto&& i : components)
		{
 			i->setActive();
 			i->confirmActive();
		}
 	}
}

void Model::loadFile(wxString file)
{
	smz.reset();
	pmz.reset();
	
	smz = std::make_unique<Scanner>(file);
	pmz = std::make_unique<Parser>(smz.get());

	if(smz->checkFile())
	{
		validInstructions = pmz->readIn();
	}
	else
	{
		validInstructions = false;
	}
	getParsedInstructions();
}

void Model::saveFile(wxString file)
{
	if(file != _T(""))
	{
		wxString curLine;
		uint address = 0;
		bool err;
		std::ofstream sfile(file.fn_str());
		do
		{
			pmz->getInstructionString(address, curLine, err);
			sfile.write((const char*)curLine.fn_str(), curLine.Len());
			address += 4;
		}
		while(address < MAX_INSTRUCTIONS);
		sfile << "Data:\n";
		address = 0;
		do
		{
			pmz->getMemString(address, curLine);
			sfile.write((const char*)curLine.fn_str(), curLine.Len());
			address ++;
			if(!(address % 16))
			{
				sfile << "\n";
			}
		}
		while(address < MAX_DATA);
		sfile.close();
	}
}

void Model::parse(wxString str)
{
	wxString localStr = str;
	localStr.Replace(_T("Data"), _T(""));
	if(str != _T(""))
	{
		// Add tags and data to string for parsing.
		localStr = localStr + _T("\nDATA:\n");
		for(uint i = 0; i < MAX_DATA; i ++)
		{
			localStr << memories[ID_DATA_LIST]->getData(i) << _T(", ");
			if(i % 16)
			{
				localStr << _T("\n");
			}
		}
		validInstructions = pmz->parse(localStr);
	}
	else
	{
		validInstructions = pmz->parse(localStr);
	}
}

luint Model::getParserMemData(luint address)
{
	return pmz->getMemData(address);
}

void Model::setParserMemData(luint address, luint data)
{
	pmz->setMemData(address, data);
}

void Model::getParsedInstructions()
{
	if(validInstructions)
	{
		for(uint i = 0; i < MAX_INSTRUCTIONS; i += 4)
		{
			memories[ID_INSTRUCTION_LIST]->setData(i, pmz->getInstruction(i));
		}
	}
}

void Model::getFieldedInstruction(luint address, wxString & str)
{
	bool valid[6];
	uint p[6];
	str.Clear();
	
	luint data = memories[ID_INSTRUCTION_LIST]->getData(address);
	Symbol sym = pmz->getInstructionSymbol(address);
	
	for(uint i = 0; i < 6; i++)
	{
		valid[i] = true;
	}
	
	p[0] = data >> 26;
	if(sym != SYM_NOOP && sym != SYM_BAD)
	{
		p[1] = (data - (p[0] << 26)) >> 21;
		p[2] = (data - (p[0] << 26) - (p[1] << 21)) >> 16;
		if(sym == SYM_LW || sym == SYM_SW || sym == SYM_ADDI || sym == SYM_BEQ)
		{
			valid[3] = false;
			valid[4] = false;
			p[5] = data % (1 << 16);
		}
		else
		{
			p[3] = (data - (p[0] << 26) - (p[1] << 21) - (p[2] << 16)) >> 11;
			p[4] = data % (1 << 11);
			valid[5] = false;
		}
	}
	else
	{
		for(uint i = 1; i < 6; i++)
		{
			valid[i] = false;
		}
	}
	for(uint j = 0; j < 6; j++)
	{
		if(valid[j])
		{
			
			str += Maths::convertToBase(p[j], false, j == 5 ? true : false);
			str += _T(", ");
		}
	}
	str.RemoveLast();
	str.RemoveLast();
}

void Model::setup()
{
	if(layout == LAYOUT_SIMPLE)
	{
		auto PC0 = std::make_shared<PC>(layout, 6, 67, 5, 8, _T("PC"));
		components.push_back(PC0);
		programCounter = PC0;
		PC0->setInstrActive(1, SYM_MAX, true);
		PC0->setInstrActive(1, SYM_NOOP, true);
		auto InstructionMemory1 = std::make_shared<InstructionMemory>(23, 59, 25, 25, _T("Instruction\n Memory"));
		memories[ID_INSTRUCTION_LIST] = InstructionMemory1;
		InstructionMemory1->setInstrActive(1, SYM_MAX, true);
		InstructionMemory1->setInstrActive(1, SYM_NOOP, true);
		components.push_back(InstructionMemory1);
		auto Mux2 = std::make_shared<Mux>(75, 47, 5, 15, _T("M\nU\nX\n4"));
		Mux2->setLinkBits(0, 16, 5);
		Mux2->setLinkBits(1, 11, 5);
		components.push_back(Mux2);
		Mux2->setInstrActive(3, SYM_EOF, true);
		Mux2->setInstrActive(3, SYM_LW, true);
		Mux2->setInstrActive(3, SYM_ADDI, true);
		auto MainControl3 = std::make_shared<MainControl>(layout, 69, 90, 16, 34, _T("Control"));
		MainControl3->setLinkData(8, 1);
		components.push_back(MainControl3);
		MainControl3->setInstrActive(MAINCONTROL_REGDST, SYM_EOF, true);
		MainControl3->setInstrActive(MAINCONTROL_REGDST, SYM_LW, true);
		MainControl3->setInstrActive(MAINCONTROL_MEMREAD, SYM_LW, true);
		MainControl3->setInstrActive(MAINCONTROL_MEMTOREG, SYM_LW, true);
		MainControl3->setInstrActive(MAINCONTROL_MEMTOREG, SYM_EOF, true);
		MainControl3->setInstrActive(MAINCONTROL_REGWRITE, SYM_EOF, true);
		MainControl3->setInstrActive(MAINCONTROL_REGWRITE, SYM_LW, true);
		MainControl3->setInstrActive(MAINCONTROL_ALUOP, SYM_MAX, true);
		MainControl3->setInstrActive(MAINCONTROL_ALUSRC, SYM_MAX, true);
		MainControl3->setInstrActive(MAINCONTROL_BRANCH, SYM_MAX, true);
		MainControl3->setInstrActive(MAINCONTROL_BRANCH, SYM_NOOP, true);
		MainControl3->setInstrActive(MAINCONTROL_MEMTOREG, SYM_ADDI, true);
		MainControl3->setInstrActive(MAINCONTROL_REGDST, SYM_ADDI, true);
		MainControl3->setInstrActive(MAINCONTROL_REGWRITE, SYM_ADDI, true);
		MainControl3->setInstrActive(MAINCONTROL_MEMWRITE, SYM_SW, true);
		auto ALU4 = std::make_shared<ALU>(30, 137, 10, 25, _T("Add"), true);
		components.push_back(ALU4);
		ALU4->setLinkData(1, 4); // Set the PC adder to have the second input set to 4 permanently.
		ALU4->setLinkActive(1); // Set the second input to active so outputs propogate.
		ALU4->setLinkData(2, 2); // Set the PC adder to always add.
		ALU4->setLinkActive(2); // Set control input to active.
		ALU4->setInstrActive(3, SYM_MAX, true);
		ALU4->setInstrActive(3, SYM_NOOP, true);
		auto Registers5 = std::make_shared<Registers>(layout, 91, 59, 25, 25, _T("Registers"));
		memories[ID_REGISTER_LIST] = Registers5;
		components.push_back(Registers5);
		Registers5->setInstrActive(5, SYM_MAX, true);
		Registers5->setInstrActive(6, SYM_EOF, true);
		Registers5->setInstrActive(6, SYM_BEQ, true);
		Registers5->setInstrActive(6, SYM_SW, true);
		auto SignExtend6 = std::make_shared<SignExtend>(107, 35, 9, 15, _T("  Sign\nExtend"));
		SignExtend6->setLinkBits(0, 0, 16);
		components.push_back(SignExtend6);
		SignExtend6->setInstrActive(1, SYM_ADDI, true);
		SignExtend6->setInstrActive(1, SYM_BEQ, true);
		SignExtend6->setInstrActive(1, SYM_SW, true);
		SignExtend6->setInstrActive(1, SYM_LW, true);
		auto Mux7 = std::make_shared<Mux>(141, 42, 5, 15, _T("M\nU\nX\n1"));
		components.push_back(Mux7);
		Mux7->setInstrActive(3, SYM_MAX, true);
		auto ShiftLeft28 = std::make_shared<ShiftLeft2>(129, 127, 9, 15, _T("Shift\n Left\n  2"), true);
		components.push_back(ShiftLeft28);
		ShiftLeft28->setInstrActive(1, SYM_BEQ, true);
		auto ALU9 = std::make_shared<ALU>(155, 129, 10, 25, _T("Add"), true);
		components.push_back(ALU9);
		ALU9->setLinkData(2, 2); // Set the Adder to always add.
		ALU9->setLinkActive(2); // Set the control input to active.
		ALU9->setInstrActive(3, SYM_BEQ, true);
		auto ALU10 = std::make_shared<ALU>(152, 44, 10, 25, _T("ALU"));
		components.push_back(ALU10);
		ALU10->setInstrActive(3, SYM_EOF, true);
		ALU10->setInstrActive(3, SYM_LW, true);
		ALU10->setInstrActive(3, SYM_ADDI, true);
		ALU10->setInstrActive(3, SYM_SW, true);
		ALU10->setInstrActive(4, SYM_BEQ, true);
		auto ALUControl11 = std::make_shared<ALUControl>(153, 22, 9, 15, _T("   ALU\nControl"));
		ALUControl11->setLinkBits(1, 0, 6);
		components.push_back(ALUControl11);
		ALUControl11->setInstrActive(2, SYM_MAX, true);
		auto DataMemory12 = std::make_shared<DataMemory>(layout, 178, 36, 25, 25, _T("  Data\nMemory"));
		memories[ID_DATA_LIST] = DataMemory12;
		components.push_back(DataMemory12);
		DataMemory12->setInstrActive(4, SYM_LW, true);
		auto AndGate13 = std::make_shared<AndGate>(177, 118, 8, 5, _T(""), true);
		components.push_back(AndGate13);
		AndGate13->setInstrActive(2, SYM_NOOP, true);
		AndGate13->setInstrActive(2, SYM_MAX, true);
		branchCheckGate = AndGate13;
		auto Mux14 = std::make_shared<Mux>(183, 137, 5, 15, _T("M\nU\nX\n2"), true);
		components.push_back(Mux14);
		Mux14->setInstrActive(3, SYM_MAX, true);
		Mux14->setInstrActive(3, SYM_NOOP, true);
		auto Mux15 = std::make_shared<Mux>(212, 52, 5, 15, _T("M\nU\nX\n3"));
		components.push_back(Mux15);
		Mux15->setInstrActive(3, SYM_EOF, true);
		Mux15->setInstrActive(3, SYM_LW, true);
		Mux15->setInstrActive(3, SYM_ADDI, true);
		auto Node16 = std::make_shared<Node>(53, 71, 2, 2, _T(""));
		components.push_back(Node16);
		Node16->setInstrActive(5, SYM_MAX, true);
		Node16->setInstrActive(5, SYM_NOOP, true);
		Node16->setInstrActive(6, SYM_MAX, true);
		auto Node17 = std::make_shared<Node>(16, 71, 2, 2, _T(""));
		components.push_back(Node17);
		Node17->setInstrActive(5, SYM_MAX, true);
		Node17->setInstrActive(7, SYM_MAX, true);
		Node17->setInstrActive(5, SYM_NOOP, true);
		Node17->setInstrActive(7, SYM_NOOP, true);
		auto Node18 = std::make_shared<Node>(143, 149, 2, 2, _T(""));
		components.push_back(Node18);
		Node18->setInstrActive(5, SYM_EOF, true);
		Node18->setInstrActive(5, SYM_NOOP, true);
		Node18->setInstrActive(5, SYM_LW, true);
		Node18->setInstrActive(5, SYM_ADDI, true);
		Node18->setInstrActive(5, SYM_SW, true);
		Node18->setInstrActive(5, SYM_BEQ, true);
		Node18->setInstrActive(7, SYM_BEQ, true);
		auto Node19 = std::make_shared<Node>(125, 52, 2, 2, _T(""));
		components.push_back(Node19);
		Node19->setInstrActive(7, SYM_EOF, true);
		Node19->setInstrActive(7, SYM_BEQ, true);
		Node19->setInstrActive(6, SYM_SW, true);
		auto Node20 = std::make_shared<Node>(121, 46, 2, 2, _T(""));
		components.push_back(Node20);
		Node20->setInstrActive(5, SYM_BEQ, true);
		Node20->setInstrActive(7, SYM_ADDI, true);
		Node20->setInstrActive(7, SYM_SW, true);
		Node20->setInstrActive(7, SYM_LW, true);
		auto Node21 = std::make_shared<Node>(171, 56, 2, 2, _T(""));
		components.push_back(Node21);
		Node21->setInstrActive(5, SYM_EOF, true);
		Node21->setInstrActive(5, SYM_ADDI, true);
		Node21->setInstrActive(7, SYM_LW, true);
		Node21->setInstrActive(7, SYM_SW, true);
		auto Node22 = std::make_shared<Node>(53, 74, 2, 2, _T(""));
		components.push_back(Node22);
		Node22->setInstrActive(5, SYM_MAX, true);
		Node22->setInstrActive(5, SYM_NOOP, true);
		Node22->setInstrActive(7, SYM_MAX, true);
		auto Node23 = std::make_shared<Node>(53, 69, 2, 2, _T(""));
		components.push_back(Node23);
		Node23->setInstrActive(7, SYM_EOF, true);
		Node23->setInstrActive(7, SYM_ADDI, true);
		Node23->setInstrActive(7, SYM_BEQ, true);
		Node23->setInstrActive(7, SYM_SW, true);
		Node23->setInstrActive(7, SYM_LW, true);
		Node23->setInstrActive(6, SYM_MAX, true);
		auto Node24 = std::make_shared<Node>(53, 51, 2, 2, _T(""));
		components.push_back(Node24);
		Node24->setInstrActive(6, SYM_MAX, true);
		Node24->setInstrActive(7, SYM_EOF, true);
		auto Node25 = std::make_shared<Node>(87, 42, 2, 2, _T(""));
		components.push_back(Node25);
		Node25->setInstrActive(6, SYM_EOF, true);
		Node25->setInstrActive(7, SYM_ADDI, true);
		Node25->setInstrActive(7, SYM_BEQ, true);
		Node25->setInstrActive(7, SYM_SW, true);
		Node25->setInstrActive(7, SYM_LW, true);
		auto Node26 = std::make_shared<Node>(65, 69, 2, 2, _T(""));
		components.push_back(Node26);
		Node26->setInstrActive(6, SYM_ADDI, true);
		Node26->setInstrActive(6, SYM_LW, true);
		Node26->setInstrActive(7, SYM_EOF, true);
		Node26->setInstrActive(7, SYM_BEQ, true);
		Node26->setInstrActive(7, SYM_SW, true);
		PC0->addLinkVertex(0, 205, 144);
		PC0->addLinkVertex(0, 205, 169);
		PC0->addLinkVertex(0, 2, 169);
		PC0->addLinkVertex(0, 2, 71);
		PC0->connect(1, Node17.get(), 0);
		InstructionMemory1->connect(1, Node16.get(), 0);
		Mux2->addLinkVertex(0, 65, 57);
		Mux2->connect(3, Registers5.get(), 4);
		MainControl3->addLinkVertex(0, 53, 107);
		MainControl3->connect(1, AndGate13.get(), 1);
		MainControl3->connect(2, Mux15.get(), 2);
		MainControl3->connect(3, DataMemory12.get(), 1);
		MainControl3->connect(4, ALUControl11.get(), 0);
		MainControl3->connect(5, DataMemory12.get(), 0);
		MainControl3->connect(6, Mux7.get(), 2);
		MainControl3->connect(7, Registers5.get(), 0);
		MainControl3->connect(8, Mux2.get(), 2);
		ALU4->addLinkVertex(0, 16, 157);
		ALU4->connect(3, Node18.get(), 0);
		Registers5->addLinkVertex(0, 103, 94);
		Registers5->addLinkVertex(1, 225, 59);
		Registers5->addLinkVertex(1, 225, 89);
		Registers5->addLinkVertex(1, 84, 89);
		Registers5->addLinkVertex(1, 84, 79);
		Registers5->addLinkVertex(4, 83, 54);
		Registers5->addLinkVertex(4, 83, 64);
		Registers5->connect(5, ALU10.get(), 0);
		Registers5->connect(6, Node19.get(), 1);
		SignExtend6->connect(1, Node20.get(), 2);
		Mux7->addLinkVertex(2, 143, 98);
		Mux7->connect(3, ALU10.get(), 1);
		ShiftLeft28->addLinkVertex(0, 121, 134);
		ShiftLeft28->connect(1, ALU9.get(), 1);
		ALU9->connect(3, Mux14.get(), 1);
		ALU10->addLinkVertex(0, 133, 79);
		ALU10->addLinkVertex(0, 133, 64);
		ALU10->connect(3, Node21.get(), 0);
		ALU10->connect(4, AndGate13.get(), 0);
		ALUControl11->addLinkVertex(0, 129, 107);
		ALUControl11->addLinkVertex(0, 129, 32);
		ALUControl11->addLinkVertex(1, 87, 26);
		ALUControl11->connect(2, ALU10.get(), 2);
		DataMemory12->addLinkVertex(0, 183, 102);
		DataMemory12->addLinkVertex(1, 198, 111);
		DataMemory12->addLinkVertex(3, 125, 41);
		DataMemory12->connect(4, Mux15.get(), 1);
		AndGate13->addLinkVertex(0, 166, 54);
		AndGate13->addLinkVertex(0, 166, 122);
		AndGate13->connect(2, Mux14.get(), 2);
		Mux14->addLinkVertex(0, 143, 160);
		Mux14->addLinkVertex(0, 172, 160);
		Mux14->addLinkVertex(0, 172, 147);
		Mux14->addLinkVertex(2, 195, 120);
		Mux14->addLinkVertex(2, 195, 160);
		Mux14->addLinkVertex(2, 185, 160);
		Mux14->connect(3, PC0.get(), 0);
		Mux15->addLinkVertex(0, 171, 69);
		Mux15->addLinkVertex(0, 206, 69);
		Mux15->addLinkVertex(0, 206, 62);
		Mux15->addLinkVertex(1, 215, 56);
		Mux15->addLinkVertex(2, 214, 115);
		Mux15->connect(3, Registers5.get(), 1);
		Node16->connect(5, Node22.get(), 2);
		Node16->connect(6, Node23.get(), 1);
		Node17->connect(5, ALU4.get(), 0);
		Node17->connect(7, InstructionMemory1.get(), 0);
		Node18->connect(5, Mux14.get(), 0);
		Node18->connect(7, ALU9.get(), 0);
		Node19->addLinkVertex(1, 125, 64);
		Node19->connect(6, DataMemory12.get(), 3);
		Node19->connect(7, Mux7.get(), 0);
		Node20->addLinkVertex(2, 121, 42);
		Node20->connect(5, ShiftLeft28.get(), 0);
		Node20->connect(7, Mux7.get(), 1);
		Node21->connect(5, Mux15.get(), 0);
		Node21->connect(7, DataMemory12.get(), 2);
		Node22->connect(5, MainControl3.get(), 0);
		Node22->connect(7, Registers5.get(), 2);
		Node23->connect(6, Node24.get(), 1);
		Node23->connect(7, Node26.get(), 0);
		Node24->connect(6, Node25.get(), 0);
		Node24->connect(7, Mux2.get(), 1);
		Node25->addLinkVertex(0, 53, 42);
		Node25->connect(6, ALUControl11.get(), 1);
		Node25->connect(7, SignExtend6.get(), 0);
		Node26->connect(6, Mux2.get(), 0);
		Node26->connect(7, Registers5.get(), 3);
		PC0->setActive();
		PC0->confirmActive();
	}
	if(layout == LAYOUT_PIPELINE)
	{
		auto PC0 = std::make_shared<PC>(layout, 8, 66, 5, 8, _T("PC"));
		components.push_back(PC0);
		programCounter = PC0;
		PC0->setPipelineStage(0);
		PC0->setInstrActive(1, SYM_MAX, true);
		PC0->setInstrActive(1, SYM_NOOP, true);
		auto InstructionMemory1 = std::make_shared<InstructionMemory>(21, 58, 25, 25, _T("Instruction\n Memory"));
		components.push_back(InstructionMemory1);
		memories[ID_INSTRUCTION_LIST] = InstructionMemory1;
		InstructionMemory1->setPipelineStage(0);
		InstructionMemory1->setInstrActive(1, SYM_MAX, true);
		InstructionMemory1->setInstrActive(1, SYM_NOOP, true);
		auto Mux2 = std::make_shared<Mux>(132, 13, 5, 15, _T("M\nU\nX\n4"));
		components.push_back(Mux2);
		Mux2->setPipelineStage(2);
		Mux2->setInstrActive(3, SYM_EOF, true);
		Mux2->setInstrActive(3, SYM_LW, true);
		Mux2->setInstrActive(3, SYM_ADDI, true);
		auto MainControl3 = std::make_shared<MainControlPipelined>(layout, 74, 119, 16, 34, _T("Control"));
		components.push_back(MainControl3);
		MainControl3->setPipelineStage(1);
		MainControl3->setLinkData(8, 1);
		MainControl3->setLinkBits(0, 26, 6);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_WB, SYM_EOF, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_WB, SYM_ADDI, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_WB, SYM_LW, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_MEM, SYM_MAX, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_MEM, SYM_NOOP, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_EX, SYM_MAX, true);
		auto ALU4 = std::make_shared<ALU>(29, 101, 10, 25, _T("Add"), true);
		components.push_back(ALU4);
		ALU4->setLinkData(1, 4); // Set the PC adder to have the second input set to 4 permanently.
		ALU4->setLinkActive(1); // Set the second input to active so outputs propogate.
		ALU4->setLinkData(2, 2); // Set the PC adder to always add.
		ALU4->setLinkActive(2); // Set control input to active.
		ALU4->setInstrActive(3, SYM_MAX, true);
		ALU4->setInstrActive(3, SYM_NOOP, true);
		auto Registers5 = std::make_shared<Registers>(layout, 82, 60, 25, 25, _T("Registers"));
		components.push_back(Registers5);
		memories[ID_REGISTER_LIST] = Registers5;
		Registers5->setPipelineStage(1);
		Registers5->setInstrActive(5, SYM_MAX, true);
		Registers5->setInstrActive(6, SYM_EOF, true);
		Registers5->setInstrActive(6, SYM_BEQ, true);
		Registers5->setInstrActive(6, SYM_SW, true);
		auto SignExtend6 = std::make_shared<SignExtend>(97, 30, 9, 15, _T("  Sign\nExtend"));
		components.push_back(SignExtend6);
		SignExtend6->setLinkBits(0, 0, 16);
		SignExtend6->setPipelineStage(1);
		SignExtend6->setInstrActive(1, SYM_MAX, true);
		auto Mux7 = std::make_shared<Mux>(133, 55, 5, 15, _T("M\nU\nX\n1"));
		components.push_back(Mux7);
		Mux7->setInstrActive(3, SYM_MAX, true);
		Mux7->setPipelineStage(2);
		auto ShiftLeft28 = std::make_shared<ShiftLeft2>(139, 91, 9, 15, _T("Shift\n Left\n  2"), true);
		components.push_back(ShiftLeft28);
		ShiftLeft28->setPipelineStage(2);
		ShiftLeft28->setInstrActive(1, SYM_BEQ, true);
		auto ALU9 = std::make_shared<ALU>(153, 93, 10, 25, _T("Add"), true);
		components.push_back(ALU9);
		ALU9->setPipelineStage(2);
		ALU9->setLinkData(2, 2); // Set the Adder to always add.
		ALU9->setLinkActive(2); // Set the control input to active.
		ALU9->setInstrActive(3, SYM_BEQ, true);
		auto ALU10 = std::make_shared<ALU>(150, 57, 10, 25, _T("ALU"));
		components.push_back(ALU10);
		ALU10->setPipelineStage(2);
		ALU10->setInstrActive(3, SYM_EOF, true);
		ALU10->setInstrActive(3, SYM_LW, true);
		ALU10->setInstrActive(3, SYM_ADDI, true);
		ALU10->setInstrActive(3, SYM_SW, true);
		ALU10->setInstrActive(4, SYM_BEQ, true);
		auto ALUControl11 = std::make_shared<ALUControl>(151, 33, 9, 15, _T("   ALU\nControl"));
		components.push_back(ALUControl11);
		ALUControl11->setLinkBits(1, 0, 6);
		ALUControl11->setPipelineStage(2);
		ALUControl11->setInstrActive(2, SYM_MAX, true);
		auto DataMemory12 = std::make_shared<DataMemory>(layout, 189, 49, 25, 25, _T("  Data\nMemory"));
		components.push_back(DataMemory12);
		memories[ID_DATA_LIST] = DataMemory12;
		DataMemory12->setPipelineStage(3);
		DataMemory12->setInstrActive(4, SYM_LW, true);
		auto AndGate13 = std::make_shared<AndGate>(193, 91, 8, 5, _T(""), true);
		components.push_back(AndGate13);
		branchCheckGate = AndGate13;
		AndGate13->setPipelineStage(3);
		AndGate13->setInstrActive(2, SYM_NOOP, true);
		AndGate13->setInstrActive(2, SYM_MAX, true);
		auto Mux14 = std::make_shared<Mux>(54, 149, 5, 15, _T("M\nU\nX\n2"), true);
		components.push_back(Mux14);
		Mux14->setPipelineStage(0);
		Mux14->setInstrActive(3, SYM_MAX, true);
		Mux14->setInstrActive(3, SYM_NOOP, true);
		auto Mux15 = std::make_shared<Mux>(236, 65, 5, 15, _T("M\nU\nX\n3"));
		components.push_back(Mux15);
		Mux15->setPipelineStage(4);
		Mux15->setInstrActive(3, SYM_EOF, true);
		Mux15->setInstrActive(3, SYM_LW, true);
		Mux15->setInstrActive(3, SYM_ADDI, true);
		auto IFIDReg16 = std::make_shared<IFIDReg>(layout, 54, 11, 5, 120, _T("IF/ID"));
		components.push_back(IFIDReg16);
		buffers[0] = IFIDReg16;
		IFIDReg16->setPipelineStage(1);
		IFIDReg16->setInstrActive(3, SYM_BEQ, true);
		IFIDReg16->setInstrActive(2, SYM_MAX, true);
		IFIDReg16->setInstrActive(2, SYM_NOOP, true);
		auto IDEXReg17 = std::make_shared<IDEXReg>(layout, 117, 10, 5, 140, _T("ID/EX"));
		components.push_back(IDEXReg17);
		buffers[1] = IDEXReg17;
		IDEXReg17->setLinkBits(0, 11, 5);
		IDEXReg17->setLinkBits(1, 16, 5);
		IDEXReg17->setPipelineStage(2);
		IDEXReg17->setInstrActive(19, SYM_EOF, true);
		IDEXReg17->setInstrActive(19, SYM_ADDI, true);
		IDEXReg17->setInstrActive(19, SYM_LW, true);
		IDEXReg17->setInstrActive(18, SYM_MAX, true);
		IDEXReg17->setInstrActive(18, SYM_NOOP, true);
		IDEXReg17->setInstrActive(17, SYM_EOF, true);
		IDEXReg17->setInstrActive(17, SYM_ADDI, true);
		IDEXReg17->setInstrActive(17, SYM_LW, true);
		IDEXReg17->setInstrActive(16, SYM_MAX, true);
		IDEXReg17->setInstrActive(15, SYM_MAX, true);
		IDEXReg17->setInstrActive(14, SYM_BEQ, true);
		IDEXReg17->setInstrActive(13, SYM_MAX, true);
		IDEXReg17->setInstrActive(12, SYM_EOF, true);
		IDEXReg17->setInstrActive(12, SYM_BEQ, true);
		IDEXReg17->setInstrActive(12, SYM_SW, true);
		IDEXReg17->setInstrActive(11, SYM_MAX, true);
		IDEXReg17->setInstrActive(10, SYM_LW, true);
		IDEXReg17->setInstrActive(9, SYM_EOF, true);
		IDEXReg17->setInstrActive(10, SYM_ADDI, true);
		auto EXMEMReg18 = std::make_shared<EXMEMReg>(layout, 174, 10, 5, 140, _T("EX/MEM"));
		components.push_back(EXMEMReg18);
		buffers[2] = EXMEMReg18;
		EXMEMReg18->setPipelineStage(3);
		EXMEMReg18->setInstrActive(15, SYM_EOF, true);
		EXMEMReg18->setInstrActive(15, SYM_ADDI, true);
		EXMEMReg18->setInstrActive(15, SYM_LW, true);
		EXMEMReg18->setInstrActive(14, SYM_LW, true);
		EXMEMReg18->setInstrActive(13, SYM_SW, true);
		EXMEMReg18->setInstrActive(12, SYM_MAX, true);
		EXMEMReg18->setInstrActive(12, SYM_NOOP, true);
		EXMEMReg18->setInstrActive(11, SYM_BEQ, true);
		EXMEMReg18->setInstrActive(10, SYM_EOF, true);
		EXMEMReg18->setInstrActive(10, SYM_LW, true);
		EXMEMReg18->setInstrActive(10, SYM_ADDI, true);
		EXMEMReg18->setInstrActive(10, SYM_SW, true);
		EXMEMReg18->setInstrActive(9, SYM_BEQ, true);
		EXMEMReg18->setInstrActive(8, SYM_SW, true);
		EXMEMReg18->setInstrActive(7, SYM_LW, true);
		EXMEMReg18->setInstrActive(7, SYM_ADDI, true);
		EXMEMReg18->setInstrActive(7, SYM_EOF, true);
		auto MEMWBReg19 = std::make_shared<MEMWBReg>(layout, 221, 10, 5, 120, _T("MEM/WB"));
		components.push_back(MEMWBReg19);
		buffers[3] = MEMWBReg19;
		MEMWBReg19->setPipelineStage(4);
		MEMWBReg19->setInstrActive(8, SYM_LW, true);
		MEMWBReg19->setInstrActive(8, SYM_ADDI, true);
		MEMWBReg19->setInstrActive(8, SYM_EOF, true);
		MEMWBReg19->setInstrActive(7, SYM_LW, true);
		MEMWBReg19->setInstrActive(7, SYM_ADDI, true);
		MEMWBReg19->setInstrActive(7, SYM_EOF, true);
		MEMWBReg19->setInstrActive(6, SYM_ADDI, true);
		MEMWBReg19->setInstrActive(6, SYM_EOF, true);
		MEMWBReg19->setInstrActive(5, SYM_LW, true);
		MEMWBReg19->setInstrActive(4, SYM_LW, true);
		MEMWBReg19->setInstrActive(4, SYM_ADDI, true);
		MEMWBReg19->setInstrActive(4, SYM_EOF, true);
		auto Node20 = std::make_shared<Node>(16, 70, 2, 2, _T(""));
		components.push_back(Node20);
		Node20->setInstrActive(5, SYM_MAX, true);
		Node20->setInstrActive(4, SYM_MAX, true);
		Node20->setInstrActive(5, SYM_NOOP, true);
		Node20->setInstrActive(4, SYM_NOOP, true);
		auto Node21 = std::make_shared<Node>(43, 113, 2, 2, _T(""));
		components.push_back(Node21);
		Node21->setInstrActive(5, SYM_MAX, true);
		Node21->setInstrActive(5, SYM_NOOP, true);
		Node21->setInstrActive(5, SYM_BEQ, true);
		Node21->setInstrActive(7, SYM_BEQ, true);
		auto Node22 = std::make_shared<Node>(126, 65, 2, 2, _T(""));
		components.push_back(Node22);
		Node22->setPipelineStage(2);
		Node22->setInstrActive(7, SYM_EOF, true);
		Node22->setInstrActive(7, SYM_BEQ, true);
		Node22->setInstrActive(4, SYM_SW, true);
		auto Node23 = std::make_shared<Node>(124, 37, 2, 2, _T(""));
		components.push_back(Node23);
		Node23->setPipelineStage(2);
		Node23->setInstrActive(4, SYM_ADDI, true);
		Node23->setInstrActive(4, SYM_BEQ, true);
		Node23->setInstrActive(4, SYM_SW, true);
		Node23->setInstrActive(4, SYM_LW, true);
		Node23->setInstrActive(7, SYM_EOF, true);
		auto Node24 = std::make_shared<Node>(124, 59, 2, 2, _T(""));
		components.push_back(Node24);
		Node24->setPipelineStage(2);
		Node24->setInstrActive(5, SYM_BEQ, true);
		Node24->setInstrActive(7, SYM_ADDI, true);
		Node24->setInstrActive(7, SYM_SW, true);
		Node24->setInstrActive(7, SYM_LW, true);
		auto Node25 = std::make_shared<Node>(65, 70, 2, 2, _T(""));
		components.push_back(Node25);
		Node25->setPipelineStage(1);
		Node25->setInstrActive(5, SYM_MAX, true);
		Node25->setInstrActive(5, SYM_NOOP, true);
		Node25->setInstrActive(4, SYM_EOF, true);
		Node25->setInstrActive(4, SYM_LW, true);
		Node25->setInstrActive(4, SYM_ADDI, true);
		Node25->setInstrActive(4, SYM_BEQ, true);
		Node25->setInstrActive(7, SYM_EOF, true);
		Node25->setInstrActive(7, SYM_BEQ, true);
		Node25->setInstrActive(7, SYM_SW, true);
		auto Node27 = std::make_shared<Node>(65, 75, 2, 2, _T(""));
		components.push_back(Node27);
		Node27->setPipelineStage(1);
		Node27->setInstrActive(5, SYM_MAX, true);
		Node27->setInstrActive(5, SYM_NOOP, true);
		Node27->setInstrActive(4, SYM_MAX, true);
		auto Node28 = std::make_shared<Node>(65, 37, 2, 2, _T(""));
		components.push_back(Node28);
		Node28->setPipelineStage(1);
		Node28->setInstrActive(4, SYM_EOF, true);
		Node28->setInstrActive(4, SYM_ADDI, true);
		Node28->setInstrActive(4, SYM_LW, true);
		Node28->setInstrActive(7, SYM_MAX, true);
		auto Node29 = std::make_shared<Node>(65, 23, 2, 2, _T(""));
		components.push_back(Node29);
		Node29->setInstrActive(4, SYM_LW, true);
		Node29->setInstrActive(5, SYM_EOF, true);
		Node29->setInstrActive(4, SYM_ADDI, true);
		Node29->setPipelineStage(1);
		auto Node30 = std::make_shared<Node>(182, 69, 2, 2, _T(""));
		components.push_back(Node30);
		Node30->setPipelineStage(3);
		Node30->setInstrActive(5, SYM_EOF, true);
		Node30->setInstrActive(5, SYM_ADDI, true);
		Node30->setInstrActive(7, SYM_LW, true);
		Node30->setInstrActive(7, SYM_SW, true);
		PC0->addLinkVertex(0, 62, 156);
		PC0->addLinkVertex(0, 62, 168);
		PC0->addLinkVertex(0, 3, 168);
		PC0->addLinkVertex(0, 3, 70);
		PC0->connect(1, Node20.get(), 0);
		InstructionMemory1->connect(1, IFIDReg16.get(), 0);
		Mux2->addLinkVertex(2, 169, 136);
		Mux2->addLinkVertex(2, 169, 50);
		Mux2->addLinkVertex(2, 134, 50);
		Mux2->connect(3, EXMEMReg18.get(), 0);
		MainControl3->addLinkVertex(0, 65, 136);
		MainControl3->connect(1, IDEXReg17.get(), 8);
		MainControl3->connect(2, IDEXReg17.get(), 7);
		MainControl3->connect(3, IDEXReg17.get(), 6);
		ALU4->addLinkVertex(0, 16, 121);
		ALU4->connect(3, Node21.get(), 0);
		Registers5->addLinkVertex(0, 238, 125);
		Registers5->addLinkVertex(0, 238, 168);
		Registers5->addLinkVertex(0, 94, 168);
		Registers5->addLinkVertex(1, 246, 72);
		Registers5->addLinkVertex(1, 246, 1);
		Registers5->addLinkVertex(1, 62, 1);
		Registers5->addLinkVertex(1, 62, 80);
		Registers5->addLinkVertex(4, 230, 20);
		Registers5->addLinkVertex(4, 230, 5);
		Registers5->addLinkVertex(4, 75, 5);
		Registers5->addLinkVertex(4, 75, 65);
		Registers5->connect(5, IDEXReg17.get(), 4);
		Registers5->connect(6, IDEXReg17.get(), 3);
		SignExtend6->connect(1, IDEXReg17.get(), 2);
		Mux7->addLinkVertex(2, 135, 127);
		Mux7->connect(3, ALU10.get(), 1);
		ShiftLeft28->addLinkVertex(0, 124, 98);
		ShiftLeft28->connect(1, ALU9.get(), 1);
		ALU9->connect(3, EXMEMReg18.get(), 4);
		ALU10->addLinkVertex(0, 142, 80);
		ALU10->addLinkVertex(0, 142, 77);
		ALU10->connect(3, EXMEMReg18.get(), 3);
		ALU10->connect(4, EXMEMReg18.get(), 2);
		ALUControl11->addLinkVertex(0, 167, 135);
		ALUControl11->addLinkVertex(0, 167, 52);
		ALUControl11->addLinkVertex(0, 145, 52);
		ALUControl11->addLinkVertex(0, 145, 43);
		ALUControl11->connect(2, ALU10.get(), 2);
		DataMemory12->addLinkVertex(0, 203, 137);
		DataMemory12->addLinkVertex(0, 203, 79);
		DataMemory12->addLinkVertex(0, 194, 79);
		DataMemory12->addLinkVertex(1, 205, 138);
		DataMemory12->addLinkVertex(1, 205, 79);
		DataMemory12->addLinkVertex(1, 209, 79);
		DataMemory12->connect(4, MEMWBReg19.get(), 1);
		AndGate13->addLinkVertex(0, 184, 136);
		AndGate13->addLinkVertex(0, 184, 95);
		AndGate13->addLinkVertex(1, 184, 67);
		AndGate13->addLinkVertex(1, 184, 92);
		AndGate13->connect(2, Mux14.get(), 2);
		Mux14->addLinkVertex(0, 43, 159);
		Mux14->addLinkVertex(1, 194, 105);
		Mux14->addLinkVertex(1, 194, 162);
		Mux14->addLinkVertex(1, 70, 162);
		Mux14->addLinkVertex(1, 70, 143);
		Mux14->addLinkVertex(1, 48, 143);
		Mux14->addLinkVertex(1, 48, 153);
		Mux14->addLinkVertex(2, 210, 93);
		Mux14->addLinkVertex(2, 210, 171);
		Mux14->addLinkVertex(2, 56, 171);
		Mux14->connect(3, PC0.get(), 0);
		Mux15->addLinkVertex(0, 230, 82);
		Mux15->addLinkVertex(0, 230, 75);
		Mux15->addLinkVertex(2, 238, 124);
		Mux15->connect(3, Registers5.get(), 1);
		IFIDReg16->connect(2, Node25.get(), 0);
		IFIDReg16->connect(3, IDEXReg17.get(), 5);
		IDEXReg17->addLinkVertex(0, 65, 17);
		IDEXReg17->connect(9, Mux2.get(), 1);
		IDEXReg17->connect(10, Mux2.get(), 0);
		IDEXReg17->connect(11, Node23.get(), 0);
		IDEXReg17->connect(12, Node22.get(), 0);
		IDEXReg17->connect(13, ALU10.get(), 0);
		IDEXReg17->connect(14, ALU9.get(), 0);
		IDEXReg17->connect(15, Mux7.get(), 2);
		IDEXReg17->connect(16, ALUControl11.get(), 0);
		IDEXReg17->connect(17, Mux2.get(), 2);
		IDEXReg17->connect(18, EXMEMReg18.get(), 5);
		IDEXReg17->connect(19, EXMEMReg18.get(), 6);
		EXMEMReg18->addLinkVertex(1, 126, 54);
		EXMEMReg18->connect(7, MEMWBReg19.get(), 0);
		EXMEMReg18->connect(8, DataMemory12.get(), 3);
		EXMEMReg18->connect(9, AndGate13.get(), 1);
		EXMEMReg18->connect(10, Node30.get(), 2);
		EXMEMReg18->connect(11, Mux14.get(), 1);
		EXMEMReg18->connect(12, AndGate13.get(), 0);
		EXMEMReg18->connect(13, DataMemory12.get(), 0);
		EXMEMReg18->connect(14, DataMemory12.get(), 1);
		EXMEMReg18->connect(15, MEMWBReg19.get(), 3);
		MEMWBReg19->addLinkVertex(2, 182, 82);
		MEMWBReg19->addLinkVertex(3, 212, 144);
		MEMWBReg19->addLinkVertex(3, 212, 125);
		MEMWBReg19->connect(4, Registers5.get(), 4);
		MEMWBReg19->connect(5, Mux15.get(), 1);
		MEMWBReg19->connect(6, Mux15.get(), 0);
		MEMWBReg19->connect(7, Mux15.get(), 2);
		MEMWBReg19->connect(8, Registers5.get(), 0);
		Node20->connect(4, InstructionMemory1.get(), 0);
		Node20->connect(5, ALU4.get(), 0);
		Node21->connect(5, Mux14.get(), 0);
		Node21->connect(7, IFIDReg16.get(), 1);
		Node22->connect(4, EXMEMReg18.get(), 1);
		Node22->connect(7, Mux7.get(), 0);
		Node23->connect(4, Node24.get(), 2);
		Node23->connect(7, ALUControl11.get(), 1);
		Node24->connect(5, ShiftLeft28.get(), 0);
		Node24->connect(7, Mux7.get(), 1);
		Node25->connect(4, Node28.get(), 1);
		Node25->connect(5, Node27.get(), 2);
		Node25->connect(7, Registers5.get(), 3);
		Node27->connect(4, Registers5.get(), 2);
		Node27->connect(5, MainControl3.get(), 0);
		Node28->connect(4, Node29.get(), 1);
		Node28->connect(7, SignExtend6.get(), 0);
		Node29->connect(4, IDEXReg17.get(), 1);
		Node29->connect(5, IDEXReg17.get(), 0);
		Node30->connect(5, MEMWBReg19.get(), 2);
		Node30->connect(7, DataMemory12.get(), 2);
		PC0->setActive();
		PC0->confirmActive();
	}
	if(layout == LAYOUT_FORWARDING)
	{
		auto PC0 = std::make_shared<PC>(layout, 7, 74, 5, 8, _T("PC"));
		components.push_back(PC0);
		programCounter = PC0;
		PC0->setPipelineStage(0);
		PC0->setInstrActive(1, SYM_MAX, true);
		PC0->setInstrActive(1, SYM_NOOP, true);
		auto InstructionMemory1 = std::make_shared<InstructionMemory>(19, 66, 25, 25, _T("Instruction\n Memory"));
		components.push_back(InstructionMemory1);
		memories[ID_INSTRUCTION_LIST] = InstructionMemory1;
		InstructionMemory1->setPipelineStage(0);
		InstructionMemory1->setInstrActive(1, SYM_MAX, true);
		InstructionMemory1->setInstrActive(1, SYM_NOOP, true);
		auto Mux2 = std::make_shared<Mux>(125, 19, 5, 15, _T("M\nU\nX\n4"));
		components.push_back(Mux2);
		Mux2->setPipelineStage(2);
		Mux2->setInstrActive(3, SYM_EOF, true);
		Mux2->setInstrActive(3, SYM_LW, true);
		Mux2->setInstrActive(3, SYM_ADDI, true);
		auto MainControl3 = std::make_shared<MainControlPipelined>(layout, 77, 130, 16, 34, _T("Control"));
		components.push_back(MainControl3);
		MainControl3->setLinkData(8, 1);
		MainControl3->setLinkBits(0, 26, 6);
		MainControl3->setPipelineStage(1);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_WB, SYM_EOF, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_WB, SYM_ADDI, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_WB, SYM_LW, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_MEM, SYM_MAX, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_MEM, SYM_NOOP, true);
		MainControl3->setInstrActive(MAINCONTROLPIPELINED_EX, SYM_MAX, true);
		auto ALU4 = std::make_shared<ALU>(26, 115, 10, 25, _T("Add"), true);
		components.push_back(ALU4);
		ALU4->setLinkData(1, 4); // Set the PC adder to have the second input set to 4 permanently.
		ALU4->setLinkActive(1); // Set the second input to active so outputs propogate.
		ALU4->setLinkData(2, 2); // Set the PC adder to always add.
		ALU4->setLinkActive(2); // Set control input to active.
		ALU4->setInstrActive(3, SYM_MAX, true);
		ALU4->setInstrActive(3, SYM_NOOP, true);
		auto Registers5 = std::make_shared<Registers>(layout, 67, 68, 25, 25, _T("Registers"));
		components.push_back(Registers5);
		memories[ID_REGISTER_LIST] = Registers5;
		Registers5->setPipelineStage(1);
		Registers5->setInstrActive(5, SYM_MAX, true);
		Registers5->setInstrActive(6, SYM_EOF, true);
		Registers5->setInstrActive(6, SYM_BEQ, true);
		Registers5->setInstrActive(6, SYM_SW, true);
		auto SignExtend6 = std::make_shared<SignExtend>(84, 39, 9, 15, _T("  Sign\nExtend"));
		components.push_back(SignExtend6);
		SignExtend6->setLinkBits(0, 0, 16);
		SignExtend6->setPipelineStage(1);
		SignExtend6->setInstrActive(1, SYM_MAX, true);
		auto Mux7 = std::make_shared<Mux>(147, 63, 5, 15, _T("M\nU\nX\n1"));
		components.push_back(Mux7);
		Mux7->setPipelineStage(2);
		Mux7->setInstrActive(3, SYM_MAX, true);
		auto Mux37 = std::make_shared<Mux3>(132, 49, 5, 15, _T("M\nU\nX\nB"));
		components.push_back(Mux37);
		Mux37->setInstrActive(4, SYM_EOF, true);
		Mux37->setInstrActive(4, SYM_BEQ, true);
		Mux37->setInstrActive(4, SYM_SW, true);
		Mux37->setPipelineStage(2);
		auto Mux38 = std::make_shared<Mux3>(132, 78, 5, 15, _T("M\nU\nX\nA"));
		components.push_back(Mux38);
		Mux38->setInstrActive(4, SYM_MAX, true);
		Mux38->setPipelineStage(2);
		auto ShiftLeft29 = std::make_shared<ShiftLeft2>(133, 105, 9, 15, _T("Shift\n Left\n  2"), true);
		components.push_back(ShiftLeft29);
		ShiftLeft29->setPipelineStage(2);
		ShiftLeft29->setInstrActive(1, SYM_BEQ, true);
		auto ALU10 = std::make_shared<ALU>(147, 107, 10, 25, _T("Add"), true);
		components.push_back(ALU10);
		ALU10->setLinkData(2, 2); // Set the Adder to always add.
		ALU10->setLinkActive(2); // Set the control input to active.
		ALU10->setPipelineStage(2);
		ALU10->setInstrActive(3, SYM_BEQ, true);
		auto ALU11 = std::make_shared<ALU>(157, 65, 10, 25, _T("ALU"));
		components.push_back(ALU11);
		ALU11->setInstrActive(3, SYM_EOF, true);
		ALU11->setInstrActive(3, SYM_LW, true);
		ALU11->setInstrActive(3, SYM_ADDI, true);
		ALU11->setInstrActive(3, SYM_SW, true);
		ALU11->setInstrActive(4, SYM_BEQ, true);
		ALU11->setPipelineStage(2);
		auto ALUControl12 = std::make_shared<ALUControl>(158, 36, 9, 15, _T("   ALU\nControl"));
		components.push_back(ALUControl12);
		ALUControl12->setLinkBits(1, 0, 6);
		ALUControl12->setPipelineStage(2);
		ALUControl12->setInstrActive(2, SYM_MAX, true);
		auto DataMemory13 = std::make_shared<DataMemory>(layout, 192, 57, 25, 25, _T("  Data\nMemory"));
		components.push_back(DataMemory13);
		memories[ID_DATA_LIST] = DataMemory13;
		DataMemory13->setPipelineStage(3);
		DataMemory13->setInstrActive(4, SYM_LW, true);
		auto AndGate14 = std::make_shared<AndGate>(194, 107, 8, 5, _T(""), true);
		components.push_back(AndGate14);
		AndGate14->setPipelineStage(3);
		AndGate14->setInstrActive(2, SYM_NOOP, true);
		AndGate14->setInstrActive(2, SYM_MAX, true);
		branchCheckGate = AndGate14;
		auto Mux15 = std::make_shared<Mux>(51, 145, 5, 15, _T("M\nU\nX\n2"), true);
		components.push_back(Mux15);
		Mux15->setPipelineStage(0);
		Mux15->setInstrActive(3, SYM_MAX, true);
		Mux15->setInstrActive(3, SYM_NOOP, true);
		auto Mux16 = std::make_shared<Mux>(238, 73, 5, 15, _T("M\nU\nX\n3"));
		components.push_back(Mux16);
		Mux16->setPipelineStage(4);
		Mux16->setInstrActive(3, SYM_EOF, true);
		Mux16->setInstrActive(3, SYM_LW, true);
		Mux16->setInstrActive(3, SYM_ADDI, true);
		auto IFIDReg17 = std::make_shared<IFIDReg>(layout, 50, 18, 5, 120, _T("IF/ID"));
		components.push_back(IFIDReg17);
		buffers[0] = IFIDReg17;
		IFIDReg17->setInstrActive(3, SYM_BEQ, true);
		IFIDReg17->setInstrActive(2, SYM_MAX, true);
		IFIDReg17->setInstrActive(2, SYM_NOOP, true);
		IFIDReg17->setPipelineStage(1);
		auto IDEXReg18 = std::make_shared<IDEXReg>(layout, 103, 18, 5, 140, _T("ID/EX"));
		components.push_back(IDEXReg18);
		buffers[1] = IDEXReg18;
		IDEXReg18->setLinkBits(0, 11, 5);
		IDEXReg18->setLinkBits(1, 16, 5);
		IDEXReg18->setLinkBits(20, 21, 5);
		IDEXReg18->setPipelineStage(2);
		IDEXReg18->setInstrActive(21, SYM_MAX, true);
		IDEXReg18->setInstrActive(19, SYM_EOF, true);
		IDEXReg18->setInstrActive(19, SYM_ADDI, true);
		IDEXReg18->setInstrActive(19, SYM_LW, true);
		IDEXReg18->setInstrActive(18, SYM_MAX, true);
		IDEXReg18->setInstrActive(18, SYM_NOOP, true);
		IDEXReg18->setInstrActive(17, SYM_EOF, true);
		IDEXReg18->setInstrActive(17, SYM_ADDI, true);
		IDEXReg18->setInstrActive(17, SYM_LW, true);
		IDEXReg18->setInstrActive(16, SYM_MAX, true);
		IDEXReg18->setInstrActive(15, SYM_MAX, true);
		IDEXReg18->setInstrActive(14, SYM_BEQ, true);
		IDEXReg18->setInstrActive(13, SYM_MAX, true);
		IDEXReg18->setInstrActive(12, SYM_EOF, true);
		IDEXReg18->setInstrActive(12, SYM_BEQ, true);
		IDEXReg18->setInstrActive(12, SYM_SW, true);
		IDEXReg18->setInstrActive(11, SYM_EOF, true);
		IDEXReg18->setInstrActive(11, SYM_ADDI, true);
		IDEXReg18->setInstrActive(11, SYM_BEQ, true);
		IDEXReg18->setInstrActive(11, SYM_SW, true);
		IDEXReg18->setInstrActive(11, SYM_LW, true);
		IDEXReg18->setInstrActive(10, SYM_LW, true);
		IDEXReg18->setInstrActive(9, SYM_EOF, true);
		IDEXReg18->setInstrActive(10, SYM_ADDI, true);
		auto EXMEMReg19 = std::make_shared<EXMEMReg>(layout, 177, 18, 5, 140, _T("EX/MEM"));
		components.push_back(EXMEMReg19);
		buffers[2] = EXMEMReg19;
		EXMEMReg19->setPipelineStage(3);
		EXMEMReg19->setInstrActive(15, SYM_EOF, true);
		EXMEMReg19->setInstrActive(15, SYM_ADDI, true);
		EXMEMReg19->setInstrActive(15, SYM_LW, true);
		EXMEMReg19->setInstrActive(14, SYM_LW, true);
		EXMEMReg19->setInstrActive(13, SYM_SW, true);
		EXMEMReg19->setInstrActive(12, SYM_MAX, true);
		EXMEMReg19->setInstrActive(12, SYM_NOOP, true);
		EXMEMReg19->setInstrActive(11, SYM_BEQ, true);
		EXMEMReg19->setInstrActive(10, SYM_EOF, true);
		EXMEMReg19->setInstrActive(10, SYM_LW, true);
		EXMEMReg19->setInstrActive(10, SYM_ADDI, true);
		EXMEMReg19->setInstrActive(10, SYM_SW, true);
		EXMEMReg19->setInstrActive(9, SYM_BEQ, true);
		EXMEMReg19->setInstrActive(8, SYM_SW, true);
		EXMEMReg19->setInstrActive(7, SYM_LW, true);
		EXMEMReg19->setInstrActive(7, SYM_ADDI, true);
		EXMEMReg19->setInstrActive(7, SYM_EOF, true);
		auto MEMWBReg20 = std::make_shared<MEMWBReg>(layout, 224, 18, 5, 120, _T("MEM/WB"));
		components.push_back(MEMWBReg20);
		buffers[3] = MEMWBReg20;
		MEMWBReg20->setPipelineStage(4);
		MEMWBReg20->setInstrActive(8, SYM_LW, true);
		MEMWBReg20->setInstrActive(8, SYM_ADDI, true);
		MEMWBReg20->setInstrActive(8, SYM_EOF, true);
		MEMWBReg20->setInstrActive(7, SYM_LW, true);
		MEMWBReg20->setInstrActive(7, SYM_ADDI, true);
		MEMWBReg20->setInstrActive(7, SYM_EOF, true);
		MEMWBReg20->setInstrActive(6, SYM_ADDI, true);
		MEMWBReg20->setInstrActive(6, SYM_EOF, true);
		MEMWBReg20->setInstrActive(5, SYM_LW, true);
		MEMWBReg20->setInstrActive(4, SYM_LW, true);
		MEMWBReg20->setInstrActive(4, SYM_ADDI, true);
		MEMWBReg20->setInstrActive(4, SYM_EOF, true);
		auto Forwarding21 = std::make_shared<Forwarding>(132, 9, 30, 10, _T("Forwarding\n     Unit"));
		components.push_back(Forwarding21);
		Forwarding21->setPipelineStage(2);
		Forwarding21->setInstrActive(4, SYM_EOF, true);
		Forwarding21->setInstrActive(4, SYM_BEQ, true);
		Forwarding21->setInstrActive(4, SYM_SW, true);
		Forwarding21->setInstrActive(5, SYM_MAX, true);
		auto Node22 = std::make_shared<Node>(111, 29, 2, 2, _T(""));
		components.push_back(Node22);
		Node22->setInstrActive(4, SYM_LW, true);
		Node22->setInstrActive(4, SYM_ADDI, true);
		Node22->setInstrActive(5, SYM_LW, true);
		Node22->setInstrActive(5, SYM_ADDI, true);
		Node22->setPipelineStage(2);
		auto Node23 = std::make_shared<Node>(188, 26, 2, 2, _T(""));
		components.push_back(Node23);
		Node23->setPipelineStage(3);
		Node23->setInstrActive(4, SYM_LW, true);
		Node23->setInstrActive(4, SYM_ADDI, true);
		Node23->setInstrActive(4, SYM_EOF, true);
		Node23->setInstrActive(7, SYM_LW, true);
		Node23->setInstrActive(7, SYM_ADDI, true);
		Node23->setInstrActive(7, SYM_EOF, true);
		auto Node24 = std::make_shared<Node>(235, 11, 2, 2, _T(""));
		components.push_back(Node24);
		Node24->setPipelineStage(4);
		Node24->setInstrActive(4, SYM_LW, true);
		Node24->setInstrActive(4, SYM_ADDI, true);
		Node24->setInstrActive(4, SYM_EOF, true);
		Node24->setInstrActive(5, SYM_LW, true);
		Node24->setInstrActive(5, SYM_ADDI, true);
		Node24->setInstrActive(5, SYM_EOF, true);
		auto Node25 = std::make_shared<Node>(184, 77, 2, 2, _T(""));
		components.push_back(Node25);
		Node25->setPipelineStage(3);
		Node25->setInstrActive(5, SYM_EOF, true);
		Node25->setInstrActive(5, SYM_ADDI, true);
		Node25->setInstrActive(7, SYM_LW, true);
		Node25->setInstrActive(7, SYM_SW, true);
		Node25->setInstrActive(4, SYM_LW, true);
		Node25->setInstrActive(4, SYM_SW, true);
		Node25->setInstrActive(4, SYM_EOF, true);
		Node25->setInstrActive(4, SYM_ADDI, true);
		auto Node26 = std::make_shared<Node>(120, 53, 2, 2, _T(""));
		components.push_back(Node26);
		Node26->setInstrActive(5, SYM_LW, true);
		Node26->setInstrActive(5, SYM_SW, true);
		Node26->setInstrActive(5, SYM_EOF, true);
		Node26->setInstrActive(5, SYM_ADDI, true);
		Node26->setInstrActive(7, SYM_LW, true);
		Node26->setInstrActive(7, SYM_SW, true);
		Node26->setInstrActive(7, SYM_EOF, true);
		Node26->setInstrActive(7, SYM_ADDI, true);
		Node26->setPipelineStage(3);
		auto Node27 = std::make_shared<Node>(117, 2, 2, 2, _T(""));
		components.push_back(Node27);
		Node27->setPipelineStage(4);
		Node27->setInstrActive(4, SYM_EOF, true);
		Node27->setInstrActive(4, SYM_LW, true);
		Node27->setInstrActive(4, SYM_ADDI, true);
		Node27->setInstrActive(5, SYM_EOF, true);
		Node27->setInstrActive(5, SYM_LW, true);
		Node27->setInstrActive(5, SYM_ADDI, true);
		auto Node28 = std::make_shared<Node>(117, 56, 2, 2, _T(""));
		components.push_back(Node28);
		Node28->setPipelineStage(4);
		Node28->setInstrActive(4, SYM_EOF, true);
		Node28->setInstrActive(4, SYM_LW, true);
		Node28->setInstrActive(4, SYM_ADDI, true);
		Node28->setInstrActive(5, SYM_EOF, true);
		Node28->setInstrActive(5, SYM_LW, true);
		Node28->setInstrActive(5, SYM_ADDI, true);
		auto Node29 = std::make_shared<Node>(143, 62, 2, 2, _T(""));
		components.push_back(Node29);
		Node29->setPipelineStage(2);
		Node29->setInstrActive(5, SYM_EOF, true);
		Node29->setInstrActive(5, SYM_BEQ, true);
		Node29->setInstrActive(7, SYM_SW, true);
		auto Node30 = std::make_shared<Node>(111, 46, 2, 2, _T(""));
		components.push_back(Node30);
		Node30->setPipelineStage(2);
		Node30->setInstrActive(4, SYM_ADDI, true);
		Node30->setInstrActive(4, SYM_BEQ, true);
		Node30->setInstrActive(4, SYM_SW, true);
		Node30->setInstrActive(4, SYM_LW, true);
		Node30->setInstrActive(5, SYM_EOF, true);
		auto Node31 = std::make_shared<Node>(14, 78, 2, 2, _T(""));
		components.push_back(Node31);
		Node31->setInstrActive(5, SYM_MAX, true);
		Node31->setInstrActive(4, SYM_MAX, true);
		Node31->setInstrActive(5, SYM_NOOP, true);
		Node31->setInstrActive(4, SYM_NOOP, true);
		auto Node32 = std::make_shared<Node>(40, 127, 2, 2, _T(""));
		components.push_back(Node32);
		Node32->setInstrActive(5, SYM_MAX, true);
		Node32->setInstrActive(5, SYM_NOOP, true);
		Node32->setInstrActive(5, SYM_BEQ, true);
		Node32->setInstrActive(7, SYM_BEQ, true);
		auto Node33 = std::make_shared<Node>(59, 78, 2, 2, _T(""));
		components.push_back(Node33);
		Node33->setPipelineStage(1);
		Node33->setInstrActive(4, SYM_MAX, true);
		Node33->setInstrActive(5, SYM_MAX, true);
		Node33->setInstrActive(5, SYM_NOOP, true);
		Node33->setInstrActive(7, SYM_EOF, true);
		Node33->setInstrActive(7, SYM_BEQ, true);
		Node33->setInstrActive(7, SYM_SW, true);
		auto Node34 = std::make_shared<Node>(59, 83, 2, 2, _T(""));
		components.push_back(Node34);
		Node34->setPipelineStage(1);
		Node34->setInstrActive(5, SYM_MAX, true);
		Node34->setInstrActive(5, SYM_NOOP, true);
		Node34->setInstrActive(4, SYM_MAX, true);
		auto Node35 = std::make_shared<Node>(59, 46, 2, 2, _T(""));
		components.push_back(Node35);
		Node35->setPipelineStage(1);
		Node35->setInstrActive(4, SYM_MAX, true);
		Node35->setInstrActive(7, SYM_MAX, true);
		auto Node36 = std::make_shared<Node>(59, 33, 2, 2, _T(""));
		components.push_back(Node36);
		Node36->setInstrActive(4, SYM_LW, true);
		Node36->setInstrActive(4, SYM_EOF, true);
		Node36->setInstrActive(4, SYM_ADDI, true);
		Node36->setInstrActive(7, SYM_MAX, true);
		Node36->setLinkControl(true, 7);
		Node36->setPipelineStage(1);
		auto Node37 = std::make_shared<Node>(59, 29, 2, 2, _T(""));
		components.push_back(Node37);
		Node37->setInstrActive(7, SYM_LW, true);
		Node37->setInstrActive(4, SYM_EOF, true);
		Node37->setInstrActive(7, SYM_ADDI, true);
		Node37->setPipelineStage(1);
		auto Node38 = std::make_shared<Node>(111, 71, 2, 2, _T(""));
		components.push_back(Node38);
		Node38->setPipelineStage(2);
		Node38->setInstrActive(5, SYM_BEQ, true);
		Node38->setInstrActive(7, SYM_ADDI, true);
		Node38->setInstrActive(7, SYM_SW, true);
		Node38->setInstrActive(7, SYM_LW, true);
		PC0->addLinkVertex(0, 63, 152);
		PC0->addLinkVertex(0, 63, 171);
		PC0->addLinkVertex(0, 2, 171);
		PC0->addLinkVertex(0, 2, 78);
		PC0->connect(1, Node31.get(), 2);
		InstructionMemory1->connect(1, IFIDReg17.get(), 0);
		Mux2->addLinkVertex(2, 127, 139);
		Mux2->connect(3, EXMEMReg19.get(), 0);
		MainControl3->addLinkVertex(0, 59, 147);
		MainControl3->connect(1, IDEXReg18.get(), 8);
		MainControl3->connect(2, IDEXReg18.get(), 7);
		MainControl3->connect(3, IDEXReg18.get(), 6);
		ALU4->addLinkVertex(0, 14, 135);
		ALU4->connect(3, Node32.get(), 0);
		Registers5->addLinkVertex(0, 240, 133);
		Registers5->addLinkVertex(0, 240, 171);
		Registers5->addLinkVertex(0, 68, 171);
		Registers5->addLinkVertex(0, 68, 98);
		Registers5->addLinkVertex(0, 79, 98);
		Registers5->addLinkVertex(1, 57, 2);
		Registers5->addLinkVertex(1, 57, 88);
		Registers5->addLinkVertex(4, 235, 4);
		Registers5->addLinkVertex(4, 61, 4);
		Registers5->addLinkVertex(4, 61, 73);
		Registers5->connect(5, IDEXReg18.get(), 4);
		Registers5->connect(6, IDEXReg18.get(), 3);
		SignExtend6->connect(1, IDEXReg18.get(), 2);
		Mux37->addLinkVertex(0, 123, 73);
		Mux37->addLinkVertex(0, 123, 59);
		Mux37->addLinkVertex(3, 139, 69);
		Mux37->addLinkVertex(3, 134, 69);
		Mux37->connect(4, Node29.get(), 0);
		Mux38->addLinkVertex(1, 117, 85);
		Mux38->addLinkVertex(3, 154, 32);
		Mux38->addLinkVertex(3, 172, 32);
		Mux38->addLinkVertex(3, 172, 98);
		Mux38->addLinkVertex(3, 134, 98);
		Mux38->addLinkVertex(2, 120, 82);
		Mux38->connect(4, ALU11.get(), 0);
		ShiftLeft29->addLinkVertex(0, 111, 112);
		ShiftLeft29->connect(1, ALU10.get(), 1);
		ALU10->connect(3, EXMEMReg19.get(), 4);
		ALU11->connect(3, EXMEMReg19.get(), 3);
		ALU11->connect(4, EXMEMReg19.get(), 2);
		ALUControl12->addLinkVertex(0, 125, 138);
		ALUControl12->addLinkVertex(0, 125, 46);
		ALUControl12->addLinkVertex(0, 149, 46);
		ALUControl12->addLinkVertex(1, 111, 40);
		ALUControl12->connect(2, ALU11.get(), 2);
		DataMemory13->addLinkVertex(0, 208, 147);
		DataMemory13->addLinkVertex(0, 208, 87);
		DataMemory13->addLinkVertex(0, 197, 87);
		DataMemory13->addLinkVertex(1, 212, 148);
		DataMemory13->connect(4, MEMWBReg20.get(), 1);
		AndGate14->addLinkVertex(0, 188, 146);
		AndGate14->addLinkVertex(0, 188, 111);
		AndGate14->addLinkVertex(1, 188, 75);
		AndGate14->addLinkVertex(1, 188, 108);
		AndGate14->connect(2, Mux15.get(), 2);
		Mux7->addLinkVertex(2, 123, 137);
		Mux7->addLinkVertex(2, 123, 100);
		Mux7->addLinkVertex(2, 149, 100);
		Mux7->addLinkVertex(1, 141, 71);
		Mux7->addLinkVertex(1, 141, 67);
		Mux7->addLinkVertex(0, 143, 73);
		Mux7->connect(3, ALU11.get(), 1);
		Mux15->addLinkVertex(0, 40, 155);
		Mux15->addLinkVertex(1, 195, 119);
		Mux15->addLinkVertex(1, 195, 166);
		Mux15->addLinkVertex(1, 46, 166);
		Mux15->addLinkVertex(1, 46, 149);
		Mux15->addLinkVertex(2, 205, 109);
		Mux15->addLinkVertex(2, 205, 168);
		Mux15->addLinkVertex(2, 53, 168);
		Mux15->connect(3, PC0.get(), 0);
		Mux16->addLinkVertex(0, 232, 90);
		Mux16->addLinkVertex(0, 232, 83);
		Mux16->addLinkVertex(2, 240, 132);
		Mux16->connect(3, Node27.get(), 3);
		IFIDReg17->connect(2, Node33.get(), 0);
		IFIDReg17->connect(3, IDEXReg18.get(), 5);
		IDEXReg18->addLinkVertex(0, 59, 23);
		IDEXReg18->connect(9, Mux2.get(), 1);
		IDEXReg18->connect(10, Node22.get(), 0);
		IDEXReg18->connect(11, Node30.get(), 0);
		IDEXReg18->connect(12, Mux37.get(), 0);
		IDEXReg18->connect(13, Mux38.get(), 0);
		IDEXReg18->connect(14, ALU10.get(), 0);
		IDEXReg18->connect(15, Mux7.get(), 2);
		IDEXReg18->connect(16, ALUControl12.get(), 0);
		IDEXReg18->connect(17, Mux2.get(), 2);
		IDEXReg18->connect(18, EXMEMReg19.get(), 5);
		IDEXReg18->connect(19, EXMEMReg19.get(), 6);
		IDEXReg18->connect(21, Forwarding21.get(), 1);
		EXMEMReg19->connect(7, Node23.get(), 0);
		EXMEMReg19->connect(8, DataMemory13.get(), 3);
		EXMEMReg19->connect(9, AndGate14.get(), 1);
		EXMEMReg19->connect(10, Node25.get(), 0);
		EXMEMReg19->connect(11, Mux15.get(), 1);
		EXMEMReg19->connect(12, AndGate14.get(), 0);
		EXMEMReg19->connect(13, DataMemory13.get(), 0);
		EXMEMReg19->connect(14, DataMemory13.get(), 1);
		EXMEMReg19->connect(15, MEMWBReg20.get(), 3);
		MEMWBReg20->addLinkVertex(2, 184, 90);
		MEMWBReg20->addLinkVertex(3, 217, 155);
		MEMWBReg20->addLinkVertex(3, 217, 133);
		MEMWBReg20->connect(4, Node24.get(), 1);
		MEMWBReg20->connect(5, Mux16.get(), 1);
		MEMWBReg20->connect(6, Mux16.get(), 0);
		MEMWBReg20->connect(7, Mux16.get(), 2);
		MEMWBReg20->connect(8, Registers5.get(), 0);
		Forwarding21->addLinkVertex(0, 111, 11);
		Forwarding21->addLinkVertex(1, 115, 33);
		Forwarding21->addLinkVertex(1, 115, 16);
		Forwarding21->addLinkVertex(3, 188, 16);
		Forwarding21->connect(4, Mux37.get(), 3);
		Forwarding21->connect(5, Mux38.get(), 3);
		Node22->connect(4, Forwarding21.get(), 0);
		Node22->connect(5, Mux2.get(), 0);
		Node23->connect(4, Forwarding21.get(), 3);
		Node23->connect(7, MEMWBReg20.get(), 0);
		Node24->addLinkVertex(1, 235, 26);
		Node24->connect(4, Forwarding21.get(), 2);
		Node24->connect(5, Registers5.get(), 4);
		Node25->connect(4, Node26.get(), 2);
		Node25->connect(5, MEMWBReg20.get(), 2);
		Node25->connect(7, DataMemory13.get(), 2);
		Node26->addLinkVertex(2, 184, 7);
		Node26->addLinkVertex(2, 120, 7);
		Node26->connect(5, Mux38.get(), 2);
		Node26->connect(7, Mux37.get(), 2);
		Node27->addLinkVertex(3, 246, 80);
		Node27->addLinkVertex(3, 246, 2);
		Node27->connect(4, Node28.get(), 2);
		Node27->connect(5, Registers5.get(), 1);
		Node28->connect(4, Mux38.get(), 1);
		Node28->connect(5, Mux37.get(), 1);
		Node29->addLinkVertex(0, 143, 56);
		Node29->connect(5, Mux7.get(), 0);
		Node29->connect(7, EXMEMReg19.get(), 1);
		Node30->connect(4, Node38.get(), 2);
		Node38->connect(5, ShiftLeft29.get(), 0);
		Node38->connect(7, Mux7.get(), 1);
		Node30->connect(5, ALUControl12.get(), 1);
		Node31->connect(4, InstructionMemory1.get(), 0);
		Node31->connect(5, ALU4.get(), 0);
		Node32->connect(5, Mux15.get(), 0);
		Node32->connect(7, IFIDReg17.get(), 1);
		Node33->connect(4, Node35.get(), 0);
		Node33->connect(5, Node34.get(), 2);
		Node33->connect(7, Registers5.get(), 3);
		Node34->connect(4, Registers5.get(), 2);
		Node34->connect(5, MainControl3.get(), 0);
		Node35->connect(4, Node36.get(), 1);
		Node35->connect(7, SignExtend6.get(), 0);
		Node36->connect(4, Node37.get(), 1);
		Node36->connect(7, IDEXReg18.get(), 20);
		Node37->connect(4, IDEXReg18.get(), 0);
		Node37->connect(7, IDEXReg18.get(), 1);
		PC0->setActive();
		PC0->confirmActive();
	}
}

void Model::setHighlightInstruction(luint instr, bool valid)
{		
	Component::setCurrentInstruction(-1);	
	if(valid)
	{
		for(uint i = 0; i < 5; i++)
		{
			if(instr == currAddr[i])
			{
				Component::setCurrentInstruction(i);
			}
		}
	}	
}

void Model::step()
{
	for(int steps = 0; steps < (layout == LAYOUT_SIMPLE ? 1 : (Component::pipelineClockSteps)) ; steps++)
	{
		
		bool reset = false;
	 	// Step all components (update outputs based on inputs).
	 	for(auto&& i : components)
		{
			if(i->getType() != NODE_TYPE)
			{
	 			i->step();
	 		}
	 	}
		// Pre Step all components (update inputs based on outputs connected to said inputs).
		for(auto&& i : components)
		{
	 		i->preStep();
	 	}
	 	// Prevent activity changes from propagating two steps in one.
	 	for(auto&& i : components)
		{
			if(i->getType() != NODE_TYPE)
			{
	 			i->confirmActive();
			}
			if(i->getType() == PC_TYPE)
			{
				if(i->allInputsActive())
				{
					reset = true;
				}
			}
	 	}
	 	// Flush pipeline if branch is taken.
	 	if(layout != LAYOUT_SIMPLE)
	 	{
	 		// Branch is being taken.
	 		if(branchCheckGate->getOutput() == 1)
	 		{
	 			buffers[0]->reset();
	 		}
	 		
	 		// With non simple layout set all components to active.
	 		for(auto&& i : components)
			{
				if(i->getType() != NODE_TYPE)
				{
	 				i->setActive();
				}
				i->resetOldActiveLinkColor();
	 		}
	 	}
	 	
	 	if(reset && layout == LAYOUT_SIMPLE)
	 	{
	 		this->reset();
	 	}
	}
	luint addr = programCounter->getOutput();
	for(int i = 4; i > 0; i--)
	{
		currInstr[i] = currInstr[i - 1];
	}
	currInstr[0] = pmz->getInstructionSymbol(addr);
	lastAddrLoc++;
	lastAddrLoc %= 5;
	currAddr[lastAddrLoc] = addr;
	if(layout != LAYOUT_SIMPLE)
	{
		Component::incrementPipelineCycle();
	}
}

void Model::reset()
{
	for(auto&& i : components)
	{
		if((i->getType() != NODE_TYPE) && (i->getType() != PC_TYPE))
		{
			i->setActive(false);
			// Multiple step propagation irrelevant because setting everything to false.
 			i->confirmActive();
		}
		else if(i->getType() == NODE_TYPE)
		{
			i->setActive(false);
		}
		else if(i->getType() == PC_TYPE)
		{
			i->resetOldActiveLinkColor();
		}
	}
}

void Model::addVertex(wxPoint mousePos)
{
	Coord prevVertex;
	Coord newVertex;

	// Set the previous vertex to be the last in the vertex list or the output link
	if(vertices.empty())
	{
		prevVertex.x = oLink->getAbsX();
		prevVertex.y = oLink->getAbsY();
	}
	else
	{
		prevVertex.x = vertices.back().x;
		prevVertex.y = vertices.back().y;
	}
	
	// Snap to closest either x or y axis.
	if(abs(mousePos.x - prevVertex.x) < abs(mousePos.y - prevVertex.y))
	{
		newVertex.x = prevVertex.x;
		newVertex.y = mousePos.y;
	}
	else
	{
		newVertex.x = mousePos.x;
		newVertex.y = prevVertex.y;
	}
	
	vertices.push_back(newVertex);
}

Model::~Model()
{

}

void Model::draw(wxPoint mousePos, double scale)
{	
	bool simpleLayout = (layout == LAYOUT_SIMPLE);
	bool showControl = bools[SHOW_CONTROL_LINES];
	bool showPC = bools[SHOW_PC_LINES];
	
	Component::drawBackground();
	for(auto&& i : components)
	{
		if(showControl || (!(i->getIsControl())))
		{
			if(showPC || (!(i->getIsPC())))
			{
 				
 				i->drawConnectors(showControl, showPC, currInstr, simpleLayout);
 				i->drawConnections(showControl, showPC, currInstr, simpleLayout);
 				i->draw(showControl, showPC, currInstr, simpleLayout, scale);
			}
		}

 
 	}
 	if(creatingConnection)
 	{
 		int newX = (int)(oLink->getComp()->getX() + oLink->getX() * oLink->getComp()->getW());
 		int newY = (int)(oLink->getComp()->getY() + oLink->getY() * oLink->getComp()->getH());
 		glPushAttrib(GL_COLOR_BUFFER_BIT);
		glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_LINE_STRIP);
		glVertex2f(newX, newY);
		for(auto&& vertex : vertices)
		{
			glVertex2f(vertex.x, vertex.y);
		}		
		glVertex2f(mousePos.x, mousePos.y);
		glEnd();
		glPopAttrib();
 	}
}

Component* Model::findComponent(wxPoint mousePos)
{
	for(auto&& comp : components)
	{
		if(comp->getX() < mousePos.x && comp->getX() + comp->getW() > mousePos.x)
		{
			if(comp->getY() < mousePos.y && comp->getY() + comp->getH() > mousePos.y)
			{
				return comp.get();
			}
		}
	}
	
	return nullptr;
}

Link* Model::findLink(wxPoint mousePos)
{
	uint slot;
	for(auto&& comp : components)
	{
		if(comp->getType() != NODE_TYPE)
		{
			slot = 0;
			Link *ln = comp->getLink(slot);
			while(ln != 0)
			{
				if(!ln->isOutput() || ln->isConnected())
				{
					if(ln->getTipPos(LEFT) < mousePos.x && ln->getTipPos(RIGHT) > mousePos.x)
					{
						if(ln->getTipPos(BOTTOM) < mousePos.y && ln->getTipPos(TOP) > mousePos.y)
						{
							return ln;
						}
					}
				}
				ln = comp->getLink(++slot);
			}
		}
	}
	
	return 0;
}

luint Model::getMemoryData(uint mem, luint address)
{
	return memories[mem]->getData(address);
}

void Model::setMemoryData(uint mem, luint address, luint data)
{
	memories[mem]->setData(address, data);
}










