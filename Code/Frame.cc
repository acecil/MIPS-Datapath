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

#include "GLCanvas.h"
#include "Dialog.h"
#include "Datalist.h"
#include "Model.h"
#include "Maths.h"
#include "Config.h"
#include "Icons.h"
#include "Component.h"

#include "Frame.h"

BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_SPLITTER_SASH_POS_CHANGED(ID_SPLITTER_WINDOW, Frame::OnSplitterSizeChanged)
	EVT_SPLITTER_SASH_POS_CHANGED(ID_EDSPLITTER_WINDOW, Frame::OnSplitterSizeChanged)
	EVT_BUTTON(wxID_FORWARD, Frame::StepButtonClicked)
    EVT_BUTTON(wxID_STOP, Frame::ResetButtonClicked)
    EVT_MENU(ID_MENU_LAYOUT_SIMPLE, Frame::SetSimpleLayout)
    EVT_MENU(ID_MENU_LAYOUT_PIPELINE, Frame::SetPipelineLayout)
    EVT_MENU(ID_MENU_LAYOUT_FORWARDING, Frame::SetForwardingLayout)
    EVT_MENU(ID_MENU_FORMAT_BIN, Frame::SetFormatBin)
    EVT_MENU(ID_MENU_FORMAT_DEC, Frame::SetFormatDec)
    EVT_MENU(ID_MENU_FORMAT_HEX, Frame::SetFormatHex)
    EVT_MENU(ID_MENU_EDIT_OPTIONS, Frame::EditOptions)
    EVT_MENU(ID_MENU_FILE_LOAD, Frame::LoadFile)
    EVT_MENU(ID_MENU_HELP_CONTENTS, Frame::ShowContents)
    EVT_MENU(ID_MENU_HELP_INDEX, Frame::ShowIndex)
    EVT_MENU(ID_MENU_HELP_ABOUT, Frame::ShowAbout)
    EVT_BUTTON(ID_LOAD_INSTRUCTIONS, Frame::LoadFile)
    EVT_BUTTON(ID_SAVE_INSTRUCTIONS, Frame::SaveFile)
    EVT_BUTTON(ID_PARSE_INSTRUCTIONS, Frame::Parse)
    EVT_GRID_CMD_CELL_CHANGE(ID_MEMORY_LIST, Frame::MemoryGridChanged)
    EVT_LIST_ITEM_SELECTED(ID_INSTRUCTION_LIST, Frame::OnSelectInstruction)
	EVT_SLIDER(ID_ZOOM_SLIDER, Frame::OnZoomSliderChanged)
END_EVENT_TABLE()

Frame::Frame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
	init = false;
	initSize = false;
	setupMenubar();
	
	Config& c = Config::Instance();
	c.save();
	
	wxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	
	setupSimulatorPage(notebook);
	setupEditorPage(notebook);
	
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 0);
    mainSizer->SetMinSize(wxSize(400, 200));
    SetSizer(mainSizer);
    
	if(c.getBool(SHOW_SIMPLE_LAYOUT))
	{
		processor->resetup(LAYOUT_SIMPLE);
	}
	else if(c.getBool(SHOW_PIPELINE_LAYOUT))
	{
		processor->resetup(LAYOUT_PIPELINE);
	}
	else if(c.getBool(SHOW_FORWARDING_LAYOUT))
	{
		processor->resetup(LAYOUT_FORWARDING);
	}
	
	if(c.getBool(SHOW_FORMAT_BIN))
	{
		Maths::setFormat(FORMAT_BINARY);
	}
	else if(c.getBool(SHOW_FORMAT_DEC))
	{
		Maths::setFormat(FORMAT_DECIMAL);
	}
	else if(c.getBool(SHOW_FORMAT_HEX))
	{
		Maths::setFormat(FORMAT_HEX);
	}

	processor->setBool(SHOW_INSTRUCTION_FIELDS, c.getBool(SHOW_INSTRUCTION_FIELDS));
	processor->setBool(SHOW_CONTROL_LINES, c.getBool(SHOW_CONTROL_LINES));
	processor->setBool(SHOW_PC_LINES, c.getBool(SHOW_PC_LINES));
	processor->setBool(SHOW_POPUPS, c.getBool(SHOW_POPUPS));
	Component::setAreDataLinesBold(c.getBool(SHOW_BOLD_DATA_LINES));
	Component::setHighlightSingleInstruction(c.getBool(HIGHLIGHT_SINGLE_INSTRUCTION));
	
	// Initialise help
	help = std::make_unique<wxHtmlHelpController>(wxHF_CONTENTS | wxHF_SEARCH, this);
	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	help->AddBook(wxFileName(_T("MIPS-Datapath.htb")), true);
	
	// Initialise image handlers
	wxInitAllImageHandlers();
	
	// Import icons
	ico = std::make_unique<wxIconBundle>();
	ico->AddIcon(wxIcon(Icons::Cog_2_48_n_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_48_h_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_32_n_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_32_h_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_24_n_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_24_h_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_16_n_p_xpm));
	ico->AddIcon(wxIcon(Icons::Cog_2_16_h_p_xpm));
	SetIcons(*ico);
	
	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_CTRL, (int)'B', ID_MENU_FORMAT_BIN);
	entries[1].Set(wxACCEL_CTRL, (int)'D', ID_MENU_FORMAT_DEC);
	entries[2].Set(wxACCEL_CTRL, (int)'H', ID_MENU_FORMAT_HEX);
	entries[3].Set(wxACCEL_SHIFT, (int)'A', ID_MENU_HELP_ABOUT);
	
	wxAcceleratorTable accel(3, entries);
	SetAcceleratorTable(accel);
	
	this->SetMinSize(wxSize(640, 480));
	this->Maximize(true);
}

