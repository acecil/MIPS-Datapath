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

// Public Methods

Scanner::Scanner(wxString defname)
{
	names["data"] = SYM_DATA;
	names["lw"] = SYM_LW;
	names["sw"] = SYM_SW;
	names["add"] = SYM_ADD;
	names["sub"] = SYM_SUB;
	names["slt"] = SYM_SLT;
	names["addi"] = SYM_ADDI;
	names["or"] = SYM_OR;
	names["and"] = SYM_AND;
	names["beq"] = SYM_BEQ;
	names["nop"] = SYM_NOOP;
	
	if(defname != _T(""))
	{	
	  	deffile.open(defname.fn_str());
	  	  	
	  	if (!deffile) 
	    {
	      	std::cout << "File Error" << std::endl;
	    }
	  	else 
	    {
	      	getChar(true);
	    }
	}
}

Scanner::~Scanner()
{
}

void Scanner::getFileContents(wxString & str)
{
	deffile.seekg(0);
	str.Clear();
	while(eofile == false)
	{
		getChar(true);
		str += curch;
	}
	// store contents locally
	// set position to 0.
	contents = str;
	pos = 0;
	eofile = false;
}

void Scanner::setFileContents(wxString & str)
{
	contents = str;
	pos = 0;
	eofile = false;
}

void Scanner::getLine(wxString & line)
{
	line = lastLine;
}

void Scanner::getSymbol(Symbol & s, int & num)
{
  	if (!eofile)
    {
  		skipSpaces();
  
	  	if(eofile)
	    {
	      	s = SYM_EOF;
	    }
	 	else if(curch == '\n') 
	    {
	      	s = SYM_EOL;
	      	lastLine = curLine;
	      	curLine = _T("");
	      	getChar();
	    }
	  	else if(isdigit(curch) || curch == '-')
	    { 
	      	getNumber(num);
	      	s = SYM_NUM;
	    }
	  	else if(isalpha(curch))
	    {
	      	s = getName();
	    }
	  	else
	    {
	      	switch (curch) 
			{
				case ':':
					s = SYM_COLON;
					break;
				case ',': 
					s = SYM_COMMA; 
					break;
				case '$': 
					s = SYM_DOLLAR; 
					break;
				case '(': 
					s = SYM_LBRACKET; 
					break;
				case ')': 
					s = SYM_RBRACKET; 
					break;
				default: 
					s = SYM_BAD; 
			}
	      	getChar();
	    }
    }
  	else 
    {
     	s = SYM_EOF;
    }
}

void Scanner::unget()
{
	deffile.unget();
}

// Gets the next character from the definition file
// If character is '\n' (end of line), sets eoline to true to keep current character position correct
void Scanner::getChar(bool fromFile)
{
	if(fromFile)
	{
	  	eofile = deffile.get(curch).eof();
	}
	else
	{
		if(pos < contents.Len())
		{
			curch = contents.GetChar(pos);
			pos++;
			curLine += curch;
		}
		else
		{
			eofile = true;
			curch = 0;
		}
	}
}

bool Scanner::checkFile()
{
	return deffile.good();
}

// Private Methods

// Continues getting characters from the definition file until non-space found
void Scanner::skipSpaces()
{
  	while (!eofile && isspace(curch) && curch != '\n')
    {
      	getChar();
    }
}

// Gets the number from definition file until non-digit character found
void Scanner::getNumber(int &num)
{
  	num = 0;
  	bool negative = false;
  	if(curch == '-')
  	{
  		negative = true;
  		getChar();
  	}
  	
  	while ( (!eofile) && (!isdigit(curch)==0) )
    { 
      	num = 10 * num + atoi(&curch);
      	getChar();
    }
    
    if(negative)
    {
    	num = -num;
    }
}

// Gets the name from the definition file until non-alpha or non-digit character found
Symbol Scanner::getName()
{
  	std::string tempName;
  
  	while((!eofile ) && ((!isalpha(curch)==0) || (!isdigit(curch)==0)))
    {
      	tempName += tolower(curch);
      	getChar();
    }

	auto nameIt = names.find(tempName);
	if( nameIt == names.end() )
	{
		return SYM_BAD;
	}
     
  	return nameIt->second;
}


