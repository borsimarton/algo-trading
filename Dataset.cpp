#include "Dataset.h"


Dataset::Dataset(const string& fileName)
{
	ifstream myFile(fileName);
	string line;

	//count lines
	int numberOfLines = 0;

	while (getline(myFile, line))
	{
		numberOfLines++;
	}
	numberOfLines -= 2;		//first and last line

	myFile.close();


	//load data
	stringstream ss;
	char comma;
	int counter = 0;

	myFile.open(fileName);

	int time;
	double open, high, low, close, volume;

	getline(myFile, line);
	while (getline(myFile, line))
	{
		ss << line;
		ss >> time >> comma >> open >> comma >> high >> comma >> low >> comma >> close >> comma >> volume;

		candles.push_back(new Candle(time, open, high, low, close, volume));

		ss.str(string());		//clears content
		ss.clear();				//clears errors

		counter++;
		if (counter % 10000 == 0)
		{
			cout << "loading candles: " << counter << "/" << numberOfLines << endl;
		}
	}
	cout << endl;

	delete candles[candles.size() - 1];		//delete the last candle, which is still forming
	candles.pop_back();

	myFile.close();
}



Dataset::Dataset(const Dataset& other)
{
	clear();
	for (int i = 0; i < other.candles.size(); i++)
	{
		candles.push_back(new Candle(*other.candles[i]));
	}
}


void Dataset::clear()
{
	for (int i = 0; i < candles.size(); i++)
	{
		delete candles[i];
	}
	candles = {};
}




const Candle& Dataset::operator[](int index) const
{
	if (index < 0 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::operator[] const" << endl;
		return *candles[0];
	}
	return *candles[index];
}


Candle& Dataset::operator[](int index)
{
	if (index < 0 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::operator[]" << endl;
		return *candles[0];
	}
	return *candles[index];
}


int Dataset::getTimeFrameLength() const
{
	return candles[1]->getTime() - candles[0]->getTime();
}




int Dataset::timeToIndex(int time) const
{
	if (time <= candles[0]->getTime() || time > candles[candles.size() - 1]->getTime())
	{
		cout << "error: wrong index in Dataset::timeToIndex" << endl;
		return -1;
	}
	if (foundIndex(static_cast<int>(candles.size()) - 1, time))	//last candle is the one
	{
		return static_cast<int>(candles.size()) - 1;
	}

	int minIndex = 0;
	int maxIndex = static_cast<int>(candles.size()) - 1;
	int newIndex;

	while (maxIndex - minIndex > 1)
	{
		newIndex = minIndex + (maxIndex - minIndex) / 2;
		if (foundIndex(newIndex, time))	//index is found
		{
			return newIndex;
		}
		else
		{
			if (candles[newIndex]->getTime() < time)	//newIndex is the new lower boundary
			{
				minIndex = newIndex;
			}
			else //mewIndex is the new higher boundary
			{
				maxIndex = newIndex;
			}
		}
	}

	if (foundIndex(minIndex + 1, time))
	{
		return minIndex + 1;
	}
	else
	{
		cout << "some error occured in timeToIndex" << endl;
		return -1;
	}
}



bool Dataset::higherHigh(int index) const
{
	if (index < 0 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::higherHigh" << endl;
		return false;
	}

	if (candles[index]->getPriceChange() > 0)	//green candle
	{
		int movingIndex = index;

		while (candles[movingIndex]->getPriceChange() > 0)	//skip green candles
		{
			movingIndex--;
			if (movingIndex < 0)
			{
				return false;
			}
		}

		while (candles[movingIndex]->getPriceChange() < 0)	//skip red candles
		{
			movingIndex--;
			if (movingIndex < 0)
			{
				return false;
			}
		}
		return candles[movingIndex]->getClose() < candles[index]->getClose();
	}

	return false;
}


