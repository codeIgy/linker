#include "linker.h"
#include <iostream>
#include <iomanip>

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

		table.checkIfPlaceable();
		//merge sections with the same name, read from relocation table and fill in the information needed
		//skip UND and ABS sections and copy others
		for (unsigned i = 2; i < table.numSectionsGlobal; i++) {
			TableEntry& entry = table.getSymbolGlobal(i);
			string data = "";

			for (auto section : sections) {
				if (section.label == entry.label) {
					data += section.data;
				}
			}

			code.insert({ entry.value, data });
		}

		for (auto section : sections) {
			TableEntry& sectionEntry = table.getSymbol(section.id, section.fileId);
			for (auto rEntry : section.relocTable) {
				TableEntry& entry = table.getSymbol(rEntry.ordinal, section.fileId);
				rEntry.offset += sectionEntry.value;

				auto mem = code.lower_bound(rEntry.offset);
				string block;
				if (mem == code.end()) {
					mem = prev(code.end());
				}
				else if (mem->first != rEntry.offset)
				{
					mem--;
				}
				int offset = rEntry.offset - mem->first;
				if (rEntry.relType == RelocationEntry::R_386_16) {
					short value = (mem->second[offset] << 8) | (mem->second[offset + 1]);
					value += entry.value;
					mem->second[offset] = (value & 0xFF00) >> 8;
					mem->second[offset + 1] = (value & 0xFF);
				}
				else if (rEntry.relType == RelocationEntry::R_386_16D) {
					short value = (mem->second[offset + 1] << 8) | (mem->second[offset]);
					value += entry.value;
					mem->second[offset] = (value & 0xFF);
					mem->second[offset + 1] = (value & 0xFF00) >> 8;
				}
				else if (rEntry.relType == RelocationEntry::R_386_PC16) {
					if (entry.id == 1) //ABS
					{
						short value = (mem->second[offset] << 8) | (mem->second[offset + 1]);
						value -= rEntry.offset;
						mem->second[offset] = (value & 0xFF00) >> 8;
						mem->second[offset + 1] = (value & 0xFF);
					}
					else {
						short value = (mem->second[offset] << 8) | (mem->second[offset + 1]);
						value += entry.value - rEntry.offset;
						mem->second[offset] = (value & 0xFF00) >> 8;
						mem->second[offset + 1] = (value & 0xFF);
					}
				}
				else if (rEntry.relType == RelocationEntry::R_386_PC16D) {
					if (entry.id == 1) //ABS
					{
						short value = (mem->second[offset + 1] << 8) | (mem->second[offset]);
						value -= rEntry.offset;
						mem->second[offset] = (value & 0xFF);
						mem->second[offset + 1] = (value & 0xFF00) >> 8;
					}
					else {
						short value = (mem->second[offset + 1] << 8) | (mem->second[offset]);
						value += entry.value - rEntry.offset;
						mem->second[offset] = (value & 0xFF);
						mem->second[offset + 1] = (value & 0xFF00) >> 8;
					}
				}
			}
		}

		writeHexFile();
	}
	catch (LinkerException e) {
		cout << "Error :" << e.getMsg() << endl;
	}
}

void Linker::generateLinkable() {

	table.createGlobalTableLinkable();

	//merge section data and reloc tables
	for (unsigned i = 2; i < table.numSectionsGlobal; i++) {
		TableEntry& entry = table.getSymbolGlobal(i);

		SectionData newSecData;

		newSecData.id = entry.id;
		newSecData.label = entry.label;
		newSecData.size = entry.size;
		newSecData.data = "";

		for (auto section : sections) {
			if (section.label == entry.label) {
				newSecData.data += section.data;
				int offsetSection = table.getSymbol(section.id, section.fileId).value;

				for (auto rel : section.relocTable) {
					rel.offset += offsetSection;

					TableEntry& symbol = table.getSymbol(rel.ordinal, section.fileId);
					rel.ordinal = symbol.globalId;

					TableEntry& symbolSection = table.getSymbol(symbol.section, section.fileId);

					if (rel.relType == RelocationEntry::R_386_PC16 && newSecData.id == symbolSection.globalId) {
						short addend = ((newSecData.data[rel.offset] & 0xFF) << 8) | (newSecData.data[rel.offset + 1] & 0xFF);
						addend += symbol.value - rel.offset;
						newSecData.data[rel.offset] = (addend & 0xFF00) >> 8;
						newSecData.data[rel.offset + 1] = (addend & 0xFF);
					}

					newSecData.relocTable.push_back(rel);
				}

			}
		}

		sectionsGlobal.push_back(newSecData);
	}

	writeObjectFile();
}

void Linker::writeHexFile()
{
	int prevAddr = -1;
	unsigned offset = -1;
	int addr;
	for (auto block : code) {
		addr = 8 * (block.first / 8);
		string& data = block.second;

		if (prevAddr != addr) {
			if (prevAddr != -1) {
				outputFile << endl << endl;
			}
			offset = block.first % 8;
			outputFile << setw(4) << setfill('0') << hex << addr << ": ";
			for (unsigned i = 0; i < offset; i++) {
				outputFile << "   ";
			}
		}

		for (unsigned i = 0; i < data.size(); i++) {
			if (offset == 8) {
				offset = 0;
				addr += 8;
				outputFile << endl;
				outputFile << setw(4) << setfill('0') << hex << addr << ": ";
			}
			outputFile << setw(2) << setfill('0') << hex << ((unsigned)data[i] & 0xFF);

			if (++offset < 8) {
				outputFile << " ";
			}
		}
		prevAddr = addr;
	}
}

void Linker::writeObjectFile()
{
	table.printSymbolTableGlobal(outputFile);
	int outputColumnCnt;
	int addr;

	for (auto section : sectionsGlobal) {

		outputFile << right << "$." << section.label << ":" << hex << section.size << endl;
		outputColumnCnt = 0;
		addr = 0;

		outputFile << setw(4) << setfill('0') << hex << addr << ": ";

		for (unsigned i = 0; i < section.data.size(); i++) {
			if (outputColumnCnt == 8) {
				outputColumnCnt = 0;
				addr += 8;
				outputFile << endl;
				outputFile << setw(4) << setfill('0') << hex << addr << ": ";
			}
			outputFile << setw(2) << setfill('0') << hex << ((unsigned)section.data[i] & 0xFF);

			if (++outputColumnCnt < 8) {
				outputFile << " ";
			}
		}

		outputFile << endl << "$relocation data" << endl << left << hex << setw(10) << setfill(' ') << "offset" << setw(12) << "type"
			<< setw(10) << "ordinal" << endl;

		for (auto r : section.relocTable) {
			string relocType = "";

			switch (r.relType) {
			case RelocationEntry::R_386_16:
				relocType = "R_386_16";
				break;
			case RelocationEntry::R_386_16D:
				relocType = "R_386_16D";
				break;
			case RelocationEntry::R_386_PC16:
				relocType = "R_386_PC16";
				break;
			case RelocationEntry::R_386_PC16D:
				relocType = "R_386_PC16D";
				break;
			default: break;
			}
			outputFile << setw(10) << r.offset << setw(12) << relocType << r.ordinal << endl;
		}

		outputFile << endl;
	}
}
