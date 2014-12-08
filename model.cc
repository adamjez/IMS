/**
 * @file 	model.cc
 * @author	Adam Jez (xjezad00@stud.fit.vutbr.cz)
 * @author	Roman Blanco (xblanc01@stud.fit.vutbr.cz)
 * @date 	7.12.2014
 * @brief	Metody pro tridy, ktere modeluji struktury
 *			na vodni ceste, ktere nejvice ovlivnuji dobu prepluti
 */


#include "model.h"
#include "parser.h"


 // global objects: 
vector<pair<int, int>> Map;
vector<Structure *> Info;
vector<pair<pair<int, int>, float>> Traffic;
vector<pair<pair<int, int>, Histogram*>> ShipTables;
Histogram Chambers("Plavebni komory", 0, 200, 100);
Histogram Bridges("Mosty", 0, 200, 100);
Histogram Tunnels("Tunely", 0, 200, 100);
Histogram Rivers("Reka", 0, 200, 100);
Histogram Cannals("Plavebni kanaly", 0, 200, 100);
long TraveledDistance = 0;


/**
 * Hlavni funkce main
 */
int main(int argc, char** argv) { // experiment description 
	if(argc < 2)
	{
		cerr << "Zadal jste prilis malo parametru" << endl;
		cerr << "  Prvni argument musi byt nazev zdrojoveho souboru: " << endl;
		cerr << "  napr. ship-VYSOKY-2050, ship-VYSOKY-2020, ship-NIZKY-2020, ship-NIZKY-2050, ship-TREND-2020, ship-TREND-2050" << endl;
		cerr << "  Vystup se zobrazi v souboru: [nazev_zdrojoveho_souboru].out" << endl;
		return 1;
	}

	Print(" Vodni cesta - SIMLIB/C++ \n"); 
	SetOutput((string(argv[1]) + string(".out")).c_str()); 

	ConnectionParser parser1("input/connection.tsv");
	Map = parser1.Run();

	StructuresParser parser2("input/info.tsv");
	Info = parser2.Run();

	try{
		TrafficParser parser3((string("input/") + string(argv[1]) + string(".tsv")).c_str());
		Traffic = parser3.Run();
	}
	catch(exception)
	{
		cerr << "CHYBA: Nepodarilo se otevrit zdrojovy soubor." << endl;
	}


	long sedcondsInYear = 365*24*60*60;

	RandomSeed(time(0));

	Init(0,sedcondsInYear); // experiment initialization for time ONE YEAR

	for(auto &item : Traffic)
	{
		// Vytvorime dva histogramy, pro kazdy smer jeden
		string name1 = string("Lode - Z: ") + to_string(item.first.first) + string(" Do: ") + to_string(item.first.second);
		ShipTables.push_back(
			make_pair(
				make_pair(
					item.first.first, item.first.second), 
					new Histogram(converToAscii(name1), 0, 5000, 100)
					)
			);

		string name2 = string("Lode - Z: ") + to_string(item.first.second) + string(" Do: ") + to_string(item.first.first);
		ShipTables.push_back(
			make_pair(
				make_pair(
					item.first.second, item.first.first), 
					new Histogram(converToAscii(name2), 0, 5000, 100)
					)
			);


	}

	for(auto &item : Traffic)
	{
		int diff = sedcondsInYear / (item.second / 4) ; //((Pocet tun k prevezeni) Pocet lodi)
		(new Generator(diff, item.first.first, item.first.second))->Activate(); // customer generator 
	}
	
	Run(); // simulation Box.Output(); // print of results 
	
	std::cout << "USPESNE KONCIM S UJETOU VZDALENOSTI: " << TraveledDistance << std::endl;
	
	for(auto &item : Info)
	{
		delete item;
	}
	for(auto &item : ShipTables)
	{
		item.second->Output();
	}
	
/*
	Chambers.Output();
	Bridges.Output();
	Tunnels.Output();
	Rivers.Output();
	Cannals.Output();
*/

	return 0; 
}

/**
 * Funkce simlujici lod, ktera se pokusi obsadit plavebni komoru
 * @param	ship 	lod pokousejici se obsadit komoru
 */
void Chamber::Seize(CargoShip *ship)
{
	ship->Prichod = Time;
	//std::cout << "Seize" << std::endl;
	if(in != NULL)
	{
		// Jestli je lod nahore -> vlozit do horni fronty a opak
		if(ship->getDirection())
			Q1->Insert(ship);
		else
			Q2->Insert(ship);

		ship->Passivate();

	}
	else if (ship->getDirection() != this->_pos)
	{
		if(ship->getDirection())
			Q1->Insert(ship);
		else
			Q2->Insert(ship);

		if(_tm.Idle())
		{
			_tm.SetProc(ship);
			_tm.Activate(Time + _waitTime);
		}

		ship->Passivate();

	}


	// Delete self from Queue
	/*if(ship->getDirection() && Q1->front() == ship)
		Q1->GetFirst();
	else if(Q2->front() == ship)
		Q2->GetFirst();
	*/

	if(ship->isInQueue())
		ship->Out();


	in = ship;
	_tm.Cancel();

}

