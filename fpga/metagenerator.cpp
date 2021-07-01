/*

COMPILE


USAGE

g++ metagenerator.cpp -o metagenerator
./metagenerator example.meta.h > example.meta.h.cpp
g++ example.meta.h.cpp -o example.meta.h.generator
./example.meta.h.generator

*/

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <fstream>

#include <string.h>

using namespace std;

string startSym = "<%";
string stopSym = "%>";

bool contains(string& s1, string& s2)
{
	return (strstr(s1.c_str(),s2.c_str()));
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


string upToStart(string& line)
{
	string ret;
	
	int pos = line.find(startSym);
	ret = line.substr(0, pos);
	
	line = line.substr(pos+2);
	
	return ret;
}

string upToStop(string& line)
{
	string ret;
	
	int pos = line.find(stopSym);
	ret = line.substr(0, pos);
	
	line = line.substr(pos+2);
	
	return ret;
}

bool getLine(ifstream& read, string& line)
{
	bool doRun = !read.eof();
	line.clear();
	getline(read, line);
	
	if (hasEnding(line, "\n"))
		line = line.substr(0, line.size()-1);	// remove last \n
	
	return doRun;
}

void replaceAll(string& input, const string& from, const string& to)
{
    size_t pos = 0;
    
    while(true)
    {
        size_t startPosition = input.find(from, pos);
        
	if(startPosition == string::npos)
            break;
        input.replace(startPosition, from.length(), to);
	
	pos = startPosition + to.size();
    }
}

string encode(string str)
{
	string ret = str;
		
	replaceAll(ret, "\"", "\\\"");
	replaceAll(ret, "\\n", "\\\\n");
	replaceAll(ret, "%", "%%");
	return ret;
}

void dumpPart(string str)
{
	cout <<"printf(\"" ;
	cout << encode(str);
	cout << "\");";
	cout << endl;
}

void dump(string str)
{
	cout <<"printf(\"" ;
	cout << encode(str);
	cout << "\\n\");";
	cout << endl;
}

void raw(string str)
{
	cout << str;
	cout << endl;
}


void rawPart(string str)
{
	cout << str;
	//cout << endl;
}


void dumpMain()
{
	
	cout << "int main(int argc, char* args[])" << endl;
	cout << "{" << endl;
}

// we have two dimesion, dimension Target, dimension Generator, in target everything is encapulated in a print

#define STATE_UNKNOWN		0
#define STATE_IN_TARGET 	1
#define STATE_IN_GENERATOR 	2 
#define STATE_IN_PRE_GENERATOR 	3 

int main(int argc, char* args[])
{
	// usage : metagenerator inputfile

    ifstream read(args[1]);
    string line;

    if(read.fail())
    {
    	cerr << "Input file " << args[1] << " does not exist" << endl;
	return -1;    
    }

	int state = STATE_UNKNOWN; 	// unknown
	
	//auto doRun =
	
	
	cout << "#include <stdio.h>" << endl;

	bool doRun = getLine(read, line);
	
    while(doRun) 
    {
	if (state == STATE_UNKNOWN)
	{
		if (contains(line, startSym))
		{
			// consume up to start symbol
	    		string part = upToStart(line);
	    		//dumpPart(part);
	    		state = STATE_IN_PRE_GENERATOR;
		}
		else
		{
			dumpMain();
			state = STATE_IN_TARGET;	
		}

	}
    	else if (state == STATE_IN_TARGET)
    	{
	    	if (!contains(line, startSym)) 
	    	{
	    		dump(line);
	    		
	    		doRun = getLine(read, line);
	    	}
	    	else
	    	{
	    		// consume up to start symbol
	    		string part = upToStart(line);
	    		dumpPart(part);
	    		state = STATE_IN_GENERATOR;
	    		
	    	}
    	}
    	else if (state == STATE_IN_GENERATOR)
   	{
    		if (!contains(line, stopSym))
    		{
    			raw(line);
    			doRun = getLine(read, line);
    		}
    		else
    		{
    			// consume up to the stop symbol
    			string part = upToStop(line);
    			rawPart(part);
    			state = STATE_IN_TARGET;
    		}
    	
    	}
	else if (state == STATE_IN_PRE_GENERATOR)
   	{
    		if (!contains(line, stopSym))
    		{
    			raw(line);			
    			doRun = getLine(read, line);
    		}
    		else
    		{
    			// consume up to the stop symbol
    			string part = upToStop(line);
    			rawPart(part);
			dumpMain();
    			state = STATE_IN_TARGET;
    		}
    	
    	}
	    	
		//doRun = std::getline( input, line );
    }
    
    cout << "}" << endl;

    return 0;
}
