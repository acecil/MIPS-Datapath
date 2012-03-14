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

#ifndef PARSER_H_
#define PARSER_H_

#include <map>
#include <vector>
#include <iostream>

#include "Types.h"
#include "Enums.h"

class Scanner;

enum ParseError
{
	ERR_MIN,
	ERR_INSTRUCTION,
	ERR_DOLLAR,
	ERR_REGISTER,
	ERR_REG_NUM,
	ERR_COMMA,
	ERR_OFFSET,
	ERR_LBRACKET,
	ERR_RBRACKET,
	ERR_IMMEDIATE,
	ERR_EOL,
	ERR_NO_INSTRUCTION_TAG,
	ERR_NO_DATA_TAG,
	ERR_NO_COLON,
	ERR_MAX,
};

class Parser
{
public:
	Parser(Scanner *smz);
	bool parse(wxString & str);
	bool readIn();
	void printInstructions();
	luint getInstruction(luint address);
	void getError(int num, wxString& error);
	int getNumberOfErrors(){ return errortotal; };
	void getInstructionString(luint address, wxString & instr, bool & err);
	void getMemString(luint address, wxString & str);
	luint getMemData(luint address);
	void setMemData(luint address, luint data);
	void getFileContents(wxString & str);
	Symbol getInstructionSymbol(luint address);
private:
	struct instruction {
		wxString str;
		bool err;
	};
	void readMemory(luint address);
	void readInstruction();
	void readLoadStoreInstruction();
	void readImmediateInstruction();
	void readRegisterInstruction();
	void readNoopInstruction();
	void readRegister(int &reg);
	void readPunct(Symbol sym, ParseError err);
	void readNum(int & val, luint & imm, ParseError err);
	void readEOL();
	void invalidInput(ParseError err);
	Scanner *smz;
	int errortotal;
	int instrErrors;
	Symbol curSymbol;
	int curNum;
	int rs, rt, rd, val, op, function;
	luint imm;
	wxString opstring;
	std::map<Symbol, int> opcodes;
	std::map<Symbol, wxString> opstrings;
	std::map<Symbol, int> functcodes;
	luint address;
	std::map<luint, luint> instructions;
	std::map<luint, instruction> formattedInstructions;
	std::map<luint, Symbol> instructionSymbols;
	std::map<luint, luint> initialMemData;
	wxString fileContents;
	std::map<int, ParseError> errors;
	std::map<ParseError, wxString> errorStrings;
	static const luint OP_OFFSET = 1<<26;
	static const luint RS_OFFSET = 1<<21;
	static const luint RT_OFFSET = 1<<16;
	static const luint RD_OFFSET = 1<<11;
};

#endif /*PARSER_H_*/
