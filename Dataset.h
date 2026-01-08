/*
	CONTAINER FOR THE RAW CANDLE DATA WITH TECHNICAL ANALYSIS TOOLS
*/

#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "Candle.h"
#include "Indicators.h"




using namespace std;



class Dataset
{
	private:
		
		vector<Candle*> candles;


	public:

		Dataset(const string& fileName);
		Dataset(const Dataset& other);
		~Dataset() { clear(); }

		void clear();

		int getSize() const { return static_cast<int>(candles.size()); }
		const Candle& operator[](int index) const;
		Candle& operator[](int index);
		int getTimeFrameLength() const;		//in seconds

		//return index from time
		int timeToIndex(int time) const;	//returns index of last candle opening before this point in time (or exactly at this point in time)
		bool foundIndex(int index, int time) const { return candles[index]->getTime() <= time && candles[index + 1]->getTime() > time; }	//auxiliary function for previous one

		//price action
		bool higherHigh(int index) const;
		bool lowerLow(int index) const;
		bool bullishOrderBlock(int candidateIndex, int currentIndex) const;		//low leading to higher high (currentIndex is the last one tested)
		bool bearishOrderBlock(int candidateIndex, int currentIndex) const;		//high leading to lower low (currentIndex is the last one tested)
		int intactBullishOrderBlock(int candidateIndex, int currentIndex, bool entryCorrection = false, bool stopCorrection = false) const;	//checks if block is left and not broken by close, returns index leaving the block (else -1), can modify stop/entry according to wicks
		int intactBearishOrderBlock(int candidateIndex, int currentIndex, bool entryCorrection = false, bool stopCorrection = false) const;	//checks if block is left and not broken by close, returns index leaving the block (else -1), can modify stop/entry according to wicks
		bool bullishSFP(int index) const;		//neighboring swing failure pattern
		bool bearishSFP(int index) const;		//neighboring swing failure pattern
		bool unbrokenLevel(double level, int startIndex, int endIndex) const;	//no close on the other side than the close at startIndex; endIndex is included
		int levelTests(double level, int startIndex, int endIndex) const;		//number of wicks/closes over to the other side than the close at startIndex (endIndex is included)
		double getHigh(int startIndex, int endIndex) const;			//endIndex is the last one checked
		double getLow(int startIndex, int endIndex) const;			//endIndex is the last one checked
		bool bullishBreaker(int candidateIndex, int currentIndex, bool entryCorrection) const;		//broken opposite order block
		bool bearishBreaker(int candidateIndex, int currentIndex, bool entryCorrection) const;		//broken opposite order block
		int intactBullishBreaker(int candidateIndex, int currentIndex, bool entryCorrection, bool stopCorrection) const;	//checks if block is left and not broken by close, returns index leaving the block (else -1), can modify stop/entry according to wicks
		int intactBearishBreaker(int candidateIndex, int currentIndex, bool entryCorrection, bool stopCorrection) const;	//checks if block is left and not broken by close, returns index leaving the block (else -1), can modify stop/entry according to wicks
				

		//indicators
		double getSMA(int index, int length = 9) const;
		double getEMA(int index, int length = 9) const;
		double getDeviation(int startIndex, int endIndex) const;		//endIndex is included
		RSI getRSI(int index, int length = 14, int maLength = 14) const;	//sma initiated rma (leave about 300 candles before it)
		BB getBB(int index, int length = 20, double devDistance = 2) const;	//ma is for typical price
		MACD getMACD(int index, int smoothLength = 9, int fastLength = 12, int slowLength = 26) const;
		STOCH getSTOCH(int index, int length = 14, int maLength = 3) const;
		double getStochLine(int index, int length) const;		//auxiliary function for the previous one
		RVI getRVI(int index, int length = 10) const;
		double getRviLine(int index, int length) const;			//auxiliary function for the previous one
		double getOBV(int index, int length) const;
		double getTotalVolume(int index, int length) const;
		double getAverageVolume(int index, int length) const;

		//divergences	(return the first of the two indices corresponding to the divergence)
		int rsiBullDiv(int index, int divRange, int length = 14) const;	
		int rsiBearDiv(int index, int divRange, int length = 14) const;	
		int stochBullDiv(int index, int divRange, int length = 14, int maLength = 3) const;
		int stochBearDiv(int index, int divRange, int length = 14, int maLength = 3) const;
		int rviBullDiv(int index, int divRange, int length = 14) const;
		int rviBearDiv(int index, int divRange, int length = 14) const;
		int obvBullDiv(int index, int divRange) const;
		int obvBearDiv(int index, int divRange) const;




};