/**
 * Funkce slouzi k simulaci propluti komorou
 */
void Chamber::PerformAction()
{
	
	if(in->getDirection() != this->_pos)
	{
		//std::cout << "Preplnuju naprazdno :(" << std::endl;
		// Potrebuju vyrovnat hladinu s lodi
		in->Wait(TIME_CLOSE_GATE  + TIME_OPEN_GATE + _fillTime);
		this->_pos = !this->_pos;
	}
	else
		;//std::cout << "Nemusim preplnovat:)" << std::endl;

	in->Wait(TIME_CLOSE_GATE  + TIME_OPEN_GATE + TIME_GO_OUT + TIME_GO_IN + _fillTime);
	
	this->_pos = !this->_pos;
}

/**
 * Funkce simlujici lod, ktera opousti plavebni komoru
 */
void Chamber::Release()
{
	in = NULL;

	if(_pos)
	{
		if(Q1->Length() > 0)
		{
			(Q1->GetFirst())->Activate();
		}
		else if(Q2->Length() > 0)
		{
			//std::cout << "ACTIVATE1" << std::endl;
			_tm.SetProc(Q2->front());
			_tm.Activate(Time + _waitTime);
			//std::cout << "ACTIVATE2" << std::endl;
		}
	}
	else
	{
		if(Q2->Length() > 0)
		{
			(Q2->GetFirst())->Activate();
		}
		else if(Q1->Length() > 0)
		{
			//std::cout << "ACTIVATE1" << std::endl;
			_tm.SetProc(Q1->front());
			_tm.Activate(Time + _waitTime);
			//std::cout << "ACTIVATE2" << std::endl;
		}
	}


	
}

/**
 * Funkce simlujici lod, ktera se pokusi proplout tunelem
 * @param	ship 	lod pokousejici se proplout tunelem
 */