bool Dataset::lowerLow(int index) const
{
	if (index < 0 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::lowerLow" << endl;
		return false;
	}

	if (candles[index]->getPriceChange() < 0)	//red candle
	{
		int movingIndex = index;

		while (candles[movingIndex]->getPriceChange() < 0)	//skip red candles
		{
			movingIndex--;
			if (movingIndex < 0)
			{
				return false;
			}
		}

		while (candles[movingIndex]->getPriceChange() > 0)	//skip green candles
		{
			movingIndex--;
			if (movingIndex < 0)
			{
				return false;
			}
		}

		return candles[movingIndex]->getClose() > candles[index]->getClose();
	}

	return false;
}



bool Dataset::bullishOrderBlock(int candidateIndex, int currentIndex) const
{
	if (candidateIndex < 0 || candidateIndex > currentIndex || currentIndex >= candles.size())
	{
		cout << "error: wrong index in Dataset::bullishOrderBlock" << endl;
		return false;
	}

	if (candles[candidateIndex]->getPriceChange() < 0.0)
	{
		int index = candidateIndex + 1;

		while (index <= currentIndex)
		{
			if (higherHigh(index))
			{
				return true;
			}
			if (candles[index]->getPriceChange() < 0.0)
			{
				return false;
			}
			index++;
		}
	}
	return false;
}



bool Dataset::bearishOrderBlock(int candidateIndex, int currentIndex) const
{
	if (candidateIndex < 0 || candidateIndex > currentIndex || currentIndex >= candles.size())
	{
		cout << "error: wrong index in Dataset::bearishOrderBlock" << endl;
		return false;
	}

	if (candles[candidateIndex]->getPriceChange() > 0.0)
	{
		int index = candidateIndex + 1;

		while (index <= currentIndex)
		{
			if (lowerLow(index))
			{
				return true;
			}
			if (candles[index]->getPriceChange() > 0.0)
			{
				return false;
			}
			index++;
		}
	}
	return false;
}





int Dataset::intactBullishOrderBlock(int candidateIndex, int currentIndex, bool entryCorrection, bool stopCorrection) const
{

	if (candidateIndex < 0 || candidateIndex > currentIndex || currentIndex  >= candles.size())
	{
		cout << "error: wrong index in Dataset::intactBullishOrderBlock" << endl;
		return -1;
	}

	if (candles[candidateIndex]->getPriceChange() < 0 && candles[candidateIndex + 1]->getPriceChange() > 0)		//red candle followed by a green one
	{
		int breakIndex = -1;
		bool foundHigherHigh = false;
		double entryLevel = candles[candidateIndex]->getHigh();
		double stopLevel = candles[candidateIndex]->getLow();

		for (int i = candidateIndex + 1; i <= currentIndex ; i++)
		{
			if (!foundHigherHigh)	//search for higher high
			{
				if (higherHigh(i))
				{
					foundHigherHigh = true;
				}
				if (candles[i]->getPriceChange() < 0)	//red candle before higher high
				{
					return -1;
				}
			}

			if (candles[i]->getClose() < stopLevel)		//check if order block was broken (close below it)
			{
				return -1;
			}

			if (breakIndex == -1 && stopCorrection && candles[i]->getLow() < stopLevel)		//modify stop if wicked below the block
			{
				stopLevel = candles[i]->getLow();
			}

			if (breakIndex == -1 && candles[i]->getClose() > entryLevel)	//search for breakIndex
			{
				breakIndex = i;
			}

			if (breakIndex == -1 && entryCorrection && candles[i]->getHigh() > entryLevel)		//modify entry if wicked above the block
			{
				entryLevel = candles[i]->getHigh();
			}
			
		}
		return foundHigherHigh ? breakIndex : -1;
	}
	return -1;
}



