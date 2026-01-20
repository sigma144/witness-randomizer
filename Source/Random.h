#pragma once
#include <random>
#include <stdlib.h>
#include <vector>
#include <set>

struct Random {

	static std::mt19937 gen;

	static void seed(int val) {
		gen = std::mt19937(val);
	}
	static int rand() {
		return abs((int)gen());
	}
	static int rand(int mod) {
		return rand() % mod;
	}
	static int rand(size_t mod) {
		return rand() % mod;
	}
	static int rand(int min, int max) {
		return rand() % (max - min + 1) + min;
	}
	template <class T> static T pickRandom(const std::vector<T>& vec) {
		return vec[rand() % vec.size()];
	}
	template <class T> static T pickRandom(const std::set<T>& set) {
		auto it = set.begin();
		std::advance(it, rand() % set.size());
		return *it;
	}
	template <class T> static T popRandom(std::vector<T>& vec) {
		int i = rand() % vec.size();
		T item = vec[i]; vec.erase(vec.begin() + i);
		return item;
	}
	template <class T> static T popRandom(std::set<T>& set) {
		T item = pickRandom(set);
		set.erase(item);
		return item;
	}
};
