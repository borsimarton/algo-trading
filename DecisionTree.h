/*
	REPRESENTATION OF A SINGLE BINARY DECISION TREE (USING TREE STRUCTURE)
*/

#pragma once
#include <fstream>
#include <iostream>
#include "LearningData.h"


class Node
{
	private:
		
		int id;
		bool decision;

		//added for testing
		int nodeSize;
		double nodeIncome;

		int splitPredictor;
		double splitValue;
		Node* smallerNode;
		Node* largerNode;


	public:

		Node(int id, bool decision, int nodeSize, double nodeIncome) : id(id), decision(decision), nodeSize(nodeSize), nodeIncome(nodeIncome), splitPredictor(-1),
			splitValue(0), smallerNode(nullptr), largerNode(nullptr) {}
		void treeConstructor(Node* newNode) const;		//recursively copies nodes (used by the tree copyconstructor)

		void clean();		//recursively deletes nodes (used by the tree destructor)

		int getId() const { return id; }
		bool getDecision() const { return decision; }
		int getSplitPredictor() const { return splitPredictor; }
		double getSplitValue() const { return splitValue; }
		int getNodeSize() const { return nodeSize; }
		double getNodeIncome() const { return nodeIncome; }
		const Node* getSmallerNode() const { return smallerNode; }
		const Node* getLargerNode() const { return largerNode; }

		bool isTerminal() const { return smallerNode == nullptr; }


		//recursive functions used by the tree class functions of the same name

		bool decide(const LearningDataElement& data) const;
		Node* findNode(int searchId);		//returns nullptr if could not find searchId
		bool splitNode(int searchId, int maxId, int splitPredictor, double splitValue, int smallerNodeSize, double smallerNodeIncome, bool smallerDecision, bool largerDecision);	//returns false if could not find searchId
		
		void saveToFile(ofstream& myFile) const;

};






class DecisionTree
{

	private:

		Node* rootNode;
		int maxId;



	public:

		DecisionTree(bool rootDecision, int nodeSize, double nodeIncome) : maxId(0) { rootNode = new Node(0, rootDecision, nodeSize, nodeIncome); }
		DecisionTree(const DecisionTree& other);
		~DecisionTree() { rootNode->clean(); }

		int getSize() const { return maxId + 1; }
		int getSplitNumber() const { return (getSize() - 1) / 2; }

		const Node* findNode(int searchId) const;
		bool decide(const LearningDataElement& data) const { return rootNode->decide(data); }
		bool splitNode(int searchId, int splitPredictor, double splitValue, int smallerNodeSize, double smallerNodeIncome, bool smallerDecision, bool largerDecision);

		void saveToFile(const string& fileName) const;



};


