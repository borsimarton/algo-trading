/*
	SIMPLE EXAMPLE STRATEGY
	* if a candle closes through the simple moving average line, market buy/sell
	* stop: candle open
	* target: based on fix riskreward
	 
	any strategy must include
	* a derived class of the abstract Parameters (Journal.h)
	* openPosition and closePosition functions
	* a loadTrades function that reads the journal from file and deals with the strategy dependent parameters
	*/

#pragma once
#include "Dataset.h"
#include "Journal.h"








using namespace std;


namespace exampleStrategy
{

	class ExampleParameters : public Parameters
	{
		private:

			TradeType type;
			double leverage;
			double breakRatio;		//ratio of the sizes of candle parts on the two sides of the sma line
			double rsi;				//rsi at break candle


		public:

			ExampleParameters(TradeType type, double leverage, double breakRatio, double rsi) : type(type), leverage(leverage), breakRatio(breakRatio), rsi(rsi) {}
			Parameters* clone() { return new ExampleParameters(*this); }
			vector<double> doubleTransform() const { return { type == longType ? 0.0 : 1.0, leverage, breakRatio, rsi }; }
			string header() const { return "type\tleverage\tbreakRatio\trsi"; }
			string dataText() const {return (type == longType ? string("0") : string("1")) + "\t" + to_string(leverage) + "\t" + to_string(breakRatio) + "\t" + to_string(rsi); }

	};



	//condition to open a position, specifies: type, entry, stop, target, fee, leverage, parameters AND pushes back into activeTrades
	bool openPosition(const vector<Dataset*>& dataVector, int index, Journal& activeTrades, double feeTaker, double feeMaker, double capital, double slipRate)
	{
		const Dataset& myData = *dataVector[0];
		index--;		//strategy is based on previous candle close so we shift to that candle

		//constants
		double risk = 1;
		double maxLeverage = 10.0;
		double riskReward = 1.8;

		//tradeinfo
		TradeType type;
		double entry = 0;
		double stop = 0;
		double target = 0;
		double fee = 0;
		double leverage = 0;

		//parameters
		double breakRatio = 0;
		double rsi = 0;

		//LONG case
		if (myData[index-1].getClose() < myData.getSMA(index) && myData[index].getClose() > myData.getSMA(index))	//breaks MA upwards
		{
			type = longType;
			entry = myData[index].getClose();
			stop = myData[index].getOpen();
			target = entry + riskReward * abs(entry - stop);
			fee = feeTaker;
			leverage = risk * entry / (100 * abs(entry - stop));

			if (leverage < maxLeverage)
			{
				breakRatio = abs(myData[index].getClose() - myData.getSMA(index)) / abs(myData[index].getOpen() - myData.getSMA(index));
				rsi = myData.getRSI(index).getLine();

				activeTrades.addElement(TradeInfo(myData[index + 1].getTime(), type, entry, stop, target, risk, leverage, new ExampleParameters(type, leverage, breakRatio, rsi), fee,
					true, myData[index + 1], capital, slipRate));		//note: current candle is used as slippage candle and start time (index+1)
				return true;
			}
		}

		//SHORT case
		if (myData[index - 1].getClose() > myData.getSMA(index) && myData[index].getClose() < myData.getSMA(index))	//breaks MA downwards
		{
			type = shortType;
			entry = myData[index].getClose();
			stop = myData[index].getOpen();
			target = entry - riskReward * abs(entry - stop);
			fee = feeTaker;
			leverage = risk * entry / (100 * abs(entry - stop));

			if (leverage < maxLeverage)
			{
				breakRatio = abs(myData[index].getOpen() - myData.getSMA(index)) / abs(myData[index].getClose() - myData.getSMA(index));
				rsi = 100 - myData.getRSI(index).getLine();

				activeTrades.addElement(TradeInfo(myData[index + 1].getTime(), type, entry, stop, target, risk, leverage, new ExampleParameters(type, leverage, breakRatio, rsi), fee,
					true, myData[index + 1], capital, slipRate));		//note: current candle is used as slippage candle and start time (index+1)
				return true;
			}
		}

		return false;
	}






	//condition to close a successful position, specifies: exit, feeExit
	bool closePosition(const vector<Dataset*>& dataVector, int index, TradeInfo& myTradeInfo, double feeTaker, double feeMaker, double fundingRate, double capital, double slipRate)
	{
		const Dataset& myData = *dataVector[0];

		if (myTradeInfo.getType() == longType)
		{
			if (myData[index].getHigh() > myTradeInfo.getTarget())
			{
				myTradeInfo.finalize(myData, index, myTradeInfo.getTarget(), feeMaker, fundingRate, false);
				return true;
			}
			return false;
		}
		else
		{
			if (myData[index].getLow() < myTradeInfo.getTarget())
			{
				myTradeInfo.finalize(myData, index, myTradeInfo.getTarget(), feeMaker, fundingRate, false);
				return true;
			}
			return false;
		}
	}





	Journal loadTrades(const string& fileName)
	{
		Journal trades;

		ifstream myFile(fileName);

		string line;
		stringstream ss;

		int type, tradeStart, tradeEnd, duration;
		double entry, stop, target, feeEntry, risk, leverage, exit, feeExit, funding, slippageLoss, income;
		double capital;

		double breakRatio, rsi;

		getline(myFile, line);
		while (getline(myFile, line))
		{
			ss << line;
			ss >> tradeStart >> type >> entry >> stop >> target >> risk >> leverage >> tradeEnd >> duration >> exit >> feeEntry >> feeExit >> funding >> slippageLoss >> income >> capital
				>> type >> leverage >> breakRatio >> rsi;

			trades.addElement(TradeInfo(type == 0 ? longType : shortType, entry, stop, target, feeEntry, new ExampleParameters(type == 0 ? longType : shortType, leverage, breakRatio, rsi),
				tradeStart, risk, leverage, exit, feeExit, tradeEnd, duration, funding, income, slippageLoss));

			ss.str(string());		//clears content
			ss.clear();				//clears errors
		}

		myFile.close();

		return trades;
	}





}