void Frame::finishInit()
{ 
	init = true; 
	Config& c = Config::Instance();
	if(!c.getBool(SHOW_LEFT_PANEL) && mainSplitter->IsSplit())
	{
		mainSplitter->Unsplit(leftPanel);
	}
	updateDataList(true, false);
	resetLeftPanelSizes();
};

void Frame::setupSimulatorPage(wxNotebook *notebook)
{
	wxPanel *simulatorPage = new wxPanel(notebook);
	
	mainSplitter = new wxSplitterWindow(simulatorPage, ID_SPLITTER_WINDOW);
	GLSizer = new wxGridSizer(1, 1, wxSize());
	wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
    
    // Set up Left panel.
    Config &c = Config::Instance();
    leftPanel = new wxPanel(mainSplitter, wxID_ANY);
    leftPanel->SetMinSize(wxSize(LEFT_PANEL_MIN_WIDTH, LEFT_PANEL_MIN_HEIGHT));
    leftPanel->SetSizer(leftSizer);
    leftSizer->Add(new wxButton(leftPanel, wxID_FORWARD, _T("&Step")), 0, wxEXPAND | wxALL, 0);
    leftSizer->Add(new wxButton(leftPanel, wxID_STOP, _T("&Reset")), 0, wxEXPAND | wxALL, 0);
    wxNotebook *dataBook = new wxNotebook(leftPanel, wxID_ANY);
    leftSizer->Add(dataBook, 1, wxEXPAND | wxALL, 3);
    wxPanel *instructionPage = new wxPanel(dataBook);
    wxPanel *registerPage = new wxPanel(dataBook);
    wxPanel *dataPage = new wxPanel(dataBook);
    dataBook->AddPage(instructionPage, _T("&Instructions"), true);
    dataBook->AddPage(registerPage, _T("&Registers"), false);
    dataBook->AddPage(dataPage, _T("&Data"), false);
    
    dataList[ID_INSTRUCTION_LIST] = new Datalist(4, Model::MAX_INSTRUCTIONS, _T("Instr."), instructionPage, ID_INSTRUCTION_LIST);
    dataList[ID_REGISTER_LIST] = new Datalist(1, Model::MAX_REGISTERS, _T("Addr."), registerPage, ID_REGISTER_LIST);
    dataList[ID_DATA_LIST] = new Datalist(1, Model::MAX_DATA, _T("Addr."), dataPage, ID_DATA_LIST);
    
    wxSizer *instructionSizer = new wxBoxSizer(wxVERTICAL);
    wxSizer *registerSizer = new wxBoxSizer(wxVERTICAL);
    wxSizer *dataSizer = new wxBoxSizer(wxVERTICAL);
    instructionSizer->Add(dataList[ID_INSTRUCTION_LIST], 1, wxEXPAND | wxALL, 3);
    registerSizer->Add(dataList[ID_REGISTER_LIST], 1, wxEXPAND | wxALL, 3);
    dataSizer->Add(dataList[ID_DATA_LIST], 1, wxEXPAND | wxALL, 3);
    instructionPage->SetSizer(instructionSizer);
    registerPage->SetSizer(registerSizer);
    dataPage->SetSizer(dataSizer);

	// Add a slider for control the zoom of the simulator.
	leftSizer->Add(new wxSlider(leftPanel, ID_ZOOM_SLIDER, 0, -100, 100), 0, wxEXPAND | wxALL, 0);
        
    // Set up GL canvas.
	int attribList[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, wxFULL_REPAINT_ON_RESIZE};
    processor = std::make_unique<Model>();
    processor->resetup();
    canvas = new GLCanvas(processor.get(), mainSplitter, this, wxID_ANY, wxDefaultPosition, 0, _T("GLCanvas"), attribList, wxNullPalette);
   	GLSizer->Add(canvas, 1, wxEXPAND | wxALL, 0);
   
   	mainSplitter->SetMinimumPaneSize(20);
    mainSplitter->SplitVertically(leftPanel, canvas, c.getNumber(SIMULATOR_SASH_POS));
 
   	wxBoxSizer *simulatorSizer = new wxBoxSizer(wxHORIZONTAL);
   	simulatorSizer->Add(mainSplitter, 1, wxEXPAND | wxALL, 0);
   
   	simulatorPage->SetSizer(simulatorSizer);	
	
	notebook->AddPage(simulatorPage, _T("&Simulator"), true);

}

