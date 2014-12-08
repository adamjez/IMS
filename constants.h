///////////////////////
//	IMS projekt
//	Autor: xjezad00
///////////////////////

#ifndef CONSTANTS_H
#define CONSTANTS_H

////////////////////////////////////////////
// NEED TO REVISION VALUES AND DOCUMENT THEM
////////////////////////////////////////////

const int TIME_CLOSE_GATE = 60; 
const int TIME_OPEN_GATE = 30;

const int TIME_GO_IN = 516; 
const int TIME_GO_OUT = 355;

const float DIFF_CHAMBER_METER = 12.5; // Rozdil mezi velkou a malou komoru


//const int LENGTH_CHAMBER = 190;		// Studie z roku 2006 str. 44
const int DIFF_TUNNEL_METER = 3470; // Hranice, kdy se zacne pouzivat seskupovani
const int DIFF_BRIDGE_METER = 2090; // Hranice, kdy se zacne pouzivat seskupovani

const float TIME_METER_MEDIUM_CHAMBER = 40; 	// 40 sec/m
const float TIME_METER_LARGE_CHAMBER = 25.45; 	// 25.45 sec/m

const float SPEED_IN_BRIDGE = 2.77; 	// 2.22 m/s -> 10 Km/h
const float SPEED_IN_TUNNEL = 2.22; 	// 2.22 m/s -> 8 Km/h
const float SPEED_IN_CHANNEL = 3.33; 	// 3.33 m/s -> 12 Km/h
const float SPEED_WITH_STREAM = 4.16; 	// 4.16 m/s -> 15 Km/h
const float SPEED_UP_STREAM = 1.66;		// 1.66 m/s -> 6 Km/h

const int MAX_SHIP_VOLUME = 4000; // 4 000 tun

#endif