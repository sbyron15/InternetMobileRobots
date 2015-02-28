#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

int main() {

	string line;
	ifstream imageFile("straight.pgm");

	int leftScore = 0;
	int rightScore = 0;
	int width = 640;

	if (imageFile.is_open()) {
		// Get the first line and do nothing
		getline(imageFile, line);

		// Get the second line which has the dimensions
		getline(imageFile, line);

		// Get the third line which has the maximum value, do nothing with it
		getline(imageFile, line);

		int i = 0;

		while (getline(imageFile, line, ' ')) {

			if (i>(width-1)) {
				i=0;
			}

			istringstream buffer(line);
			int value;
			buffer >> value;

			if (i<(width/2)) {
				leftScore += value;
			} else {
				rightScore += value;
			}

			i++;
		}

		imageFile.close();
	} else {
		cout << "Unable to open file";
	}

	cout << "Left score: " << leftScore << "\n";
	cout << "Right score: " << rightScore << "\n";

	float delta = 1.05;

	if (leftScore > rightScore) {
		float score = (float)leftScore/(float)rightScore;
		if (score > delta) {
			cout << "Turn Right\n";
		} else {
			cout << "Go Straight\n";
		}
	} else {
		float score = (float)rightScore/(float)leftScore;
		if (score > delta) {
			cout << "Turn Left\n";
		} else {
			cout << "Go Straight\n";
		}
	}

	return(0);
}