void Frame::setupEditorPage(wxNotebook *notebook)
{
	wxPanel *editorPage = new wxPanel(notebook);
	
	editorSplitter = new wxSplitterWindow(editorPage, ID_EDSPLITTER_WINDOW);
	
	wxSizer *editorSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *parserSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *memorySizer = new wxBoxSizer(wxVERTICAL);
	editorSizer->Add(editorSplitter, 1, wxEXPAND | wxALL, 3);
	editorSplitter->SetMinimumPaneSize(20);
	Config& c = Config::Instance();
	memoryPane = new wxPanel(editorSplitter, wxID_ANY, wxDefaultPosition, wxSize(c.getNumber(EDITOR_SASH_POS), 300));
	memoryPane->SetSizer(memorySizer);
	memoryPane->SetMinSize(wxSize(200, 300));
	wxPanel* parserPane = new wxPanel(editorSplitter, wxID_ANY, wxDefaultPosition, wxSize(200, 300));
	parserPane->SetSizer(parserSizer);
	parserPane->SetMinSize(wxSize(200, 300));
	editorSplitter->SplitVertically(memoryPane, parserPane, LEFT_PANEL_DEFAULT_SIZE);

	// Parser Section
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(new wxButton(parserPane, ID_LOAD_INSTRUCTIONS, _T("&Load")), 1, wxEXPAND | wxALL, 0);
    buttonSizer->Add(new wxButton(parserPane, ID_SAVE_INSTRUCTIONS, _T("&Save")), 1, wxEXPAND | wxALL, 0);
    buttonSizer->Add(new wxButton(parserPane, ID_PARSE_INSTRUCTIONS, _T("&Parse")), 1, wxEXPAND | wxALL, 0);
    
	// Require wxTE_RICH style for editor text control to allow styling under Windows.
	editorText = new wxTextCtrl(parserPane, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH);
	errorText = new wxTextCtrl(parserPane, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	
	parserSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 3);
	parserSizer->Add(new wxStaticText(parserPane, wxID_ANY, _T("Instructions:")), 0, wxEXPAND | wxALL, 3);
	parserSizer->Add(editorText, 2, wxEXPAND | wxALL, 3);
	parserSizer->Add(new wxStaticText(parserPane, wxID_ANY, _T("Messages:")), 0, wxEXPAND | wxALL, 3);
	parserSizer->Add(errorText, 1, wxEXPAND | wxALL, 3);

	// Memory Editor section
	memoryList = new wxGrid(memoryPane, ID_MEMORY_LIST, wxDefaultPosition, wxDefaultSize);
	memoryList->CreateGrid(Model::MAX_INSTRUCTIONS, 1);
	memoryList->SetColLabelValue(0, _T("Data"));
	memoryList->SetColFormatNumber(0);
	memoryList->SetColSize(0, 240);
	memoryList->SetRowLabelSize(43);
	for(uint i = 0; i < Model::MAX_DATA; i++)
	{
		memoryList->SetCellValue(i, 0, _T("0"));
		memoryList->SetRowLabelValue(i, wxString::Format(_T("%d"), i));
	}
	memoryList->DisableDragGridSize();
	memoryList->DisableDragColSize();
	memoryList->DisableDragRowSize();
	
	
	memorySizer->Add(new wxStaticText(memoryPane, wxID_ANY, _T("Initial Memory Data:")), 0, wxEXPAND | wxALL, 3);
	memorySizer->Add(memoryList, 2, wxEXPAND | wxALL, 3);
	
	// Final Setup
	editorPage->SetSizer(editorSizer);
	
	notebook->AddPage(editorPage, _T("&Editor"), false);
}

