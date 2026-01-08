#include "RandomForest.h"





RandomForest::RandomForest(const RandomForest& other)
{
	for (int i = 0; i < other.getSize(); i++)
	{
		trees.push_back(new DecisionTree(*other.trees[i]));
	}
}



void RandomForest::clear()
{
	for (int i = 0; i < trees.size(); i++)
	{
		delete trees[i];
	}
	trees = {};
}


bool RandomForest::decide(const LearningDataElement& data) const
{
	int vote = 0;
	for (int i = 0; i < trees.size(); i++)
	{
		vote += trees[i]->decide(data) == true ? 1 : -1;
	}
	return vote > 0;
}






void RandomForest::addTree(const LearningData& trainingData, const Metaparameters& metaparameters, vector<double>& ordinaryImprovements, vector<double>& forcedImprovements)
{


	//BUILD TREE
	int stepSize = max(1, static_cast<int>(metaparameters.stepFraction * trainingData.getSize()));
	int minNodeSize = roundDouble(metaparameters.minNodePercent * trainingData.getSize());
	DecisionTree* newTree = new DecisionTree(trainingData.getIncome() > 0 ? true : false, static_cast<int>(trainingData.getSize()), trainingData.getIncome());

	//node infos stored in vectors
	vector<LearningData> nodeData = { trainingData };				//data corresponding to node indices
	vector<double> nodeIncomes = { nodeData[0].getIncome()};		//incomes corresponding to node indices
	vector<vector<double>> splitValues = { {} };					//best split values corresponding to node indices and predictors
	vector<vector<double>> improvements = { {} };					//best split improvements corresponding to node indices and predictors
	storeBestSplits(nodeData[0], stepSize, minNodeSize, splitValues[0], improvements[0]);

	vector<int> predictors;
	int predictorNumber = max(1, roundDouble(metaparameters.predictorFraction * metaparameters.predictorList.size()));
	int currentPredictor = 0;

	double leftIncome = 0;
	double rightIncome = 0;
	int bestNode = -1;				//also used as an indicator of a successful split (nonegative value)
	int bestPredictor = -1;
	double bestSplitValue = 0;
	double bestImprovement = 0;
	double improvement = 0;
	bool finished = false;
	
	do
	{
		bestNode = -1;

		//choose predictors
		predictors = metaparameters.predictorList;
		while (predictors.size() > predictorNumber)
		{
			predictors.erase(predictors.begin() + randomInt(0, static_cast<int>(predictors.size()) - 1));
		}
		
		//look for best ordinary split
		bestImprovement = 0.0;

		for (int nodeIndex = 0; nodeIndex < newTree->getSize(); nodeIndex++)
		{
			if (nodeData[nodeIndex].getSize() >= 2 * minNodeSize)		//splittable
			{
				for (int predictorIndex = 0; predictorIndex < predictors.size(); predictorIndex++)
				{
					currentPredictor = predictors[predictorIndex];

					if (improvements[nodeIndex][currentPredictor] > bestImprovement)
					{
						bestNode = nodeIndex;
						bestPredictor = currentPredictor;
						bestSplitValue = splitValues[nodeIndex][currentPredictor];
						bestImprovement = improvements[nodeIndex][currentPredictor];
					}
				}
			}
		}
		if (bestNode != -1)		//ordinary split found
		{
			//calculate decisions, rearrange nodeData, nodeIncomes and do the split
			nodeData.push_back(trainingData.emptyCopy());		//new slots corresponding to the two new nodes
			nodeData.push_back(trainingData.emptyCopy());		//new slots corresponding to the two new nodes
			for (int i = 0; i < nodeData[bestNode].getSize(); i++)
			{
				if (nodeData[bestNode][i][bestPredictor] < bestSplitValue)
				{
					nodeData[static_cast<int>(nodeData.size()) - 2].addElement(nodeData[bestNode],i);
				}
				else
				{
					nodeData[static_cast<int>(nodeData.size() - 1)].addElement(nodeData[bestNode],i);
				}
			}
			nodeData[bestNode].clear();
			nodeIncomes.push_back(nodeData[static_cast<int>(nodeData.size()) - 2].getIncome());
			nodeIncomes.push_back(nodeData[static_cast<int>(nodeData.size()) - 1].getIncome());
			newTree->splitNode(bestNode, bestPredictor, bestSplitValue, static_cast<int>(nodeData[static_cast<int>(nodeData.size()) - 2].getSize()),
				nodeIncomes[static_cast<int>(nodeIncomes.size()) - 2], nodeIncomes[static_cast<int>(nodeIncomes.size()) - 2] > 0 ? true : false,
				nodeIncomes[static_cast<int>(nodeIncomes.size()) - 1] > 0 ? true : false);

			ordinaryImprovements[bestPredictor] += bestImprovement;


		}
		
		else		//no ordinary split was found, try to force one based on average income improvement (less frequent, so it is not calculated in advance)
		{
			bestImprovement = 0;
			for (int nodeIndex = 0; nodeIndex < newTree->getSize(); nodeIndex++)
			{
				if (nodeData[nodeIndex].getSize() >= 2 * minNodeSize)		//check if there is enough data for a split
				{

					if (nodeIncomes[nodeIndex] < 0)		//only decision=false nodes
					{
						for (int predictorIndex = 0; predictorIndex < predictors.size(); predictorIndex++)
						{
							currentPredictor = predictors[predictorIndex];
							nodeData[nodeIndex].sort(currentPredictor);

							//calculate income for the two groups, that can be updated quickly
							leftIncome = nodeData[nodeIndex].getIncome(0, minNodeSize - 1);
							rightIncome = nodeIncomes[nodeIndex] - leftIncome;

							for (int splitIndex = minNodeSize; splitIndex <= nodeData[nodeIndex].getSize() - minNodeSize; splitIndex += stepSize)		//splitIndex = first index of the second group
							{
								//update incomes
								for (int movingIndex = splitIndex - stepSize; movingIndex < splitIndex; movingIndex++)
								{
									leftIncome += nodeData[nodeIndex][movingIndex].getResponse();
									rightIncome -= nodeData[nodeIndex][movingIndex].getResponse();
								}

								while (splitIndex <= nodeData[nodeIndex].getSize() - minNodeSize
									&& !isDifferent(nodeData[nodeIndex][splitIndex][currentPredictor], nodeData[nodeIndex][splitIndex - 1][currentPredictor]))	//look for first nonequal neighbors
								{
									leftIncome += nodeData[nodeIndex][splitIndex].getResponse();
									rightIncome -= nodeData[nodeIndex][splitIndex].getResponse();
									splitIndex++;
								}

								if (splitIndex <= nodeData[nodeIndex].getSize() - minNodeSize
									&& isDifferent(nodeData[nodeIndex][splitIndex][currentPredictor], nodeData[nodeIndex][splitIndex - 1][currentPredictor]))	//split is between nonequal elements
								{
									improvement = max(leftIncome / splitIndex, rightIncome / (nodeData[nodeIndex].getSize() - splitIndex)) 
										- (nodeIncomes[nodeIndex] / nodeData[nodeIndex].getSize());		//improvement of average

									if (improvement > bestImprovement)	//either the first split or the best improvement so far  -> take it!
									{
										bestNode = nodeIndex;
										bestPredictor = currentPredictor;
										bestSplitValue = (nodeData[nodeIndex][splitIndex - 1][currentPredictor] + nodeData[nodeIndex][splitIndex][currentPredictor]) / 2;
										bestImprovement = improvement;
									}
								}
							}
						}
					}
				}
			}

			if (bestNode != -1)		//forced split found
			{
				//rearrange nodeData, nodeIncomes and do the split
				nodeData.push_back(trainingData.emptyCopy());		//new slots corresponding to the two new nodes
				nodeData.push_back(trainingData.emptyCopy());		//new slots corresponding to the two new nodes
				for (int i = 0; i < nodeData[bestNode].getSize(); i++)
				{
					if (nodeData[bestNode][i][bestPredictor] < bestSplitValue)
					{
						nodeData[static_cast<int>(nodeData.size()) - 2].addElement(nodeData[bestNode],i);
					}
					else
					{
						nodeData[static_cast<int>(nodeData.size()) - 1].addElement(nodeData[bestNode],i);
					}
				}
				nodeData[bestNode].clear();
				nodeIncomes.push_back(nodeData[static_cast<int>(nodeData.size()) - 2].getIncome());
				nodeIncomes.push_back(nodeData[static_cast<int>(nodeData.size()) - 1].getIncome());
				newTree->splitNode(bestNode, bestPredictor, bestSplitValue, static_cast<int>(nodeData[static_cast<int>(nodeData.size()) - 2].getSize()),
					nodeIncomes[static_cast<int>(nodeIncomes.size()) - 2], false, false);


				forcedImprovements[bestPredictor] += bestImprovement;
			}

		}
		
		//calculate new best splits if not finished yet
		finished = bestNode == -1 || (metaparameters.maxSplitNumber != -1 && newTree->getSplitNumber() >= metaparameters.maxSplitNumber);
		if (!finished)
		{
			splitValues.push_back({});
			splitValues.push_back({});
			improvements.push_back({});
			improvements.push_back({});
			storeBestSplits(nodeData[nodeData.size() - 2], stepSize, minNodeSize, splitValues[nodeData.size() - 2], improvements[nodeData.size() - 2]);
			storeBestSplits(nodeData[nodeData.size() - 1], stepSize, minNodeSize, splitValues[nodeData.size() - 1], improvements[nodeData.size() - 1]);
		}

	} while (!finished);		//check if a new split could be found and if further splits are allowed

	trees.push_back(newTree);
}








