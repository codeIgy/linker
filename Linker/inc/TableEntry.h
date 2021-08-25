#ifndef TABLEENTRY_H
#define TABLEENTRY_H

#include <string>

using namespace std;

struct TableEntry
{
public:
	TableEntry(){}
	TableEntry(int id, string label, int section, int value, char visibility, bool isExt = false) : id(id), label(label), section(section),
		value(value), visibility(visibility), isExt(isExt) {}

	~TableEntry(){}

	int id = 0;
	string label = "";//label name
	int section = 0; //0 = undefined, 1 = absolute symbol, otherwise section id
	int value = 0;//label value
	char visibility = 'l';//'l' for local and 'g' for global
	bool isExt = false;//whether symbol is extern
	int size = 0;//section size
	int fileId = 0;
};
#endif // !TABLEENTRY_H