int Dataset::intactBearishOrderBlock(int candidateIndex, int currentIndex, bool entryCorrection, bool stopCorrection) const
{
	if (candidateIndex < 0 || candidateIndex > currentIndex || currentIndex >= candles.size())
	{
		cout << "error: wrong index in Dataset::intactBearishOrderBlock" << endl;
		return -1;
	}

	if (candles[candidateIndex]->getPriceChange() > 0 && candles[candidateIndex + 1]->getPriceChange() < 0)		//green candle followed by a red one
	{
		int breakIndex = -1;
		bool foundLowerLow = false;
		double entryLevel = candles[candidateIndex]->getLow();
		double stopLevel = candles[candidateIndex]->getHigh();

		for (int i = candidateIndex + 1; i <= currentIndex; i++)
		{
			if (!foundLowerLow)	//search for lower low
			{
				if (lowerLow(i))
				{
					foundLowerLow = true;
				}
				if (candles[i]->getPriceChange() > 0)	//green candle before lower low
				{
					return -1;
				}
			}

			if (candles[i]->getClose() > stopLevel)		//check if order block was broken (close above it)
			{
				return -1;
			}

			if (breakIndex == -1 && stopCorrection && candles[i]->getHigh() > stopLevel)		//modify stop if wicked above the block
			{
				stopLevel = candles[i]->getHigh();
			}

			if (breakIndex == -1 && candles[i]->getClose() < entryLevel)	//search for breakIndex
			{
				breakIndex = i;
			}

			if (breakIndex == -1 && entryCorrection && candles[i]->getLow() < entryLevel)		//modify entry if wicked below the block
			{
				entryLevel = candles[i]->getLow();
			}
		}
		return foundLowerLow ? breakIndex : -1;
	}
	return -1;
}




bool Dataset::bullishSFP(int index) const
{
	if (index < 1 || index >= candles.size())
	{
		cout << "error: wrong index is Dataset::bullishSFP" << endl;
		return false;
	}
	return candles[index]->getPriceChange() > 0 && candles[index - 1]->getPriceChange() < 0 && candles[index]->getDownWick() < candles[index - 1]->getDownWick();
}



bool Dataset::bearishSFP(int index) const
{
	if (index < 1 || index >= candles.size())
	{
		cout << "error: wrong index is Dataset::bearishSFP" << endl;
		return false;
	}
	return candles[index]->getPriceChange() < 0 && candles[index - 1]->getPriceChange() > 0 && candles[index]->getUpWick() > candles[index - 1]->getUpWick();
}



bool Dataset::unbrokenLevel(double level, int startIndex, int endIndex) const
{
	if (startIndex < 0 || startIndex >= candles.size())
	{
		cout << "error: wrong startIndex in Dataset::unbrokenLevel" << endl;
		return 0;
	}
	if (endIndex < 0 || endIndex >= candles.size())
	{
		cout << "error: wrong endIndex in Dataset::unbrokenLevel" << endl;
		return 0;
	}
	if (startIndex > endIndex)
	{
		cout << "error: startIndex is higher than endIndex in Dataset::unbrokenLevel" << endl;
		return 0;
	}

	bool isAbove = candles[startIndex]->getClose() > level;

	for (int i = startIndex + 1; i <= endIndex; i++)
	{
		if (isAbove ? candles[i]->getClose() < level : candles[i]->getClose() > level)
		{
			return false;
		}
	}

	return true;
}



int Dataset::levelTests(double level, int startIndex, int endIndex) const
{
	if (startIndex <= 0 || startIndex >= candles.size())
	{
		cout << "error: wrong startIndex in Dataset::levelTests" << endl;
		return 0;
	}
	if (endIndex < 0 || endIndex >= candles.size())
	{
		cout << "error: wrong endIndex in Dataset::levelTests" << endl;
		return 0;
	}
	if (startIndex > endIndex)
	{
		cout << "error: startIndex is higher than endIndex in Dataset::levelTests" << endl;
		return 0;
	}

	int counter = 0;
	double generalizedOpen = 0.0;
	
	if (candles[startIndex]->getClose() > level)		//above level
	{
		for (int i = startIndex + 1; i <= endIndex; i++)
		{
			generalizedOpen = max(candles[i-1]->getClose(), candles[i]->getOpen());

			if ((generalizedOpen > level || !isDifferent(generalizedOpen, level)) && (candles[i]->getLow() < level || !isDifferent(candles[i]->getLow(), level)))
			{
				counter++;
			}
		}
	}
	else	//below level
	{
		for (int i = startIndex + 1; i <= endIndex; i++)
		{
			generalizedOpen = min(candles[i - 1]->getClose(), candles[i]->getOpen());

			if ((generalizedOpen < level || !isDifferent(generalizedOpen, level)) && (candles[i]->getHigh() > level || !isDifferent(candles[i]->getHigh(), level)))
			{
				counter++;
			}
		}
	}

	return counter;
}





