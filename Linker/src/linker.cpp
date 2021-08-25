#include "linker.h"
#include <iostream>

void Linker::addObjectFile(ifstream& inputFile) {
	try {
		size_t tableSize;
		inputFile.read(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
		for (unsigned i = 0; i < tableSize; i++) {
			TableEntry entry;
			inputFile.read(reinterpret_cast<char*>(&entry.id), sizeof(entry.id));
			size_t labelSize;
			inputFile.read(reinterpret_cast<char*>(&labelSize), sizeof(labelSize));
			char* buffer = new char[labelSize];
			inputFile.read(buffer, labelSize);
			entry.label.assign(buffer, labelSize);
			delete buffer;
			inputFile.read(reinterpret_cast<char*>(&entry.section), sizeof(entry.section));
			inputFile.read(reinterpret_cast<char*>(&entry.value), sizeof(entry.value));
			inputFile.read(reinterpret_cast<char*>(&entry.visibility), sizeof(entry.visibility));
			inputFile.read(reinterpret_cast<char*>(&entry.isExt), sizeof(entry.isExt));
			inputFile.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));

			entry.fileId = numOfFiles;

			if (entry.id != 0 && entry.id != 1) //undefined and absolute sections
				table.insertSymbol(entry);
		}

		//read sections: code and reloc tables
		int numberOfSections;
		inputFile.read(reinterpret_cast<char*>(&numberOfSections), sizeof(numberOfSections));
		numberOfSections -= 2; //remove UND and ABS

		for (int i = 0; i < numberOfSections; i++) {
			size_t labelSize;
			SectionData s;
			inputFile.read(reinterpret_cast<char*>(&s.id), sizeof(&s.id));
			inputFile.read(reinterpret_cast<char*>(&labelSize), sizeof(labelSize));
			char* buffer = new char[labelSize];
			inputFile.read(buffer, labelSize);
			s.label.assign(buffer, labelSize);

			inputFile.read(reinterpret_cast<char*>(&s.size), sizeof(s.size));
			delete buffer;
			buffer = new char[s.size];
			inputFile.read(buffer, s.size);
			s.data.assign(buffer, s.size);
			delete buffer;
			s.fileId = numOfFiles;

			size_t relocSize;
			inputFile.read(reinterpret_cast<char*>(&relocSize), sizeof(relocSize));
			for (unsigned j = 0; j < relocSize; j++) {
				RelocationEntry r;
				inputFile.read(reinterpret_cast<char*>(&r.offset), sizeof(r.offset));
				inputFile.read(reinterpret_cast<char*>(&r.relType), sizeof(r.relType));
				inputFile.read(reinterpret_cast<char*>(&r.ordinal), sizeof(r.ordinal));

				s.relocTable.push_back(r);
			}

			sections.push_back(s);
		}
		numOfFiles++;
	}
	catch (LinkerException g) {
		cout << "Error :" << g.getMsg() << endl;
	}
}


void Linker::generateHex() {
	try {
		auto symbols = table.getUnknownUsedSymbols();

		if (symbols.size() > 0) {
			string msg = "Undefined symbols ";
			for (auto s : symbols) {
				msg += s + " ";
			}
			msg += '!';
			throw LinkerException(msg);
		}

		table.createGlobalTable(places);

		
	}
	catch (LinkerException e) {
		cout << "Error :" << e.getMsg() << endl;
	}
}

void Linker::generateLinkable() {

}