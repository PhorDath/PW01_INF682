#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class strf
{
public:
	strf();
	static vector<string> split(string line, char token);
	static vector<string> split2(string line, char token1, char token2);
	static vector<string> split3(string line, char token1, char token2, char token3);
	static string rnd(string);
	static float rndac(string);
	~strf();
};

