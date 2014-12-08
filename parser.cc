/**
 * @file 	parser.cc
 * @author	Adam Jez (xjezad00@stud.fit.vutbr.cz)
 * @author	Roman Blanco (xblanc01@stud.fit.vutbr.cz)
 * @date 	7.12.2014
 * @brief	Metody pro tridy, ktere parsuji zdrojove
 *			soubory a vraci je ve vektoru
 */

#include "parser.h"

/**
 * Funkce vrati vektor hodnot v tretim parametru
 * @param	vec 	prohledavany seznam
 * @param 	second 	cislo, ktere se v seznamu hleda
 * @param 	out		vektor hodnot, ktery byli v paru s hledanou hodnotou
 */
void getFirstBySecond(vector<pair<int, int>> *vec, int second, vector<int> *out)
{
	out->clear();
	for(auto conn = vec->begin(); conn != vec->end(); ++conn)
	{
		if(conn->second == second)
			out->push_back(conn->first);
	}

}

/**
 * Funkce vrati vektor hodnot v tretim parametru
 * @param	vec 	prohledavany seznam
 * @param 	first 	cislo, ktere se v seznamu hleda
 * @param 	out		vektor hodnot, ktery byli v paru s hledanou hodnotou
 */
void getSecondByFirst(vector<pair<int, int>> *vec, int first, vector<int> *out)
{
	out->clear();
	for(auto conn = vec->begin(); conn != vec->end(); ++conn)
	{
		if(conn->first == first)
			 out->push_back(conn->second);
	}

}

/**
 * Funkce vrati mozne nasledovniky/predchazejici zadane hodnoty
 * @param	vec 	prohledavany seznam
 * @param 	item 	cislo, ktere se v seznamu hleda
 * @param 	out		vektor hodnot, ktery byli v paru s hledanou hodnotou
 */
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

/**
 * Vrati nasledujici hodnotu ze seznamu po hodnote from 
 * @param	vec 	prohledavany seznam
 * @param 	from 	cislo od ktereho se hleda nasledujici
 * @param 	to		cislo, ke kteremu se chceme dostat
 * @return	vrati hodnotu nasledujiciho prvku, kdyz nenajde, vrati -1
 */
int findNext(vector<pair<int, int>> *vec, int from, int to)
{
	vector<vector<pair<int, int>>> result;

	getNext(vec, from, &result);

	/*for(auto path = result.begin(); path != result.end(); ++path)
	{	
		cout << path->front().first << "  " << path->front().second << endl;
	}*/

	u_int size; 
	bool change;
	while(true)
	{
		change = false;
		for(auto path = result.begin(); path != result.end(); ++path)
		{
			size = path->size();

			auto find = path->back();

			if(find.first == to || find.second == to)
			{
				// Nasli jsme cestu, vratime id dalsiho prvku
				auto item = path->front();
				//cout << "Nasli jsme: " << item.first << "  " << item.second << endl;
				if(item.first == from)
					return item.second;
				else
					return item.first;

			}
			//cout << "posledni: " << find.first << "  " << find.second << endl;
			for(auto conn = vec->begin(); conn != vec->end(); ++conn)
			{

	
				if(from < to && conn->first == find.second && conn->second != find.first)
				{ 
					// Nasli jsem cestu
					path->push_back(make_pair(conn->first, conn->second));
				}

				if(from > to && conn->second == find.first && conn->first != find.second)
				{
					// Nasli jsem cestu
					path->push_back(make_pair(conn->first, conn->second));
				}
			}	

			if(size != path->size())
				change = true;

		}

		if(!change)
			break;
	}

	return -1;
}

/**
 * Funkce projde zdrojovy soubor ve formatu: "X Y\n", kde X a Y jsou cisla spojeni
 * @return	vrati seznam paru spojeni
 */
vector<pair<int, int>> ConnectionParser::Run()
{
	int x,y;
	vector<pair<int, int>> result;

	_readFile = new ifstream(_fileName);

	while(_readFile->good())
	{
		(*_readFile) >> x >> ws >> y >> ws;
		result.push_back(make_pair(x, y));
	}

	_readFile->close();

	return result;
}

/**
 * Funkce projde zdrojovy soubor ve formatu: "A STR X Y Z\n", kde X, Y a Z 
 * jsou cisla spojene s pridavanym objektem, STR je nazev objektu a A je cislo typu objektu
 * @return	vrati seznam objektu (most, reka, pristav, apod.)
 */
vector<Structure *> StructuresParser::Run()
{
	int type;
	string name;
	float x, y, z;
	Structure *structure;
	vector<Structure *> result;

	_readFile = new ifstream(_fileName);

	while(_readFile->good())
	{
		(*_readFile) >> type >> ws >> name >> ws >> x >> ws >> y >> z >> ws;

		switch(type)
		{
			case tunnel:
			{
				Tunnel * tun = new Tunnel(name, x);
				structure = (Structure*)tun;
				break;
			}
			case chamber:
			{
				Chamber * cham = new Chamber(name, x, 5*30);
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
				Bridge * brid = new Bridge(name, x);
				structure = (Structure*)brid;
				break;
			}
			default:
				;


		}
		result.push_back(structure);
	}

	_readFile->close();

	return result;
}

/**
 * Funkce projde zdrojovy soubor ve formatu: "X Y A\n", kde X, Y jsou id objektu a A je pocet tisic tun
 * @return	vrati seznam dvojitych paru, obsahujici id objektu od do a pocet tisic tun mezi nimi za rok
 */
vector<pair<pair<int, int>, float>> TrafficParser::Run()
{
	int id1, id2;
	float x;
	vector<pair<pair<int, int>, float>> result;

	_readFile = new ifstream(_fileName);

	while(_readFile->good())
	{
		(*_readFile) >> id1 >> ws >> id2 >> ws >> x >> ws;

		result.push_back(make_pair(make_pair(id1, id2),x));
	}

	_readFile->close();

	return result;
}