FitInfo RandomForest::growForest(const LearningData& fullData, const ForestParameters& forestParameters, bool console, const string& fileName)
{
	LearningData testData = fullData.emptyCopy();
	LearningData trainingData = fullData.emptyCopy();
	int sampleSize = roundDouble(forestParameters.samplingFraction * static_cast<double>(fullData.getSize()));
	int randomIndex;

	int forestSize = 0;
	double deviation = 0;

	vector<int> trainingVotes(fullData.getSize(), 0);
	vector<int> testVotes(fullData.getSize(), 0);
	int trainingCounter = 0;
	int testCounter = 0;
	double trainingIncome = 0;
	double testIncome = 0;
	vector<double> lastIncomes;

	vector<double> ordinaryImprovements(fullData[0].getDimension(), 0.0);
	vector<double> forcedImprovements(fullData[0].getDimension(), 0.0);

	if (fileName != "")
	{
		ofstream incomeFile(fileName);
		incomeFile << "step\trainingTrades\ttrainingIncome\ttestTrades\ttestIncome" << endl;
		incomeFile.close();
	}
	

	//GENERATE TREES
	clear();
	while (forestSize < forestParameters.maxForestSize && (lastIncomes.size() < forestParameters.deviationLength || deviation > forestParameters.deviationThreshold))
	{

		//sample data
		testData = fullData;	
		trainingData.clear();

		while (trainingData.getSize() < sampleSize)
		{
			randomIndex = randomInt(0, static_cast<int>(testData.getSize()) - 1);
			trainingData.addElement(testData, randomIndex);
			testData.removeElement(randomIndex);
		}

		//fit new tree
		addTree(trainingData, forestParameters, ordinaryImprovements, forcedImprovements);

		//document training and test decision
		for (int tradeIndex = 0; tradeIndex < fullData.getSize(); tradeIndex++)
		{
			if (trainingData.isElement(fullData[tradeIndex].getId()))
			{
				trainingVotes[tradeIndex] += trees[trees.size() - 1]->decide(fullData[tradeIndex]) ? 1 : -1;
			}
			else
			{
				testVotes[tradeIndex] += trees[trees.size() - 1]->decide(fullData[tradeIndex]) ? 1 : -1;
			}
		}

		//error calculation
		trainingIncome = 0;
		testIncome = 0;
		trainingCounter = 0;
		testCounter = 0;
		for (int tradeIndex = 0; tradeIndex < fullData.getSize(); tradeIndex++)
		{
			if (trainingVotes[tradeIndex] > 0)
			{
				trainingIncome += fullData[tradeIndex].getResponse();
				trainingCounter++;
			}
			if (testVotes[tradeIndex] > 0)
			{
				testIncome += fullData[tradeIndex].getResponse();
				testCounter++;
			}
		}

		//save to file
		if (fileName != "")
		{
			ofstream incomeFile(fileName, ios::app);
			incomeFile << forestSize << "\t" << trainingCounter << "\t" << trainingIncome << "\t" << testCounter << "\t" << testIncome << endl;
			incomeFile.close();
		}

		//updata lastIncomes vector
		lastIncomes.push_back(testIncome);
		if (lastIncomes.size() > forestParameters.deviationLength)
		{
			lastIncomes.erase(lastIncomes.begin());
		}
		deviation = standardDeviation(lastIncomes);

		//cout communication
		if (console)
		{
			if (forestSize % 100 == 0)
			{
				cout << forestSize << " / " << forestParameters.maxForestSize << "\t\tavg: " << average(lastIncomes) << "\t\tdev: " << deviation << endl;
			}
		}
		

		forestSize++;
	}


	//IMPROVEMENTS
	for (int i = 0; i < fullData[0].getDimension(); i++)
	{
		ordinaryImprovements[i] /= forestSize;
		forcedImprovements[i] /= forestSize;
	}

	if (fileName != "")
	{
		string improvementFileName = fileName;
		improvementFileName.insert(improvementFileName.size() - 4, "-improvements");

		ofstream predictorFile(improvementFileName);
		predictorFile << "predictor\tordinary\tforced" << endl;
		for (int i = 0; i < fullData[0].getDimension(); i++)
		{
			predictorFile << i << "\t" << ordinaryImprovements[i] << "\t" << forcedImprovements[i] << endl;
		}
		predictorFile.close();
	}
	

	return FitInfo(testCounter, average(lastIncomes), this->getSize(), ordinaryImprovements, forcedImprovements);

}




