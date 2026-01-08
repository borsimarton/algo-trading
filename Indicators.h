#pragma once
#include <iostream>



using namespace std;



class BB
{
	private:
		double ma;
		double upBand;
		double downBand;

	public:
		BB(double ma = 0, double upBand = 0, double downBand = 0) : ma(ma), upBand(upBand), downBand(downBand) {}
		double getMA() const { return ma; }
		double getUpBand() const { return upBand; }
		double getDownBand() const { return downBand; }
		double getRange() const { return upBand - downBand; }
};


inline ostream& operator<<(ostream& os, const BB& bb)
{
	os << "ma\t\t" << bb.getMA() << endl;
	os << "upBand\t\t" << bb.getUpBand() << endl;
	os << "downBand\t" << bb.getDownBand() << endl;
	return os;
}



class MACD
{
private:
	double line;
	double signal;

public:
	MACD(double line = 0, double signal = 0) : line(line), signal(signal) {}
	double getLine() const { return line; }
	double getSignal() const { return signal; }

	bool isAboveSignal() const { return line > signal; }
	bool isBelowSignal() const { return line < signal; }
	double difference() const { return line - signal; }

};



inline ostream& operator<<(ostream& os, const MACD& bb)
{
	os << "line\t\t" << bb.getLine() << endl;
	os << "signal\t\t" << bb.getSignal() << endl;
	return os;
}




class RSI
{
private:
	double line;
	double ma;


public:
	RSI(double line, double ma) : line(line), ma(ma) {}
	double getLine() const { return line; }
	double getMA() const { return ma; }
	bool isAbove() const { return line > ma; }
	bool isBelow() const { return line < ma; }

};


inline ostream& operator<<(ostream& os, const RSI& rsi)
{
	os << "line\t\t" << rsi.getLine() << endl;
	os << "ma\t\t" << rsi.getMA() << endl;
	return os;
}



class RVI
{
private:
	double line;
	double signal;

public:
	RVI(double line = 0, double signal = 0) : line(line), signal(signal) {}
	double getLine() const { return line; }
	double getSignal() const { return signal; }

	bool isAbove() const { return line > signal; }
	bool isBelow() const { return line < signal; }

};



inline ostream& operator<<(ostream& os, const RVI& stoch)
{
	os << "line\t\t" << stoch.getLine() << endl;
	os << "signal\t\t" << stoch.getSignal() << endl;
	return os;
}


class STOCH
{
	private:
		double kLine;
		double dLine;

	public:
		STOCH(double kLine = 0, double dLine = 0) : kLine(kLine), dLine(dLine) {}
		double getKLine() const { return kLine; }
		double getDLine() const { return dLine; }

		bool isAbove() const { return kLine > dLine; }
		bool isBelow() const { return kLine < dLine; }

};


inline ostream& operator<<(ostream& os, const STOCH& stoch)
{
	os << "%K\t" << stoch.getKLine() << endl;
	os << "%D\t" << stoch.getDLine() << endl;
	return os;
}



