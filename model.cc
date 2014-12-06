///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#include "model.h"
#include "parser.h"


 // global objects: 
Chamber Chamber("Cernotin", 22, 30*60); 

Histogram Table2("Tunel",0,100,20); 
Tunnel Tunnel(500, 5*60);
vector<pair<int, int>> Map;
vector<Structure *> Info;

class Generator : public Event { // model of system's input 
	void Behavior() { // --- behavior specification ---

	(new CargoShip)->Activate(); // new customer 

	Activate(Time+Exponential(1e4)); // 
} 
}; 

int main() { // experiment description 
	Print(" Vodni cesta - SIMLIB/C++ example\n"); 
	SetOutput("model.out"); 

	ConnectionParser parser1("input/connection.tsv");
	Map = parser1.Run();

	StructuresParser parser2("input/info.tsv");
	Info = parser2.Run();

	std::cout << "Hledam cestu z: " << 34 << " do: " << 30 << std::endl;
	auto test = findNext(&Map, 34, 30);
	std::cout << "Dalsi prvek je: " << test << std::endl;

	RandomSeed(time(0));

	Init(0,100000); // experiment initialization for time 0..1000 

	(new Generator)->Activate(); // customer generator 
	Run(); // simulation Box.Output(); // print of results 
	std::cout << "KONEC" << std::endl;
	
	Table2.Output();

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
			Q3->Insert(ship);

		ship->Passivate();

	}
	else if (ship->getDirection() != this->_pos)
	{
		if(ship->getDirection())
			Q1->Insert(ship);
		else
			Q3->Insert(ship);

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
	else if(Q3->front() == ship)
		Q3->GetFirst();
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
		else if(Q3->Length() > 0)
		{
			//std::cout << "ACTIVATE1" << std::endl;
			_tm.SetProc(Q3->front());
			_tm.Activate(Time + _waitTime);
			//std::cout << "ACTIVATE2" << std::endl;
		}
	}
	else
	{
		if(Q3->Length() > 0)
		{
			(Q3->GetFirst())->Activate();
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
	if(!_activated)
	{
		Activate(); // hack protoze nejde aktivovat event v konstruktoru
	}


start:
	// Jestli jeste muzu jet a lod jede ve spravnem smeru
	if(_waitTo > Time && ship->getDirection() == _pos)
		return;

	// Jestli je lod nahore -> vlozit do horni fronty a opak
	if(ship->getDirection())
		Q1->Insert(ship);
	else
		Q3->Insert(ship);

	ship->Passivate();


	goto start;

}

void Tunnel::PerformAction(CargoShip *ship)
{
	std::cout << "PERFORM_TUNEL: "<< _crossTime << std::endl;
	ship->Wait(_crossTime);
}


void Tunnel::Release()
{
	std::cout << "RELEASE_TUNEL" << std::endl;
}

void Tunnel::ActivateQueue(Queue *q)
{
	for(int i = 0; i < q->Length(); i++)
		(q->GetFirst())->Activate();
}

void Tunnel::ChangeDir()
{
	//std::cout << "MENIM DIRECTION " << std::endl;
			
			
	_pos = !_pos;

	_waitTo = _waitTime + Time;
	// Delete self from Queue
	if(_pos)
		ActivateQueue(Q1);
	else 
		ActivateQueue(Q3);

	//std::cout << "WAITTO " << _waitTo<< std::endl;

}

void ChangeDirection::Behavior()
{

	if(_tun != NULL)
		_tun->ChangeDir();

	//std::cout << "Aktivuju za: " << (Time + _interval) << std::endl;
	Activate(Time + _interval);
	
}

void CargoShip::Behavior()
{

	auto Prichod = Time;

	Structure *struc;
	int next;

	_cur = _from;
	while(true)
	{

		next = findNext(&Map, _cur, _to);
		_dir = next > _cur ? true : false;

		_cur = next;
		/*if(_dir)
			getSecondByFirst(&Map, _cur, &next);
		else
			getFirstBySecond(&Map, _cur, &next);*/

		if(_cur < 0)
			break;


		struc = Info.at(_cur);

		switch(struc->getType())
		{
			case tunnel:
			case bridge:
			{
				/*Tunnel * tun = (Tunnel *)struc;

				tun->Seize(this);

				tun->PerformAction(this);

				tun->Release();*/
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
				Chamber * cham = (Chamber *)struc;
				Port * port = (Port *)struc;
				if(_cur == _to)
				{
					Table2(Time-Prichod); // waiting and service time 
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