void Frame::setupMenubar()
{
	// Menu Bar
	wxMenuBar *menubar = new wxMenuBar();
	this->SetMenuBar(menubar);
	
	// File Menu
	wxMenu *fileMenu = new wxMenu();
	menubar->Append(fileMenu, _T("&File"));
	fileMenu->Append(ID_MENU_FILE_LOAD, _T("&Load Instructions..."));
	
	// Layout Menu
	wxMenu *layoutMenu = new wxMenu();
	menubar->Append(layoutMenu, _T("&Layout"));
	#ifdef wxHAS_RADIO_MENU_ITEMS
	layoutMenu->AppendRadioItem(ID_MENU_LAYOUT_SIMPLE, _T("&Simple"));
	layoutMenu->AppendRadioItem(ID_MENU_LAYOUT_PIPELINE, _T("&Pipelined"));
	layoutMenu->AppendRadioItem(ID_MENU_LAYOUT_FORWARDING, _T("&Data Forwarding"));
	Config& c = Config::Instance();
	if(c.getBool(SHOW_SIMPLE_LAYOUT))
	{
		layoutMenu->Check(ID_MENU_LAYOUT_SIMPLE, true);
	}
	else if(c.getBool(SHOW_PIPELINE_LAYOUT))
	{
		layoutMenu->Check(ID_MENU_LAYOUT_PIPELINE, true);
	}
	else if(c.getBool(SHOW_FORWARDING_LAYOUT))
	{
		layoutMenu->Check(ID_MENU_LAYOUT_FORWARDING, true);
	}
	
	#else
	layoutMenu->Append(ID_MENU_LAYOUT_SIMPLE, _T("&Simple"));
	layoutMenu->Append(ID_MENU_LAYOUT_PIPELINE, _T("&Pipelined"));
	layoutMenu->Append(ID_MENU_LAYOUT_FORWARDING, _T("&Data Forwarding"));
	#endif

	// Number Format Menu
	wxMenu *NumFormatMenu = new wxMenu();
	menubar->Append(NumFormatMenu, _T("&Format"));
	#ifdef wxHAS_RADIO_MENU_ITEMS
	NumFormatMenu->AppendRadioItem(ID_MENU_FORMAT_BIN, _T("&Binary"));
	NumFormatMenu->AppendRadioItem(ID_MENU_FORMAT_DEC, _T("&Decimal"));
	NumFormatMenu->AppendRadioItem(ID_MENU_FORMAT_HEX, _T("&Hexadecimal"));
	if(c.getBool(SHOW_FORMAT_BIN))
	{
		NumFormatMenu->Check(ID_MENU_FORMAT_BIN, true);
	}
	else if(c.getBool(SHOW_FORMAT_DEC))
	{
		NumFormatMenu->Check(ID_MENU_FORMAT_DEC, true);
	}
	else if(c.getBool(SHOW_FORMAT_HEX))
	{
		NumFormatMenu->Check(ID_MENU_FORMAT_HEX, true);
	}

	#else
	NumFormatMenu->Append(ID_MENU_FORMAT_BIN, _T("&Binary"));
	NumFormatMenu->Append(ID_MENU_FORMAT_DEC, _T("&Decimal"));
	NumFormatMenu->Append(ID_MENU_FORMAT_HEX, _T("&Hexadecimal"));
	#endif
	
	// Edit Menu
	wxMenu *EditMenu = new wxMenu();
	menubar->Append(EditMenu, _T("&Edit"));
	EditMenu->Append(ID_MENU_EDIT_OPTIONS, _T("&Options..."));
	
	// Help Menu
	wxMenu *helpMenu = new wxMenu();
	menubar->Append(helpMenu, _T("&Help"));
	helpMenu->Append(ID_MENU_HELP_CONTENTS, _T("&Contents..."));
	helpMenu->AppendSeparator();
	helpMenu->Append(ID_MENU_HELP_ABOUT, _T("&About..."));
}

