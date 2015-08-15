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
 
#include "Config.h"

Config &Config::Instance () 
{
	static Config instance;
	
	return instance;
}

Config::Config() 
{ 
	// Setup names for each enum item for reading and writing to config file.
	names[BAD_NAME] = "";
	names[COMPONENT_COLOUR] = "componentcolour";
	names[PIPEREG_COLOUR] = "piperegcolour";
	names[STAGE1_COLOUR] = "stage1colour";
	names[STAGE2_COLOUR] = "stage2colour";
	names[STAGE3_COLOUR] = "stage3colour";
	names[STAGE4_COLOUR] = "stage4colour";
	names[STAGE5_COLOUR] = "stage5colour";
	names[TEXT_COLOUR] = "textcolour";
	names[FADE_COLOUR] = "fadecolour";
	names[BORDER_COLOUR] = "bordercolour";
	names[LINK_COLOUR] = "inactivelinkcolour";
	names[BACKGROUND_COLOUR] = "backgroundcolour";
	nameIsColour[COMPONENT_COLOUR] = true;
	nameIsColour[PIPEREG_COLOUR] = true;
	nameIsColour[STAGE1_COLOUR] = true;
	nameIsColour[STAGE2_COLOUR] = true;
	nameIsColour[STAGE3_COLOUR] = true;
	nameIsColour[STAGE4_COLOUR] = true;
	nameIsColour[STAGE5_COLOUR] = true;
	nameIsColour[TEXT_COLOUR] = true;
	nameIsColour[FADE_COLOUR] = true;
	nameIsColour[BORDER_COLOUR] = true;
	nameIsColour[LINK_COLOUR] = true;
	nameIsColour[BACKGROUND_COLOUR] = true;
	
	names[SHOW_LEFT_PANEL] = "showleftpanel";
	names[SHOW_INSTRUCTION_FIELDS] = "showinstructionsinfields";
	names[SHOW_SIMPLE_LAYOUT] = "showsimplelayout";
	names[SHOW_PIPELINE_LAYOUT] = "showpipelinelayout";
	names[SHOW_FORWARDING_LAYOUT] = "showforwardinglayout";
	names[SHOW_CONTROL_LINES] = "showcontrollines";
	names[SHOW_PC_LINES] = "showpclines";
	names[SHOW_POPUPS] = "showpopups";
	names[HIGHLIGHT_SINGLE_INSTRUCTION] = "highlightsingleinstruction";
	names[SHOW_BOLD_DATA_LINES] = "showbolddatalines";
	names[SHOW_FORMAT_BIN] = "binaryformat";
	names[SHOW_FORMAT_DEC] = "decimalformat";
	names[SHOW_FORMAT_HEX] = "hexadecimalformat";
	
	names[SIMULATOR_SASH_POS] = "simulatorsashpos";
	names[EDITOR_SASH_POS] = "editorsashpos";
	isNameANumber[SIMULATOR_SASH_POS] =  true;
	isNameANumber[EDITOR_SASH_POS] = true;
	
	for(auto i = names.begin(); i != names.end(); ++i)
	{
		snames[(*i).second] = (*i).first;
	}
	
	// If file exists, load it and parse its contents into local maps.
	// else, set local contents to defaults and create file (call save).
	configFile.open("mips.conf");
	
	
	setDefaults();
	if(configFile)
	{
		parse();
	}
	
	configFile.close();
}