double Dataset::getHigh(int startIndex, int endIndex) const
{
	if (startIndex < 0 || startIndex >= candles.size())
	{
		cout << "error: wrong startIndex in Dataset::getHigh" << endl;
		return 0;
	}
	if (endIndex < 0 || endIndex >= candles.size())
	{
		cout << "error: wrong endIndex in Dataset::getHigh" << endl;
		return 0;
	}
	if (startIndex > endIndex)
	{
		cout << "error: startIndex is higher than endIndex in Dataset::getHigh" << endl;
		return 0;
	}

	double high = candles[startIndex]->getHigh();
	for (int i = startIndex + 1; i <= endIndex; i++)
	{
		if (candles[i]->getHigh() > high)
		{
			high = candles[i]->getHigh();
		}
	}
	return high;
}



double Dataset::getLow(int startIndex, int endIndex) const
{
	if (startIndex < 0 || startIndex >= candles.size())
	{
		cout << "error: wrong startIndex in Dataset::getLow" << endl;
		return 0;
	}
	if (endIndex < 0 || endIndex >= candles.size())
	{
		cout << "error: wrong endIndex in Dataset::getLow" << endl;
		return 0;
	}
	if (startIndex > endIndex)
	{
		cout << "error: startIndex is higher than endIndex in Dataset::getLow" << endl;
		return 0;
	}

	double low = candles[startIndex]->getLow();
	for (int i = startIndex + 1; i <= endIndex; i++)
	{
		if (candles[i]->getLow() < low)
		{
			low = candles[i]->getLow();
		}
	}
	return low;
}



bool Dataset::bullishBreaker(int candidateIndex, int currentIndex, bool entryCorrection) const
{
	if (candidateIndex < 0 || candidateIndex >= candles.size())
	{
		cout << "error: wrong candidateIndex in Dataset::bullishBreaker" << endl;
		return false;
	}

	if (currentIndex < 0 || currentIndex >= candles.size())
	{
		cout << "error: wrong currentIndex in Dataset::bullishBreaker" << endl;
		return false;
	}

	int breakIndex = -1;
	int movingIndex = candidateIndex + 1;
	double entryLevel = candles[candidateIndex]->getHigh();

	while (movingIndex <= currentIndex && breakIndex == -1)
	{
		if (candles[movingIndex]->getClose() > entryLevel)	//check for candle breaking the level
		{
			breakIndex = movingIndex;
		}

		if (breakIndex == -1 && entryCorrection && candles[movingIndex]->getHigh() > entryLevel)
		{
			entryLevel = candles[movingIndex]->getHigh();
		}
		movingIndex++;
	}

	if (breakIndex == -1)
	{
		return false;
	}
	else
	{
		if (bearishOrderBlock(candidateIndex, movingIndex - 2))
		{
			return true;
		}
		return false;
	}
}




bool Dataset::bearishBreaker(int candidateIndex, int currentIndex, bool entryCorrection) const
{
	if (candidateIndex < 0 || candidateIndex >= candles.size())
	{
		cout << "error: wrong candidateIndex in Dataset::bearishBreaker" << endl;
		return false;
	}

	if (currentIndex < 0 || currentIndex >= candles.size())
	{
		cout << "error: wrong currentIndex in Dataset::bearishBreaker" << endl;
		return false;
	}

	int breakIndex = -1;
	int movingIndex = candidateIndex + 1;
	double entryLevel = candles[candidateIndex]->getLow();

	while (movingIndex <= currentIndex && breakIndex == -1)
	{
		if (candles[movingIndex]->getClose() < entryLevel)	//check for candle breaking the level
		{
			breakIndex = movingIndex;
		}

		if (breakIndex == -1 && entryCorrection && candles[movingIndex]->getLow() < entryLevel)
		{
			entryLevel = candles[movingIndex]->getLow();
		}
		movingIndex++;
		}

	if (breakIndex == -1)
	{
		return false;
	}
	else
	{
		if (bullishOrderBlock(candidateIndex, movingIndex - 2))
		{
			return true;
		}
		return false;
	}
}



