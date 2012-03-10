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
 
#ifndef MYAPP_H_
#define MYAPP_H_

#include <wx/wx.h>
#include "Enums.h"
#include "MyFrame.h"
#include "Config.h"

class MyApp : public wxApp
{
	public:
		MyApp();
		virtual ~MyApp();
		virtual bool OnInit();
	private:
		MyFrame *frame;
};
#ifndef DEBUG_BUILD
IMPLEMENT_APP(MyApp)
#endif

#endif /*MYAPP_H_*/