Frame::~Frame()
{
}

void Frame::updateEditorText()
{
	editorText->Clear();
	wxString instr;
	processor->getFileContents(instr);
	editorText->AppendText(instr);
	editorText->SetInsertionPoint(0);
}

void Frame::updateEditorTextFromStore()
{
	editorText->Clear();
	for(uint i = 0; i < Model::MAX_INSTRUCTIONS; i+=4)
	{
		wxString instr;
		bool err;
		processor->getInstructionString(i, instr, err);
		wxTextAttr textAttr1 = wxTextAttr(wxColour(255, 0, 0));
		editorText->AppendText(instr);
		if(err)
		{
			int lastPos = editorText->GetLastPosition();
			editorText->SetStyle(lastPos - instr.Len(), lastPos, textAttr1);
		}
	}
	editorText->SetInsertionPoint(0);
}

void Frame::updateErrorText()
{
	errorText->Clear();
	int numErrors = processor->getNumberOfErrors();
	
	if(numErrors == 0)
	{
		errorText->AppendText(_T("Instructions successfully parsed. Your program is now ready to run."));
	}
	else if(numErrors == 1)
	{
		errorText->AppendText(_T("There was 1 error when parsing the instructions.\n"));
		errorText->AppendText(_T("These errors are listed below and highlighted above.\n\n"));
	}
	else if(numErrors > 1)
	{
		errorText->AppendText(_T("There were ") + wxString::Format(_T("%d"), numErrors) + _T(" errors when parsing the instructions.\n"));
		errorText->AppendText(_T("These errors are listed below and highlighted above.\n\n"));
	}
	
	wxString err;
	for(int i = 0; i < 500; i++)
	{
		processor->getError(i, err);
		if(err != _T(""))
		{
			errorText->AppendText(err + _T("\n"));
		}
		else
		{
			break;
		}
	}
}

void Frame::updateDataList(bool initialCall, bool selectInstruction)
{
	for(const auto& dl: dataList)
	{
		uint cMem = dl.first;
		Datalist* list = dl.second;

		// For complete redraw, completely empty datalist then refill with columns.
		if(initialCall)
		{
			list->DeleteAllItems();
			while(list->GetColumnCount() > 0)
			{
				list->DeleteColumn(0);
			}
			list->InsertFirstColumn();
			list->InsertColumn(1, _T("Data"));
		}
		
		uint maxVals = list->getMaxVals();
		uint multiplier = list->getMultiplier();
		wxString val;
		for(uint i = 0; i < maxVals; i++)
		{
			// Insert items for complete redraw.
			if(initialCall)
			{
				list->InsertItem(i, wxString::Format(_T("%d"), i * multiplier));
				
				// Keep selected item between removing and reinserting all the rows. 
				if(list->getSelectedIndex() == i * multiplier)
				{
					list->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
				}
			}
			
			// Get raw data for data list.
			if(cMem != ID_INSTRUCTION_LIST)
			{
				val = Maths::convertToBase(processor->getMemoryData(cMem, i * multiplier));
			}
			else
			{
				val = Maths::convertToBase(processor->getMemoryData(cMem, i * multiplier), true);
			}
			
			// Get fielded instructions if we're getting instructions and we want fields.
			if(cMem == ID_INSTRUCTION_LIST && initialCall && processor->getBool(SHOW_INSTRUCTION_FIELDS))
			{
				processor->getFieldedInstruction(i * multiplier, val);
			}

			// Update item unless it is instructions & not a complete redraw.
			if(!(cMem == ID_INSTRUCTION_LIST && !initialCall))
			{
				list->SetItem(i, 1, val);
			}
		}
	}
	updateDataListHighlighting();
}

void Frame::updateDataListHighlighting()
{
	processor->setHighlightInstruction(dataList[ID_INSTRUCTION_LIST]->getSelectedIndex(), dataList[ID_INSTRUCTION_LIST]->isSelectedIndexValid());
	uint curPos = 0;
	uint multiplier = dataList[ID_INSTRUCTION_LIST]->getMultiplier();
	uint maxVals = dataList[ID_INSTRUCTION_LIST]->getMaxVals();
	wxColour col;
	for(uint i = 0; i < maxVals; i++)
	{
		uint lineLength = editorText->GetLineLength(i);
		col = processor->getColourForInstruction(i * multiplier);
		// Set background colour for current instruction to red in left panel.
		dataList[ID_INSTRUCTION_LIST]->SetItemBackgroundColour(i, col);
		wxTextAttr textAttr1 = wxTextAttr(wxNullColour, col);
		textAttr1.SetFlags(wxTEXT_ATTR_BACKGROUND_COLOUR);
		editorText->SetStyle(curPos, curPos + lineLength, textAttr1);
		curPos += lineLength + 1;
	}
}