int Dataset::intactBullishBreaker(int candidateIndex, int currentIndex, bool entryCorrection, bool stopCorrection) const
{
	if (candidateIndex < 0 || candidateIndex >= candles.size())
	{
		cout << "error: wrong candidateIndex in Dataset::intactBullishBreaker" << endl;
		return -1;
	}

	if (currentIndex < 0 || currentIndex >= candles.size())
	{
		cout << "error: wrong currentIndex in Dataset::intactBullishBreaker" << endl;
		return -1;
	}

	int breakIndex = -1;
	int movingIndex = candidateIndex + 1;
	double entryLevel = candles[candidateIndex]->getHigh();
	int obBreakIndex = -1;

	while (movingIndex <= currentIndex && breakIndex == -1)
	{
		if (candles[movingIndex]->getClose() > entryLevel)	//check for candle breaking the level
		{
			breakIndex = movingIndex;
		}

		if (breakIndex == -1 && entryCorrection && candles[movingIndex]->getHigh() > entryLevel)		//modify entry level based on wicks
		{
			entryLevel = candles[movingIndex]->getHigh();
		}
		movingIndex++;
	}

	if (breakIndex == -1)
	{
		return -1;
	}
	else
	{
		if (!bearishOrderBlock(candidateIndex, movingIndex - 2))
		{
			return -1;
		}

		double stopLevel = candles[candidateIndex]->getLow();
		if (stopCorrection && (obBreakIndex = intactBearishOrderBlock(candidateIndex, movingIndex - 2, stopCorrection, entryCorrection)) != -1)		//modify stop level based on wicks
		{
			stopLevel = getLow(candidateIndex, obBreakIndex - 1);
		}

		for (int i = movingIndex; i <= currentIndex; i++)
		{
			if (candles[i]->getClose() < stopLevel)		//level broken by a close
			{
				return -1;
			}
		}
		return breakIndex;
	}
}








int Dataset::intactBearishBreaker(int candidateIndex, int currentIndex, bool entryCorrection, bool stopCorrection) const
{
	if (candidateIndex < 0 || candidateIndex >= candles.size())
	{
		cout << "error: wrong candidateIndex in Dataset::intactBearishBreaker" << endl;
		return -1;
	}

	if (currentIndex < 0 || currentIndex >= candles.size())
	{
		cout << "error: wrong currentIndex in Dataset::intactBearishBreaker" << endl;
		return -1;
	}

	int breakIndex = -1;
	int movingIndex = candidateIndex + 1;
	double entryLevel = candles[candidateIndex]->getLow();
	int obBreakIndex = -1;

	while (movingIndex <= currentIndex && breakIndex == -1)
	{
		if (candles[movingIndex]->getClose() < entryLevel)	//check for candle breaking the level
		{
			breakIndex = movingIndex;
		}

		if (breakIndex == -1 && entryCorrection && candles[movingIndex]->getLow() < entryLevel)		//modify entry level based on wicks
		{
			entryLevel = candles[movingIndex]->getLow();
		}
		movingIndex++;
	}

	if (breakIndex == -1)
	{
		return -1;
	}
	else
	{
		if (!bullishOrderBlock(candidateIndex, movingIndex - 2))
		{
			return -1;
		}

		double stopLevel = candles[candidateIndex]->getHigh();
		if (stopCorrection && (obBreakIndex = intactBullishOrderBlock(candidateIndex, movingIndex - 2, stopCorrection, entryCorrection)) != -1)		//modify stop level based on wicks
		{
			stopLevel = getHigh(candidateIndex, obBreakIndex - 1);
		}

		for (int i = movingIndex; i <= currentIndex; i++)
		{
			if (candles[i]->getClose() > stopLevel)		//level broken by a close
			{
				return -1;
			}
		}
		return breakIndex;
	}
}












