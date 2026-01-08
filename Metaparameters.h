/*
	STRUCTS CONTAINING METAPARAMETERS
*/

#pragma once
#include <vector>


struct Metaparameters	//single tree fit
{
	double samplingFraction;		//fraction of points used for the fit (rest used for test)
	double predictorFraction;		//fraction of predictors considered at splits
	double stepFraction;			//fraction of points forming a unit step when looking for best split point
	double minNodePercent;			//minimal fraction of points that can be split off
	int maxSplitNumber;				//maximal number of splits (-1 value means no restriction)
	vector<int> predictorList;		//list of allowed predictors ids

	Metaparameters(double samplingFraction = 0, double predictorFraction = 0, double stepFraction = 0, double minNodePercent = 0, int maxSplitNumber = 0, vector<int> predictorList = {}) :
		samplingFraction(samplingFraction), predictorFraction(predictorFraction), stepFraction(stepFraction), minNodePercent(minNodePercent), maxSplitNumber(maxSplitNumber),
		predictorList(predictorList) {}

};


struct ForestParameters : public Metaparameters		//random forest fit
{
	int maxForestSize;				//maximum number of trees
	int deviationLength;			//number of previous steps used to calculate the deviation
	double deviationThreshold;		//value below which the deviation of last steps' results should fall in order to terminate process

	ForestParameters(const Metaparameters& metaparameters = Metaparameters(), int maxForestSize = 0, int deviationLength = 0, double deviationThreshold = 0) :
		Metaparameters(metaparameters), maxForestSize(maxForestSize), deviationLength(deviationLength), deviationThreshold(deviationThreshold) {}

};


struct DynamicalParameters : public ForestParameters		//dynamical fit (random forest fit for each point based on preceding data)
{
	int sampleSize;			//number of preceding data points the fit is performed on (-1 value means a fit to all previous data points)
	double threshold;		//we accept the best fit if the average test income value from the fits is higher than threshold*deviation
	int iterations;			//number of fits on a given sample 
	double scaleAt1k;		//weight used for the exponential downscaling of the 1000th datapoint (equivalent to some exponent)

	DynamicalParameters(const ForestParameters& forestParameters = ForestParameters(), int sampleSize = 0, double threshold = 0, int iterations = 0, double scaleAt1k = 0) :
		ForestParameters(forestParameters), sampleSize(sampleSize), threshold(threshold), iterations(iterations), scaleAt1k(scaleAt1k) {}

};