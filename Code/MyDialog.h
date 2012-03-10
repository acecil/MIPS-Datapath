/*
 *  
 *  MIPS-Datapath - Graphical MIPS CPU Simulator.
 *  Copyright 2008 Andrew Gascoyne-Cecil.
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
 
#ifndef MYDIALOG_H_
#define MYDIALOG_H_

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/window.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/colour.h>
#include <wx/colordlg.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/image.h>
#include "Component.h"
#include "Model.h"
#include "Color.h"
#include "Config.h"

#include <iostream>

class MyDialog : public wxDialog
{
	public:
	    MyDialog(wxWindow* parent, wxWindowID id, wxIconBundle *ico, Model* proc, const wxString& title, bool mainSplitterSplit, bool instructionsInFields, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = _T("dialogBox"));
		virtual ~MyDialog();
		bool getBool(configName name){ return checkBoxes[name]->GetValue(); };
		wxColour getColour(configName colour){ return colours[colour]; };
	private:
		void createColourButton(wxFlexGridSizer *parent, wxString caption, configName id);
		void showColourPicker(configName id);
    	void EditColour(wxCommandEvent& event);
		Model *proc;
		map<configName, wxCheckBox*> checkBoxes;
		map<configName, wxColour> colours;
		map<configName, wxBitmapButton*> buttons;
		wxIconBundle *ico;
		DECLARE_EVENT_TABLE()
};

#endif /*MYDIALOG_H_*/
