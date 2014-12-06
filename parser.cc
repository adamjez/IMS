///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#include "parser.h"

void getFirstBySecond(vector<pair<int, int>> *vec, int second, vector<int> *out)
{
	out->clear();
	for(auto conn = vec->begin(); conn != vec->end(); ++conn)
	{
		if(conn->second == second)
			out->push_back(conn->first);
	}

}


void getSecondByFirst(vector<pair<int, int>> *vec, int first, vector<int> *out)
{
	out->clear();
	for(auto conn = vec->begin(); conn != vec->end(); ++conn)
	{
		if(conn->first == first)
			 out->push_back(conn->second);
	}

}

void getNext(vector<pair<int, int>> *vec, int item, vector<vector<pair<int, int>>> *out)
{
	for(auto conn = vec->begin(); conn != vec->end(); ++conn)
	{
		if(conn->first == item || conn->second == item)
		{
			vector<pair<int, int>> v {make_pair(conn->first, conn->second)};
			out->push_back(v);
		}
	}

}

int findNext(vector<pair<int, int>> *vec, int from, int to)
{
	vector<vector<pair<int, int>>> result;

	getNext(vec, from, &result);

	/*vector<int> found{from};
	vector<int> next{};*/
	while(true)
	{
		for(auto path = result.begin(); path != result.end(); ++path)
		{
			for(auto conn = vec->begin(); conn != vec->end(); ++conn)
			{
				if(conn->first == to || conn->second == to)
				{
					// Nasli jsme cestu, vratime id dalsiho prvku
					auto item = path->front();
					if(item.first == from)
						return item.second;
					else
						return item.first;

				}
				auto last = path->back();
				if(conn->first == last.second && conn->second != last.first)
				{
					// Nasli jsem cestu
					path->push_back(make_pair(conn->first, conn->second));
				}

				if(conn->second == last.first && conn->first != last.second)
				{
					// Nasli jsem cestu
					path->push_back(make_pair(conn->first, conn->second));
				}

				/*
				if(conn->first == id)
					 next->push_back(conn->second);
				if(conn->second == id)
					next->push_back(conn->first);*/
			}	


		}



		if(result.empty())
			break;
	}

	return -1;
}

vector<pair<int, int>> ConnectionParser::Run()
{
	int x,y;
	vector<pair<int, int>> result;

	_readFile = new ifstream(_fileName);
	//ifstream->open();

	while(_readFile->good())
	{
		(*_readFile) >> x >> ws >> y >> ws;
		result.push_back(make_pair(x, y));
	}

	_readFile->close();

	return result;
}

vector<Structure *> StructuresParser::Run()
{
	int type;
	string name;
	float x, y, z;
	Structure *structure;
	vector<Structure *> result;

	_readFile = new ifstream(_fileName);
	//ifstream->open();

	while(_readFile->good())
	{
		(*_readFile) >> type >> ws >> name >> ws >> x >> ws >> y >> z >> ws;

		switch(type)
		{
			case tunnel:
			{
				Tunnel * tun = new Tunnel(x, 5*30);
				structure = (Structure*)tun;
				break;
			}
			case chamber:
			{
				Chamber * cham = new Chamber(name.c_str(), x, 5*30);
				structure = (Structure*)cham;
				break;
			}
			case port:
			{
				Port * port = new Port(x);
				structure = (Structure*)port;
				break;
			}
			case channel:
			{
				Channel * chan = new Channel(x);
				structure = (Structure*)chan;
				break;
			}
			case river:
			{
				River * riv = new River(x, (bool)y);
				structure = (Structure*)riv;
				break;
			}
			case bridge:
			{
				Tunnel * tun = new Tunnel(x, 5*30);
				structure = (Structure*)tun;
				break;
			}
			default:
				;


		}
		structure->setName(name);
		result.push_back(structure);
	}

	_readFile->close();

	return result;
}