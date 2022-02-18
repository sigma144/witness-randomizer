#pragma once
#include "..\Randomizer.h"
#include <memory>
#include <set>
#include <map>

class APRandomizer {
public:
	int Seed = 12345;

	APRandomizer();

	void Connect();
	void GenerateNormal();

private:
	std::shared_ptr<Memory> _memory;

	void StartWatching();

	void PreventSnipes();
};