void Frame::setInitialMemoryVals(bool fromParser)
{
	if(fromParser)
	{
		for(uint i = 0; i < Model::MAX_INSTRUCTIONS; i++)
		{
			processor->setMemoryData(2, i, processor->getParserMemData(i));
			memoryList->SetCellValue(i, 0, Maths::convertToBase(processor->getParserMemData(i), false, false, FORMAT_DECIMAL));
		}
	}
	luint numVal;
	bool minus;
	long unsigned int tempNum;
	for(uint i = 0; i < Model::MAX_INSTRUCTIONS; i++)
	{
		// Stuff to make this work properly with correct limits on 32 bit systems.
		wxString val = memoryList->GetCellValue(i, 0);
		int numLen = val.Len();
		numVal = 0;
		minus = false;
		for(int j = 0; j < numLen; j++)
		{
			uint pos = numLen - j - 1;
			wxString chara = val[pos];
			if(pos == 0 && chara == _T('-'))
			{
				minus = true;
			}
			chara.ToULong(&tempNum);
			numVal += (uint)(pow(10.0, j)) * tempNum;
		}
		bool numValSizeOk = (0 <= numVal && numVal <= Model::MAX_MEM_VAL / 2);
		if(minus)
		{
			// convert number to 2's complement negative number.
			numVal = ((luint)(1)<<32) - numVal;
		}
		if(numVal && val.ToULong(&tempNum) && numValSizeOk)
		{
			processor->setMemoryData(2, i, numVal);
			processor->setParserMemData(i, numVal);
		}
		else
		{
			processor->setMemoryData(2, i, 0);
			processor->setParserMemData(i, 0);
			memoryList->SetCellValue(i, 0, _T("0"));
		}
	}
}

void Frame::MemoryGridChanged(wxGridEvent& WXUNUSED(event))
{
	setInitialMemoryVals();
	updateDataList();
}

void Frame::StepButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	processor->step();
	updateDataList();
	canvas->Render();
}

void Frame::ResetButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	processor->resetup();
	setInitialMemoryVals();
	processor->getParsedInstructions();
	updateDataList();
	canvas->Render();
}

void Frame::SetSimpleLayout(wxCommandEvent& WXUNUSED(event))
{
	processor->resetup(LAYOUT_SIMPLE);
	Config& c = Config::Instance();
	c.setBool(SHOW_SIMPLE_LAYOUT, true);
	c.setBool(SHOW_PIPELINE_LAYOUT, false);
	c.setBool(SHOW_FORWARDING_LAYOUT, false);
	c.save();
	setInitialMemoryVals();
	canvas->Render();
	updateDataList();
}

void Frame::SetPipelineLayout(wxCommandEvent& WXUNUSED(event))
{
	processor->resetup(LAYOUT_PIPELINE);
	Config& c = Config::Instance();
	c.setBool(SHOW_SIMPLE_LAYOUT, false);
	c.setBool(SHOW_PIPELINE_LAYOUT, true);
	c.setBool(SHOW_FORWARDING_LAYOUT, false);
	c.save();
	setInitialMemoryVals();
	canvas->Render();
	updateDataList();
}

void Frame::SetForwardingLayout(wxCommandEvent& WXUNUSED(event))
{
	processor->resetup(LAYOUT_FORWARDING);
	Config& c = Config::Instance();
	c.setBool(SHOW_SIMPLE_LAYOUT, false);
	c.setBool(SHOW_PIPELINE_LAYOUT, false);
	c.setBool(SHOW_FORWARDING_LAYOUT, true);
	c.save();
	setInitialMemoryVals();
	canvas->Render();
	updateDataList();
}

void Frame::SetFormatBin(wxCommandEvent& WXUNUSED(event))
{
	Maths::setFormat(FORMAT_BINARY);
	Config& c = Config::Instance();
	c.setBool(SHOW_FORMAT_BIN, true);
	c.setBool(SHOW_FORMAT_DEC, false);
	c.setBool(SHOW_FORMAT_HEX, false);
	c.save();
	updateDataList(true);
	resetLeftPanelSizes();
}

