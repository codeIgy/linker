#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include "TableEntry.h"
#include "linkerException.h"


using namespace std;

class MySymbolTable
{
public:
	MySymbolTable();
	~MySymbolTable();

	void insertSymbol(TableEntry& entry);

	void createGlobalTable(unordered_map<string, int>& places);

	//when we reach end of a section we need to update its size in the table
	void updateSectionSize(int entryNum, int size);
	
	//save symbol if it has not been already defined or declared as extern so that we can check if there are some unknown symbols 
	void markAsUsed(string label);

	//delete all rows
	void clearTable();

	//check if there are unknown symbols
	bool areAllSymbolsKnown();

	//after the first pass we need to enumerate non-section table entries (give them ids)
	void setOrdinals();

	//return symbol data
	TableEntry& getSymbol(string name);

	//acquire undefined and undeclared symbols if any
	unordered_set<string> getUnknownUsedSymbols();

	void printSymbolTable(ofstream& outputFileTxt, ofstream& outputFileBinary);

private:
	unordered_set<string> definedGlobalSymbols;
	unordered_set<string> usedGlobalSymbols;
	vector<TableEntry> table;
	vector<TableEntry> tableGlobal;
	int sectionId = 0;
	unsigned numSections = 2;
	int idGlobal = 0;;
	unsigned numSectionsGlobal = 0;

	bool canBeDeclaredGlobal(TableEntry& entry);

};

#endif

