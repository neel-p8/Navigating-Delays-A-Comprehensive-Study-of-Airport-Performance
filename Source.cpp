#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <iomanip>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

struct AirportData {
    string code; //Airport 3-letter Code
    string name; //Airport Name
    string month; //Travel Month
    string year; //Year Stats
    int carrier; //Number of delays due to carrier
    int late; //Number of delays due to late arrival of aircraft
    int navis; //Number of delays due to National Aviation System
    int security; //Number of delays due to security
    int weather; //Number of delays due to weather
    int canceled; //Number of canceled flights
    int delayed; //Number of delayed flights
    int total_flights; //Number of Total Flights
};

// Structure for Trie Node
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    vector<AirportData> airport_data;
};
// Function to extract airport name from full name
string AirportName(const string& fullName) {
    size_t pos = fullName.find(": ");
    if (pos != string::npos && pos + 2 < fullName.size()) {
        return fullName.substr(pos + 2);
    }
    return fullName;
}
// Function to get Airport Information
void GetAirportInfo(AirportData& data, vector<string>& columns) {
    data.code = columns[0];
    data.name = AirportName(columns[1]);
    data.month = columns[4];
    data.year = columns[5];
    data.carrier = stoi(columns[6]);
    data.late = stoi(columns[7]);
    data.navis = stoi(columns[8]);
    data.security = stoi(columns[9]);
    data.weather = stoi(columns[10]);
    data.canceled = stoi(columns[12]);
    data.delayed = stoi(columns[13]);
    data.total_flights = stoi(columns[16]);
}
// Function to split CSV line if it contains comma within the columns
// https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c/53845961
vector<string> splitLine(const string & line) {
    vector<string> columns;
    string data;
    bool quoted = false;
    char prev = NULL; // Previous character to detect quote escaping
    for (char c : line) {
        if (c == '\"') {
            if (prev != '\\') { // Check for escaped quotes
                quoted = !quoted; // Toggle quoted state
            }
        }
        else if (c == ',' && !quoted) { // Outside quotes, comma is delimiter
            columns.push_back(data);
            data.clear();
        }
        else {
            data.push_back(c);
        }
        prev = c;
    }
    columns.push_back(data);
    return columns;
}
// Function to insert data into trie
// https://www.geeksforgeeks.org/trie-insert-and-search/
void insertTrie(TrieNode* root, const AirportData& data) {
    TrieNode* current = root;
    for (char c : data.code) {
        if (current->children.find(c) == current->children.end()) {
            current->children[c] = new TrieNode();
        }
        current = current->children[c];
    }
    current->airport_data.push_back(data);
}
// Function to read CSV and build trie
// https://www.geeksforgeeks.org/trie-insert-and-search/
TrieNode* buildTrie(const string& filename) {
    TrieNode* root = new TrieNode();
    ifstream file(filename);
    string line;
    getline(file, line); // Skip the header line
    while (getline(file, line)) {
        vector<string> columns = splitLine(line);
        AirportData data;
        GetAirportInfo(data, columns);
        insertTrie(root, data);
    }
    return root;
}

// Function to build hash table from CSV
unordered_map<string, vector<AirportData>> buildHashTable(const string&
filename) {
    unordered_map<string, vector<AirportData>> dataMap;
    ifstream file(filename);
    string line;
    getline(file, line); // Skip the header line
    while (getline(file, line)) {
        vector<string> columns = splitLine(line);
        AirportData data;
        GetAirportInfo(data, columns);
        dataMap[data.code].push_back(data);
    }
    return dataMap;
}
// Function to calculate percentage
double calculatePercentage(int numerator, int denominator) {
    if (denominator == 0) {
        return 0.0; // Avoid division by zero
    }
    return (static_cast<double>(numerator) / denominator) * 100.0;
}
// Function to convert a string to uppercase
string toUpper(const string& str) {
    string upperStr = str;
    transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr;
}

