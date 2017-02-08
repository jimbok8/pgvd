#include "clfw.hpp"
#include "GLUtilities/Sketcher.h"
#include <iostream>
#include <fstream>
using namespace GLUtilities;

#define a_couple 2 
#define a_few 30
#define a_lot 1000000 

inline vector<cl_int> generateDeterministicRandomIntegers(int totalElements, int seed = 0, int minimum = -1024, int maximum = 1024) {
  /* To give deterministic results. */
  srand(seed);
  vector<cl_int> numbers(totalElements);
  for (int i = 0; i < totalElements; ++i) {
    numbers[i] = (rand() % (maximum - minimum)) + minimum;
  }
  return numbers;
}

inline vector<int2> generateDeterministicRandomInt2s(int totalElements, int seed = 0, int minimum = -1024, int maximum = 1024) {
  /* To give deterministic results. */
  srand(seed);
  vector<int2> numbers(totalElements);
  for (int i = 0; i < totalElements; ++i) {
    numbers[i].x = (rand() % (maximum - minimum)) + minimum;
    numbers[i].y = (rand() % (maximum - minimum)) + minimum;
  }
  return numbers;
}

inline vector<cl_float> generateDeterministicRandomFloats(int totalElements, int seed = 0, float minimum = -1024.0, float maximum = 1024.0) {
  /* To give deterministic results. */
  srand(seed);
  vector<cl_float> numbers(totalElements);
  for (int i = 0; i < totalElements; ++i) {
    numbers[i] = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maximum - minimum))) + minimum;
  }
  return numbers;
}

inline vector<float2> generateDeterministicRandomFloat2s(int totalElements, int seed = 0, float minimum = -1024.0, float maximum = 1024.0) {
  /* To give deterministic results. */
  srand(seed);
  vector<float2> points(totalElements);
  for (int i = 0; i < totalElements; ++i) {
    points[i].x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maximum - minimum))) + minimum;
    points[i].y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maximum - minimum))) + minimum;
  }
  return points;
}

inline vector<BigUnsigned> generateDeterministicRandomBigUnsigneds(int totalElements, int seed = 0, unsigned minimum = 0, unsigned maximum = 1 << 18) {
  /* To give deterministic results. */
  srand(seed);
  vector<BigUnsigned> numbers(totalElements);
  for (int i = 0; i < totalElements; ++i) {
    initBlkBU(&numbers[i], (rand() % (maximum - minimum)) + minimum);
  }
  return numbers;
}

inline vector<Conflict> generateDeterministicRandomConflicts(int totalElements, int seed = 0) {
	/* To give deterministic results. */
	srand(seed);
	vector<Conflict> conflicts(totalElements);
	for (int i = 0; i < totalElements; i++) {
		conflicts[i].color = (rand() % 2) - 2;
	}
	return conflicts;

}

inline bool comparePairByKey(std::pair<cl_int, cl_int> &a, std::pair<cl_int, cl_int> &b)
{
  return a.first < b.first;
}

template<typename T>
inline void writeToFile(vector<T> input, string filename) {
	ofstream myfile;
	myfile.open(filename, ios::out | ios::app | ios::binary);
	if (myfile.is_open()) {
		myfile.write((char*)input.data(), sizeof(T) * input.size());
		myfile.close();
	}
}

template<typename T>
inline void writeToFile(T input, string filename) {
  ofstream myfile;
  myfile.open(filename, ios::out | ios::app | ios::binary);
  if (myfile.is_open()) {
    myfile.write((char*)&input, sizeof(T));
    myfile.close();
  }
}

template<typename T>
inline vector<T> readFromFile(string filename, int totalElements) {
	vector<T> result(totalElements);
  ifstream myfile;
  myfile.open(filename, ios::in | ios::binary);
  if (myfile.is_open()) {
    myfile.read((char*)result.data(), totalElements * sizeof(T));
    myfile.close();
  }
	return result;
}

template<typename T>
inline T readFromFile(string filename) {
	T result = -1;
	ifstream myfile;
	myfile.open(filename, ios::in | ios::binary);
	if (myfile.is_open()) {
		myfile.read((char*)&result, sizeof(T));
		myfile.close();
	}
	return result;
}

inline void DrawBrtNode(vector<BrtNode> &brt, vector<cl_int> colors, int i, float maxLevel, float currentLevel, float shift) {
	BrtNode me = brt[i];
	float width = brt.size();

	float4 color;
	if (colors[i] == -1) {
		color = { 0.0, 0.0, 0.0, 1.0 };
	}
	else if (colors[i] == -2) {
		color = { 0.0, 1.0, 0.0, 1.0 };
	}
	else if (brt[i].parent != -1 && brt[brt[i].parent].left == i) {
		color = { 1.0, 0.0, 0.0, 1.0 };
	}
	else if (brt[i].parent != -1 && colors[brt[brt[i].parent].left] == colors[i]) {
		color = { 1.0, 0.0, 0.0, 1.0 };
	}
	else {
		color = { 0.0, 0.0, 1.0, 1.0 };
	}

	/* render me */
	Point p = {
		{ shift, ((maxLevel - currentLevel) / maxLevel) * 1.5 - 0.75, 0.0, 1.0 },
		color
	};
	Sketcher::instance()->add(p);

	/* render my children */
	if (!me.left_leaf) {
		DrawBrtNode(brt, colors, me.left, maxLevel, currentLevel + 1, shift - (1.0 / (2 << (int)currentLevel)));
	}
	if (!me.right_leaf) {
		DrawBrtNode(brt, colors, me.left + 1, maxLevel, currentLevel + 1, shift + (1.0 / (2 << (int)currentLevel)));
	}
}

inline void DrawBRT(vector<BrtNode> &brt, vector<cl_int> colors) {
	if (brt.size() < 2) return;
	float width = brt.size() + 1;
	float maxLevel = 0;
	for (int i = 0; i < brt.size(); ++i) {
		float temp = 0;
		BrtNode n = brt[i];
		while (n.parent != -1) {
			temp++;
			n = brt[n.parent];
		}
		maxLevel = std::fmaxf(temp, maxLevel);
	}

	DrawBrtNode(brt, colors, 0, maxLevel, 0.0, 0.0);
}