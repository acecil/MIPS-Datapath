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
 
#ifndef FRAME_H_
#define FRAME_H_

#include <iostream>
#include <map>
#include <memory>

#include <wx/wx.h>
#include <wx/icon.h>
#include <wx/textctrl.h>
#include <wx/glcanvas.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/scrolwin.h>
#include <wx/listctrl.h>
#include <wx/grid.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/colour.h>
#include <wx/colordlg.h>
#include <wx/aboutdlg.h>
#include <wx/html/helpctrl.h>
#include <wx/fs_arc.h>
#include <wx/accel.h>

#include "Enums.h"
#include "Types.h"

class GLCanvas;
class ScrolledWindow;
class Model;
class Datalist;

class Frame : public wxFrame
{
public:
    Frame(const wxString& title, const wxPoint& pos, const wxSize& size);
	virtual ~Frame();
    void showHideLeftPanel(bool showLeftPanel, bool justSwitch = false);
    void updateDataList(bool initialCall = false, bool selectInstruction = false);
    void finishInit();
private:
    void StepButtonClicked(wxCommandEvent& event);
    void ResetButtonClicked(wxCommandEvent& event);
    void SetSimpleLayout(wxCommandEvent& event);
    void SetPipelineLayout(wxCommandEvent& event);
    void SetForwardingLayout(wxCommandEvent& event);
    void SetFormatBin(wxCommandEvent& event);
    void SetFormatDec(wxCommandEvent& event);
    void SetFormatHex(wxCommandEvent& event);
    void EditOptions(wxCommandEvent& event);
    void ShowContents(wxCommandEvent& event);
    void ShowIndex(wxCommandEvent& event);
    void ShowAbout(wxCommandEvent& event);
    void LoadFile(wxCommandEvent& event);
    void SaveFile(wxCommandEvent& event);
    void Parse(wxCommandEvent& event);
    void MemoryGridChanged(wxGridEvent& event);
    void setupMenubar();
	void setupSimulatorPage(wxNotebook *notebook);
	void setupEditorPage(wxNotebook *notebook);
	void updateEditorText();
	void updateEditorTextFromStore();
	void updateErrorText();
	void setInitialMemoryVals(bool fromParser = false);
	void resetLeftPanelSizes();
	void OnSplitterSizeChanged(wxSplitterEvent& event);
	void OnSelectInstruction(wxListEvent& event);
	void OnZoomSliderChanged(wxCommandEvent& event);
	void updateDataListHighlighting();
	static const int LEFT_PANEL_DEFAULT_SIZE=100;
	static const int LEFT_PANEL_MIN_WIDTH=213;
	static const int LEFT_PANEL_MIN_HEIGHT=450;
	GLCanvas *canvas;
	wxGridSizer *GLSizer;
	std::unique_ptr<Model> processor;
	wxPanel *leftPanel;
	wxPanel *memoryPane;
	wxSplitterWindow *mainSplitter;
	wxSplitterWindow *editorSplitter;
	wxTextCtrl *editorText;
	wxTextCtrl *errorText;
	std::map<uint, Datalist*> dataList;
	wxGrid *memoryList;
	std::unique_ptr<wxHtmlHelpController> help;
	std::unique_ptr<wxIconBundle> ico;
	bool init;
	bool initSize;
    DECLARE_EVENT_TABLE()
};

enum
{
    ID_INSTRUCTION_MEM = wxID_HIGHEST,
    ID_REGISTERS_MEM,
    ID_DATA_MEM,
    ID_MENU_LAYOUT_SIMPLE,
    ID_MENU_LAYOUT_PIPELINE,
    ID_MENU_LAYOUT_FORWARDING,
    ID_MENU_LAYOUT_SHOW_CONTROL,
    ID_MENU_LAYOUT_HIDE_CONTROL,
    ID_MENU_LAYOUT_SHOW_PC,
    ID_MENU_LAYOUT_HIDE_PC,
    ID_MENU_SHOW_POPUPS,
    ID_MENU_HIDE_POPUPS,
    ID_MENU_FORMAT_BIN,
    ID_MENU_FORMAT_DEC,
    ID_MENU_FORMAT_HEX,
    ID_MENU_FILE_LOAD,
    ID_MENU_EDIT_OPTIONS,
    ID_MENU_EDIT_COMPONENT_COLOR,
    ID_MENU_EDIT_PIPELINE_COLOR,
    ID_MENU_HELP_CONTENTS,
    ID_MENU_HELP_INDEX,
    ID_MENU_HELP_ABOUT,
    ID_LOAD,
    ID_LOAD_INSTRUCTIONS,
    ID_SAVE_INSTRUCTIONS,
    ID_PARSE_INSTRUCTIONS,
    ID_MEMORY_LIST,
    ID_SPLITTER_WINDOW,
    ID_EDSPLITTER_WINDOW,
	ID_ZOOM_SLIDER
};

#endif /*FRAME_H_*/
