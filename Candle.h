/*
	REPRESENTATION OF A SINGLE CANDLE
*/

#pragma once
#include <iostream>
#include <string>
#include "CommonFunctions.h"


using namespace std;

class Candle
{
private:
	int time;
	double open;
	double high;
	double low;
	double close;
	double volume;
	



public:
	Candle(int time = 0, double open = 0, double high = 0, double low = 0, double close = 0, double volume = 0) : time(time), open(open), high(high), low(low), close(close), volume(volume) {}
	int getTime() const { return time; }
	double getOpen() const { return open; }
	double getHigh() const { return high; }
	double getLow() const { return low; }
	double getClose() const { return close; }
	double getVolume() const { return volume; }
	double getSize() const { return abs(open - close); }
	double getPriceChange() const { return close - open; }
	double getUpWick() const;
	double getDownWick() const;
	double getTypicalPrice() const;
	double getRange() const { return high - low; }
	int getDay() const { return (time / (24 * 3600) + 3) % 7; }
	bool overlap(const Candle& other) const { return !(other.high < low || !isDifferent(other.high, low) || other.low > high || !isDifferent(other.low, high)); }

};


ostream& operator<<(ostream& os, const Candle& myCandle);