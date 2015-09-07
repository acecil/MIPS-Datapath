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
 
#include "Types.h"

#include "Dialog.h"

BEGIN_EVENT_TABLE(Dialog, wxDialog)
    EVT_BUTTON(COMPONENT_COLOUR, Dialog::EditColour)
    EVT_BUTTON(PIPEREG_COLOUR, Dialog::EditColour)
    EVT_BUTTON(TEXT_COLOUR, Dialog::EditColour)
    EVT_BUTTON(FADE_COLOUR, Dialog::EditColour)
    EVT_BUTTON(BORDER_COLOUR, Dialog::EditColour)
    EVT_BUTTON(BACKGROUND_COLOUR, Dialog::EditColour)
    EVT_BUTTON(STAGE1_COLOUR, Dialog::EditColour)
    EVT_BUTTON(STAGE2_COLOUR, Dialog::EditColour)
    EVT_BUTTON(STAGE3_COLOUR, Dialog::EditColour)
    EVT_BUTTON(STAGE4_COLOUR, Dialog::EditColour)
    EVT_BUTTON(STAGE5_COLOUR, Dialog::EditColour)
    EVT_BUTTON(LINK_COLOUR, Dialog::EditColour)
END_EVENT_TABLE()

Dialog::Dialog(wxWindow* parent, wxWindowID id, wxIconBundle *ico, Model *proc, const wxString& title, 
				bool mainSplitterSplit, bool instructionsInFields, const wxPoint& pos, const wxSize& size, 
				long style, const wxString& name)
				:wxDialog(parent, id, title, pos, wxDefaultSize, style, name)
{
	// Setup options dialog.
	this->proc = proc;
	this->ico = ico;
	
	SetIcons(*ico);
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSizer);
	wxSizer *allColoursSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("&Colours"));
	mainSizer->Add(allColoursSizer, 0, wxEXPAND | wxALL, 0);
	wxFlexGridSizer *colourSizer = new wxFlexGridSizer(5, 6, 3, 3);
	colourSizer->SetFlexibleDirection(wxHORIZONTAL);
	colourSizer->AddGrowableCol(0,5);
	createColourButton(colourSizer, _T("&Component:"), COMPONENT_COLOUR);
	createColourButton(colourSizer, _T("&Inactive link:"), LINK_COLOUR);
	createColourButton(colourSizer, _T("&Stage 1:"), STAGE1_COLOUR);
	createColourButton(colourSizer, _T("&Pipeline register:"), PIPEREG_COLOUR);
	createColourButton(colourSizer, _T("&Text:"), TEXT_COLOUR);
	createColourButton(colourSizer, _T("&Stage 2:"), STAGE2_COLOUR);
	createColourButton(colourSizer, _T("&Component fade:"), FADE_COLOUR);
	createColourButton(colourSizer, _T("&Border:"), BORDER_COLOUR);
	createColourButton(colourSizer, _T("&Stage 3:"), STAGE3_COLOUR);
	colourSizer->Add(new wxStaticText(this, wxID_ANY, _T("")));
	colourSizer->Add(new wxStaticText(this, wxID_ANY, _T("")));
	createColourButton(colourSizer, _T("&Background:"), BACKGROUND_COLOUR);
	createColourButton(colourSizer, _T("&Stage 4:"), STAGE4_COLOUR);
	for(int i = 0; i < 4; i++)
	{
		colourSizer->Add(new wxStaticText(this, wxID_ANY, _T("")));
	}
	createColourButton(colourSizer, _T("&Stage 5:"), STAGE5_COLOUR);
	
	// Add all the colours to the colours sizer.
	allColoursSizer->Add(colourSizer, 0, wxEXPAND | wxALL, 3);

	wxSizer *optionsSizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("&Display options"));
	mainSizer->Add(optionsSizer, 0, wxEXPAND | wxALL, 3);
	checkBoxes[SHOW_LEFT_PANEL] = new wxCheckBox(this, wxID_ANY, _T("Show left panel"));
	checkBoxes[SHOW_INSTRUCTION_FIELDS] = new  wxCheckBox(this, wxID_ANY, _T("Split instructions into fields"));
	checkBoxes[SHOW_CONTROL_LINES] = new wxCheckBox(this, wxID_ANY, _T("Show control lines"));
	checkBoxes[SHOW_PC_LINES] = new wxCheckBox(this, wxID_ANY, _T("Show program counter lines"));
	checkBoxes[SHOW_POPUPS] = new wxCheckBox(this, wxID_ANY, _T("Show pop-ups"));
	checkBoxes[HIGHLIGHT_SINGLE_INSTRUCTION] = new wxCheckBox(this, wxID_ANY, _T("Highlight single instruction on pipelined layouts"));
	checkBoxes[SHOW_BOLD_DATA_LINES] = new wxCheckBox(this, wxID_ANY, _T("Show bold data lines at all times"));
	
	Config& c = Config::Instance();
	// Get default values for all the checkboxes & add then to the dialog.
	for(auto &i : checkBoxes)
	{
		optionsSizer->Add(i.second, 0, wxEXPAND | wxALL, 3);
		i.second->SetValue(c.getBool(i.first));
	}

	wxSizer *buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
	mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 3);
	this->Fit();
}

Dialog::~Dialog()
{
}

void Dialog::createColourButton(wxFlexGridSizer *parent, wxString caption, configName id)
{
	// Create the colour button with the specified caption
	// Also get the default colour from the config file.
	Config& c = Config::Instance();
	wxColour col = c.getColour(id);
	colours[id] = col;
	
	wxImage image(20, 20);
	image.SetRGB(wxRect(wxSize(20, 20)), col.Red(), col.Green(), col.Blue());
	wxBitmap bmp(image, -1);
	buttons[id] = new wxBitmapButton((wxWindow*)(this), (wxWindowID)id, bmp);
	parent->Add(new wxStaticText(this, wxID_ANY, caption), 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
	parent->Add(buttons[id], 0, wxEXPAND | wxRIGHT, 3);
}

void Dialog::showColourPicker(configName id)
{
	const wxColour& col = colours[id];

	// Show the colour picker, setting the original colour based on the required one,
	// and storing the new colour in the same location.
	wxColourData data;
  	data.SetChooseFull(true);
	data.SetColour(col);
	      
	wxColourDialog *dialog = new wxColourDialog(this, &data);
	if(dialog->ShowModal() == wxID_OK)
	{
		wxColourData retData = dialog->GetColourData();
	    colours[id] = retData.GetColour();
	}
	
	wxImage image(20, 20);
	image.SetRGB(wxRect(wxSize(20, 20)), col.Red(), col.Green(), col.Blue());
	wxBitmap bmp(image, -1);
	buttons[id]->SetBitmapLabel(bmp);
	
	dialog->Destroy();
}

void Dialog::EditColour(wxCommandEvent& event)
{
	// show the colour picker using the relevant colour based on the calling button.
	showColourPicker((configName)event.GetId());
}


