#ifndef RELOCATIONENTRY_H
#define RELOCATIONENTRY_H

#include <string>

struct RelocationEntry {
	enum type{R_386_16 , R_386_PC16, R_386_16D , R_386_PC16D};
	RelocationEntry(){}
	RelocationEntry(int offset, type relType, int ordinal) : offset(offset), 
		relType(relType), ordinal(ordinal) {}
	
	int offset;
	type relType;
	int ordinal;
};

#endif // !RELOCATIONENTRY_H

