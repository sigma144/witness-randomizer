#pragma once
#include <vector>
#include <map>
#include "Panel.h"
#include "Random.h"

struct PuzzleSymbols {
	std::map<Symbol, std::vector<std::pair<int, int>>> symbols;
	std::vector<std::pair<int, int>>& operator[](Symbol symbolType) { return symbols[symbolType]; }
	int style;
	int getNum(Symbol symbolType) {
		int total = 0;
		for (auto& pair : symbols[symbolType]) total += pair.second;
		return total;
	}
	bool any(Symbol symbolType) { return symbols[symbolType].size() > 0; }
	int popRandomSymbol() {
		std::vector<Symbol> types;
		for (auto& pair : symbols)
			if (pair.second.size() > 0 && pair.first != Start && pair.first != Exit && pair.first != Gap && pair.first != Eraser)
				types.push_back(pair.first);
		Symbol randType = Random::pickRandom(types);
		int randIndex = Random::rand(symbols[randType].size());
		while (symbols[randType][randIndex].second == 0 || symbols[randType][randIndex].second >= 25) {
			randType = Random::pickRandom(types);
			randIndex = Random::rand(symbols[randType].size());
		}
		symbols[randType][randIndex].second--;
		return symbols[randType][randIndex].first;
	}
	PuzzleSymbols(std::vector<std::pair<int, int>> symbolVec) {
		for (std::pair<int, int> s : symbolVec) {
			if (s.first == Gap || s.first == Start || s.first == Exit)
				symbols[static_cast<Symbol>(s.first)].push_back(s);
			else if (s.first & Dot)
				symbols[Dot].push_back(s);
			else
				symbols[static_cast<Symbol>(s.first & 0xF00)].push_back(s);
		}
		style = 0;
		if (any(Dot)) style |= HAS_DOTS;
		if (any(Stone)) style |= HAS_STONES;
		if (any(Star)) style |= HAS_STARS;
		if (any(Poly)) style |= HAS_SHAPERS;
		if (any(Triangle)) style |= HAS_TRIANGLES;
	}
};

