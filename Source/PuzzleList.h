#pragma once
#include "Generate.h"
#include "Special.h"
#include "Random.h"

class PuzzleList {

public: 

	void GenerateAllN();
	void GenerateAllH();

	PuzzleList() {
		generator = std::make_shared<Generate>();
		specialCase = std::make_shared<Special>(generator);
	}

	PuzzleList(std::shared_ptr<Generate> generator) {
		this->generator = generator;
		this->specialCase = std::make_shared<Special>(generator);
	}

	void setLoadingHandle(HWND handle) {
		_handle = handle;
		generator->setLoadingHandle(handle);
	}

	void setSeed(int seed, bool isRNG, bool colorblind) {
		this->seed = seed;
		this->seedIsRNG = isRNG;
		this->colorblind = colorblind;
		if (seed >= 0) generator->seed(seed);
		else generator->seed(Random::rand());
		generator->colorblind = colorblind;
	}

	void CopyTargets();
	void RestoreLineWidths();

	//--------------------------Normal difficulty---------------------------

	void GenerateTutorialN();
	void GenerateSymmetryN();
	void GenerateQuarryN();
	void GenerateBunkerN(); //Can't randomize because panels refuse to render the symbols
	void GenerateSwampN();
	void GenerateTreehouseN();
	void GenerateTownN();
	void GenerateVaultsN();
	void GenerateTrianglePanelsN();
	void GenerateMountainN();
	void GenerateCavesN();

	//Environmental areas - unless I can figure out how to mess with the game's assets, randomizing most of these puzzles isn't happening anytime soon
	void GenerateOrchardN();
	void GenerateDesertN(); //Just scramble the positions for now
	void GenerateShadowsN(); //Can't randomize
	void GenerateKeepN();
	void GenerateMonasteryN(); //Can't randomize
	void GenerateJungleN();

	//-------------------------Hard difficulty--------------------------

	void GenerateTutorialH();
	void GenerateSymmetryH();
	void GenerateQuarryH();
	void GenerateBunkerH();
	void GenerateSwampH();
	void GenerateTreehouseH();
	void GenerateTownH();
	void GenerateVaultsH();
	void GenerateTrianglePanelsH();
	void GenerateMountainH();
	void GenerateCavesH();

	//Environmental areas - unless I can figure out how to mess with the game's assets, randomizing some of these puzzles isn't happening anytime soon
	void GenerateOrchardH();
	void GenerateDesertH(); //Just scramble the positions for now
	void GenerateShadowsH(); //Can't randomize
	void GenerateKeepH();
	void GenerateMonasteryH(); //Can't randomize
	void GenerateJungleH();

private:
	std::shared_ptr<Generate> generator;
	std::shared_ptr<Special> specialCase;
	HWND _handle = nullptr;
	int seed = 0;
	bool seedIsRNG = false;
	bool colorblind = false;

	template <class T> T pick_random(std::vector<T>& vec) { return vec[Random::rand() % vec.size()]; }
	template <class T> T pick_random(std::set<T>& set) { auto it = set.begin(); std::advance(it, Random::rand() % set.size()); return *it; }
	template <class T> T pop_random(std::vector<T>& vec) {
		int i = Random::rand() % vec.size();
		T item = vec[i];
		vec.erase(vec.begin() + i);
		return item;
	}
	template <class T> T pop_random(std::set<T>& set) {
		auto it = set.begin();
		std::advance(it, Random::rand() % set.size());
		T item = *it;
		set.erase(item);
		return item;
	}
};