void storeBestSplits(LearningData& data, int stepSize, int minNodeSize, vector<double>& splitValues, vector<double>& improvements)
{
	if (data.getSize() < 2 * minNodeSize)
	{
		return;
	}

	double leftIncome = 1.0;
	double rightIncome = 1.0;
	double improvement = 0;
	double bestImprovement = 0;
	double bestSplitValue = 0;

	for (int predictor = 0; predictor < data.getDimension(); predictor++)
	{
		bestImprovement = -1.0;		//might be left on this but exact value does not matter if negative
		data.sort(predictor);

		//initialize to pre first update state
		leftIncome = data.getIncome(0, minNodeSize - 1);
		rightIncome = data.getIncome(minNodeSize - 1, data.getSize());

		for (int splitIndex = minNodeSize; splitIndex <= data.getSize() - minNodeSize; splitIndex += stepSize)		//splitIndex = first index of the second group
		{
			//update incomes
			for (int movingIndex = splitIndex - stepSize; movingIndex < splitIndex; movingIndex++)
			{
				leftIncome += data[movingIndex].getResponse();
				rightIncome -= data[movingIndex].getResponse();
			}

			//look for the first different neighbors
			while (splitIndex <= data.getSize() - minNodeSize && !isDifferent(data[splitIndex][predictor], data[splitIndex - 1][predictor]))		
			{
				leftIncome += data[splitIndex].getResponse();
				rightIncome -= data[splitIndex].getResponse();
				splitIndex++;
			}

			if (splitIndex <= data.getSize() - minNodeSize && isDifferent(data[splitIndex][predictor], data[splitIndex - 1][predictor]))	//accept
			{
				improvement = max(leftIncome, rightIncome) - max(data.getIncome(), 0.0);

				if (improvement > bestImprovement)
				{
					bestImprovement = improvement;
					bestSplitValue = (data[splitIndex][predictor] + data[splitIndex - 1][predictor]) / 2;
				}
			}
			
		}

		splitValues.push_back(bestSplitValue);
		improvements.push_back(bestImprovement);
	}
}





















