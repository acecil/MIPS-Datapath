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
 
#ifndef MATHS_H_
#define MATHS_H_

#include <cmath>
#include <iostream>

#include <wx/string.h>

#include "Types.h"

enum numFormat
{
	FORMAT_DECIMAL,
	FORMAT_BINARY,
	FORMAT_HEX
};

class Maths
{
	public:
		static wxString convertToBase(luint number, bool noNegative = false, bool negBit16 = false, numFormat format = numberFormat);
		static void setFormat(numFormat newFormat){ numberFormat = newFormat; };
	private:
		static numFormat numberFormat;
};


#endif /*MATHS_H_*/
