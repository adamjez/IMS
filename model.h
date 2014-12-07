///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#ifndef MODEL_H
#define MODEL_H

#include "simlib.h"
#include "constants.h"
#include "limits.h"
#include <iostream>


using namespace  std;

void ActivateQueue(Queue *q, int max = INT_MAX);
const char *converToAscii(string letter);



class Timeout : public Event {
	public:
		Timeout() : Event() {}

		void Behavior(){
			if(_proc != NULL)
			{
				_proc->Activate();
				Cancel();
			}
			else if(_queue != NULL)
			{
				ActivateQueue(_queue, _max);
				Cancel();
			}
		}

		void SetQueue(Queue * que, int max)
		{
			_queue = que;
			_max = max;
		}
		void SetProc(Entity * proc)
		{
			_proc = proc;
		}
	private:
		Entity *_proc = NULL;
		Queue *_queue = NULL;
		int _max;
};

enum STRUCTURE {
	tunnel = 0,	// Tunel
	chamber,	// Plavebni komora
	port,		// Pristav
	channel,	// Kanal - stejna rychlost v obou smerech
	river,		// Ricni usek - ruzna rychlost ve smerech
	bridge,
};

// Cargo ship as process
class CargoShip : public Process {
	private:
		double _arrived = Time;
		int _from;
		int _to;
		bool _dir;
		int _cur = 0; // TO-DO
	public:
		double Prichod; // atribute of each customer 
	public:
		bool getDirection()
		{
			return _dir;
		}
		int getArrivedTime()
		{
			return _arrived;
		}
		void Behavior();
		void Timeout()
		{ 
			Activate(); 
		}
		CargoShip(int from, int to)
		 : _from(from), _to(to)
		{
			_dir = Random() < 0.5;
		}

};



// base class for all structures
class Structure {
protected:
	Histogram *_table = NULL;
	public:
		~Structure() {
			if(_table != NULL)
			{
				_table->Output();
				delete _table;
			}
			Q1->Output();
			Q2->Output();
			delete Q1;
			delete Q2;
		};
		virtual int getType() = 0;
		double Start()
		{
			return Time;
		}

		double End(double time)
		{
			(*_table)(Time - time);
		}

		Queue *Q1 = new Queue();
		Queue *Q2 = new Queue();


};

// Chamber as facility
class Chamber : public Structure {
	private:
		float _height;
		int _waitTime;
		int _fillTime;
		bool _pos;
		Timeout _tm;

		CargoShip *in = NULL;

	public:
		Chamber(string name, float height, float waitTime) 
			: _height(height), _waitTime(waitTime), _pos(Random() < 0.5) 
		{

			_table = new Histogram(converToAscii(string("Komora: " +name)),0,200,20); 

			if (_height > DIFF_CHAMBER_METER)
				_fillTime = _height * TIME_METER_LARGE_CHAMBER;
			else
				_fillTime = _height * TIME_METER_MEDIUM_CHAMBER;

		}

		virtual int getType()
		{
			return chamber;
		}



		void PerformAction();
		void Seize(CargoShip *ship);
		void Release();
};



// Tunnel
class Tunnel : public Structure {
	private:
		int _in = 0;
		float _len;
		int _waitTime;
		int _crossTime;
		bool _pos;
		long _waitFor = 0;
		Timeout _tm;

	public:
		Tunnel(string name, int len) 
			: _len(len), _pos(Random() < 0.5)
		{
			//Vytvorime histogram pro tunel
			_table = new Histogram(converToAscii(string("Tunel: " +name)),0,500,20); 
			// Doba za kterou lod prepluje tunel
			_crossTime = _len / SPEED_IN_TUNNEL;
			// Pro kolik lodi se ceka
			_waitFor = _len < DIFF_TUNNEL_METER ? 1 : 3; 

			_waitTime = 2*_crossTime * _waitFor;
		}

		virtual int getType()
		{
			return tunnel;
		}

		void ChangeDir();
		void PerformAction(CargoShip *ship);
		void Seize(CargoShip *ship);
		void Release();
		
};

// Bridge
class Bridge : public Structure {
	private:
		int _in = 0;
		float _len;
		int _waitTime;
		int _crossTime;
		bool _pos;
		long _waitFor;
		Timeout _tm;


	public:
		Bridge(string name, int len) 
			: _len(len), _pos(Random() < 0.5)
		{
			_table = new Histogram(converToAscii(string("Most: " +name)),0,200,20);

			_crossTime = _len / SPEED_IN_BRIDGE;

			_waitFor = _len < DIFF_BRIDGE_METER ? 1 : 3; 

			_waitTime = 2*_crossTime * _waitFor;
		}


		virtual int getType()
		{
			return tunnel;
		}

		void ChangeDir();
		void PerformAction(CargoShip *ship);
		void Seize(CargoShip *ship);
		void Release();
		
};

class Channel : public Structure {
	private:
		int _length;
		int _crossTime;
	public:
		Channel(int length) : _length(length)
		{
			_crossTime = length / SPEED_IN_CHANNEL;
		}
		void PerformAction(CargoShip *ship)
		{
			ship->Wait(_crossTime);
		}
		virtual int getType()
		{
			return channel;
		}
};

class Port : public Structure {
	private:
		float _prob;
	public:
		Port(float probability) : _prob(probability)
		{	}

		void PerformAction(CargoShip *ship)
		{
			;
		}

		virtual int getType()
		{
			return port;
		}
};

class River : public Structure {
	private:
		bool _dir;
		int _length;
		int _crossTimeWithStream;
		int _crossTimeUpStream;
	public:
		River(int length, bool dir) : _length(length), _dir(dir)
		{
			_crossTimeWithStream = length / SPEED_WITH_STREAM;
			_crossTimeUpStream = length / SPEED_UP_STREAM;
		}

		void PerformAction(CargoShip *ship)
		{
			if(ship->getDirection() == _dir)
				ship->Wait(_crossTimeWithStream);
			else
				ship->Wait(_crossTimeUpStream);
		}
		virtual int getType()
		{
			return channel;
		}
};






#endif