DynamicalFitInfo dynamicalFit(const LearningData& fullData, int index, const DynamicalParameters& dynamicalParameters)
{

	RandomForest* newForest = new RandomForest;
	RandomForest* bestForest = new RandomForest;
	FitInfo fitInfo;
	FitInfo bestFitInfo;
	vector<double> incomes;
	DynamicalParameters localParameters = dynamicalParameters;


	//CREATE SAMPLE
	LearningData sample = fullData.emptyCopy();

	for (int i = (dynamicalParameters.sampleSize == -1 ? 0 : index - dynamicalParameters.sampleSize); i < index; i++)
	{
		sample.addElement(fullData, i);
	}
	LearningData scaledSample(sample, fullData[index].getId(), dynamicalParameters.scaleAt1k);		//creating deep copy that is scaled

	//LOOK FOR BEST TREE 
	bestFitInfo = bestForest->growForest(scaledSample, localParameters, false);		//i=0 step to initialize best variables
	incomes.push_back(bestFitInfo.fitIncome);
	
	for (int i = 1; i < dynamicalParameters.iterations; i++)
	{
		cout << "generating statistics: iteration " << i + 1 << "/" << dynamicalParameters.iterations << endl;

		fitInfo = newForest->growForest(scaledSample, localParameters, false);
		incomes.push_back(fitInfo.fitIncome);
		if (fitInfo.fitIncome > bestFitInfo.fitIncome)
		{
			bestFitInfo = fitInfo;
			delete bestForest;
			bestForest = newForest;
			newForest = new RandomForest;
		}

	}
	cout << "fit completed" << endl << endl << endl;
	
	delete newForest;


	//CALCULATE RESULTS

	double incomeAverage = average(incomes);
	double incomeDeviation = standardDeviation(incomes);


	if (incomeAverage - dynamicalParameters.threshold * incomeDeviation > 0.0)
	{
		bool decision = bestForest->decide(fullData[index]);
		delete bestForest;
		return DynamicalFitInfo(bestFitInfo, incomeAverage, incomeDeviation, decision);
	}
	else
	{
		delete bestForest;
		return DynamicalFitInfo(bestFitInfo, incomeAverage, incomeDeviation, false);
	}

}



