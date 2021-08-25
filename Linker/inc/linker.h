#ifndef LINKER_H
#define LINKER_H

#include <fstream>
#include "symbolTable.h"
#include "relocationEntry.h"
#include "SectionData.h"
#include <unordered_map>

class Linker
{
public:
	Linker(unordered_map<string, int>& places, ofstream& outputFile) : places(places), outputFile(outputFile) {}
	~Linker() {}

	void addObjectFile(ifstream& inputFile);
	void generateHex();
	void generateLinkable();
private:
	ofstream& outputFile;
	MySymbolTable table;
	vector<RelocationEntry> relocTable;
	vector<SectionData> sections;
	unordered_map<string, int>& places;
	unsigned numOfFiles = 0;

	TableEntry& findSymbol(string name);
};
#endif // !LINKER_H
