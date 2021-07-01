#ifndef SW_VERSIONS_H
#define SW_VERSIONS_H

#include <string.h>
#include <stdio.h>
#include <string>

using namespace std;

string ones(int len);
string zeros(int len);
int countZeros(string& a);

int SHD_SW(string& pattern, string& text, int th);
int Shouji_SW(string& pattern, string& text, int threshold);
int Shouji_Alser(string& pattern, string& text, int threshold);
int Sneaky_SW(string pattern, string text, int th);
int Kmers_SW(string pattern, string text, int threshold);


#endif
