/**
 * @file 	parser.h
 * @author	Adam Jez (xjezad00@stud.fit.vutbr.cz)
 * @author	Roman Blanco (xblanc01@stud.fit.vutbr.cz)
 * @date 	7.12.2014
 * @brief	Hlavicky soubor pro tridy, ktere parsuji zdrojove
 *			soubory a vraci je ve vektoru
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "model.h"

using namespace  std;

/**
 * @class 	ConnectionParser
 * @brief	Trida parsujici zdrojovy soubor se spojenim
 *			spojeni - dvojce ID od, do
 */
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

/**
 * @class 	StructuresParser
 * @brief	Trida parsujici zdrojovy soubor s objekty
 *			objekt - most, tunel, reka apod.
 */
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

/**
 * @class 	TrafficParser
 * @brief	Trida parsujici zdrojovy soubor s provozem
 *			ve formatu [OD] [DO] [pocet tisic tun za rok]
 */
class TrafficParser
{

private:
	string _fileName;
	ifstream * _readFile;
public:
	TrafficParser(string filename)
		: _fileName(filename)
	{

	}
	vector<pair<pair<int, int>, float>> Run();
};

void getFirstBySecond(vector<pair<int, int>> *vec, int second, vector<int> *out);
void getSecondByFirst(vector<pair<int, int>> *vec, int first, vector<int> *out);
void getNext(vector<pair<int, int>> *vec, int item, vector<vector<pair<int, int>>> *out);
int findNext(vector<pair<int, int>> *vec, int from, int to);