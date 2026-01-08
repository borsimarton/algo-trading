/*
	CONTAINER FOR THE MACHINE LEARNING DATA OBTAINED FROM A JOURNAL
*/

#pragma once
#include <vector>
#include "Journal.h"

using namespace std;




class LearningDataElement	//data from a single trade
{
	private:

		int id;							//given by tradeStart
		vector<double> predictors;
		double response;				//in capital percent


	public:

		LearningDataElement(int id, vector<double> predictors, double response) : id(id), predictors(predictors), response(response) {}
		LearningDataElement(const TradeInfo& tradeInfo) : id(tradeInfo.getTradeStart()), predictors(tradeInfo.getPredictors()), response(tradeInfo.getIncome()) {}
		LearningDataElement(const LearningDataElement& other) : id(other.id), predictors(other.predictors), response(other.response) {}

		int getId() const { return id; }
		double operator[](int index) const;
		const vector<double>& getPredictors() const { return predictors; }
		double getPredictor(int index) const;
		int getDimension() const { return static_cast<int>(predictors.size()); }
		double getResponse() const { return response; }
		double getScaledResponse(int presentTime, double exponent) const { return response * exp(- exponent * (presentTime - id)); }	//exponentially scale down older data points (used in dynamicFitting)


};




class LearningData
{

	private:

		vector<const LearningDataElement*> data;

		//tools necessary for shallow copies (the goal is to reduce execution time by avoiding deep copies)
		bool isOriginal;
		int* copyCounter;


		
	public:

		LearningData(const Journal& trades);		//creates original from a journal
		LearningData(const LearningData& other);	//creates shallow copy
		LearningData(const LearningData& other, int presentTime, double scaleAt1k);		//ceates an original with scaled responses
		~LearningData();			//distinct behavior is needed for original and copies

		LearningData emptyCopy() const;		
		void clear();		//distinct behavior is needed for original and copies

		int getSize() const { return static_cast<int>(data.size()); }
		int getDimension() const { return data.size() > 0 ? data[0]->getDimension() : -1; }
		bool isElement(int searchId) const;
		double getIncome(int startIndex, int endIndex) const;		//sum for the interval (endIndex is not included)
		double getIncome() const;			//sum for all data
		double getAverageIncome() const;
		LearningData getSubset(int startIndex, int endIndex) const;		//returns shallow copy (endIndex not included)
		
		void addElement(const LearningData& other, int index);		//only for not originals and other must have the same original
		void removeElement(int index);		//only for not originals

		void sort(int predictor);	//sort by the given predictor using the quicksort above
		void quickSort(int predictor, int low, int high);	//auxiliary for sort
		int partition(int predictor, int low, int high);	//auxiliary for quicksort
		
		LearningData& operator=(const LearningData& other);		//creates shallow copy (can be called on copies and originals with no existing copies)
		const LearningDataElement& operator[](int index) const;		//distinct behavior is needed for original and copies


};



Journal selectTrades(const Journal& allTrades, const LearningData& selectedTrades);			//returns trades from allTrades that are also present in selectedTrades based on id












