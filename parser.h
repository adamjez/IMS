///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#include <iostream>
#include <fstream>
#include <vector>
#include "model.h"

using namespace  std;

class ConnectionParser
{

private:
	string _fileName;
	ifstream * _readFile;
public:
	ConnectionParser(string filename)
		: _fileName(filename)
	{

	}

	vector<pair<int, int>> Run();

};

class StructuresParser
{

private:
	string _fileName;
	ifstream * _readFile;
public:
	StructuresParser(string filename)
		: _fileName(filename)
	{

	}
	vector<Structure *> Run();
};

void getFirstBySecond(vector<pair<int, int>> *vec, int second, vector<int> *out);
void getSecondByFirst(vector<pair<int, int>> *vec, int first, vector<int> *out);
void getNext(vector<pair<int, int>> *vec, int item, vector<vector<pair<int, int>>> *out);
int findNext(vector<pair<int, int>> *vec, int from, int to);