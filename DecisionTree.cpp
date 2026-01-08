#include "DecisionTree.h"



//NODE


void Node::treeConstructor(Node* newNode) const
{
	newNode->splitPredictor = this->splitPredictor;
	newNode->splitValue = this->splitValue;
	newNode->smallerNode = new Node(this->smallerNode->id, this->smallerNode->decision, this->smallerNode->nodeSize, this->smallerNode->nodeIncome);
	newNode->largerNode = new Node(this->largerNode->id, this->largerNode->decision, this->largerNode->nodeSize, this->largerNode->nodeIncome);

	if (!this->smallerNode->isTerminal())
	{
		this->smallerNode->treeConstructor(newNode->smallerNode);
	}
	if (!this->largerNode->isTerminal())
	{
		this->largerNode->treeConstructor(newNode->largerNode);
	}
	return;
}




void Node::clean()
{
	if (isTerminal() == true)
	{
		return;
	}

	smallerNode->clean();
	delete smallerNode;
	largerNode->clean();
	delete largerNode;

	return;
}


bool Node::decide(const LearningDataElement& data) const
{
	if (isTerminal())
	{
		return decision;
	}

	if (data[splitPredictor] < splitValue)
	{
		return smallerNode->decide(data);
	}
	else
	{
		return largerNode->decide(data);
	}

}



Node* Node::findNode(int searchId)
{
	if (id == searchId)	//found
	{
		return this;
	}

	if (isTerminal())	//terminal node
	{
		return nullptr;
	}

	Node* result = smallerNode->findNode(searchId);
	if (result != nullptr)	//found in the smaller branch
	{
		return result;
	}

	return largerNode->findNode(searchId);


}


bool Node::splitNode(int searchId, int maxId, int splitPredictor, double splitValue, int smallerNodeSize, double smallerNodeIncome, bool smallerDecision, bool largerDecision)
{
	if (id == searchId)		//create new node
	{
		if (!isTerminal())
		{
			cout << "error: tried to split a nonterminal node in Node:splitNode" << endl;
			return false;
		}

		this->splitPredictor = splitPredictor;
		this->splitValue = splitValue;
		this->smallerNode = new Node(maxId + 1, smallerDecision, smallerNodeSize, smallerNodeIncome);
		this->largerNode = new Node(maxId + 2, largerDecision, nodeSize - smallerNodeSize, nodeIncome - smallerNodeIncome);

		return true;
	}

	if (isTerminal())
	{
		return false;
	}

	//else: check the two daughter nodes
	bool result = smallerNode->splitNode(searchId, maxId, splitPredictor, splitValue, smallerNodeSize, smallerNodeIncome, smallerDecision, largerDecision);
	if (result == true)
	{
		return true;
	}

	return largerNode->splitNode(searchId, maxId, splitPredictor, splitValue, smallerNodeSize, smallerNodeIncome, smallerDecision, largerDecision);

}



void Node::saveToFile(ofstream& myFile) const
{
	myFile << id << "\t" << decision << "\t" << splitPredictor << "\t" << splitValue << "\t" << nodeSize << "\t" << nodeIncome << "\t";
	if (isTerminal())
	{
		myFile << -1 << "\t" << -1 << endl;
	}
	else
	{
		myFile << smallerNode->id << "\t" << largerNode->id << endl;
		smallerNode->saveToFile(myFile);
		largerNode->saveToFile(myFile);
	}
}




//DECISION TREE




DecisionTree::DecisionTree(const DecisionTree& other) : maxId(other.maxId), rootNode(new Node(0, other.rootNode->getDecision(), other.rootNode->getNodeSize(), other.rootNode->getNodeIncome()))
{
	other.rootNode->treeConstructor(rootNode);
}


const Node* DecisionTree::findNode(int searchId) const
{
	if (searchId > maxId)
	{
		cout << "error: searchId is too high in DecisionTree:findNode" << endl;
		return nullptr;
	}

	return rootNode->findNode(searchId);
}



bool DecisionTree::splitNode(int searchId, int splitPredictor, double splitValue, int smallerNodeSize, double smallerNodeIncome, bool smallerDecision, bool largerDecision)
{
	if (rootNode->splitNode(searchId, maxId, splitPredictor, splitValue, smallerNodeSize, smallerNodeIncome, smallerDecision, largerDecision))
	{
		maxId += 2;
		return true;
	}
	else
	{
		cout << "error: could not split the node in DecisionTree::splitNode" << endl;
		return false;
	}
}


void DecisionTree::saveToFile(const string& fileName) const
{
	ofstream myFile(fileName);
	myFile << "id\tdecision\tsplitPredictor\tsplitValue\tnodeSize\tnodeIncome\tsmallerId\tlargerId" << endl;
	rootNode->saveToFile(myFile);
	myFile.close();
}


