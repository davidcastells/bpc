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
 * File:   TextUtils.h
 * Author: dcr
 *
 * Created on February 13, 2020, 11:03 AM
 */

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <string>

using namespace std;

string humanUnits(double v, const char* units);


string rtrim(std::string& s);
string ltrim(std::string& s );
string trim(std::string& s );



#endif /* TEXTUTILS_H */

