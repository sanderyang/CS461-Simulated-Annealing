#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <random>
using namespace std;

/* Global Variables */
const string IFILENAME("roommates.txt"); // Input stream
const string OFILENAME("results.txt"); // Output stream
const int totalStudents = 200; // Total number of students
const int studentsPerRoom = 4; // Total number of students per room
const int initialTValue = 10000; // Initial T value
const double initialGeometricValue = 0.95; // Initial cooling value

class Roommates {
public:
	vector<vector<int>> compatibilityNumbers; // Stores the input file (list of compatibility scores)
	vector<vector<int>> rooms; // Stores the students into rooms.
	vector<int> compatibilityList; // Stores the total sum of compatibility for each room.
	double T; // T value
	int currentSwaps; // Keeps track of current swaps.
	int currentAttempts; // Keeps track of current attempts.

	struct Y { // Structure for 1 student.
		int studentNumber = 0;
		int roomNumber = 0;
		int indexNumber = 0;
		int roomScore = 0;
		vector<int> tempRoom;
	};
	Y firstPick; // First pick student.
	Y secondPick; // Second pick student.

	struct pair { // Structure for a pair of students.
		int roomNumber = 0;
		int roomScore = 0;
		vector<int> tempRoom;
	};
	pair firstRoom; // First pair of students.
	pair secondRoom; // Second pair of students.

	Roommates(vector<vector<int>> compList, vector<vector<int>> roomList) { //Default parameterized constructor.
		compatibilityNumbers = compList;
		rooms = roomList;
		compatibilityList = calculateCompatibility();
		T = initialTValue;
		currentSwaps = 0;
		currentAttempts = 0;
	}
	vector<int> calculateCompatibility(); // Calculates the compatibility of the room vector.
	void randomNumberPicks(); // Picks random students (rooms cannot be the same).
	void randomPairPicks(); // Picks random pairs of students (rooms cannot be the same).
	int calculateRoomScore(vector<int>); // Calculates the compatibility score of a given vector room.
	void createTempRoom(int, int, int, int, int, int); // For single picks, creates a temp vector.
	void createPairTemp(); // For pair picks, creates a temp vector.
	bool toSwap(double); // For single picks, can we swap?
	void swapStudents();  // For single picks, swap.
	void swapPairs(); // For pair picks, swap.
	void setSwap(int); // Setter function for swap variable.
	void setAttempts(int); // Setter function for attempt variable.
	void beginCooling(); // Begin cooling.
	int randomNumGenerator(int, int); // Random number generator - takes int min and int max.
private:
	int getCompatibilityScore(int, int); // Get the compatibility score between 2 students.
	int getRoomNumber(int); // Get the room number given the student ID number.
	int getIndexNumber(int, int); // Get the index of the student in that room given the student ID number and room number.
	double randomNumberComparison(); // Generate a value between double 0.1 and 1.0.
	double calculateProbability(double); // Calculate double probability given a delta.
};

vector<vector<int>> readFile(ifstream &); // Reads an input file.
void simulatedAnnealing(vector<vector<int>>, vector<vector<int>>, Roommates); // Runs the simulated annealing search.
vector<vector<int>> assignRooms(); // Initial assignment of rooms (ordered).
void writeFile(ofstream&, vector<vector<int>>, vector<int>); // Writes to an output file.

int main() {
	ifstream dataFile(IFILENAME); // Create an input file.
	Roommates Building(readFile(dataFile), assignRooms()); // Create a Roommates object called "Building".
	cout << "Beginning Simulated Annealing..." << endl; // Console message.
	simulatedAnnealing(Building.compatibilityNumbers, Building.rooms, Building); // Begin simulated annealing search.
}

