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
 
#ifndef COLOR_H_
#define COLOR_H_

#include <GL/glut.h>

struct Color
{
	Color(){ init = false; };
	Color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
	{ 
		// Set initial values.
		this->c[0] = r; 
		this->c[1] = g; 
		this->c[2] = b;
		this->c[3] = a;
		init = true;
	}
	GLfloat c[4];
	bool init;
};

#endif /*COLOR_H_*/
