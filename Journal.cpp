#include "Journal.h"




TradeInfo::TradeInfo(int tradeStart, TradeType type, double entry, double stop, double target, double risk, double leverage, Parameters* parameters, double fee, bool slippage,
	const Candle& slippageCandle, double capital, double slipRate) :
	tradeStart(tradeStart), type(type), entry(entry), stop(stop), target(target), risk(risk), leverage(leverage), parameters(parameters), exit(0), feeExit(0), tradeEnd(0), duration(0), funding(0),
	income(0), slippageLoss(0)
{
	feeEntry = risk * entry * fee / (abs(entry - stop));

	if (slippage)
	{
		if (capital == 0)
		{
			cout << "error: capital=0 in TradeInfo::TradeInfo at slippage calculation" << endl;
		}
		slippageLoss += capital * leverage * (entry * slipRate / 1000000) * risk / abs(entry - stop);		//slippage estimate based on empirical slipRate from the order book
		//slippageLoss += (slippageCandle.getHigh() - slippageCandle.getLow()) * risk * leverage * capital / (slippageCandle.getVolume() * slippageCandle.getTypicalPrice() * abs(entry - stop));		//slippage estimate based on candle size
	}
}




TradeInfo::TradeInfo(const TradeInfo& other) : type(other.type), entry(other.entry), stop(other.stop), target(other.target), feeEntry(other.feeEntry), tradeStart(other.tradeStart), 
	risk(other.risk), leverage(other.leverage), exit(other.exit), feeExit(other.feeExit), tradeEnd(other.tradeEnd), duration(other.duration), 
	funding(other.funding), income(other.income), slippageLoss(other.slippageLoss)
{
	parameters = other.parameters->clone();
}


vector<double> TradeInfo::getPredictors() const
{
	return parameters->doubleTransform();
}


void TradeInfo::setParameters(Parameters* newParameters)
{
	delete parameters;
	parameters = newParameters;
}


void TradeInfo::finalize(const Dataset& myData, int index, double exitPrice, double fee, double fundingRate, bool slippage, double capital, double slipRate)
{
	exit = exitPrice;
	tradeEnd = myData[index].getTime();
	feeExit = risk * fee * exit / (abs(entry - stop));
	duration = (tradeEnd - tradeStart) / myData.getTimeFrameLength();
	funding = (myData.getTimeFrameLength() / 3600.0) * duration * risk * myData.getSMA(myData.timeToIndex(tradeEnd), max(duration, 1)) * fundingRate / (8 * abs(entry - stop));
	if (slippage)
	{
		if (capital == 0)
		{
			cout << "error: capital=0 in TradeInfo::TradeInfo at slippage calculation" << endl;
		}
		slippageLoss += capital * leverage * (exitPrice * slipRate / 1000000) * risk / abs(entry - stop);		//slippage estimate based on empirical slipRate from the order book
		//slippageLoss += (myData[index].getHigh() - myData[index].getLow()) * risk * leverage * capital / (myData[index].getVolume() * myData[index].getTypicalPrice() * abs(entry - stop));		//slippage estimate based on candle size
	}
	income = (type == longType ? 1 : -1) * risk * (exit - entry) / abs(entry - stop) - feeEntry - feeExit - funding - slippageLoss;
}







TradeInfo TradeInfo::operator=(const TradeInfo& other)
{
	if (this != &other)
	{
		type = other.type;
		entry = other.entry;
		stop = other.stop;
		target = other.target;
		feeEntry = other.feeEntry;
		tradeStart = other.tradeStart;
		risk = other.risk;
		leverage = other.leverage;
		exit = other.exit;
		feeExit = other.feeExit;
		tradeEnd = other.tradeEnd;
		duration = other.duration;
		funding = other.funding;
		income = other.income;
		slippageLoss = other.slippageLoss;

		delete parameters;
		parameters = other.parameters->clone();
	}
	return *this;
}





ostream& operator<<(ostream& os, const TradeInfo& myTradeInfo)
{
	os << myTradeInfo.tradeStart << "\t" << myTradeInfo.type << "\t" << myTradeInfo.entry << "\t" << myTradeInfo.stop << "\t" << myTradeInfo.target 
		<< "\t" << myTradeInfo.risk << "\t" << myTradeInfo.leverage << "\t" << myTradeInfo.tradeEnd << "\t" << myTradeInfo.duration << "\t" << myTradeInfo.exit
		<< "\t" << myTradeInfo.feeEntry << "\t" << myTradeInfo.feeExit << "\t" << myTradeInfo.funding << "\t" << myTradeInfo.slippageLoss << "\t" << myTradeInfo.income;

	return os;
}







Journal::Journal(const Journal& other)
{
	for (int i = 0; i < other.trades.size(); i++)
	{
		trades.push_back(new TradeInfo(*other.trades[i]));
	}
}


void Journal::removeElement(int index)
{
	if (index >= trades.size())
	{
		cout << "error: index too high in Journal::eraseElement" << endl;
		return;
	}
	
	delete trades[index];
	trades.erase(trades.begin() + index);
}


void Journal::clear()
{
	for (int i = 0; i < trades.size(); i++)
	{
		delete trades[i];
	}
	trades = {};
}


void Journal::randomSplit(int subsetSize, const string& fileName1, const string& fileName2) const
{
	if (subsetSize > getSize())
	{
		cout << "error: subsetSize is too high in Journal::randomSplit" << endl;
	}

	Journal subset1;
	Journal subset2 = *this;
	vector<int> indices;
	
	for (int i = 0; i < subset2.getSize(); i++)
	{
		indices.push_back(i);
	}
	while (indices.size() > subsetSize)
	{
		indices.erase(indices.begin() + randomInt(0, static_cast<int>(indices.size()) - 1));
	}
	for (int i = 0; i < indices.size(); i++)
	{
		subset1.addElement(subset2[indices[i]]);
	}
	for (int i = 1; i <= indices.size(); i++)
	{
		subset2.removeElement(indices[indices.size() - i]);
	}

	saveTrades(subset1, fileName1);
	saveTrades(subset2, fileName2);
}




const TradeInfo& Journal::operator[](int index) const
{
	if (index >= trades.size())
	{
		cout << "error: index too high in Journal::operator[] const" << endl;
		return *trades[0];
	}
	return *trades[index];
}


TradeInfo& Journal::operator[](int index)
{
	if (index >= trades.size())
	{
		cout << "error: index too high in Journal::operator[]" << endl;
		return *trades[0];
	}
	return *trades[index];
}




Journal& Journal::operator=(const Journal& other)
{
	if (this == &other)
	{
		return *this;
	}

	clear();

	for (int i = 0; i < other.trades.size(); i++)
	{
		trades.push_back(new TradeInfo(*other.trades[i]));
	}

	return *this;
}


void saveTrades(const Journal& trades, const string& fileName)
{
	ofstream myFile(fileName);
	double capital = 0;

	if (trades.getSize() == 0)
	{
		myFile << "there were no trades" << endl;
		myFile.close();
		return;
	}

	myFile << "tradeStart\ttype\tentry\tstop\ttarget\trisk\tleverage\ttradeEnd\tduration\texit\tfeeEntry\tfeeExit\tfunding\tslippageLoss\tincome\tcapital\t"
		+ trades[0].getParameters()->header() << endl;
	for (int i = 0; i < trades.getSize(); i++)
	{
		myFile << trades[i];
		capital += trades[i].getIncome();
		myFile << "\t" << capital;
		myFile << "\t" + trades[i].getParameters()->dataText() << endl;
	}
	myFile.close();

}

