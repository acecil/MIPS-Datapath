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
 
#include "Scanner.h"

#include "Parser.h"

Parser::Parser(Scanner *smz)
{
	this->smz = smz;
	
	opcodes[SYM_LW] = 35;
	opcodes[SYM_SW] = 43;
	opcodes[SYM_ADD] = 0;
	opcodes[SYM_ADDI] = 8;
	opcodes[SYM_AND] = 0;
	opcodes[SYM_OR] = 0;
	opcodes[SYM_SLT] = 0;
	opcodes[SYM_BEQ] = 4;
	opcodes[SYM_SUB] = 0;
	opcodes[SYM_NOOP] = 0;
	
	opstrings[SYM_LW] = _T("LW");
	opstrings[SYM_SW] = _T("SW");
	opstrings[SYM_ADD] = _T("ADD");
	opstrings[SYM_ADDI] = _T("ADDI");
	opstrings[SYM_AND] = _T("AND");
	opstrings[SYM_OR] = _T("OR");
	opstrings[SYM_SLT] = _T("SLT");
	opstrings[SYM_BEQ] = _T("BEQ");
	opstrings[SYM_SUB] = _T("SUB");
	opstrings[SYM_NOOP] = _T("NOP");
	
	
	functcodes[SYM_ADD] = 32;
	functcodes[SYM_AND] = 36;
	functcodes[SYM_OR] = 37;
	functcodes[SYM_SLT] = 42;
	functcodes[SYM_SUB] = 34;

	errorStrings[ERR_MIN] = _T("Unknown Error.");
	errorStrings[ERR_INSTRUCTION] = _T("Instruction is required.");
	errorStrings[ERR_DOLLAR] = _T("'$' is required.");
	errorStrings[ERR_REGISTER] = _T("Register number is required.");
	errorStrings[ERR_REG_NUM] = _T("Register number was outside allowable bounds (0-31).");
	errorStrings[ERR_COMMA] = _T("',' is required.");
	errorStrings[ERR_OFFSET] = _T("Offset number is required.");
	errorStrings[ERR_LBRACKET] = _T("'(' is required.");
	errorStrings[ERR_RBRACKET] = _T("')' is required.");
	errorStrings[ERR_IMMEDIATE] = _T("Immediate number is required.");
	errorStrings[ERR_EOL] = _T("End of line required after instruction.");
	errorStrings[ERR_NO_INSTRUCTION_TAG]  = _T("'Instruction' label is required.");
	errorStrings[ERR_NO_DATA_TAG]  = _T("'Data' label is required.");
	errorStrings[ERR_NO_COLON]  = _T("':' is required.");
}

bool Parser::parse(wxString & str)
{
	curSymbol = SYM_MIN;
	errortotal = 0;
	address = 0;
	if(str != _T(""))
	{
		fileContents = str;
		smz->setFileContents(str);
	}
	
	// Clear temporary instruction store.
	instructions.clear();
	formattedInstructions.clear();
	instructionSymbols.clear();
	errors.clear();
	smz->getChar();

	do
	{
		readInstruction();
	}
	while(curSymbol != SYM_EOF && curSymbol != SYM_DATA);
	
	if(curSymbol != SYM_DATA)
	{
		invalidInput(ERR_NO_DATA_TAG);
	}
	do
	{
	smz->getSymbol(curSymbol, curNum);
	}
	while(curSymbol != SYM_COLON && curSymbol != SYM_EOF);
	if(curSymbol != SYM_EOF)
	{
		luint address = 0;
		do
		{
			readMemory(address);
			address++;
		}
		while(curSymbol != SYM_EOF);
	}
	
	if(errortotal != 0) 
    {
	//cout << "Errors: " << errortotal << endl;
      	return false;
    }
  	else 
    {
     	return true;
    }
}

void Parser::readMemory(luint address)
{
	do
	{
		smz->getSymbol(curSymbol, curNum);
	}
	while(curSymbol != SYM_NUM && curSymbol != SYM_EOF && curSymbol != SYM_COMMA);
	if(curSymbol != SYM_NUM)
	{
		return;	
	}
	else
	{
		// Memory data found.
		if(curNum < 0)
		{
			initialMemData[address] = ((luint)(1)<<32) - abs(curNum);
		}
		else
		{
			initialMemData[address] = curNum;
		}
	}
	smz->getSymbol(curSymbol, curNum);
	if(curSymbol != SYM_COMMA)
	{
		return;
	}
}