// Function to convert a string to lowercase
string toLower(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// code below for calulating top 5 most delay airports
double calculateDelayRate(const vector<AirportData>& data) {
    int totalFlights = 0;
    int totalDelayedCanceled = 0;
    for (const auto& entry : data) {
        totalFlights += entry.total_flights;
        totalDelayedCanceled += entry.delayed + entry.canceled;
    }
    if (totalFlights == 0) {
        return 0.0;
    }
    return static_cast<double>(totalDelayedCanceled) / totalFlights * 100.0;
}

// same thing as above except with month and year
double calculateDelayRate(const vector<AirportData>& data, const string& month, const string& year) {
    int totalFlights = 0;
    int totalDelayedCanceled = 0;
    for (const auto& entry : data) {
        if (entry.month == month && entry.year == year) {
            totalFlights += entry.total_flights;
            totalDelayedCanceled += entry.delayed + entry.canceled;
        }
    }
    if (totalFlights == 0) {
        return 0.0;
    }
    return static_cast<double>(totalDelayedCanceled) / totalFlights * 100.0;
}


// traverse trie for airport data for calculating top 5
void traverseTrie(TrieNode* root, vector<pair<string, vector<AirportData>>>& airportData) {
    if (root == nullptr) {
        return;
    }
    if (!root->airport_data.empty()) {
        airportData.emplace_back(root->airport_data[0].code, root->airport_data);
    }
    for (const auto& child : root->children) {
        traverseTrie(child.second, airportData);
    }
}

// Function to check if a month is valid
string isValidMonth(const string& month) {
    map<string, string> validMonths = { {"january", "January"}, {"february", "February"}, {"march", "March"},
                                        {"april", "April"}, {"may", "May"}, {"june", "June"},
                                        {"july", "July"}, {"august", "August"}, {"september", "September"},
                                        {"october", "October"}, {"november", "November"}, {"december", "December"} };
    auto it = validMonths.find(toLower(month));
    if (it != validMonths.end()) {
        return it->second;
    }
    return "";
}

int main() {
    string file = "airlines.csv";

    // Create a map to store delay/cancellation rates for different time periods
    unordered_map<string, double> delayRatesByMonth;
    unordered_map<string, double> delayRatesByYear;

    // User Input for choice
    int choice;
    cout << "Choose a data structure to use:" << endl;
    cout << "1. Hash Table" << endl;
    cout << "2. Trie" << endl;
    cout << "Enter your choice (1 or 2): ";
    cin >> choice;
    switch (choice) {
        case 1: {
            auto data = buildHashTable(file);
            // User Input
            string airport_code, travel_month;
            cout << "Enter the airport code: ";
            cin >> airport_code;
            airport_code = toUpper(airport_code);
            cout << "Enter travel month name: ";
            cin >> travel_month;
            string validMonth = isValidMonth(travel_month);  // Check if month is valid
            if (validMonth.empty()) {
                cout << "Invalid month. Please enter a valid month." << endl;
                return 0;
            }
            travel_month = validMonth;  // Convert to appropriate case
            cout << endl;

            auto it = data.find(airport_code);
            if (it != data.end()) {
                cout << "Accessing Airport Data using Hash Table..." << endl;
                cout << "-----------------------------------------------" << endl;
                cout << "Airport Code: " << it->first << endl;
                cout << "Airport Name: " << it->second[0].name << endl;
                cout << "Travel Month: " << travel_month << endl;
                cout << "-----------------------------------------------" << endl;
                // Initialize variables to calculate total number of flights, delays, and cancellations
                int totalFlights = 0;
                int totalDelayed = 0;
                int totalCarrier = 0;
                int totalLate = 0;
                int totalNavis = 0;
                int totalSecurity = 0;
                int totalWeather = 0;
                int totalCanceled = 0;
                for (const auto& entry : it->second) {
                    string monthYear = entry.month + " " + entry.year;
                    delayRatesByMonth[monthYear] = calculateDelayRate(it->second, entry.month, entry.year);
                    delayRatesByYear[entry.year] += calculateDelayRate(it->second, entry.month, entry.year);

                    if (entry.month == travel_month) {
                        cout << "Year: " << entry.year << endl;
                        cout << "Flights Canceled: " << entry.canceled <<
                             endl;
                        cout << "Flights Delayed: " << entry.delayed << endl;
                        cout << "Total Flights: " << entry.total_flights <<
                             endl;
                        cout << endl;
                        totalFlights += entry.total_flights;
                        totalDelayed += entry.delayed;
                        totalCanceled += entry.canceled;
                        totalCarrier += entry.carrier;
                        totalLate += entry.late;
                        totalNavis += entry.navis;
                        totalSecurity += entry.security;
                        totalWeather += entry.weather;
                    }
                }
                // Calculate and display the percentage of delays and cancellations
                double percentageDelayed = calculatePercentage(totalDelayed, totalFlights);
                double percentageCanceled = calculatePercentage(totalCanceled, totalFlights);
                cout << "----------------------------------------------------------------" << endl;
                cout << "Welcome to " << it->second[0].name << " Airport!" << endl;
                cout << setw(8) << "---" << travel_month << " Statistics--- "<< endl;
                cout << "Total Flights Canceled in " << airport_code << ": " << totalCanceled << endl;
                cout << "Total Flights Delayed in " << airport_code << ": " << totalDelayed << endl;
                cout << "Total Number of Flights in " << airport_code << ": " << totalFlights << endl;
                cout << endl;
                cout << "Percentage of Flights Canceled: " << setprecision(3) << percentageCanceled << "%" << endl;
                cout << "Percentage of Flights Delayed: " << setprecision(4) << percentageDelayed << "%" << endl;
                cout << endl;
                cout << "Breakdown of Flights Delayed:" << endl;
                cout << setw(3) << "" << "- Security Screening: " << totalSecurity << endl;
                cout << setw(3) << "" << "- Weather Conditions: " << totalWeather << endl;
                cout << setw(3) << "" << "- Late Arrival of Aircraft: " << totalLate << endl;
                cout << setw(3) << "" << "- Carrier (maintenance, cleaning, fueling, etc.): " << totalCarrier << endl;
                cout << setw(3) << "" << "- National Aviation System (airport operations, etc.): " << totalNavis << endl;
                cout << "----------------------------------------------------------------" << endl;

                vector<pair<string, double>> delayRates;
                for (const auto& entry : data) {
                    double rate = calculateDelayRate(entry.second);
                    delayRates.emplace_back(entry.first, rate);
                }

                // Sort the vector in descending order based on the delay/cancellation rates
                sort(delayRates.begin(), delayRates.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
                    return a.second > b.second;
                });

                // Print the top 5 airports with the highest delay/cancellation rates
                cout << "Top 5 Airports with the Highest Delay/Cancellation Rates:" << endl;
                for (int i = 0; i < 5 && i < delayRates.size(); ++i) {
                    const auto& airport = data.find(delayRates[i].first)->second;
                    cout << setw(3) << i + 1 << ". " << setw(3) << delayRates[i].first << " - " << airport[0].name << ": "
                         << setprecision(2) << fixed << delayRates[i].second << "%" << endl;
                }
                cout << "----------------------------------------------------------------" << endl;

                // Print the trends
                cout << "Delay/Cancellation Trends for " << it->second[0].name << " (" << airport_code << "):" << endl;
                cout << "\nBy Year:" << endl;

                // Create a map to store delay rates by year
                map<string, double> delayRatesByYearMap;

                // Normalize the delay/cancellation rates for years
                for (auto& entry : delayRatesByYear) {
                    double normalRate = entry.second /= 12.0;  // Assuming 12 months in a year
                    delayRatesByYearMap[entry.first] = normalRate;
                }

                // Print the delay rates sorted by year
                for (const auto& entry : delayRatesByYearMap) {
                    cout << setw(3) << entry.first << ": " << setprecision(2) << fixed << entry.second << "%" << endl;
                }
                cout << "\n(Note) 2016 will not be accurate." << endl;
                cout << "----------------------------------------------------------------" << endl;
            }
            else {
                cout << "No data found for the entered airport code." <<
                     endl;
            }
            break;
        }
        case 2: {
            TrieNode* root = buildTrie(file);
            // User Input
            string airport_code, travel_month;
            cout << "Enter the airport code: ";
            cin >> airport_code;
            airport_code = toUpper(airport_code);
            cout << "Enter travel month name: ";
            cin >> travel_month;
            string validMonth = isValidMonth(travel_month);  // Check if month is valid
            if (validMonth.empty()) {
                cout << "Invalid month. Please enter a valid month." << endl;
                return 0;
            }
            travel_month = validMonth;  // Convert to appropriate case
            cout << endl;
            // Find node corresponding to airport code
            TrieNode* node = root;
            for (char c : airport_code) {
                if (node->children.find(c) == node->children.end()) {
                    cout << "No data found for the entered airport code." <<
                         endl;
                    return 0;
                }
                node = node->children[c];
            }
            cout << "Accessing Airport Data using Trie..." << endl;
            cout << "-----------------------------------------------" << endl;
            cout << "Airport Code: " << airport_code << endl;
            cout << "Airport Name: " << node->airport_data[0].name << endl;
            cout << "Travel Month: " << travel_month << endl;
            cout << "-----------------------------------------------" << endl;
            // Initialize variables to calculate total number of flights, delays, and cancellations
            int totalFlights = 0;
            int totalDelayed = 0;
            int totalCarrier = 0;
            int totalCanceled = 0;
            int totalLate = 0;
            int totalNavis = 0;
            int totalSecurity = 0;
            int totalWeather = 0;
            // Iterate over all airport data under the specified airport code
            for (const auto& entry : node->airport_data) {

                string monthYear = entry.month + " " + entry.year;
                delayRatesByMonth[monthYear] = calculateDelayRate(node->airport_data, entry.month, entry.year);
                delayRatesByYear[entry.year] += calculateDelayRate(node->airport_data, entry.month, entry.year);

                // Check if the entry matches the specified travel month
                if (entry.month == travel_month) {
                    cout << "Year: " << entry.year << endl;
                    cout << "Flights Canceled: " << entry.canceled << endl;
                    cout << "Flights Delayed: " << entry.delayed << endl;
                    cout << "Total Flights: " << entry.total_flights << endl;
                    cout << endl;
                    totalFlights += entry.total_flights;
                    totalDelayed += entry.delayed;
                    totalCanceled += entry.canceled;
                    totalCarrier += entry.carrier;
                    totalLate += entry.late;
                    totalNavis += entry.navis;
                    totalSecurity += entry.security;
                    totalWeather += entry.weather;
                }
            }
            // Calculate and display the percentage of delays and cancellations
            double percentageDelayed = calculatePercentage(totalDelayed, totalFlights);
            double percentageCanceled = calculatePercentage(totalCanceled, totalFlights);
            cout << "----------------------------------------------------------------" << endl;
            cout << "Welcome to " << node->airport_data[0].name << " Airport!" << endl;
            cout << setw(8) << "---" << travel_month << " Statistics---" << endl;
            cout << "Total Flights Canceled in " << airport_code << ": " << totalCanceled << endl;
            cout << "Total Flights Delayed in " << airport_code << ": " << totalDelayed << endl;
            cout << "Total Number of Flights in " << airport_code << ": " << totalFlights << endl;
            cout << endl;
            cout << "Percentage of Flights Canceled: " << setprecision(3) << percentageCanceled << "%" << endl;
            cout << "Percentage of Flights Delayed: " << setprecision(4) << percentageDelayed << "%" << endl;
            cout << endl;
            cout << "Breakdown of Flights Delayed:" << endl;
            cout << setw(3) << "" << "- Security Screening: " << totalSecurity << endl;
            cout << setw(3) << "" << "- Weather Conditions: " << totalWeather << endl;
            cout << setw(3) << "" << "- Late Arrival of Aircraft: " << totalLate << endl;
            cout << setw(3) << "" << "- Carrier (maintenance, cleaning, fueling, etc.): " << totalCarrier << endl;
            cout << setw(3) << "" << "- National Aviation System (airport operations, etc.): " << totalNavis << endl;
            cout << "----------------------------------------------------------------" << endl;

            vector<pair<string, vector<AirportData>>> airportData;
            traverseTrie(root, airportData);

            // Create a vector to store airport codes and their delay/cancellation rates
            vector<pair<string, double>> delayRates;
            for (const auto& entry : airportData) {
                double rate = calculateDelayRate(entry.second);
                delayRates.emplace_back(entry.first, rate);
            }

            // Sort the vector in descending order based on the delay/cancellation rates
            sort(delayRates.begin(), delayRates.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
                return a.second > b.second;
            });

            // Print the top 5 airports with the highest delay/cancellation rates
            cout << "Top 5 Airports with the Highest Delay/Cancellation Rates:" << endl;
            for (int i = 0; i < 5 && i < delayRates.size(); ++i) {
                cout << setw(3) << i + 1 << ". " << setw(3) << delayRates[i].first << " - " << airportData[i].second[0].name << ": "
                     << setprecision(2) << fixed << delayRates[i].second << "%" << endl;
            }
            cout << "----------------------------------------------------------------" << endl;

            // Print the trends
            cout << "Delay/Cancellation Trends for " << node->airport_data[0].name << " (" << airport_code << "):" << endl;
            cout << "\nBy Year:" << endl;

            // Create a map to store delay rates by year
            map<string, double> delayRatesByYearMap;

            // Normalize the delay/cancellation rates for years
            for (auto& entry : delayRatesByYear) {
                double normalRate = entry.second /= 12.0;  // Assuming 12 months in a year
                delayRatesByYearMap[entry.first] = normalRate;
            }

            // Print the delay rates sorted by year
            for (const auto& entry : delayRatesByYearMap) {
                cout << setw(3) << entry.first << ": " << setprecision(2) << fixed << entry.second << "%" << endl;
            }
            cout << "\n(Note) 2016 will not be accurate." << endl;
            cout << "----------------------------------------------------------------" << endl;

            break;
        }
        default:
            cout << "Invalid choice." << endl;
    }
    return 0;
}
