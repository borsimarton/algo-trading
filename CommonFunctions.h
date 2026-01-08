#pragma once
#include <cmath>
#include <vector>
#include <iostream>



using namespace std;



inline int roundDouble(double number)
{
	return static_cast<int>(number + 0.5);
}


inline double average(const vector<double>& data)
{
	if (data.size() == 0)
	{
		cout << "error: average function received empty vector" << endl;
		return 0;
	}

	double sum = 0;
	for (size_t i = 0; i < data.size(); i++)
	{
		sum += data[i];
	}
	return sum / static_cast<double>(data.size());
}

inline double average(const vector<int>& data)
{
	vector<double> doubleData;
	for (int i = 0; i < data.size(); i++)
	{
		doubleData.push_back(static_cast<double>(data[i]));
	}
	return average(doubleData);
}




inline double standardDeviation(const vector<double>& data)
{
	if (data.size() == 0)
	{
		cout << "error: standardDeviation function received empty vector" << endl;
		return 0;
	}

	double sum = 0;
	double dataAverage = average(data);
	for (int i = 0; i < data.size(); i++)
	{
		sum += (data[i] - dataAverage) * (data[i] - dataAverage);
	}
	return sqrt(sum / static_cast<double>(data.size()));
}


inline double standardDeviation(const vector<int>& data)
{
	vector<double> doubleData;
	for (int i = 0; i < data.size(); i++)
	{
		doubleData.push_back(static_cast<double>(data[i]));
	}
	return standardDeviation(doubleData);
}



inline bool isDifferent(double number1, double number2) { return abs(number1 - number2) > 1e-10; }	