bool Parser::readIn()
{
	// Take contents of file and store in fileContents string.
	smz->getFileContents(fileContents);
	
	return true;
}

void Parser::printInstructions()
{
	for(auto i = instructions.begin(); i != instructions.end(); i++)
	{
		std::cout << i->first << " " << i->second << std::endl;
	}
}

luint Parser::getInstruction(luint address)
{
	return instructions[address];
}

void Parser::getError(int num, wxString& errString)
{
	ParseError err = errors[num];
	
	if(err == 0)
	{
		errString = _T("");
	}
	else
	{
		errString = errorStrings[err];
	}
}

void Parser::getInstructionString(luint address, wxString & instr, bool & err)
{
	instr = formattedInstructions[address].str;
	err = formattedInstructions[address].err;
}

void Parser::getMemString(luint address, wxString & str)
{
	str.Printf(_T("%u, "), initialMemData[address]);
}

luint Parser::getMemData(luint address)
{
	return initialMemData[address];
}

void Parser::setMemData(luint address, luint data)
{
	initialMemData[address] = data;
}

Symbol Parser::getInstructionSymbol(luint address)
{
	return instructionSymbols[address];
}

void Parser::getFileContents(wxString & str)
{
	str = fileContents;
}

void Parser::readInstruction()
{
	smz->getSymbol(curSymbol, curNum);
	
	instrErrors = 0;
	
	switch(curSymbol)
	{
		case SYM_LW:
		case SYM_SW:
			readLoadStoreInstruction();
			break;
		case SYM_ADDI:
		case SYM_BEQ:
			readImmediateInstruction();
			break;
		case SYM_ADD:
		case SYM_SUB:
		case SYM_OR:
		case SYM_AND:
		case SYM_SLT:
			readRegisterInstruction();
			break;
		case SYM_NOOP:
			readNoopInstruction();
			break;
		case SYM_EOL:
		case SYM_EOF:
		case SYM_DATA:
			// Do no processing.
			break;
		default:
			invalidInput(ERR_INSTRUCTION);
			wxString line;
			smz->getLine(line);
			formattedInstructions[address].str = line;
			formattedInstructions[address].err = true;	
			address += 4;
	}
}

void Parser::readLoadStoreInstruction()
{
	op = opcodes[curSymbol];
	opstring = opstrings[curSymbol];
	instructionSymbols[address] = curSymbol;
	readRegister(rt);
	readPunct(SYM_COMMA, ERR_COMMA);
	readNum(val, imm, ERR_OFFSET);	
	// Check bounds on offset.
	if(val < -((1<<15) - 1) || ((1<<15) - 1) < val)
	{
		invalidInput(ERR_OFFSET);
	}
	readPunct(SYM_LBRACKET, ERR_LBRACKET);
	readRegister(rs);
	readPunct(SYM_RBRACKET, ERR_RBRACKET);
	readEOL();

	if(instrErrors == 0)
	{
		instructions[address] = OP_OFFSET * op + RS_OFFSET * rs + RT_OFFSET * rt + imm;	
		formattedInstructions[address].str = opstring + _T(" $") + wxString::Format(_T("%d"), rt) + _T(", ") + wxString::Format(_T("%d"), val) + _T("($") + wxString::Format(_T("%d"), rs) + _T(")\n");
		formattedInstructions[address].err = false;
	}
	else
	{
		wxString line;
		smz->getLine(line);
		formattedInstructions[address].str = line;
		formattedInstructions[address].err = true;
	}
	address += 4;
}

