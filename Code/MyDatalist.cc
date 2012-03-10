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
 
#include "MyDatalist.h"

BEGIN_EVENT_TABLE(MyDatalist, wxListCtrl)
    EVT_LIST_COL_BEGIN_DRAG(ID_INSTRUCTION_LIST, MyDatalist::OnDataListColSizeAdjusted)
    //EVT_LISTBOX_DCLICK(ID_INSTRUCTION_LIST, MyDatalist::OnDataListColSizeAdjusted)
    EVT_LIST_COL_BEGIN_DRAG(ID_REGISTER_LIST, MyDatalist::OnDataListColSizeAdjusted)
    EVT_LIST_COL_BEGIN_DRAG(ID_DATA_LIST, MyDatalist::OnDataListColSizeAdjusted)
    EVT_LIST_ITEM_SELECTED(ID_INSTRUCTION_LIST, MyDatalist::OnSelect)
END_EVENT_TABLE()

MyDatalist::MyDatalist(uint multiplier, uint maxVals, wxString firstColTitle, wxWindow* parent, wxWindowID id)
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL)
{
	this->multiplier = multiplier;
	this->maxVals = maxVals;
	this->firstColTitle = firstColTitle;
	selectedIndex = -1;
}

void MyDatalist::InsertFirstColumn()
{
	this->InsertColumn(0, firstColTitle);
}

void MyDatalist::OnDataListColSizeAdjusted(wxListEvent& event)
{
	// Prevent adjusting column sizes on these lists.
	event.Veto();
}

void MyDatalist::OnSelect(wxListEvent& event)
{
	selectedIndex = event.GetIndex() * multiplier;
	event.Skip();
}
