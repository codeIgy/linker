#ifndef SECTIONDATA_H
#define SECTIONDATA_H

#include "relocationEntry.h"
#include <vector>
#include <string>

using namespace std;

class SectionData
{
public:
	SectionData(){}
	~SectionData(){}
	
	int fileId;
	int id;
	string label;
	int size;
	vector<RelocationEntry> relocTable;
	string data;
};

#endif