void dynamicalTest(const LearningData& data, const vector<int>& indices, const DynamicalParameters& dynamicalParameters, const string& fileName)
{
	DynamicalFitInfo info;
	double capital = 0.0;
	vector<double> ordinaryImprovements(data.getDimension(), 0);
	vector<double> forcedImprovements(data.getDimension(), 0);

	//fitInfo file header
	ofstream fitInfoFile(fileName);
	fitInfoFile << "id\tacceptFit\tbestNodeFraction\taverageIncome\tdeviation\tfitIncome\ttradeNumber\tforestSize\tdecision\toutcome\tcapital" << endl;
	fitInfoFile.close();

	//simulate trading
	for (int i = 0; i < indices.size(); i++)
	{
		cout << "data point " << i + 1 << "/" << indices.size() << endl;

		info = dynamicalFit(data, indices[i], dynamicalParameters);

		if (info.decision)
		{
			capital += data[indices[i]].getResponse();
		}

		for (int predictor = 0; predictor < data.getDimension(); predictor++)
		{
			ordinaryImprovements[predictor] += info.ordinaryImprovements[predictor];
			forcedImprovements[predictor] += info.forcedImprovements[predictor];
		}

		ofstream fitInfoFile(fileName, ios::app);
		fitInfoFile << data[indices[i]].getId() << "\t" << (info.averageIncome - dynamicalParameters.threshold * info.deviation > 0.0) << "\t" << info << "\t"
			<< data[indices[i]].getResponse() << "\t" << capital << endl;
		fitInfoFile.close();

	}

	//save improvements
	string improvementFileName = fileName;
	improvementFileName.insert(improvementFileName.size() - 4, "-improvements");
	ofstream improvementFile(improvementFileName);

	improvementFile << "predictor\tordinary\tforced" << endl;
	for (int predictor = 0; predictor < data.getDimension(); predictor++)
	{
		improvementFile << predictor << "\t" << ordinaryImprovements[predictor] / indices.size() << "\t" << forcedImprovements[predictor] / indices.size() << endl;
	}
	improvementFile.close();

}






