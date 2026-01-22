#pragma once
#include "Generate.h"
#include "Special.h"
#include "Random.h"

class PuzzleList {

public: 
	PuzzleList() {
		special = Special(&g);
	}

	void setLoadingHandle(HWND handle) {
		_handle = handle;
		g.setLoadingHandle(handle);
	}

	void setSeed(int seed, bool isRNG) {
		this->seed = seed;
		this->seedIsRNG = isRNG;
		this->colorblind = colorblind;
		if (seed >= 0) g.seed(seed);
		else g.seed(Random::rand());
	}

	void CopyTargets();

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

	void GenerateAllN();

	//-------------------------Expert difficulty--------------------------

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

	void GenerateAllH();

private:
	Generate g;
	Special special;
	HWND _handle = nullptr;
	int seed = 0;
	bool seedIsRNG = false;
	bool colorblind = false;
};