void Frame::SetFormatDec(wxCommandEvent& WXUNUSED(event))
{
	Maths::setFormat(FORMAT_DECIMAL);
	Config& c = Config::Instance();
	c.setBool(SHOW_FORMAT_BIN, false);
	c.setBool(SHOW_FORMAT_DEC, true);
	c.setBool(SHOW_FORMAT_HEX, false);
	c.save();
	updateDataList(true);
	resetLeftPanelSizes();
}

void Frame::SetFormatHex(wxCommandEvent& WXUNUSED(event))
{
	Maths::setFormat(FORMAT_HEX);
	Config& c = Config::Instance();
	c.setBool(SHOW_FORMAT_BIN, false);
	c.setBool(SHOW_FORMAT_DEC, false);
	c.setBool(SHOW_FORMAT_HEX, true);
	c.save();
	updateDataList(true);
	resetLeftPanelSizes();
}

void Frame::showHideLeftPanel(bool showLeftPanel, bool justSwitch)
{
	Config &c = Config::Instance();
	if(mainSplitter->IsSplit() && (!showLeftPanel || justSwitch))
	{
		mainSplitter->Unsplit(leftPanel);
		c.setBool(SHOW_LEFT_PANEL, false); 
	}
	else if(!mainSplitter->IsSplit() && (showLeftPanel || justSwitch))
	{
		mainSplitter->SplitVertically(leftPanel, canvas, c.getNumber(SIMULATOR_SASH_POS));
		c.setBool(SHOW_LEFT_PANEL, true); 
	}
	resetLeftPanelSizes();
	c.save();
}

void Frame::EditOptions(wxCommandEvent& WXUNUSED(event))
{
	Dialog *dlg = new Dialog(this, wxID_ANY, ico.get(), processor.get(), _T("Options"), mainSplitter->IsSplit(), processor->getBool(SHOW_INSTRUCTION_FIELDS));
	Config &c = Config::Instance();
		
	if(dlg->ShowModal() == wxID_OK)
	{
		showHideLeftPanel(dlg->getBool(SHOW_LEFT_PANEL));
		processor->setBool(SHOW_INSTRUCTION_FIELDS, dlg->getBool(SHOW_INSTRUCTION_FIELDS));
		c.setBool(SHOW_INSTRUCTION_FIELDS, dlg->getBool(SHOW_INSTRUCTION_FIELDS));
		processor->setBool(SHOW_CONTROL_LINES, dlg->getBool(SHOW_CONTROL_LINES));
		c.setBool(SHOW_CONTROL_LINES, dlg->getBool(SHOW_CONTROL_LINES));
		processor->setBool(SHOW_PC_LINES, dlg->getBool(SHOW_PC_LINES));
		c.setBool(SHOW_PC_LINES, dlg->getBool(SHOW_PC_LINES));
		processor->setBool(SHOW_POPUPS, dlg->getBool(SHOW_POPUPS));
		c.setBool(SHOW_POPUPS, dlg->getBool(SHOW_POPUPS));
		Component::setAreDataLinesBold(dlg->getBool(SHOW_BOLD_DATA_LINES));
		c.setBool(SHOW_BOLD_DATA_LINES, dlg->getBool(SHOW_BOLD_DATA_LINES));
		Component::setHighlightSingleInstruction(dlg->getBool(HIGHLIGHT_SINGLE_INSTRUCTION));
		c.setBool(HIGHLIGHT_SINGLE_INSTRUCTION, dlg->getBool(HIGHLIGHT_SINGLE_INSTRUCTION));

		for(uint n = CONFIG_MIN_NAME; n < CONFIG_MAX_NAME; ++n)
		{
			if(c.isNameAColour((configName)n))
			{
				c.setColour((configName)n, dlg->getColour((configName)n));
				Component::setColour((configName)n, dlg->getColour((configName)n));
			}
		}
		processor->resetColours();
		
		canvas->Render();
		updateDataList(true);
		resetLeftPanelSizes();
	}
	
	dlg->Destroy();
	c.save();
}

