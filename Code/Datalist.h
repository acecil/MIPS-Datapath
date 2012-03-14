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
 
#ifndef DATALIST_H_
#define DATALIST_H_

#include <iostream>
#include <wx/wx.h>
#include <wx/listctrl.h>

#include "Types.h"
#include "Enums.h"

class Datalist : public wxListCtrl
{
	public:
		Datalist(uint multiplier, uint maxVals, wxString firstColTitle, wxWindow* parent, wxWindowID id);
		void InsertFirstColumn();
		uint getMultiplier(){ return multiplier; };
		uint getMaxVals(){ return maxVals; };
		long getSelectedIndex(){ return (selectedIndex < 0) ? 0 : selectedIndex; };
		bool isSelectedIndexValid(){ return (selectedIndex < 0) ? false : true;  };
	private:
		void OnDataListColSizeAdjusted(wxListEvent& event);
		void OnSelect(wxListEvent& event);
		uint multiplier;
		uint maxVals;
		wxString firstColTitle;
		long selectedIndex;
	DECLARE_EVENT_TABLE()
};

#endif /*DATALIST_H_*/
