# algo-trading
Test and improve trading strategies using random forest methods.

Short description of the files:

	Candle.h			representation of a single candle
	CommonFunctions.h		a few short and simple functions
	Dataset.h			container for the raw candle data with technical analysis tools
	DecisionTree.h			representation of a single binary decision tree
	ExampleStrategy.h		simple example strategy used in Tester.h
	FitInfo.h			structs containing information about the performed fit
	Indicators.h			simple indicators available in Dataset.h
	Journal.h			container for the trades generated in Tester.h
	LearningData.h			container for the machine learning data obtained from Journals
	Metaparameters.h		structs containing metaparameters
	Random.h			random number generating functions
	RandomForest.h			representation of random forests with fitter functions
	Tester.h			generator of the trading Journal using the included strategy


Standard application of the code:

	1. write a strategy file
	2. read candle data from a file into a Dataset
	3. include the strategy in the Tester and run it to obtain a Journal
	4. obtain LearningData from the Journal and use RandomForest fits to check if profitable trades can be separated
	5. use dynamical fitting to check if the model is predictive and indeed works on future setups


