#ifndef LINKER_H
#define LINKER_H

#include <fstream>
#include "symbolTable.h"
#include "relocationEntry.h"
#include "SectionData.h"
#include <unordered_map>
#include <map>

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
	map<unsigned, string> code;
	unsigned numOfFiles = 0;

	void writeHexFile();
};
#endif // !LINKER_H
