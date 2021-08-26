#include "symbolTable.h"
#include <iomanip>

void MySymbolTable::insertSymbol(TableEntry& entryT) {
	
	if (entryT.isExt) {
		if (definedGlobalSymbols.find(entryT.label) == definedGlobalSymbols.end()) {
			usedGlobalSymbols.insert(entryT.label);
		}
		table.push_back(entryT);
		return;
	}

	if (entryT.visibility == 'g') {
		if (usedGlobalSymbols.find(entryT.label) != usedGlobalSymbols.end()) {
			usedGlobalSymbols.erase(entryT.label);
		}

		if (definedGlobalSymbols.find(entryT.label) != definedGlobalSymbols.end()) {
			string msg = "Redefinition of global symbol ";
			msg += entryT.label + "!";
			throw LinkerException(msg);
		}
		else definedGlobalSymbols.insert(entryT.label);

		table.push_back(entryT);
	}
	else {
		if (entryT.id == entryT.section && entryT.label != "UND" && entryT.label != "ABS") { //section
			table.insert(table.begin() + numSections++, entryT);
		}
		else {
			table.push_back(entryT);
		}
	}
}

void MySymbolTable::createGlobalTable(unordered_map<string, int>& places)
{
	int maxAddr = 0, maxAddrSectionSize = 0;
	string name = "";
	for (auto addr : places) {
		if (addr.second > maxAddr) {
			maxAddr = addr.second;
			name = addr.first;
		}
	}

	for (unsigned i = 0; name != "" &&  i < numSections; i++) {
		if (name == table[i].label) {
			maxAddrSectionSize += table[i].size;
		}
	}

	int oldAddress = maxAddr + maxAddrSectionSize;
	bool placeAddress = false;
	for (unsigned i = 0; i < numSections; i++) {
		TableEntry newSec;
		TableEntry& sec1 = table[i];

		if (sec1.isExt) //that section has been merged already
			continue;
		int address = oldAddress;
		unordered_map<string, int>::iterator g = places.find(sec1.label);
		if (g != places.end()) {
			address = g->second;
			placeAddress = true;
		}

		newSec.id = idGlobal;
		newSec.section = idGlobal;
		newSec.label = sec1.label;
		newSec.visibility = sec1.visibility;
		newSec.value = address;
		newSec.size = sec1.size;
		newSec.isExt = true;
		newSec.fileId = 0;

		sec1.value = newSec.value;

		address = address + newSec.size;
		for (unsigned j = i + 1; j < numSections; j++) {
			TableEntry& sec2 = table[j];
			if (sec2.label == sec1.label) {
				table[j].isExt = true;//mark
				table[j].value = address;
				newSec.size += sec2.size;
				address += sec2.size;
			}
		}
		idGlobal++;
		numSectionsGlobal++;
		tableGlobal.push_back(newSec);

		if (!placeAddress) {
			oldAddress += newSec.size;
			placeAddress = false;
		}
	}

	vector<int> external;
	unordered_map<string, TableEntry> global;
	for (unsigned i = numSections; i < table.size(); i++) {
		TableEntry& symbol = table[i];
		if (symbol.isExt) {
			external.push_back(i);
			continue;
		}

		TableEntry newSym;
		string label;

		for (unsigned j = 0; j < numSections; j++) {
			TableEntry& section = table[j];
			if (section.id == symbol.section && symbol.fileId == section.fileId) {
				newSym.value = symbol.value + section.value;
				symbol.value = newSym.value;
				label = section.label;
				break;
			}
		}
		
		for (unsigned j = 0; j < numSectionsGlobal; j++) {
			if (label == table[j].label) {
				newSym.section = table[j].id;
			}
		}

		newSym.id = idGlobal;
		newSym.visibility = symbol.visibility;
		newSym.label = symbol.label;
		newSym.isExt = symbol.isExt;
		newSym.size = symbol.size;
		newSym.fileId = symbol.fileId;

		if (newSym.visibility == 'g') {
			global.insert({ newSym.label, newSym });
		}
		
		idGlobal++;
		tableGlobal.push_back(newSym);
	}

	for (unsigned i = 0; i < external.size(); i++) {
		TableEntry& extSym = table[external[i]];
		unordered_map<string, TableEntry>::iterator get = global.find(extSym.label);
		if (get != global.end()) {
			extSym.value = get->second.value;
		}
		else {
			TableEntry newSym;
			newSym = extSym;
			newSym.id = idGlobal++;
			tableGlobal.push_back(newSym);
		}
	}
}

