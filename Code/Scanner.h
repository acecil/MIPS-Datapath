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

#ifndef SCANNER_H_
#define SCANNER_H_

#include <fstream>
#include <string>
#include <map>
#include <iostream>

#include <wx/string.h>

#include "Enums.h"
#include "Types.h"

class Scanner
{
public:
	Scanner(wxString defname);
	~Scanner();
	bool checkFile();
	void getLine(wxString & line);
	void getFileContents(wxString & str);
	void setFileContents(wxString & str);
	void getSymbol(Symbol & s, int & num);
	void unget();
	void getChar(bool fromFile = false);
private:
	void skipSpaces();
	void getNumber(int &num);
	Symbol getName();
	bool eofile;
	uint pos;
	wxString contents;
	std::ifstream deffile;
	char curch;
	wxString curLine, lastLine;
	std::map<std::string, Symbol> names;
};

#endif /*SCANNER_H_*/
