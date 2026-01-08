/*
	REPRESENTATION OF RANDOM FORESTS

	single tree fit (addTree)
	* for each split: select random predictors, find split that brings maximal increase of total income
	* EXTRA: if no such split was found, force one on a "rejected" terminal node based on the AVERAGE income (decision assignments remain "rejected" for both)
	* stop condition: either by maximal splits or minimal node size condition
	* improvements at splits are cumulated for the predictors
	 
	forest fit (growForest)
	* fit on subset and calculate test incomes using the ignored subset of the data
	* procedure stops either at maximal number of trees of if the deviation of test incomes over the previos steps reaches a threshold
	* improvements are cumulated for the predictors
*/


#pragma once
#include <vector>
#include <fstream>
#include "Random.h"
#include "CommonFunctions.h"
#include "Journal.h"
#include "LearningData.h"
#include "DecisionTree.h"
#include "Metaparameters.h"
#include "FitInfo.h"





using namespace std;








class RandomForest
{
	private:

		vector<DecisionTree*> trees;



	public:

		RandomForest() {};
		RandomForest(const RandomForest& other);
		~RandomForest() { clear(); }

		void clear();

		int getSize() const { return static_cast<int>(trees.size()); }
		bool decide(const LearningDataElement& data) const;		//majority vote

		void addTree(const LearningData& trainingData, const Metaparameters& metaparameters, vector<double>& ordinaryImprovements, vector<double>& forcedImprovements);		//fits a single tree on the selected subset
		FitInfo growForest(const LearningData& fullData, const ForestParameters& forestParameters, bool console, const string& fileName = "");		//fits the entire forest
	
};


void storeBestSplits(LearningData& data, int stepSize, int minNodeSize, vector<double>& splitValues, vector<double>& improvements);		//auxiliary function for the addTree function, finds best splits for each node and predictor




//dynamical fitting
DynamicalFitInfo dynamicalFit(const LearningData& fullData, int index, const DynamicalParameters& dynamicalParameters);		//fits random forest on the scaled data preceding the given index (best of multiple fits)
void dynamicalTest(const LearningData& data, const vector<int>& indices, const DynamicalParameters& dynamicalParameters, const string& fileName);		//simulates trading for the given indices using dynamicalFit

void intervalTest(const Journal& trades, int startIndex, int endIndex,  const DynamicalParameters& dynamicalParameters, const string& fileName);			//dynamicalTest on the given interval (endIndex is not included)
void randomTest(const Journal& trades, const string& testSetFileName, const DynamicalParameters& dynamicalParameters, const string& infoFileName);			//dynamicalTest on previously randomly chosen test set
void saveTestSet(const Journal& trades, int initializerNumber, int validatorNumber, int sampleSize, const string& fileName, const vector<string>& disjointSet);		//creates  random test set for randomTest, that is disjoint from previously saved sets (ignores points at boundaries for validation and initialization)