double Dataset::getSMA(int index, int length) const
{
	if (index < length - 1 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getSMA" << endl;
		return -1;
	}

	double ma = 0;
	for (int i = 0; i < length; i++)
	{
		ma += candles[index - i]->getClose();
	}
	return ma / length;
}



double Dataset::getEMA(int index, int length) const
{
	int initFactor = 5;	//initialization of the exponential smoothing

	if (index < initFactor * length || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getEMA" << endl;
		return -1;
	}

	double smoothFactor = 2 / (1 + (double)length);
	double result = candles[index - initFactor * length]->getClose();

	for (int i = 0; i < initFactor * length; i++)
	{
		result = smoothFactor * candles[index + 1 - initFactor * length + i]->getClose() + (1 - smoothFactor) * result;
	}

	return result;
}



double Dataset::getDeviation(int startIndex, int endIndex) const
{
	if (startIndex < 0 || endIndex >= candles.size() || startIndex > endIndex)
	{
		cout << "error: wrong index in Dataset::getDev" << endl;
		return 0;
	}

	int length = endIndex - startIndex + 1;
	double average = getSMA(endIndex, length);
	double dev = 0;
	for (int index = startIndex; index <= endIndex; index++)
	{
		dev += pow(candles[index]->getClose() - average, 2);
	}
	return sqrt(dev / length);
}



RSI Dataset::getRSI(int index, int length, int maLength) const
{
	int initFactor = 15;	//initialization of exponential smoothing

	if (index < initFactor * length + maLength - 1 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getRSI" << endl;
		return RSI(0, 0);
	}

	double upAverage = 0, downAverage = 0;
	double smoothFactor = 1 / (double)length;
	double line = 0;
	double ma = 0;

	for (int i = 0; i < maLength; i++)	//calculate previous rsi values for ma
	{
		if (candles[index + 1 - initFactor * length - maLength + i]->getPriceChange() > 0)
		{
			upAverage = candles[index + 1 - initFactor * length - maLength + i]->getSize();
		}
		else
		{
			downAverage = candles[index + 1 - initFactor * length - maLength  + i]->getSize();
		}

		//calculate exponential smoothing
		for (int j = 0; j < initFactor * length; j++)
		{
			if (candles[index + 2 - initFactor * length - maLength + i + j]->getPriceChange() > 0)
			{
				upAverage = smoothFactor * candles[index + 2 - initFactor * length - maLength + i + j]->getSize() 
					+ (1 - smoothFactor) * upAverage;
				downAverage = (1 - smoothFactor) * downAverage;
			}
			else
			{
				upAverage = (1 - smoothFactor) * upAverage;
				downAverage = smoothFactor * candles[index + 2 - initFactor * length - maLength + i + j]->getSize() 
					+ (1 - smoothFactor) * downAverage;
			}
		}

		//return rsi
		if (downAverage == 0)
		{
			line = 100;
		}
		else
		{
			line = 100 - 100 / (1 + upAverage / downAverage);
		}

		ma += line;
	}

	ma /= maLength;
	return RSI(line, ma);
}




BB Dataset::getBB(int index, int length, double devDistance) const
{
	if (index < 2 * length - 2 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getBB" << endl;
		return BB();
	}

	double ma = 0, dev = 0;

	//calculate ma
	ma = getSMA(index, length);

	//calculate dev
	for (int i = 0; i < length; i++)
	{
		dev += pow(abs(candles[index - i]->getClose() - ma), 2);
	}
	dev /= length;
	dev = pow(dev, 0.5);

	//return BB
	return BB(ma, ma + devDistance * dev, ma - devDistance * dev);
}