void Parser::readImmediateInstruction()
{
	op = opcodes[curSymbol];
	opstring = opstrings[curSymbol];
	instructionSymbols[address] = curSymbol;
	readRegister(rt);
	readPunct(SYM_COMMA, ERR_COMMA);
	readRegister(rs);
	readPunct(SYM_COMMA, ERR_COMMA);
	readNum(val, imm, ERR_IMMEDIATE);
	readEOL();
	
	if(instrErrors == 0)
	{
		instructions[address] = OP_OFFSET * op + RS_OFFSET * rs + RT_OFFSET * rt + imm;
		formattedInstructions[address].str = opstring + _T(" $") + wxString::Format(_T("%d"), rt) + _T(", $") + wxString::Format(_T("%d"), rs) + _T(", ") + wxString::Format(_T("%d"), val) + _T("\n");
		formattedInstructions[address].err = false;
	}
	else
	{
		wxString line;
		smz->getLine(line);
		formattedInstructions[address].str = line;
		formattedInstructions[address].err = true;
	}
	address += 4;
}

void Parser::readRegisterInstruction()
{
	op = opcodes[curSymbol];
	opstring = opstrings[curSymbol];
	instructionSymbols[address] = curSymbol;
	function = functcodes[curSymbol];
	readRegister(rd);
	readPunct(SYM_COMMA, ERR_COMMA);
	readRegister(rs);
	readPunct(SYM_COMMA, ERR_COMMA);
	readRegister(rt);
	readEOL();
	
	if(instrErrors == 0)
	{
		instructions[address] = OP_OFFSET * op + RS_OFFSET * rs + RT_OFFSET * rt + RD_OFFSET * rd + function;	
		formattedInstructions[address].str = opstring + _T(" $") + wxString::Format(_T("%d"), rd) + _T(", $") + wxString::Format(_T("%d"), rs) + _T(", $") + wxString::Format(_T("%d"), rt) + _T("\n");
		formattedInstructions[address].err = false;
	}	
	else
	{
		wxString line;
		smz->getLine(line);
		formattedInstructions[address].str = line;
		formattedInstructions[address].err = true;
	}
	address += 4;
}

void Parser::readNoopInstruction()
{
	opstring = opstrings[curSymbol];
	instructionSymbols[address] = curSymbol;
	readEOL();
	
	if(instrErrors == 0)
	{
		instructions[address] = 0;
		formattedInstructions[address].str = opstring + _T("\n");
		formattedInstructions[address].err = false;
	}
	else
	{
		wxString line;
		smz->getLine(line);
		formattedInstructions[address].str = line;
		formattedInstructions[address].err = true;
	}
	address += 4;
}

void Parser::readRegister(int & reg)
{
	if(instrErrors > 0) return;
	// Check for dollar before register number.
	smz->getSymbol(curSymbol, curNum);
	if(curSymbol != SYM_DOLLAR)
	{
		invalidInput(ERR_DOLLAR);
	}
	if(instrErrors > 0) return;
	// Check for register number.
	smz->getSymbol(curSymbol, curNum);
	if(curSymbol != SYM_NUM)
	{
		invalidInput(ERR_REGISTER);
	}
	if(instrErrors > 0) return;
	if(curNum < 0 || 31 < curNum)
	{
		invalidInput(ERR_REG_NUM);
	}
	// Store register number in register slot chosen by calling function.
	reg = curNum;
}

void Parser::readPunct(Symbol sym, ParseError err)
{
	if(instrErrors > 0) return;
	smz->getSymbol(curSymbol, curNum);
	if(curSymbol != sym)
	{
		invalidInput(err);
	}
}

void Parser::readNum(int & val, luint & imm, ParseError err)
{
	if(instrErrors > 0) return;
	smz->getSymbol(curSymbol, curNum);
	if(curSymbol != SYM_NUM)
	{
		invalidInput(err);
	}
	if(curNum < 0)
	{
		imm = ((luint)(1)<<16) + curNum;
	}
	else
	{
		imm = (luint)curNum;
	}
	val = curNum;
}

void Parser::readEOL()
{
	if(instrErrors > 0) return;
	smz->getSymbol(curSymbol, curNum);
	if(curSymbol  != SYM_EOL && curSymbol != SYM_EOF)
	{
		invalidInput(ERR_EOL);
	}
}

void Parser::invalidInput(ParseError err)
{
	// Deal with invalid input here.
	errors[errortotal] = err;
	
	while(curSymbol != SYM_EOL && curSymbol != SYM_EOF)
	{
		smz->getSymbol(curSymbol, curNum);
	}
	
	errortotal++;
	instrErrors++;
}