void intervalTest(const Journal& trades, int startIndex, int endIndex, const DynamicalParameters& dynamicalParameters, const string& fileName)
{
	LearningData data(trades);
	vector<int> indices;
	for (int i = startIndex; i < endIndex; i++)
	{
		indices.push_back(i);
	}

	dynamicalTest(data, indices, dynamicalParameters, fileName);
}








void randomTest(const Journal& trades, const string& testSetFileName, const DynamicalParameters& dynamicalParameters, const string& infoFileName)
{
	//read test set
	vector<int> indices;
	string line;
	stringstream ss;
	int newIndex;

	ifstream testSetFile(testSetFileName);
	while (getline(testSetFile, line))
	{
		ss << line;
		ss >> newIndex;

		indices.push_back(newIndex);

		ss.str(string());		//clears content
		ss.clear();				//clears errors
	}
	testSetFile.close();
	
	LearningData data(trades);

	dynamicalTest(data, indices, dynamicalParameters, infoFileName);
}




void saveTestSet(const Journal& trades, int initializerNumber, int validatorNumber, int testSetSize, const string& fileName, const vector<string>& disjointSet)
{
	vector<int> testSet;

	vector<int> pool;
	for (int i = initializerNumber; i < trades.getSize() - validatorNumber; i++)
	{
		pool.push_back(i);
	}


	//delete indices that were used before
	int newIndex;
	vector<int> previousIndices;
	string line;
	stringstream ss;
	ifstream disjointFile;

	for (int disjointIndex = 0; disjointIndex < disjointSet.size(); disjointIndex++)
	{
		disjointFile.open(disjointSet[disjointIndex]);
		while (getline(disjointFile, line))
		{
			ss << line;
			ss >> newIndex;

			previousIndices.push_back(newIndex);

			ss.str(string());		//clears content
			ss.clear();
		}
		disjointFile.close();
	}

	for (int i = 0; i < previousIndices.size(); i++)
	{
		for (int j = 0; j < pool.size(); j++)
		{
			if (previousIndices[i] == pool[j])
			{
				pool.erase(pool.begin() + j);
			}
		}
	}

	//creat new set
	if (testSetSize > pool.size())
	{
		cout << "error: there are not enough indices in saveTestSet function" << endl;
		return;
	}
	int randomIndex;
	for (int i = 0; i < testSetSize; i++)
	{
		randomIndex = randomInt(0, static_cast<int>(pool.size()) - 1);
		testSet.push_back(pool[randomIndex]);
		pool.erase(pool.begin() + randomIndex);
	}

	sort(testSet.begin(), testSet.end());

	//save new set
	ofstream myFile(fileName);
	for (int i = 0; i < testSet.size(); i++)
	{
		myFile << testSet[i] << endl;
	}

}