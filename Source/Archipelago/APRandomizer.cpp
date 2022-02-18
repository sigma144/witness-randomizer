#include "APRandomizer.h"
#include "APWatchdog.h"

int Seed;

APRandomizer::APRandomizer() {
	_memory = std::make_shared<Memory>("witness64_d3d11.exe");
}

void APRandomizer::Connect() {

}


void APRandomizer::GenerateNormal() {
	StartWatching(); //From now on, we check completion.
	PreventSnipes(); //Prevents Snipes to preserve progression randomizer experience
}


void APRandomizer::StartWatching() {
	//test
	(new APWatchdog())->start();
	//holy shit it works
}

void APRandomizer::PreventSnipes()
{
	// Distance-gate shadows laser to prevent sniping through the bars
	_memory->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { 2.5 });
}