void simulatedAnnealing(vector<vector<int>> compNumbers, vector<vector<int>> roomList, Roommates building) {
	bool searchDone = false; // Variable to check status of annealing search.
	int coinflip = 0;
	double delta = 0.0;
	int currentFirRmComp = 0;
	int currentSecRmComp = 0;
	while (!searchDone) { // While our search is not complete, continue annealing.
		coinflip  = building.randomNumGenerator(1,totalStudents); // Choose whether to swap a student or a pair of students.

		if (coinflip <= (totalStudents/2)){ // If random number is less or equal to than half of total student, then swap a student.
			building.randomNumberPicks();
			currentFirRmComp = building.compatibilityList[building.firstPick.roomNumber - 1]; // Get room compatibility (before swap).
			currentSecRmComp = building.compatibilityList[building.secondPick.roomNumber - 1]; // Get room compatibility (before swap).
			building.createTempRoom(building.firstPick.roomNumber, building.secondPick.roomNumber, building.firstPick.indexNumber,
				building.secondPick.indexNumber, building.firstPick.studentNumber, building.secondPick.studentNumber); // Create temp rooms.
			building.firstPick.roomScore = building.calculateRoomScore(building.firstPick.tempRoom); // Calculate comp score for temp room.
			building.secondPick.roomScore = building.calculateRoomScore(building.secondPick.tempRoom); // Calculate comp score for temp room.
			delta = static_cast<double>((building.firstPick.roomScore - currentFirRmComp) + (building.secondPick.roomScore - currentSecRmComp));
			if (building.toSwap(delta)) { // If passes check out, then....
				building.swapStudents(); // Swap students. Increment swaps.
			}
			else {// If passes don't check out, then....
				building.currentAttempts++; // Increment attempt.
			}
		}
		else{ // If random number is greater than half of students, then swap a pair of students.
			building.randomPairPicks();
			currentFirRmComp = building.compatibilityList[building.firstRoom.roomNumber - 1]; // Get room compatibility (before swap).
			currentSecRmComp = building.compatibilityList[building.secondRoom.roomNumber - 1]; // Get room compatibility (before swap).
			delta = static_cast<double>((building.firstRoom.roomScore - currentFirRmComp) + (building.secondRoom.roomScore - currentSecRmComp));
			if (building.toSwap(delta)) { 
				building.swapPairs(); // Swap pair. Increment swaps.
			}
			else {
				building.currentAttempts++; // Increment attempt.
			}
		}

		if (building.currentSwaps == 0 && building.currentAttempts == 20000) { // If swaps == 0 and attempts == 20,000
			searchDone = true; // Search is done.
		}
		else if (building.currentSwaps == 2000) { // Else if swaps == 2,000
			building.setSwap(0); // Reset swap
			building.setAttempts(0); // Reset attempts
			building.beginCooling(); // Cool the temperature
		}
		else if (building.currentAttempts == 20000) { // Else if attempts == 20,000
			building.setSwap(0);// Reset swap
			building.setAttempts(0); // Reset attempts
			building.beginCooling(); // Cool the temperature
		}
	}
	ofstream resultFile(OFILENAME); // Create an output file.
	writeFile(resultFile, building.rooms, building.compatibilityList); // Write to output file.
}

void writeFile(ofstream& outputFile, vector<vector<int>> roomList, vector<int> compList) {
	if (!outputFile.is_open()) {
		cout << "Error opening file " + OFILENAME;
	}
	else {
		int sum = 0;
		int best = totalStudents;
		int worst = 0;
		for (size_t g = 0; g < compList.size(); g++) {
			sum += compList[g];
			if (compList[g] < best) {
				best = compList[g];
			}
			if (compList[g] > worst) {
				worst = compList[g];
			}
		}
		sum /= compList.size();
		outputFile << "Final Results:" << endl;
		outputFile << "Initial T value: " << initialTValue << endl;
		outputFile << "Cooling schedule: " << initialGeometricValue << endl;
		outputFile << "Average Score: " << sum << endl;
		outputFile << "Best Score: " << best << endl;
		outputFile << "Worst Score: " << worst << endl << endl;
		outputFile << "Final Room Assignment:" << endl;
		int nextLine = 0;
		for (unsigned int counter = 0; counter < roomList.size(); counter++) {
			outputFile << "Room #" << counter + 1 << "   ";
			outputFile << "Compatibility Score: " << compList[counter] << "   " ;
			for (unsigned int a = 0; a < studentsPerRoom; a++) {
				nextLine += 1;
				outputFile << roomList[counter][a] << " ";
				if (nextLine == studentsPerRoom) {
					nextLine = 0;
					outputFile << endl;
				}
			}
		}
		cout << "Search completed..." << endl;
		cout << "Results printed to results.txt file." << endl;
		outputFile.close();
	}
}

vector<vector<int>> readFile(ifstream& inputFile) {
	string theLine;
	vector<vector<int>> temp;
	if (!inputFile.is_open()) {
		cout << "Error opening file " + IFILENAME;
	}
	else {
		while (getline(inputFile, theLine))
		{
			vector<int> student;
			stringstream lineStream(theLine);
			int value;
			while (lineStream >> value)
			{
				student.push_back(value);
			}
			temp.push_back(student);
		}
		inputFile.close();
	}
	return temp;
}

vector<vector<int>> assignRooms() {
	vector<vector<int>> assignedRoom;
	int nextLine = 0;
	vector<int> students;
	for (int b = 1; b <= totalStudents; b++) {
		nextLine++;
		students.push_back(b);
		if (nextLine == studentsPerRoom) {
			assignedRoom.push_back(students);
			students.clear();
			nextLine = 0;
		}
	}
	return assignedRoom;
}

vector<int> Roommates::calculateCompatibility() {
	vector<int> temp;
	for (size_t f = 1; f < this->rooms.size() + 1; f++) {
		temp.push_back(calculateRoomScore(this->rooms[f - 1]));
	}
	return temp;
}

int Roommates::calculateRoomScore(vector<int> student) {
	int sum = 0;
	for (int c = 0; c < studentsPerRoom; c++) {
		for (int d = 0; d < studentsPerRoom; d++) {
			if (c != d && d > c) {
				sum += getCompatibilityScore(student[c], student[d]);
			}
		}
	}
	return sum;
}

int Roommates::getCompatibilityScore(int first, int second) {
	return this->compatibilityNumbers[first - 1][second - 1];
}

