#pragma once
#include <random>
#include <stdlib.h>

struct Random {

	static std::mt19937 gen;

	static void seed(int val) {
		gen = std::mt19937(val);
	}

	static int rand() {
		return abs((int)gen());
	}

	static int rand(int val) {
		return rand() % val;
	}
};
