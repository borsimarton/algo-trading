#include "LearningData.h"




double LearningDataElement::getPredictor(int index) const
{
	if (index < 0 || index >= predictors.size())
	{
		cout << "error: too high index in LearningDataElement::getPredictor" << endl;
		return 0;
	}
	return predictors[index];
	
}



double LearningDataElement::operator[](int index) const
{
	if (index < 0 || index >= predictors.size())
	{
		cout << "error: wrong index in LearningDataElement::operator[]" << endl;
		return 0;
	}
	return predictors[index];
}






LearningData::LearningData(const Journal& trades) : isOriginal(true), copyCounter(new int(0))
{
	for (int i = 0; i < trades.getSize(); i++)
	{
		data.push_back(new LearningDataElement(trades[i]));
	}
}



LearningData::LearningData(const LearningData& other) : isOriginal(false), copyCounter(other.copyCounter)
{
	(*copyCounter)++;

	for (int i = 0; i < other.data.size(); i++)
	{
		data.push_back(other.data[i]);
	}
}


LearningData::LearningData(const LearningData& other, int presentTime, double scaleAt1k) : isOriginal(true), copyCounter(new int(0))
{
	double maxScale = exp(log(scaleAt1k) * static_cast<double>(other.getSize()) / 1000.0);

	int oldest = other[0].getId();
	for (int i = 1; i < other.getSize(); i++)
	{
		if (other[i].getId() < oldest)
		{
			oldest = other[i].getId();
		}
	}

	double exponent = - (log(maxScale) / static_cast<double>(presentTime - oldest));
	
	for (int i = 0; i < other.getSize(); i++)
	{
		data.push_back(new LearningDataElement(other[i].getId(), other[i].getPredictors(), other[i].getScaledResponse(presentTime, exponent)));
	}
}



LearningData::~LearningData()
{
	if (isOriginal)		//original: clear data and delete counter
	{
		if (*copyCounter == 0)		//can be deleted safely since no other copy exists
		{
			clear();
			delete copyCounter;
		}
		else
		{
			cout << "error: LearningData destructor called on original with copies still existing" << endl;
			return;
		}
	}
	else	//shallow copy: only decrease the counter
	{
		(*copyCounter)--;
	}

}


LearningData LearningData::emptyCopy() const
{
	LearningData emptyOne(*this);
	emptyOne.clear();
	return emptyOne;
}


void LearningData::clear()
{
	if (isOriginal)		//need to delete the data properly for original ones
	{
		if (*copyCounter == 0)		//can be deleted safely since no other copy exists
		{
			for (int i = 0; i < data.size(); i++)
			{
				delete data[i];
			}
		}
		else
		{
			cout << "error: LearningData::clear called on original with copies still existing" << endl;
			return;
		}
	}

	data = {};
}






bool LearningData::isElement(int searchId) const
{
	for (int i = 0; i < data.size(); i++)
	{
		if (data[i]->getId() == searchId)
		{
			return true;
		}
	}
	return false;
}




double LearningData::getIncome(int startIndex, int endIndex) const
{
	if (startIndex < 0 || endIndex > data.size() || startIndex > endIndex)
	{
		cout << "error: wrong index in LearningData::getIncome" << endl;
		return 0;
	}
	double income = 0;
	for (int i = startIndex; i < endIndex; i++)
	{
		income += data[i]->getResponse();
	}
	return income;
}




double LearningData::getIncome() const
{
	return getIncome(0, getSize());
}





double LearningData::getAverageIncome() const
{
	return getIncome() / static_cast<double>(data.size());
}






LearningData LearningData::getSubset(int startIndex, int endIndex) const
{
	if (startIndex < 0 || endIndex >= data.size() || startIndex > endIndex)
	{
		cout << "error: wrong index in LearningData::getSubset" << endl;
		return this->emptyCopy();
	}
	LearningData subset = this->emptyCopy();
	for (int i = startIndex; i < endIndex; i++)
	{
		subset.addElement(*this, i);
	}
	return subset;
}




void LearningData::addElement(const LearningData& other, int index)
{
	if (index < 0 || index >= other.getSize())
	{
		cout << "error: wrong index in LearningData::addElement" << endl;
		return;
	}

	if (isOriginal)
	{
		cout << "error: LearningData::addElement called on original instance" << endl;
		return;
	}

	if (other.copyCounter != copyCounter)
	{
		cout << "error: LearningData::addElement tries to add from a copy with different original instance" << endl;
		return;
	}

	data.push_back(&other[index]);
}


void LearningData::removeElement(int index)
{
	if (index < 0 || index >= data.size())
	{
		cout << "error: wrong index in LearningData::removeElement" << endl;
		return;
	}

	if (isOriginal && *copyCounter != 0)
	{
		cout << "error: LearningData::removeElement called on original with copies still existing" << endl;
		return;
	}

	data.erase(data.begin() + index);
}




void LearningData::sort(int predictor)
{
	if (predictor < 0 || predictor >= data[0]->getDimension())
	{
		cout << "error: wrong predictor in LearningData::quickSort" << endl;
		return;
	}
	quickSort(predictor, 0, getSize() - 1);
}




void LearningData::quickSort(int predictor, int low, int high)
{
	if (low < high)
	{
		int partitionIndex = partition(predictor, low, high);

		quickSort(predictor, low, partitionIndex - 1);
		quickSort(predictor, partitionIndex + 1, high);
	}
}




int LearningData::partition(int predictor, int low, int high)
{
	double pivot = data[high]->getPredictor(predictor);
	int pivotIndex = low;

	for (int i = low; i < high; i++)
	{
		if (data[i]->getPredictor(predictor) <= pivot)
		{
			swap(data[i], data[pivotIndex]);
			pivotIndex++;
		}
	}

	swap(data[pivotIndex], data[high]);

	return pivotIndex;
}







LearningData& LearningData::operator=(const LearningData& other)
{
	if (this == &other)
	{
		return *this;
	}

	if (isOriginal && *copyCounter != 0)		//can not overwrite an original with copies still existing
	{
		cout << "error: LearningData::operator= called on original with copies still existing" << endl;
		return *this;
	}

	if (isOriginal)		//original: delete counter pointer
	{
		delete copyCounter;
	}
	else	//copy: decrement counter
	{
		(*copyCounter)--;
	}
	clear();

	//becomes copy of the other
	isOriginal = false;
	copyCounter = other.copyCounter;
	(*copyCounter)++;


	for (int i = 0; i < other.data.size(); i++)
	{
		data.push_back(other.data[i]);
	}

	return *this;
}



const LearningDataElement& LearningData::operator[](int index) const
{
	if (index < 0 || index > data.size())
	{
		cout << "error: wrong index in LearningData::operator[]" << endl;
		return *data[0];
	}
	return *data[index];
}




Journal selectTrades(const Journal& allTrades, const LearningData& selectedTrades)
{
	Journal result;
	for (int i = 0; i < selectedTrades.getSize(); i++)
	{
		for (int j = 0; j < allTrades.getSize(); j++)
		{
			if (selectedTrades[i].getId() == allTrades[j].getTradeStart())
			{
				result.addElement(allTrades[j]);
			}
		}
	}
	return result;
}

