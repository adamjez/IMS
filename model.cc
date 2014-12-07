///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#include "model.h"
#include "parser.h"


 // global objects: 
Histogram ShipTable("Lode", 0, 10000, 100);
vector<pair<int, int>> Map;
vector<Structure *> Info;
vector<pair<pair<int, int>, float>> Traffic;


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

int main() { // experiment description 
	Print(" Vodni cesta - SIMLIB/C++ example\n"); 
	SetOutput("model.out"); 

	ConnectionParser parser1("input/connection.tsv");
	Map = parser1.Run();

	StructuresParser parser2("input/info.tsv");
	Info = parser2.Run();


	TrafficParser parser3("input/ship-VYSOKY-2020.tsv");
	Traffic = parser3.Run();

	long sedcondsInYear = 365*24*60*60;
	//std::cout << "Pocet: " << Info.size() << std::endl;
	/*std::cout << "Hledam cestu z: " << 34 << " do: " << 30 << std::endl;
	auto test = findNext(&Map, 34, 30);
	std::cout << "Dalsi prvek je: " << test << std::endl;*/

	RandomSeed(time(0));

	Init(0,sedcondsInYear); // experiment initialization for time ONE YEAR

	for(auto &item : Traffic)
	{
		int diff = ((item.second * 1000) / 4000) / sedcondsInYear; //((Pocet tun k prevezeni) Pocet lodi)
		(new Generator(diff, item.first.first, item.first.second))->Activate(); // customer generator 
	}

	
	Run(); // simulation Box.Output(); // print of results 
	std::cout << "KONEC" << std::endl;
	
	for(auto &item : Info)
	{
		delete item;
	}
	ShipTable.Output();

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
	}

	ship->Passivate();

goIn:
	_in++;
}

void Tunnel::PerformAction(CargoShip *ship)
{
	std::cout << "PERFORM_TUNEL: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}


void Tunnel::Release()
{
	_in--;
	// Uz nikdo neni na moste, povolime vjeti dalsim
	if(_in == 0)
	{
		if(_pos && Q1->Length() > _waitFor)
		{
			ActivateQueue(Q1, _waitFor);
		}
		else if(Q2->Length() > _waitFor)
		{
			ActivateQueue(Q2, _waitFor);
		}
	}
	std::cout << "RELEASE_TUNEL" << std::endl;
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
	}

	ship->Passivate();

goIn:
	_in++;
}

void Bridge::PerformAction(CargoShip *ship)
{
	std::cout << "PERFORM_BRIDGE: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}


void Bridge::Release()
{
	_in--;
	// Uz nikdo neni na moste, povolime vjeti dalsim
	if(_in == 0)
	{
		if(_pos && Q1->Length() > _waitFor)
		{
			ActivateQueue(Q1, _waitFor);
		}
		else if(Q2->Length() > _waitFor)
		{
			ActivateQueue(Q2, _waitFor);
		}
	}
	std::cout << "RELEASE_BRIDGE" << std::endl;
}

void ActivateQueue(Queue *q, int max)
{
	for(int i = 0; i < q->Length(); i++)
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

	auto Prichod = Time;

	Structure *struc;
	int next;

	_cur = _from;
	while(true)
	{
		//cout << "From: " << _cur << " To: " << _to << endl;
		next = findNext(&Map, _cur, _to);
		_dir = next > _cur ? true : false;

		_cur = next;

		//cout << "Next: " << next << endl;
		/*if(_dir)
			getSecondByFirst(&Map, _cur, &next);
		else
			getFirstBySecond(&Map, _cur, &next);*/

		if(_cur < 0)
			break;



		if(_cur >= Info.size())
			return;

		struc = Info.at(_cur);

		switch(struc->getType())
		{
			case tunnel:
			{
				Tunnel * tun = (Tunnel *)struc;
				Prichod = tun->Start();

				tun->Seize(this);
				tun->PerformAction(this);
				tun->Release();

				tun->End(Prichod);
				break;
			}
			case bridge:
			{
				Bridge * brid = (Bridge *)struc;
				Prichod = brid->Start();

				brid->Seize(this);
				brid->PerformAction(this);
				brid->Release();

				brid->End(Prichod);
				break;
			}
			case chamber:
			{
				Chamber * cham = (Chamber *)struc;
				Prichod = cham->Start();

				cham->Seize(this); // start of service 
				cham->PerformAction();
				cham->Release(); // end of service 

				cham->End(Prichod);
				break;
			}
			case port:
			{
				Port * port = (Port *)struc;
				if(_cur == _to)
				{
					ShipTable(Time-getArrivedTime()); 
					return;
				}
				
				break;
			}
			case channel:
			{
				Channel * chan = (Channel *)struc;

				chan->PerformAction(this);
				break;
			}
			case river:
			{
				River * riv = (River *)struc;

				riv->PerformAction(this);
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