void Roommates::randomNumberPicks() {
	bool differentRooms = false;
	while (!differentRooms) {
		this->firstPick.studentNumber = randomNumGenerator(1,totalStudents);
		this->firstPick.roomNumber = getRoomNumber(this->firstPick.studentNumber);
		this->secondPick.studentNumber = randomNumGenerator(1, totalStudents);
		this->secondPick.roomNumber = getRoomNumber(this->secondPick.studentNumber);
		if (this->firstPick.roomNumber != this->secondPick.roomNumber) {
			differentRooms = true;
		}
	}
	this->firstPick.indexNumber = getIndexNumber(this->firstPick.studentNumber, this->firstPick.roomNumber);
	this->secondPick.indexNumber = getIndexNumber(this->secondPick.studentNumber, this->secondPick.roomNumber);
}

void Roommates::randomPairPicks() {
	bool differentRooms = false;
	while (!differentRooms) {
		this->firstRoom.roomNumber = this->randomNumGenerator(1, this->rooms.size());
		this->secondRoom.roomNumber = this->randomNumGenerator(1, this->rooms.size());
		if (this->firstRoom.roomNumber != this->secondRoom.roomNumber) {
			this->firstRoom.tempRoom = this->rooms[this->firstRoom.roomNumber - 1];
			this->secondRoom.tempRoom = this->rooms[this->secondRoom.roomNumber - 1];
			this->createPairTemp();
			this->firstRoom.roomScore = this->calculateRoomScore(this->firstRoom.tempRoom);
			this->secondRoom.roomScore = this->calculateRoomScore(this->secondRoom.tempRoom);
			differentRooms = true;
		}
	}
}

int Roommates::getRoomNumber(int studentNum) {
	for (size_t counter = 0; counter < this->rooms.size(); counter++) {
		for (int a = 0; a < studentsPerRoom; a++) {
			if (this->rooms[counter][a] == studentNum) {
				return counter + 1;
			}
		}
	}
}

int Roommates::getIndexNumber(int studentNum, int roomNum) {
	vector<int> temp = this->rooms[roomNum - 1];
	for (size_t counter = 0; counter < temp.size(); counter++) {
		if (temp[counter] == studentNum) {
			return counter;
		}
	}
}

/* USED CODE SOURCE: https://www.codeproject.com/Questions/676477/Generate-Random-Double-Number */
int Roommates::randomNumGenerator(int min, int max) {
	static default_random_engine generator(unsigned(time(nullptr)));
	uniform_real_distribution<double> distribution(min, max);
	return distribution(generator);
}

/* USED CODE SOURCE: https://www.codeproject.com/Questions/676477/Generate-Random-Double-Number */
double Roommates::randomNumberComparison() {
	static default_random_engine generator(unsigned(time(nullptr)));
	uniform_real_distribution<double> distribution(0.1, 1.0);
	return distribution(generator);
}

void Roommates::createTempRoom(int fRmNum, int sRmNum, int fInNum, int sInNum, int fStudent, int sStudent) {
	this->firstPick.tempRoom = this->rooms[fRmNum - 1];
	swap(this->firstPick.tempRoom[fInNum], sStudent);
	this->secondPick.tempRoom = this->rooms[sRmNum - 1];
	swap(this->secondPick.tempRoom[sInNum], fStudent);
}

bool Roommates::toSwap(double deltaE) {
	double probability = calculateProbability(deltaE);
	double randomVal = randomNumberComparison();
	if (probability > 1)
		return true;
	else if (randomVal < probability)
		return true;
	return false;
}

double Roommates::calculateProbability(double deltaE) {
	return exp(-(deltaE) / this->T);
}

void Roommates::swapStudents() {
	rooms[this->firstPick.roomNumber - 1] = this->firstPick.tempRoom;
	rooms[this->secondPick.roomNumber - 1] = this->secondPick.tempRoom;
	compatibilityList[this->firstPick.roomNumber - 1] = this->firstPick.roomScore;
	compatibilityList[this->secondPick.roomNumber - 1] = this->secondPick.roomScore;
	this->currentSwaps++;
}

void Roommates::swapPairs() {
	this->rooms[this->firstRoom.roomNumber - 1] = this->firstRoom.tempRoom;
	this->rooms[this->secondRoom.roomNumber - 1] = this->secondRoom.tempRoom;
	this->compatibilityList[this->firstRoom.roomNumber - 1] = this->firstRoom.roomScore;
	this->compatibilityList[this->secondRoom.roomNumber - 1] = this->secondRoom.roomScore;
	this->currentSwaps++;
}

void Roommates::createPairTemp() {
	swap(this->firstRoom.tempRoom[0], this->secondRoom.tempRoom[2]);
	swap(this->firstRoom.tempRoom[1], this->secondRoom.tempRoom[3]);
}

void Roommates::setSwap(int val) {
	this->currentSwaps = val;
}

void Roommates::setAttempts(int val) {
	this->currentAttempts = val;
}

void Roommates::beginCooling() {
	this->T *= initialGeometricValue;
}