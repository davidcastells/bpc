#include "SWVersions.h"

#include <stdlib.h>
#include <math.h>

extern bool verbose;
extern int gES;		// substitution errors
extern int gEI; 	// insertion errors
extern int gED; 	// deletion errors
extern int gTh;
extern int gFP;	// false positives (detected errors < theshold && real errors > threshold)  = detected errors | real errors
extern int gFN;	// false negatives (detected errors > threhold && real errors > threshold)



string bitXor(string& pattern, string& text)
{
	string ret;
	
	for (int i=0; i < text.size(); i++)
	{
		if (pattern[i] == text[i])
			ret.push_back('0');
		else
			ret.push_back('1');
	}
	
	return ret;
}

string bitHamming(string& pattern, string& text)
{
	return bitXor(pattern, text);
}


string bitNot(string& a)
{
	string ret;
	
	for (int i=0; i < a.size(); i++)
	{
		if (a[i] == '1') 
			ret.push_back('0');
		else
			ret.push_back('1');
	}
	
	return ret;
}


string bitAnd(string& a, string& b)
{
	string ret;
	
	for (int i=0; i < a.size(); i++)
	{
		if ((a[i] == '1') && (b[i] == '1'))
			ret.push_back('1');
		else
			ret.push_back('0');
	}
	
	return ret;
}

string bitOr(string& a, string& b)
{
	string ret;
	
	for (int i=0; i < a.size(); i++)
	{
		if ((a[i] == '1') || (b[i] == '1'))
			ret.push_back('1');
		else
			ret.push_back('0');
	}
	
	return ret;
}


int popCount(string& a)
{
	int n = 0;
	
	for (int i=0; i < a.size(); i++)
	{
		if (a[i] == '1')
			n++;
	}
	
	return n;
}

int countZeros(string& a)
{
	int n = 0;
	
	for (int i=0; i < a.size(); i++)
	{
		if (a[i] == '0')
			n++;
	}
	
	return n;
}

string spaces(int len)
{
	string ret;
	for (int i=0; i < len; i++)
		ret.push_back(' ');
	return ret;
}

string ones(int len)
{
	string ret;
	for (int i=0; i < len; i++)
		ret.push_back('1');
	return ret;
}


string zeros(int len)
{
	string ret;
	for (int i=0; i < len; i++)
		ret.push_back('0');
	return ret;
}

string shiftLeft(string& str, int v)
{
	string ret = str;
	int len = str.size();
	
	if (v == 0)
		return ret;
	if (v > 0)
	{
		for (int i=0; i < v; i++)
		{
			ret.push_back('-');	// invalid base
		}

		ret = ret.substr(v, len); // ret.erase(0, v);		// erase the [v] first values
	}
	else
	{
		v = -v;
		for (int i=0; i < v; i++)
		{
			ret.insert(0, "-");	// invalid base
			
		}
		
		ret = ret.substr(0, len); // ret.erase(str.size()-1, v);	// erase the [v] last values
	}
	
	return ret;
}

string removeShortZeros(string& hamming)
{
	string ret = hamming;
	bool longZero = false;
	
	for (int i=0; i<hamming.size(); i++)
	{
		if (ret[i] == '0') 
		{
			if (longZero == false)
			{
				if (i < (hamming.size()-2))
					if ((ret[i+1] == '1') || (ret[i+2] == '1'))
						ret[i] = '1';
					else
						longZero = true;
			}
		}
		else
			longZero = false;
	}
	
	return ret;
}



string range(string& str, int offset, int len)
{
	string ret = zeros(len);

	for (int i=0; i < len; i++)
		ret[i] = str[offset+i];

	return ret;
}


string leftOnes(string& str)
{
	string ret;

	bool oneSeen = false;	

	for (int i=0; i < str.size(); i++)
	{
		oneSeen = oneSeen || (str[i] == '1');  
		if (oneSeen)
			ret.push_back('1');
		else
			ret.push_back('0');
	}

	//if (verbose)
	//	printf("LeftOnes(%s) = %s\n", str.c_str(), ret.c_str());	

	return ret;
}

int countLeadingZeros(string& str)
{
	int ret = 0;
	bool oneSeen = false;	

	for (int i=0; i < str.size(); i++)
	{
		oneSeen = oneSeen || (str[i] == '1');  
		if (!oneSeen)
			ret++;
	}

	//if (verbose)
	//	printf("CLZ(%s) = %d\n", str.c_str(), ret);

	return ret;
}

void extend(string& str, char c, int len)
{
	for (int i=0; i < len; i++)
		str.push_back(c);
}



int baseToNum(char b)
{
	if (b == 'A') return 0;
	if (b == 'C') return 1;
	if (b == 'G') return 2;
	if (b == 'T') return 3;
}

int kmerIndex(string& kmer)
{
	int idx = 0;

	for (int i=0; i<kmer.size(); i++)
	{
		idx = idx*4 + baseToNum(kmer[i]);
	}

	return idx;
}

