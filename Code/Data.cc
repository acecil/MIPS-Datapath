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
 
#include "Data.h"

Data::Data(int numBits)
{
	this->numBits = numBits;
	// Fill vector to correct size with false (0). 
	for(int i = 0; i < numBits; i++)
	{
		bits.push_back(false);
	}
}

Data::Data(const Data& newData)
{
	int minLength = std::min(numBits, newData.size());
	for(int i = 0; i < minLength; i++)
	{
		bits[i] = newData.get(i);
	}
}

bool Data::get(int i) const
{
	if((i >= 0) && (i < numBits))
	{
		return bits[i]; 
	}
	return false;
}
