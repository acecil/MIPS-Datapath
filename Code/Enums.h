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
 
#ifndef ENUMS_H_
#define ENUMS_H_

enum Symbol
{
	SYM_MIN = 0,
	SYM_BAD = 0,
	SYM_LW,
	SYM_SW,
	SYM_ADDI,
	SYM_BEQ,
	SYM_ADD,
	SYM_SUB,
	SYM_SLT,
	SYM_AND,
	SYM_OR,
	SYM_NOOP,
	SYM_NUM,
	SYM_COLON,
	SYM_DOLLAR,
	SYM_COMMA,
	SYM_LBRACKET,
	SYM_RBRACKET,
	SYM_EOF,
	SYM_EOL,
	SYM_DATA,
	SYM_MAX
};

enum Layout
{
	LAYOUT_MIN = 0,
	LAYOUT_SIMPLE = 0,
	LAYOUT_PIPELINE = 1,
	LAYOUT_FORWARDING = 2,
	LAYOUT_MAX = 2
};

enum MainControlInputOutputs
{
	MAINCONTROL_MIN = 0,
	MAINCONTROL_INPUT = 0,
	MAINCONTROL_BRANCH = 1,
	MAINCONTROL_MEMTOREG = 2,
	MAINCONTROL_MEMREAD = 3,
	MAINCONTROL_ALUOP = 4,
	MAINCONTROL_MEMWRITE = 5,
	MAINCONTROL_ALUSRC = 6,
	MAINCONTROL_REGWRITE = 7,
	MAINCONTROL_REGDST = 8,
	MAINCONTROL_MAX = 8
};

enum MainControlPipelinedInputOutputs
{
	MAINCONTROLPIPELINED_MIN = 0,
	MAINCONTROLPIPELINED_INPUT = 0,
	MAINCONTROLPIPELINED_WB = 1,
	MAINCONTROLPIPELINED_MEM = 2,
	MAINCONTROLPIPELINED_EX = 3,
	MAINCONTROLPIPELINED_MAX = 3
};

enum ForwardingInputsOutputs
{
	FORWARDING_MIN = 0,
	FORWARDING_READREG1 = 1,
	FORWARDING_READREG2 = 0,
	FORWARDING_EXMEMREG = 3,
	FORWARDING_MEMWBREG = 2,
	FORWARDING_MUXA = 5,
	FORWARDING_MUXB = 4,
	FORWARDING_MAX = 5
};

enum DataLists
{
	ID_INSTRUCTION_LIST,
    ID_REGISTER_LIST,
    ID_DATA_LIST
};

#endif /*ENUMS_H_*/
