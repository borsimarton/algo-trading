/*
	GENERATOR OF THE TRADING JOURNAL USING THE INCLUDED STRATEGY
*/

#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "Dataset.h"
#include "Journal.h"
#include "Candle.h"
#include "ExampleStrategy.h"



using namespace std;
using namespace exampleStrategy;		//the chosen strategy providing the functions openPosition and closePosition








Journal tester(int startTime, int endTime, const vector<Dataset*>& dataVector)
{
	const Dataset& myData = *dataVector[0];		//first element is the execuation time frame (other time frames may be used by strategies)

	int startIndex = myData.timeToIndex(startTime);
	int endIndex = myData.timeToIndex(endTime);
	if (startIndex == -1 || endIndex == -1)
	{
		cout << "wrong startTime or endTime in tester" << endl;
	}

	//trading constants
	const double feeTaker = 0.0004;
	const double feeMaker = 0.0002;
	const double fundingRate = 0.0002;
	const double capital = 100000;
	const double slipRate = 0.0001;		//empirical value estimated from the order book

	Journal activeTrades;
	Journal closedTrades;
	bool finished = false;

	for (int index = startIndex; index < endIndex; index++)
	{
		//try to open a new trade
		openPosition(dataVector, index, activeTrades, feeTaker, feeMaker, capital, slipRate);

		//try to close active trades
		for (int tradeIndex = 0; tradeIndex < activeTrades.getSize(); tradeIndex++)
		{
			finished = false;

			//failed trade (can finalize here since exit and feeExit are independent of the strategy)
			if (activeTrades[tradeIndex].didFail(myData[index]))
			{
				activeTrades[tradeIndex].finalize(myData, index, activeTrades[tradeIndex].getStop(), feeTaker, fundingRate, false, capital, slipRate);
				finished = true;
			}
			else
			{
				//sucessful trade
				if (closePosition(dataVector, index, activeTrades[tradeIndex], feeTaker, feeMaker, fundingRate, capital, slipRate))
				{
					finished = true;
				}
			}

			//the position has been closed
			if (finished)
			{
				closedTrades.addElement(activeTrades[tradeIndex]);
				activeTrades.removeElement(tradeIndex);
				tradeIndex--;
			}
		}

		if ((index - startIndex + 1) % 5000 == 0)	//console communication
		{
			cout << "generating journal: " << index - startIndex + 1 << "/" << endIndex - startIndex << endl;
		}

	}

	cout << endl;


	return closedTrades;

}