void MySymbolTable::checkIfPlaceable()
{
		TableEntry t;
		for (unsigned i = 0; i < numSectionsGlobal; i++) {
			TableEntry& s1 = tableGlobal[i];
			unsigned startAddr = s1.value;
			unsigned endAddr = s1.value + s1.size;
			
			for (unsigned j = i + 1; j < numSectionsGlobal; j++) {
				TableEntry& s2 = tableGlobal[i];
				unsigned startAddr2 = s2.value;
				unsigned endAddr2 = s2.value + s2.size;

				if (doSectionsOverlap(startAddr, endAddr, startAddr2, endAddr2)) {
					string msg = "Sections ";
					msg += s1.label + " " + s2.label + " overlap!";
					throw LinkerException(msg);
				}
			}
		}
	
}

bool MySymbolTable::doSectionsOverlap(int start1, int end1, int start2, int end2)
{
	if (start1 < start2 && end1 < start2 || start1 > end2 && end1 > end2) {
		return true;
	}
	else {
		return false;
	}
}

void MySymbolTable::setOrdinals()
{
	for (auto it = table.begin() + sectionId; it != table.end(); it++) {
		it->id = sectionId++;
	}
}

TableEntry & MySymbolTable::getSymbol(int id, int fileId)
{
	if (id == 0) {
		return table[id];
	}
	else if (id == 1) {
		return table[id];
	}

	for (unsigned i = 0; i < table.size(); i++) {
		TableEntry& symbol = table[i];
		if (symbol.id == id && symbol.fileId == fileId) {
			return symbol;
		}
	}

	TableEntry& t =  table[0];
	return t;
}

TableEntry & MySymbolTable::getSymbolGlobal(unsigned index)
{
	return tableGlobal[index];
}

unordered_set<string> MySymbolTable::getUnknownUsedSymbols()
{
	return usedGlobalSymbols;
}

void MySymbolTable::printSymbolTable(ofstream & outputFileTxt, ofstream & outputFileBinary)
{
	outputFileTxt << setw(10) << left << hex << "id" << setw(15) << "name" << setw(10) << "section" << setw(10) << "value" << setw(12) << "visibility" << setw(10) << "is extern"<< setw(10) << "size" << endl;

	for (auto entry : table) {
		outputFileTxt << setw(10) << left << hex << entry.id << setw(15) << entry.label << setw(10) << entry.section << setw(10) << entry.value << setw(12) << entry.visibility << setw(10) << entry.isExt << setw(10) << entry.size << endl;
	}

	size_t tableSize = table.size();

	outputFileBinary.write(reinterpret_cast<char*> (&tableSize), sizeof(tableSize));
	for (auto entry : table) {
		outputFileBinary.write(reinterpret_cast<char*> (&entry.id), sizeof(entry.id));
		size_t labelSize = entry.label.size();
		outputFileBinary.write(reinterpret_cast<char*> (&labelSize), sizeof(labelSize));
		outputFileBinary.write(entry.label.c_str(), sizeof(entry.label.size()));
		outputFileBinary.write(reinterpret_cast<char*> (&entry.section), sizeof(entry.section));
		outputFileBinary.write(reinterpret_cast<char*> (&entry.value), sizeof(entry.value));
		outputFileBinary.write(reinterpret_cast<char*> (&entry.visibility), sizeof(entry.visibility));
		outputFileBinary.write(reinterpret_cast<char*> (&entry.isExt), sizeof(entry.isExt));
		outputFileBinary.write(reinterpret_cast<char*> (&entry.size), sizeof(entry.size));
	}
}

bool MySymbolTable::canBeDeclaredGlobal(TableEntry& entry)
{
	return !entry.isExt && entry.id == -1 && entry.section != 0 && entry.section != 1 && entry.visibility == 'l'; //id == -1 means that the symbol has yet to receive an id and that it is not a section
}

MySymbolTable::MySymbolTable()
{
	//insert undefined and absolute section
	table.push_back(TableEntry(sectionId++, "UND", 0, 0, 'l'));//undefined
	table.push_back(TableEntry(sectionId++, "ABS", 1, 0, 'l'));//absolute
}

MySymbolTable::~MySymbolTable()
{
}