const std::map<int, float> LineWidths =
{
{0xa171,0.65},
{0x4ca4,0.65},
{0x5d,1},
{0x5e,1},
{0x5f,1},
{0x60,1},
{0x61,1},
{0x18af,1},
{0x1b,1},
{0x12c9,1},
{0x1c,1},
{0x1d,1},
{0x1e,1},
{0x1f,1},
{0x20,1},
{0x21,1},
{0x86,1},
{0x87,1},
{0x59,1},
{0x62,0.8},
{0x5c,0.6},
{0x8d,1},
{0x81,1},
{0x83,1},
{0x84,1},
{0x82,1},
{0x343a,1},
{0x22,0.8},
{0x23,0.8},
{0x24,0.8},
{0x25,0.8},
{0x26,0.8},
{0x7c,0.8},
{0x7e,0.8},
{0x75,0.8},
{0x73,0.8},
{0x77,0.8},
{0x79,0.8},
{0x65,0.7},
{0x6d,0.8},
{0x72,0.8},
{0x6f,0.9},
{0x70,0.9},
{0x71,0.85},
{0x76,0.85},
{0xa52,1},
{0xa61,1},
{0xa57,1},
{0xa64,1},
{0xa5b,1},
{0xa68,1},
{0x17c09,0.8},
{0x1e59,1},
{0xe0c,1},
{0x1489,1},
{0x148a,1},
{0x14d9,1},
{0x14e7,1},
{0x14e8,1},
{0x557,1},
{0x5f1,1},
{0x620,1},
{0x9f5,1},
{0x146c,1},
{0x3c12d,1},
{0x3686,1},
{0x14e9,1},
{0x367c,0.9},
{0x3c125,0.65},
{0x34d4,1},
{0x21d5,1},
{0x21b3,1},
{0x21b4,1},
{0x21b0,1},
{0x21af,1},
{0x21ae,1},
{0x21b5,1},
{0x21b6,1},
{0x21b7,0.9},
{0x21bb,0.9},
{0x9db5,0.9},
{0x9db1,0.9},
{0x3c124,0.9},
{0x9db3,0.8},
{0x9db4,0.8},
{0xa3cb,0.6},
{0xa3cc,0.6},
{0xa3d0,0.6},
{0x469,0.7},
{0x472,0.7},
{0x262,0.65},
{0x474,0.65},
{0x553,0.65},
{0x56f,0.65},
{0x390,0.7},
{0x10ca,0.7},
{0x983,0.65},
{0x984,0.65},
{0x986,0.65},
{0x985,0.65},
{0x987,0.65},
{0x181a9,0.6},
{0x609,0.5},
{0x982,0.65},
{0x97f,0.65},
{0x98f,0.6},
{0x990,0.6},
{0x17c0d,0.6},
{0x17c0e,0.6},
{0x999,0.7},
{0x99d,0.65},
{0x9a0,0.65},
{0x9a1,0.65},
{0x7,0.7},
{0x8,0.65},
{0x9,0.65},
{0xa,0.6},
{0x3b2,0.6},
{0xa1e,0.6},
{0xc2e,0.6},
{0xe3a,0.6},
{0x9a6,0.5},
{0x9ab,0.7},
{0x9ad,0.65},
{0x9ae,0.65},
{0x9af,0.65},
{0x6,0.65},
{0x2,0.65},
{0x4,0.65},
{0x5,0.65},
{0x13e6,0.65},
{0x596,0.65},
{0x1,0.7},
{0x14d2,0.65},
{0x14d4,0.65},
{0x14d1,0.65},
{0x17c05,0.9},
{0x17c02,0.9},
{0x2886,0.9},
{0x17d72,1},
{0x17d8f,1},
{0x17d74,1},
{0x17dac,1},
{0x17d9e,1},
{0x17db9,1},
{0x17d9c,1},
{0x17dc2,1},
{0x17dc4,1},
{0x17dc8,1},
{0x17dc7,1},
{0x17ce4,1},
{0x17d2d,0.84},
{0x17d6c,0.84},
{0x17d9b,1},
{0x17d99,1},
{0x17daa,1},
{0x17d97,1},
{0x17bdf,1},
{0x17d91,0.9},
{0x17dc6,0.9},
{0x17db3,1},
{0x17db5,1},
{0x17db6,1},
{0x17dc0,1},
{0x17dd7,1},
{0x17dd9,1},
{0x17db8,1},
{0x17ddc,1},
{0x17dd1,0.8},
{0x17dde,1},
{0x17de3,1},
{0x17dec,0.9},
{0x17dae,0.9},
{0x17db0,0.9},
{0x17ddb,0.9},
{0x17d88,1},
{0x17db4,1},
{0x17d8c,1},
{0x17ce3,1},
{0x17dcd,1},
{0x17db2,1},
{0x17dcc,1},
{0x17dca,1},
{0x17d8e,1},
{0x17db7,1},
{0x17db1,1},
{0x17da2,1},
{0x17e3c,1},
{0x17e4d,1},
{0x17e4f,1},
{0x17e52,0.8},
{0x17e5b,0.75},
{0x17e5f,1},
{0x17e61,0.8},
{0x2899c,1},
{0x28a33,1},
{0x28abf,1},
{0x28ac0,1},
{0x28ac1,1},
{0x28ad9,0.9},
{0x28ac7,0.9},
{0x28ac8,0.9},
{0x28aca,0.9},
{0x28acb,0.9},
{0x28acc,0.9},
{0x28a69,1},
{0x34e3,1},
{0x3c0c,0.7},
{0x3c08,0.7},
{0xa0c8,0.65},
{0x17f89,0.7},
{0xa168,0.65},
{0x33ab2,0.7},
{0x33d4,0.65},
{0xcc7b,0.7},
{0x2a6,0.65},
{0xafb,0.8},
{0x17c34,0.5},
{0x9e73,1},
{0x9e75,1},
{0x9e78,1},
{0x9e79,1},
{0x9e6c,1},
{0x9e6f,1},
{0x9e6b,1},
{0x9e7a,1},
{0x9e71,1},
{0x9e72,1},
{0x9e69,1},
{0x9e7b,1},
{0x9ead,0.8},
{0x9eaf,0.8},
{0x33af5,0.8},
{0x33af7,0.8},
{0x9f6e,0.8},
{0x9fd3,1},
{0x9fd4,1},
{0x9fd6,1},
{0x9fd7,1},
{0x9fd8,1},
{0x9fcc,1},
{0x9fce,1},
{0x9fcf,1},
{0x9fd0,1},
{0x9fd1,1},
{0x9fd2,1},
{0x9e86,0.85},
{0x9e39,0.9},
{0x9eff,0.9},
{0x9f01,0.9},
{0x9fc1,0.9},
{0x9f8e,0.9},
{0x9fda,0.9},
{0x383d,0.85},
{0x383a,0.85},
{0x383f,0.85},
{0x9e56,0.85},
{0x3859,0.8},
{0x9e5a,0.85},
{0x339bb,0.8},
{0x33961,0.8},
{0x17fa2,0.7},
{0xff8,0.8},
{0x1a0d,0.7},
{0x18a0,1},
{0x9a4,0.9},
{0xa72,0.9},
{0x190,1},
{0x558,1},
{0x567,1},
{0x6fe,1},
{0x8b8,1},
{0x973,1},
{0x97b,1},
{0x97d,1},
{0x97e,1},
{0x994,1},
{0x334d5,1},
{0x995,1},
{0x996,1},
{0x998,1},
{0x32962,0.8},
{0x32966,1},
{0x1a31,0.8},
{0xb71,1},
{0x288ea,0.75},
{0x288fc,0.75},
{0x289e7,0.75},
{0x288aa,0.75},
{0xa16b,0.65},
{0xa2ce,0.65},
{0xa2d7,0.65},
{0xa2dd,0.65},
{0xa2ea,0.65},
{0x17fb9,0.9},
{0x8f,1},
{0x6b,1},
{0x8b,1},
{0x8c,1},
{0x8a,1},
{0x89,1},
{0x6a,1},
{0x6c,1},
{0x27,1},
{0x28,1},
{0x29,1},
{0x17cf2,0.8},
{0x21d7,1},
{0x9dd5,0.75},
{0xa16e,0.65},
{0x39b4,0.7},
{0x9e85,0.55},
{0x33ea,1},
{0x1be9,1},
{0x1cd3,1},
{0x1d3f,1},
{0x3317,0.4},
{0x360e,0.5},
{0x26d,1},
{0x26e,1},
{0x26f,1},
{0xc3f,1},
{0xc41,1},
{0x14b2,1},
{0x1a54,1},
{0xb0,1},
{0x1c349,1},
{0x1e5a,1},
{0x9e57,0.8},
{0x288c,1},
{0xa182,1},
{0x2700b,1},
{0x334db,1},
{0x56e,0.7},
{0x17cab,0.7},
{0xc339,1},
{0xa249,1},
{0x28998,0.9},
{0x28a0d,1},
{0x17f5f,0.7},
{0x17f93,0.8},
{0x17cfb,0.8},
{0x3c12b,0.8},
{0x17ce7,0.8},
{0x17cf0,0.8},
{0x17fa9,0.8},
{0x17fa0,0.8},
{0x17d27,0.8},
{0x17d28,0.8},
{0x17d01,0.8},
{0x17f9b,0.8},
{0x17c42,0.8},
{0x386fa,0.8},
{0x1c33f,0.8},
{0x196e2,0.8},
{0x19809,0.8},
{0x19806,0.8},
{0x196f8,0.8},
{0x1972f,0.8},
{0x19797,0.8},
{0x1979a,0.8},
{0x197e0,0.8},
{0x197e8,0.8},
{0x197e5,0.8},
{0x9f7f,0.65},
};