void Tunnel::Seize(CargoShip *ship)
{
	// Jestli je lod ve predu -> vlozit do predni fronty a opak
	if(ship->getDirection())
	{
		Q1->Insert(ship);

		// Jestli je lodi dostatek a je smer vpohode
		if(_in == 0 && Q1->Length() == _waitFor && _pos == ship->getDirection())
		{
			ActivateQueue(Q1);
			goto goIn;
		}
		else
		{
			if(_tm.Idle())
			{
				_tm.SetQueue(Q1, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
	}
	else
	{
		Q2->Insert(ship);

		// Jestli je lodi dostatek a je smer vpohode
		if(_in == 0 && Q2->Length() == _waitFor && _pos == ship->getDirection())
		{
			ActivateQueue(Q2);
			goto goIn;
		}
		else
		{
			if(_tm.Idle())
			{
				_tm.SetQueue(Q2, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
	}

	ship->Passivate();
	//cout << "Prestal jsem cekat v case: " << (Time - test) << endl;
	if(ship->isInQueue())
		ship->Out();

goIn:
	_in++;
	_tm.Cancel();
}

/**
 * Funkce simlujici proplouvani lodi tunelem
 * @param	ship 	proplouvajici lod
 */
void Tunnel::PerformAction(CargoShip *ship)
{
	//std::cout << "PERFORM_TUNEL: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}

/**
 * Funkce simlujici vyjizdeni lodi z tunelu
 */
void Tunnel::Release()
{
	_in--;
	// Uz nikdo neni v tunelu, povolime vjeti dalsim
	if(_in == 0)
	{
		_pos = !_pos;
		if(_pos)
		{
			if(Q1->Length() >= _waitFor)
			{
				ActivateQueue(Q1, _waitFor);
			}
			else if(Q1->Length() > 0)
			{
				_tm.SetQueue(Q1, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
		else 
		{
			if(Q2->Length() >= _waitFor)
			{
				ActivateQueue(Q2, _waitFor);
			}
			else if(Q2->Length() > 0)
			{
				_tm.SetQueue(Q2, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
	}
	//std::cout << "RELEASE_TUNEL" << std::endl;
}


/**
 * Funkce simlujici pokus o zabrani lodi mostem
 * @param	ship 	proplouvajici lod
 */
void Bridge::Seize(CargoShip *ship)
{

	// Jestli je lod ve predu -> vlozit do predni fronty a opak
	if(ship->getDirection())
	{
		Q1->Insert(ship);
		// Jestli je lodi dostatek a je smer vpohode
		if(_in == 0 && Q1->Length() == _waitFor && _pos == ship->getDirection())
		{
			ActivateQueue(Q1);
			goto goIn;
		}
		else
		{
			if(_tm.Idle())
			{
				_tm.SetQueue(Q1, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
	}
	else
	{
		Q2->Insert(ship);
		// Jestli je lodi dostatek a je smer vpohode
		if(_in == 0 && Q2->Length() == _waitFor && _pos == ship->getDirection())
		{
			ActivateQueue(Q2);
			goto goIn;
		}
		else
		{
			if(_tm.Idle())
			{
				_tm.SetQueue(Q2, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
	}

	ship->Passivate();


	if(ship->isInQueue())
		ship->Out();

goIn:
	_in++;
	_tm.Cancel();
}

/**
 * Funkce simlujici proplouvani lodi tunelem
 * @param	ship 	proplouvajici lod
 */
void Bridge::PerformAction(CargoShip *ship)
{
	//if(_pos != ship->getDirection())
	_pos = ship->getDirection();
	//std::cout << "PERFORM_BRIDGE: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}

/**
 * Funkce simlujici vypluti lodi z mostu
 */
void Bridge::Release()
{
	_in--;
	// Uz nikdo neni na moste, povolime vjeti dalsim
	if(_in == 0)
	{
		_pos = !_pos;
		if(_pos)
		{
			if(Q1->Length() > _waitFor)
			{
				ActivateQueue(Q1, _waitFor);
			}
			else if(Q1->Length() > 0)
			{
				_tm.SetQueue(Q1, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
		else 
		{
			if(Q2->Length() > _waitFor)
			{
				ActivateQueue(Q2, _waitFor);
			}
			else if(Q2->Length() > 0)
			{
				_tm.SetQueue(Q2, _waitFor);
				_tm.Activate(Time + _waitTime);
			}
		}
	}
	//std::cout << "RELEASE_BRIDGE" << std::endl;
}

/**
 * Funkce aktivujici prvky ve fronte
 * @param	q 	fronta
 * @param 	max maximalni pocet aktivovanych prvku ve fronte
 */
void ActivateQueue(Queue *q, u_int max)
{
	u_int to = max > q->Length() ? q->Length() : max;
	for(u_int i = 0; i < to; i++)
		(q->GetFirst())->Activate();
}

/**
 * Funkce prevadejici std:string na const char *
 * @param	letter 	retezec string
 * @return	vraci ukazel na retezec se stejnym obsahem jako parametr letter
 */
const char *converToAscii(string letter)
{
	char * result = new char[letter.length()];
	for(u_int i = 0; i < letter.length(); i++)
	{
		result[i] = letter.at(i);
	}
	return result;
}


/**
 * Funkce simlujici chovani lodi a proplouvani jednotlivymi
 * objekty cele vodni cesty
 */
void CargoShip::Behavior()
{
	Histogram * table = NULL;


	//cout << "VyJEL JSEM" << endl;
	auto Prichod = Time;

	Structure *struc;
	int next;

	for(auto &item : ShipTables)
	{
		if(item.first.first == _from && item.first.second == _to)
		{
			table = item.second;
		}
	}

	_cur = _from;
	while(true)
	{
		//cout << "From: " << _cur << " To: " << _to << endl;
		next = findNext(&Map, _cur, _to);
		_dir = next > _cur ? true : false;

		_cur = next;


		if(_cur < 0)
			break;

		//cout << "Next: " << next << endl;

		if(_cur >= (int)Info.size())
			return;

		struc = Info.at(_cur);


		switch(struc->getType())
		{
			case tunnel:
			{
				int prichod = Time;
				//cout << "Tunel: " << _cur << endl;
				Tunnel * tun = (Tunnel *)struc;
				Prichod = tun->Start();
//cout << "Tunel3" << endl;
				tun->Seize(this);
				//cout << "Tunel4" << endl;
				tun->PerformAction(this);
				tun->Release();
//cout << "Tunel4" << endl;
				tun->End(Prichod);
				//cout << "Tunel2" << endl;
				Tunnels(Time - prichod);
				break;
			}
			case bridge:
			{
				int prichod = Time;
				//cout << "Most1" << endl;
				Bridge * brid = (Bridge *)struc;
				Prichod = brid->Start();

				brid->Seize(this);
				brid->PerformAction(this);
				brid->Release();

				brid->End(Prichod);
				//cout << "Most2" << endl;
				Bridges(Time - prichod);
				break;
			}
			case chamber:
			{
				int prichod = Time;
				//cout << "Komora1" << endl;

				Chamber * cham = (Chamber *)struc;
				Prichod = cham->Start();

				cham->Seize(this); // start of service 
				cham->PerformAction();
				cham->Release(); // end of service 

				cham->End(Prichod);
				//cout << "Komora2" << endl;
				Chambers(Time - prichod);
				break;
			}
			case port:
			{
				//Port * port = (Port *)struc;

				if(_cur == _to)
				{
					if(table != NULL)
						(*table)(Time-getArrivedTime()); 
					return;
				}
				break;
			}
			case channel:
			{
				//cout << "Kanal1" << endl;
				int prichod = Time;
				Channel * chan = (Channel *)struc;

				chan->PerformAction(this);
				Cannals(Time - prichod);
				//cout << "Kanal2" << endl;
				break;
			}
			case river:
			{
				//cout << "Reka1" << endl;
				int prichod = Time;
				River * riv = (River *)struc;

				riv->PerformAction(this);
				Rivers(Time - prichod);
				//cout << "Reka2" << endl;
				break;
			}
			default:
				break;
		}

		TraveledDistance += struc->getLength();
	}
	
}
