///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#ifndef MODEL_H
#define MODEL_H

#include "simlib.h"
#include "constants.h"
#include <iostream>

using namespace  std;

class Timeout : public Event {
	public:
		Timeout() : Event() {}

		void Behavior(){
			if(_proc != NULL)
			{
				_proc->Activate();
				Cancel();
			}
		}
		void SetProc(Entity * proc)
		{
			_proc = proc;
		}
	private:
		Entity *_proc;
};

enum STRUCTURE {
	tunnel = 0,		// Tunel i most
	chamber,	// Plavebni komora
	port,		// Pristav
	channel,	// Kanal - stejna rychlost v obou smerech
	river,		// Ricni usek - ruzna rychlost ve smerech
	bridge,
};

// Cargo ship as process
class CargoShip : public Process {
	private:
		int _from = 0;
		int _to = 18;
		bool _dir = true;
		int _cur = 0; // TO-DO
	public:
		double Prichod; // atribute of each customer 
	public:
		bool getDirection()
		{
			return _dir;
		}
		void Behavior();
		void Timeout()
		{ 
			Activate(); 
		}
		CargoShip()
		{
			_dir = Random() > 0.5;
		}

};



// base class for all structures
class Structure {
	protected:
		std::string _name;
	public:
		virtual ~Structure() {};
		virtual int getType() = 0;
		void setName(std::string name)
		{
			_name = name;
		}

};

// Chamber as facility
class Chamber : public Facility, public Structure {
	private:
		float _height;
		int _waitTime;
		int _fillTime;
		bool _pos;
		Timeout _tm;
		CargoShip *in = NULL;
		Histogram *_table;
	public:
		Queue *Q3 = new Queue();
	public:
		Chamber(const char* name, float height, float waitTime) 
			: _height(height), _waitTime(waitTime), _pos(true) 
		{
			_table = new Histogram(name,0,1000,20); 

			if (_height > DIFF_CHAMBER_METER)
				_fillTime = _height * TIME_METER_LARGE_CHAMBER;
			else
				_fillTime = _height * TIME_METER_MEDIUM_CHAMBER;

		}

		virtual int getType()
		{
			return chamber;
		}
		virtual ~Chamber()
		{
			_table->Output();
			delete Q3;
		}

		double Start()
		{
			return Time;
			(*_table)(Time - in->Prichod);
		}

		double End(double time)
		{
			(*_table)(Time - time);
		}

		void PerformAction();
		void Seize(CargoShip *ship);
		void Release();
};



class Tunnel;
// Cargo ship as process
class ChangeDirection : public Event {
	private:
		Tunnel *_tun;
		int _interval;
	public:
		ChangeDirection(Tunnel * tunnel, int interval) 
		: Event(),_tun(tunnel), _interval(interval)
		{ }

		void Behavior();
};


// Chamber as facility
class Tunnel : public Structure, public Facility {
	private:
		float _len;
		int _waitTime;
		int _crossTime;
		bool _pos;
		long _waitTo = 0;
		bool _activated = false;
		ChangeDirection *chDir;
	public:
		Queue *Q3 = new Queue();
	public:
		Tunnel(int len, float waitTime) 
			: _len(len), _waitTime(waitTime), _pos(true) 
		{
			_crossTime = _len / SPEED_IN_TUNNEL;
	
		}

		void Activate()
		{
			chDir = new ChangeDirection(this, _waitTime + _crossTime);
			chDir->Activate();
			_activated = true;
		}

		virtual int getType()
		{
			return tunnel;
		}
		~Tunnel()
		{
			delete Q3;
			if(_activated)
				delete chDir;
		}
		void ChangeDir();
		void PerformAction(CargoShip *ship);
		void Seize(CargoShip *ship);
		void Release();
		void ActivateQueue(Queue *q);
		
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