#include "Candle.h"



double Candle::getUpWick() const
{
	if (close >= open)
	{
		return high - close;
	}
	else
	{
		return high - open;
	}
}




double Candle::getDownWick() const
{
	if (close >= open)
	{
		return open - low;
	}
	else
	{
		return close - low;
	}
}


double Candle::getTypicalPrice() const
{
	return (close + high + low) / 3;
}




ostream& operator<<(ostream& os, const Candle& myCandle)
{
	os << "time:\t\t" << myCandle.getTime() << endl;
	os << "open:\t\t" << myCandle.getOpen() << endl;
	os << "high:\t\t" << myCandle.getHigh() << endl;
	os << "low:\t\t" << myCandle.getLow() << endl;
	os << "close:\t\t" << myCandle.getClose() << endl;
	os << "volume\t\t" << myCandle.getVolume() << endl;
	
	return os;
}