void Config::setDefaults()
{
	setColour(COMPONENT_COLOUR, wxColour(191, 191, 153, 255));
	setColour(PIPEREG_COLOUR, wxColour(191, 153, 153, 255));
	setColour(STAGE1_COLOUR, wxColour(255, 0, 0, 255));
	setColour(STAGE2_COLOUR, wxColour(0, 255, 0, 255));
	setColour(STAGE3_COLOUR, wxColour(255, 0, 255, 255));
	setColour(STAGE4_COLOUR, wxColour(0, 0, 255, 255));
	setColour(STAGE5_COLOUR, wxColour(0, 255, 255, 255));
	setColour(TEXT_COLOUR, wxColour(0, 0, 0, 255));
	setColour(FADE_COLOUR, wxColour(255, 255, 255, 255));
	setColour(BORDER_COLOUR, wxColour(0, 0, 0, 255));
	setColour(LINK_COLOUR, wxColour(0, 0, 0, 255));
	setColour(BACKGROUND_COLOUR, wxColour(255, 255, 255, 255));
	setBool(SHOW_LEFT_PANEL, true);
	setBool(SHOW_INSTRUCTION_FIELDS, true);
	setBool(SHOW_SIMPLE_LAYOUT, true);
	setBool(SHOW_PIPELINE_LAYOUT, false);
	setBool(SHOW_FORWARDING_LAYOUT, false);
	setBool(SHOW_CONTROL_LINES, true);
	setBool(SHOW_PC_LINES, true);
	setBool(SHOW_POPUPS, true);
	setBool(HIGHLIGHT_SINGLE_INSTRUCTION, true);
	setBool(SHOW_BOLD_DATA_LINES, true);
	setBool(SHOW_FORMAT_BIN, false);
	setBool(SHOW_FORMAT_DEC, true);
	setBool(SHOW_FORMAT_HEX, false);
	setNumber(SIMULATOR_SASH_POS, 200);
	setNumber(EDITOR_SASH_POS, 200);
}

bool Config::parse()
{
	// Parse contents of configFile. ConfigFile is already open
	getChar();
	curSymbol = SYM_MIN;
	
	do
	{
		readConfigValue();
	}
	while(curSymbol != SYM_EOF);
	
	// Couple of sanity checks
	if(!getBool(SHOW_FORMAT_BIN) && !getBool(SHOW_FORMAT_DEC) && !getBool(SHOW_FORMAT_HEX))
	{
		setBool(SHOW_FORMAT_DEC, true);
	}
	if(!getBool(SHOW_SIMPLE_LAYOUT) && !getBool(SHOW_PIPELINE_LAYOUT) &&!getBool(SHOW_FORWARDING_LAYOUT))
	{
		setBool(SHOW_SIMPLE_LAYOUT, true);
	}
	return false;
}

void Config::readConfigValue()
{
	unsigned int num;
	configName name;
	
	// Get config name
	getSymbol(curSymbol, num, name);
	if(curSymbol != SYM_NAME)
	{
		readToLineEnd();
		return;
	}
	
	configName actualName = name;
	bool actualBool;
	unsigned int actualNum;
	unsigned int col[4];
	if(name == BAD_NAME)
	{
		readToLineEnd();
		return;
	}
	if(nameIsColour[name])
	{
		// This is a colour.
		getSymbol(curSymbol, num, name);
		if(curSymbol != SYM_COLON)
		{
			readToLineEnd();
			return;
		}
		// Get each part of the colour.
		for(int i = 0; i < 4; i++)
		{
			getSymbol(curSymbol, num, name);
			if(curSymbol != SYM_NUM)
			{
				readToLineEnd();
				return;
			}
			col[i] = num;
			getSymbol(curSymbol, num, name);
			if(!(curSymbol == SYM_COMMA || (curSymbol == SYM_SEMICOL && i == 3)))
			{
				readToLineEnd();
				return;
			}
			setColour(actualName, wxColour(col[0], col[1], col[2], col[3]));
		}
		
		
	}
	else if(isNameANumber[name])
	{
		// This is a number.
		getSymbol(curSymbol, num, name);
		if(curSymbol != SYM_COLON)
		{
			readToLineEnd();
			return;
		}
		getSymbol(curSymbol, num, name);
		if(curSymbol != SYM_NUM)
		{
			readToLineEnd();
			return;
		}
		else
		{
			actualNum = num;
		}
		getSymbol(curSymbol, num, name);
		setNumber(actualName, actualNum);
	}
	else
	{
		// This is a bool.
		getSymbol(curSymbol, num, name);
		if(curSymbol != SYM_COLON)
		{
			readToLineEnd();
			return;
		}
		getSymbol(curSymbol, num, name);
		if(curSymbol != SYM_NUM || !(num == 0 || num == 1))
		{
			readToLineEnd();
			return;
		}
		else
		{
			actualBool = !!num;
		}
		getSymbol(curSymbol, num, name);
		setBool(actualName, actualBool);
	}
	
}