MACD Dataset::getMACD(int index, int smoothLength, int fastLength, int slowLength) const
{
	if (index < 0 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getMACD" << endl;
		return MACD(0, 0);
	}

	int initFactor = 5;

	//calculate line
	double line = getEMA(index, fastLength) - getEMA(index, slowLength);

	//calculate signal
	double smoothFactor = 2 / (1 + (double)smoothLength);
	double signal = getEMA(index - initFactor * smoothLength, fastLength) 
		- getEMA(index - initFactor * smoothLength, slowLength);

	for (int i = 0; i < initFactor * smoothLength; i++)
	{
		signal = smoothFactor * (getEMA(index + 1 - initFactor * smoothLength + i, fastLength) 
			- getEMA(index + 1 - initFactor * smoothLength + i, slowLength)) + (1 - smoothFactor) * signal;
	}

	return MACD(line, signal);
}



STOCH Dataset::getSTOCH(int index, int length, int maLength) const
{
	if (index < length -maLength - 2 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getSTOCH" << endl;
		return STOCH();
	}

	double kLine = getStochLine(index, length);
	double dLine = 0;

	for (int i = 0; i < maLength; i++)
	{
		dLine += getStochLine(index - i, length);
	}
	dLine /= maLength;

	return STOCH(kLine, dLine);
}


double Dataset::getStochLine(int index, int length) const
{
	double high = candles[index]->getHigh();
	double low = candles[index]->getLow();

	for (int i = 1; i < length; i++)
	{
		if (candles[index - i]->getHigh() > high)
		{
			high = candles[index - i]->getHigh();
		}
		if (candles[index - i]->getLow() < low)
		{
			low = candles[index - i]->getLow();
		}
	}

	return 100 * (candles[index]->getClose() - low) / (high - low);
}




RVI Dataset::getRVI(int index, int length) const
{
	if (index - length < 0 || index > candles.size())
	{
		cout << "error: wrong index in getRVI" << endl;
		return RVI(0, 0);
	}
	double line = getRviLine(index, length);
	double signal = (line + 2 * getRviLine(index - 1, length) + 2 * getRviLine(index - 2, length) + getRviLine(index - 3, length)) 
		/ 6;
	return RVI(line, signal);
}




double Dataset::getRviLine(int index, int length) const
{
	if (index < length + 4 - 2)
	{
		cout << "wrong index in getRviLine" << endl;
		return 0;
	}

	double num = 0, denom = 0;
	for (int i = index - length + 1; i <= index; i++)
	{
		num += candles[i]->getPriceChange() + 2 * candles[i - 1]->getPriceChange() + 2 * candles[i - 2]->getPriceChange() 
			+ candles[i - 3]->getPriceChange();
		denom += candles[i]->getRange() + 2 * candles[i - 1]->getRange() + 2 * candles[i - 2]->getRange() + candles[i - 3]->getRange();
	}

	return num / denom;
}



double Dataset::getOBV(int index, int length) const
{
	if (index - length + 1 < 0 || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::getOBV" << endl;
		return 0;
	}
	double sum = 0;
	for (int i = index - length + 1; i <= index; i++)
	{
		if (candles[i]->getPriceChange() > 0)
		{
			sum += candles[i]->getVolume();
		}
		if (candles[i]->getPriceChange() < 0)
		{
			sum -= candles[i]->getVolume();
		}
	}
	return sum;
}


double Dataset::getTotalVolume(int index, int length) const
{
	if (index - length + 1 < 0 || index > candles.size())
	{
		cout << "error: wrong index in Dataset::getAverageVolume" << endl;
		return 0;
	}

	double sum = 0;
	for (int i = index - length + 1; i <= index; i++)
	{
		sum += candles[i]->getVolume();
	}
	return sum;
}




double Dataset::getAverageVolume(int index, int length) const
{
	return getTotalVolume(index, length) / length;
}








int Dataset::rsiBullDiv(int index, int divRange, int length) const
{
	if (index < divRange || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::rsiBullDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() < 0 && candles[index + 1]->getPriceChange() > 0)	//low
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() < 0 && candles[i + 1]->getPriceChange() > 0)	//prev low
			{
				return candles[i]->getClose() > candles[index]->getClose()
					&& getRSI(i, length).getLine() < getRSI(index, length).getLine() ? i : -1;	//divergence
			}
		}
	}
	return -1;
}