void Frame::LoadFile(wxCommandEvent& event)
{
	wxFileDialog openDlg(this, _T("Load Instruction File"), _T(""), _T(""), _T("Definition files (*.def)|*.def"), wxFD_OPEN);
	if(openDlg.ShowModal() == wxID_OK)
	{
		if(openDlg.GetPath().c_str() != _T(""))
		{
			processor->loadFile(openDlg.GetPath());
			processor->parse(_T(""));
			processor->resetup();
			setInitialMemoryVals(true);
			canvas->Render();
			updateDataList(true, true);
			updateEditorTextFromStore();
			updateErrorText();
			resetLeftPanelSizes();
			Parse(event);
		}
	}
}

void Frame::SaveFile(wxCommandEvent& event)
{
	Parse(event);
	wxFileDialog saveDlg(this, _T("Save Instruction File"), _T(""), _T(""), _T("Definition files (*.def)|*.def"), wxFD_SAVE);
	if(saveDlg.ShowModal() == wxID_OK)
	{
		wxString path = saveDlg.GetPath();
		if(path.c_str() != _T(""))
		{
			if(path.Right(4) != _T(".def"))
			{
				path += _T(".def");
			}
			// Check to see if the file exists. If it does, warn about overwriting it.
			std::ifstream checkfile(path.fn_str());
			checkfile.close();
			if(!checkfile.fail())
			{
				wxMessageDialog overwrite(this, saveDlg.GetFilename() + _T(" already exists.\nDo you want to replace it?"), _T("Warning"), wxYES_NO | wxICON_EXCLAMATION);
				if(overwrite.ShowModal() == wxID_YES)
				{
					processor->saveFile(path);
				}
			}
			else
			{
				processor->saveFile(path);
			}
		}
	}
}

void Frame::ShowContents(wxCommandEvent& WXUNUSED(event))
{
	help->DisplayContents();
}

void Frame::ShowIndex(wxCommandEvent& WXUNUSED(event))
{
	help->DisplayIndex();
}

void Frame::ShowAbout(wxCommandEvent& WXUNUSED(event))
{
	wxAboutDialogInfo info;
	info.SetName(_T("MIPS-Datapath"));
	info.SetDescription(_T("Graphical MIPS Datapath Simulator."));
	info.AddDeveloper(_T("Andrew Gascoyne-Cecil"));
	info.SetIcon(wxIcon(Icons::Cog_2_48_n_p_xpm));
	wxAboutBox(info);
}

void Frame::Parse(wxCommandEvent& WXUNUSED(event))
{
	wxString str = editorText->GetValue();
	processor->parse(str);
	processor->resetup();
	setInitialMemoryVals();
	canvas->Render();
	updateDataList(true, true);
	updateEditorTextFromStore();
	updateErrorText();
	resetLeftPanelSizes();
}

void Frame::resetLeftPanelSizes()
{
	Config& c = Config::Instance();
	if(!init || !(c.getBool(SHOW_LEFT_PANEL)))
	{
		return;
	}

	uint sashPosition = mainSplitter->GetSashPosition();
	uint edSashPosition = editorSplitter->GetSashPosition();
	if(!initSize)
	{
		sashPosition = c.getNumber(SIMULATOR_SASH_POS);
		edSashPosition = c.getNumber(EDITOR_SASH_POS);
		mainSplitter->SetSashPosition(sashPosition);
		editorSplitter->SetSashPosition(edSashPosition);
	}
	else
	{
		c.setNumber(EDITOR_SASH_POS, edSashPosition);
		c.setNumber(SIMULATOR_SASH_POS, sashPosition);
		c.save();
	}

	uint newColWidth = 0;
	for(uint cMem = 0; cMem < 3; cMem++)
	{
		dataList[cMem]->SetColumnWidth(0, 45);
		newColWidth = sashPosition -  dataList[cMem]->GetColumnWidth(0) - 30;	
		dataList[cMem]->SetColumnWidth(1, newColWidth);
	}
	newColWidth = edSashPosition - memoryList->GetRowLabelSize() - 50;
	memoryList->SetColSize(0, newColWidth);
	memoryList->ForceRefresh();
	initSize = true;
}

void Frame::OnSplitterSizeChanged(wxSplitterEvent& WXUNUSED(event))
{
	resetLeftPanelSizes();
}

void Frame::OnSelectInstruction(wxListEvent& WXUNUSED(event))
{
	processor->setHighlightInstruction(dataList[ID_INSTRUCTION_LIST]->getSelectedIndex(), dataList[ID_INSTRUCTION_LIST]->isSelectedIndexValid());
	canvas->Render();
}

void Frame::OnZoomSliderChanged(wxCommandEvent& event)
{
	canvas->SetZoom(-event.GetInt());
}