void Config::readToLineEnd()
{
	unsigned int num;
	configName name;
	do
	{
		getSymbol(curSymbol, num, name);
	}
	while(curSymbol != SYM_SEMICOL && curSymbol != SYM_EOF);
}

void Config::getSymbol(Symbol & s, unsigned int & num, configName & name)
{
  	if (!eofile)
    {
  		skipSpaces();
  
	  	if(eofile)
	    {
	      	s = SYM_EOF;
	    }
	  	else if(isdigit(curch))
	    { 
	      	getNumber(num);
	      	s = SYM_NUM;
	    }
	  	else if(isalpha(curch))
	    {
	    	getName(name);
	      	s = SYM_NAME;
	    }
	  	else
	    {
	      	switch (curch) 
			{
				case ',': 
					s = SYM_COMMA; 
					break;
				case ':': 
					s = SYM_COLON; 
					break;
				case ';': 
					s = SYM_SEMICOL; 
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

// Continues getting characters from the definition file until non-space found
void Config::skipSpaces()
{
  	while (!eofile && isspace(curch))
    {
      	getChar();
    }
}

// Gets the number from definition file until non-digit character found
void Config::getNumber(unsigned int &num)
{
  	num = 0;
  	while((!eofile) && isdigit(curch))
    { 
      	num = 10 * num + atoi(&curch);
      	getChar();
    }
}

// Gets the name from the definition file until non-alpha or non-digit character found
void Config::getName(configName &name)
{
  	std::string tempName;
  
  	while((!eofile) && (isalpha(curch) || isdigit(curch)))
    {
      	tempName += tolower(curch);
      	getChar();
    }

  	name = snames.find(tempName)->second;
}

// Gets the next character from the definition file
// If character is '\n' (end of line), sets eoline to true to keep current character position correct
void Config::getChar()
{
	eofile = configFile.get(curch).eof();
}

void Config::save()
{
	std::ofstream oFile;
	oFile.open("mips.conf");
	
	if(oFile)
	{
		// Write each colour.
		for(auto i = colours.begin(); i != colours.end(); ++i)
		{
			configName name = (*i).first;
			wxColour col = (*i).second;
			
			if(col.IsOk())
			{
				oFile << names[name] <<  ": " << (unsigned int)col.Red();
				oFile << ", " << (unsigned int)col.Green();
				oFile << ", " << (unsigned int)col.Blue();
				oFile << ", " << (unsigned int)col.Alpha() << ";" << std::endl;
			}
		}
		// Write each bool.
		for(auto i = bools.begin(); i != bools.end(); ++i)
		{
			configName name = (*i).first;
			bool val = (*i).second;
			
			oFile << names[name] << ": " << val << ";" << std::endl;
		}
		// Write each number.
		for(auto i = numbers.begin(); i != numbers.end(); ++i)
		{
			configName name = (*i).first;
			unsigned int val = (*i).second;
			
			oFile << names[name] << ": " << val << ";" << std::endl;
		}
	}
	else
	{
		// Cannot open file so give up.
	}
	
	oFile.close();
}

wxColour Config::getColour(configName name)
{
	return colours[name];
}

bool Config::getBool(configName name)
{
	return bools[name];
}

unsigned int Config::getNumber(configName name)
{
	return numbers[name];
}

void Config::setColour(configName name, wxColour col)
{
	colours[name] = col;
}

void Config::setBool(configName name, bool newBool)
{
	bools[name] = newBool;	
}

void Config::setNumber(configName name, unsigned int newNum)
{
	numbers[name] = newNum;
}