int Dataset::rsiBearDiv(int index, int divRange, int length) const
{
	if (index < divRange || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::rsiBearDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() > 0 && candles[index + 1]->getPriceChange() < 0)	//high
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() > 0 && candles[i + 1]->getPriceChange() < 0)	//prev high
			{
				return candles[i]->getClose() < candles[index]->getClose()
					&& getRSI(i, length).getLine() > getRSI(index, length).getLine() ? i : -1;	//divergence
			}
		}
	}
	return -1;
}






int Dataset::stochBullDiv(int index, int divRange, int length, int maLength) const
{
	if (index < divRange || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::stochBullDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() < 0 && candles[index + 1]->getPriceChange() > 0)	//low
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() < 0 && candles[i + 1]->getPriceChange() > 0)	//prev low
			{
				return candles[i]->getClose() > candles[index]->getClose()
					&& getSTOCH(i, length, maLength).getKLine() < getSTOCH(index, length, maLength).getKLine() ? i : -1;	//divergence
			}
		}
	}
	return -1;
}



int Dataset::stochBearDiv(int index, int divRange, int length, int maLength) const
{
	if (index < divRange || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::stochBearDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() > 0 && candles[index + 1]->getPriceChange() < 0)	//high
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() > 0 && candles[i + 1]->getPriceChange() < 0)	//prev high
			{
				return candles[i]->getClose() < candles[index]->getClose()
					&& getSTOCH(i, length, maLength).getKLine() > getSTOCH(index, length, maLength).getKLine() ? i : -1;	//divergence
			}
		}
	}
	return -1;
}


int Dataset::rviBullDiv(int index, int divRange, int length) const
{
	if (index < divRange || index > candles.size())
	{
		cout << "error: wrong index in Dataset::rviBullDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() < 0 && candles[index + 1]->getPriceChange() > 0)	//low
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() < 0 && candles[i + 1]->getPriceChange() > 0)	//prev low
			{
				return candles[i]->getClose() > candles[index]->getClose()
					&& getRVI(i, length).getLine() < getRVI(index, length).getLine() ? i : -1;	//divergence
			}
		}
	}
	return -1;
}


int Dataset::rviBearDiv(int index, int divRange, int length) const
{
	if (index < divRange || index > candles.size())
	{
		cout << "error: wrong index in Dataset::rviBearDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() > 0 && candles[index + 1]->getPriceChange() < 0)	//high
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() > 0 && candles[i + 1]->getPriceChange() < 0)	//prev high
			{
				return candles[i]->getClose() < candles[index]->getClose()
					&& getRVI(i, length).getLine() > getRVI(index, length).getLine() ? i : -1;	//divergence
			}
		}
	}
	return -1;
}



int Dataset::obvBullDiv(int index, int divRange) const
{
	if (index < divRange || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::obvBullDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() < 0 && candles[index + 1]->getPriceChange() > 0)	//low
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() < 0 && candles[i + 1]->getPriceChange() > 0)	//prev low
			{
				return candles[i]->getClose() > candles[index]->getClose() && getOBV(index, index - i) > 0 ? i : -1;	//divergence
			}
		}
	}
	return -1;
}


int Dataset::obvBearDiv(int index, int divRange) const
{
	if (index < divRange || index >= candles.size())
	{
		cout << "error: wrong index in Dataset::obvBearDIv" << endl;
		return -1;
	}
	if (candles[index]->getPriceChange() > 0 && candles[index + 1]->getPriceChange() < 0)	//high
	{
		for (int i = index - 1; i >= index - divRange; i--)
		{
			if (candles[i]->getPriceChange() > 0 && candles[i + 1]->getPriceChange() < 0)	//prev high
			{
				return candles[i]->getClose() < candles[index]->getClose() && getOBV(index, index - i) < 0 ? i : -1;	//divergence
			}
		}
	}
	return -1;
}