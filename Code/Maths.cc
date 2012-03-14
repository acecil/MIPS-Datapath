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
 
#include "Maths.h"

numFormat Maths::numberFormat = FORMAT_DECIMAL;

wxString Maths::convertToBase(luint number, bool noNegative, bool negBit16, numFormat format)
{
	uint numNegBits = negBit16 ? 16 : 32;
	bool negative = !!(number & ((luint)(1)<<(numNegBits - 1)));
	luint tempNum = negative ? ((luint)(1)<<(numNegBits)) - number : number;
	wxString decNum, binNum, temp, hexNum;
	bool foundOne = false;
	switch(format)
	{
		case FORMAT_DECIMAL:
			// If number is negative, convert to correct display format including minus sign.
			if(negative && !noNegative)
			{
				decNum += _T("-");
			}
			for(uint exp = 0; exp < 10; exp++)
			{
				temp += wxString::Format(_T("%u"), uint(tempNum % 10));
				tempNum = (tempNum - tempNum % 10) / 10;
			}
			
			for(int i = temp.Len() - 1; i >= 0; i--)
			{
				if(temp[i] != _T('0'))
				{
					foundOne = true;
				}
				if(foundOne || i == 0)
				{
					decNum += temp[i];
				}
			}
			return decNum;
		case FORMAT_BINARY:
			for(luint bit = (((luint)(-1)) >> 1) + 1; bit; bit >>= 1)
			{
				temp = bit & number ? _T('1') : _T('0');
				if(temp == _T('1'))
				{
					foundOne = true;
				}
				if(foundOne || bit == 1) 
				{
					binNum += temp;
				}
			}
			return binNum;
		case FORMAT_HEX:
			// If number is negative, convert to correct display format including minus sign.
			if(negative && !noNegative)
			{
				hexNum += _T("-");
			}
			for(uint exp = 0; exp < 7; exp++)
			{
				temp += wxString::Format(_T("%X"), uint(tempNum % 16));
				tempNum = (tempNum - tempNum % 16) / 16;
			}
			
			for(int i = temp.Len() - 1; i >= 0; i--)
			{
				if(temp[i] != _T('0'))
				{
					foundOne = true;
				}
				if(foundOne || i == 0)
				{
					hexNum += temp[i];
				}
			}
			return hexNum;
	}
	
	return _T("0");
}
