
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include "linker.h"

using namespace std;

int main(int argc, char** argv) {
	vector<string> inputFileNames;
	string outputFileName = "";
	unordered_map<string, int> places;
	string option = ""; // hex or linkable

	regex placeRegex("^-place=([A-Za-z_]\\w*)@(0[xX][0-9A-Fa-f]{4})");
	smatch placeMatch;

	bool o_option = false;

	for (int i = 1; i < argc; i++) {
		string word = argv[i];

		if (word.find('-') != string::npos) {//command argument
			if (o_option) {
				cout << "Wrong command line arguments!" << endl;
				return -1;
			}

			if (word == "-hex" && option.empty())
				option = word;
			else if (word == "-linkable" && option.empty())
				option = word;
			else if (regex_search(word, placeMatch, placeRegex)) {
				int address = stoi(placeMatch[2], nullptr, 16);
				places.insert({ placeMatch[1], address });
			}
			else if (word == "-o" && outputFileName.empty())
				o_option = true;
			else {
				cout << "Wrong command line arguments!" << endl;
				return -1;
			}
		}
		else {//file argument
			if (o_option) {
				o_option = false;
				outputFileName = word;
			}
			else {
				inputFileNames.push_back(word);
			}
		}
	}

	if (option.empty() || inputFileNames.empty())
	{
		cout << "Wrong command line arguments!" << endl;
		return -1;
	}

	if (outputFileName.empty()) {
		if (option == "-hex")
			outputFileName = "output.hex";
		else
			outputFileName = "output.o";
	}

	ifstream inputFile;
	ofstream outputFile;

	outputFile.open(outputFileName, ios::out);
	if (!outputFile) {
		cout << "Error: Unable to open text output file" << endl;
		outputFile.close();
		return 0;
	}

	Linker linker(places, outputFile);

	for (auto fname : inputFileNames) {
		inputFile.open(fname, ios::in | ios::binary);
		if (!inputFile) {
			cout << "Error: Unable to open input file " << fname << endl;
			outputFile.close();
			return 0;
		}

		linker.addObjectFile(inputFile);

		inputFile.close();
	}
	
	if (option == "-hex")
		linker.generateHex();
	else
		linker.generateLinkable();

	inputFile.close();
	outputFile.close();
	return 0;

}