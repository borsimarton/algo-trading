/*
	STRUCTS CONTAING INFORMATION ABOUT THE PERFORMED FIT
*/

#pragma once
#include <iostream>



struct FitInfo		//random forest fit
{
	int tradeNumber;						//number of trades taken
	double fitIncome;						//total income
	int forestSize;							//number of trees in the forest
	vector<double> ordinaryImprovements;	//average improvement brought by predictors at splits
	vector<double> forcedImprovements;		//average improvement brought by predictors at forced splits (see RandomForest.h for details)

	FitInfo(int tradeNumber = 0, double fitIncome = 0, int forestSize = 0, vector<double> ordinaryImprovements = {}, vector<double> forcedImprovements = {}) : tradeNumber(tradeNumber),
		fitIncome(fitIncome), forestSize(forestSize), ordinaryImprovements(ordinaryImprovements), forcedImprovements(forcedImprovements) {}
};


struct DynamicalFitInfo : public FitInfo		//dynamical fit (random forest fit for each point based on preceding data)
{
	double averageIncome;	//average test income from multiple fits
	double deviation;		//deviation of test incomes from multiple fits
	bool decision;			//decision given by the best fit (if average>threshold)

	DynamicalFitInfo(const FitInfo& bestInfo = FitInfo(), double averageIncome = 0, double deviation = 0, bool decision = false) :
		FitInfo(bestInfo), averageIncome(averageIncome), deviation(deviation), decision(decision) {
	}

};


inline ostream& operator<<(ostream& os, const DynamicalFitInfo& info)
{
	os << info.averageIncome << "\t" << info.deviation << "\t" << info.fitIncome << "\t" << info.tradeNumber << "\t" << info.forestSize << "\t" << info.decision;
	return os;
}

