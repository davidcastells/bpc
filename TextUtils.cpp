/*
 * Copyright (C) 2020 Universitat Autonoma de Barcelona - David Castells-Rufas <david.castells@uab.cat>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   TextUtils.cpp
 * Author: dcr
 * 
 * Created on February 13, 2020, 11:03 AM
 */

#include "TextUtils.h"

#include <stdio.h>
#include <string>
#include <unistd.h> // readlink, chdir
#include <stdarg.h>

std::string format(const char *fmt, ...)
{
    std::string result = "";

    char buffer[256];
    va_list args;
    va_start (args, fmt);
    vsprintf (buffer,fmt, args);
    
    result.append(buffer);
    
    va_end (args);
  
    return result;
}

string humanUnits(double v, const char* units)
{
    if (v > 1e9)
        return format("%f G%s", v/1e9, units);
    if (v > 1e6)
        return format("%f M%s", v/1e6, units);
    if (v > 1e3)
        return format("%f k%s", v/1e3, units);
    if (v < 1e-3)
        return format("%f m%s", v*1e3, units);
    if (v < 1e-6)
        return format("%f u%s", v*1e6, units);
    if (v < 1e-9)
        return format("%f n%s", v*1e9, units);
    
    return format("%f %s", v, units);
}

const char* ws = " \t\n\r\f\v";

// trim from end of string (right)
string rtrim(string& s)
{
    s.erase(s.find_last_not_of(ws) + 1);
    return s;
}

// trim from beginning of string (left)
string ltrim(string& s)
{
    s.erase(0, s.find_first_not_of(ws));
    return s;
}

// trim from both ends of string (right then left)
string trim(string& s)
{
	string rs = rtrim(s); 
    return ltrim(rs);
}

