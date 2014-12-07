///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#include "model.h"
#include "parser.h"


 // global objects: 
vector<pair<int, int>> Map;
vector<Structure *> Info;
vector<pair<pair<int, int>, float>> Traffic;
vector<pair<pair<int, int>, Histogram*>> ShipTables;

class Generator : public Event { // model of system's input 
	private:
		int _diff;
		int _place1;
		int _place2;
	public:
	Generator(int diff, int place1, int place2)
	 : _diff(diff), _place1(place1), _place2(place2)
	{}

	void Behavior() { // --- behavior specification ---
		int from, to;
		if(Random() < 0.5)
		{
			from = _place1; to = _place2;
		}
		else
		{
			from = _place2; to = _place1;
		}

		(new CargoShip(from, to))->Activate(); // new customer 

		Activate(Time+_diff); // 
	} 
}; 

int main(int argc, char** argv) { // experiment description 
	if(argc < 2)
	{
		cerr << "Zadal jste prilis malo parametru" << endl;
		cerr << "  Prvni argument musi byt nazev zdrojoveho souboru: napr. ship-VYSOKY-2050, ship-NIZKY-2020, ship-TREND-2020" << endl;
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
	std::cout << "KONEC" << std::endl;
	
	for(auto &item : Info)
	{
		delete item;
	}
	for(auto &item : ShipTables)
	{
		item.second->Output();
	}
	//ShipTable.Output();

	return 0; 
}

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


void Tunnel::Seize(CargoShip *ship)
{
	auto test = Time;
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

void Tunnel::PerformAction(CargoShip *ship)
{
	//std::cout << "PERFORM_TUNEL: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}


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

void Bridge::PerformAction(CargoShip *ship)
{
	//if(_pos != ship->getDirection())
	_pos = ship->getDirection();
	//std::cout << "PERFORM_BRIDGE: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}


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

void ActivateQueue(Queue *q, int max)
{
	int to = max > q->Length() ? q->Length() : max;
	for(int i = 0; i < to; i++)
		(q->GetFirst())->Activate();
}

const char * converToAscii(string letter)
{
	char * result = new char[letter.length()];
	for(int i = 0; i < letter.length(); i++)
	{
		result[i] = letter.at(i);
	}
	return result;
}



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

		if(_cur >= Info.size())
			return;

		struc = Info.at(_cur);

		switch(struc->getType())
		{
			case tunnel:
			{
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
				break;
			}
			case bridge:
			{
				//cout << "Most1" << endl;
				Bridge * brid = (Bridge *)struc;
				Prichod = brid->Start();

				brid->Seize(this);
				brid->PerformAction(this);
				brid->Release();

				brid->End(Prichod);
				//cout << "Most2" << endl;
				break;
			}
			case chamber:
			{
				//cout << "Komora1" << endl;
				Chamber * cham = (Chamber *)struc;
				Prichod = cham->Start();

				cham->Seize(this); // start of service 
				cham->PerformAction();
				cham->Release(); // end of service 

				cham->End(Prichod);
				//cout << "Komora2" << endl;
				break;
			}
			case port:
			{
				//cout << "Port1" << endl;
				Port * port = (Port *)struc;
				if(_cur == _to)
				{
					//cout << "DOJEL JSEM" << endl;
					if(table != NULL)
						(*table)(Time-getArrivedTime()); 
					return;
				}
				//cout << "Port2" << endl;
				break;
			}
			case channel:
			{
				//cout << "Kanal1" << endl;
				Channel * chan = (Channel *)struc;

				chan->PerformAction(this);
				//cout << "Kanal2" << endl;
				break;
			}
			case river:
			{
				//cout << "Reka1" << endl;
				River * riv = (River *)struc;

				riv->PerformAction(this);
				//cout << "Reka2" << endl;
				break;
			}
			default:
				break;
		}


	}
/*
	Prichod = Chamber.Start();
	Chamber.Seize(this); // start of service 

	Chamber.PerformAction();

	Chamber.Release(); // end of service 
	Chamber.End(Prichod);*/

	//Table1(Time-Prichod); // waiting and service time 

	/*std::cout << "Time at Seize: " << Time << std::endl;
	Prichod = Time; 

	
	Table2(Time-Prichod); // waiting and service time */
	
}
