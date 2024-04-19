#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>

using namespace std;

struct AirportData {
	string code; 
	string name;
};

unordered_map<string, AirportData> readCSV(const string& filename) {
	
	//hash table
	unordered_map<string, AirportData> dataMap;
	ifstream file(filename);

	string line;
	getline(file, line);

	while (getline(file, line)) {
		stringstream ss(line);
		
		//holds columns
		vector<string> columns;

		// variable hold individual column
		string column;

		while (getline(ss, column, ',')) {
			// add column to vector
			columns.push_back(column);
		}

		AirportData data;
		data.code = columns[0];
		data.name = columns[1];

		dataMap[data.code] = data;
	}
	return dataMap;
}

int main() {
	const string file = "airlines.csv";
	auto data = readCSV(file);

	//test if everything was imported correctly
	for (const auto& pair : data) {
		cout << "Airport Code: " << pair.second.code << endl;
		cout << "Airport Name: " << pair.second.name << endl;
		cout << "------------------------" << endl;

		if (pair.first == "ATL" || pair.first == "BOS" || pair.first == "BWI") {
			break;
		}
	}
}