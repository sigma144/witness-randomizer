#include "../Watchdog.h"
#include <chrono>
#include <queue>
#include <set>
#include <utility>
#include "../../App/resource.h"

enum APJingle
{
	EPProgression,
	EPUseful,
	EPFiller,
	EPTrap,

	PanelProgression,
	PanelUseful,
	PanelFiller,
	PanelTrap,
};

enum APJingleBehavior
{
	DontQueue,
	Queue,
	PlayImmediate,
};

inline const std::set<int> hardPanels = {
	0x09FDA, // Mountain Giant Puzzle
	0x17C34, // Mountain Entry Panel

	0x03542, // Mountainside Vault Box
	0x0356B, // Challenge Vault Box
	0x03481, // Tutorial Vault Box
	0x0339E, // Desert Vault Box
	0x03535, // Shipwreck Vault Box

	0x33961, 0x339BB, // Final Room Pillars
};

class APAudioPlayer : public Watchdog
{
private:
	APAudioPlayer();

	virtual void action();

	static APAudioPlayer* _singleton;

	std::queue<std::pair<APJingle,bool>> QueuedAudio = {};

	APJingle lastJinglePlayed;
	std::chrono::system_clock::time_point lastJinglePlayedTime;
	int lastJinglePlayedVersion;

	void PlayAppropriateJingle(APJingle jingle, bool epicVersion);

	std::map<APJingle, std::vector<int>> jingleVersions = {
		{PanelFiller, {IDR_WAVE5, IDR_WAVE6}},
		{PanelUseful, {IDR_WAVE15, IDR_WAVE16}},
		{PanelProgression, {IDR_WAVE8, IDR_WAVE9, IDR_WAVE10}},
		{PanelTrap, {IDR_WAVE12, IDR_WAVE13}},

		{EPFiller, {IDR_WAVE1}},
		{EPProgression, {IDR_WAVE2}},
		{EPTrap, {IDR_WAVE3}},
		{EPUseful, {IDR_WAVE4}},
	};

	std::map<APJingle, int> jingleEpicVersions = {
		{PanelFiller, IDR_WAVE7},
		{PanelUseful, IDR_WAVE17},
		{PanelProgression, IDR_WAVE11},
		{PanelTrap, IDR_WAVE14},

		{EPFiller, IDR_WAVE1},
		{EPProgression, IDR_WAVE2},
		{EPTrap, IDR_WAVE3},
		{EPUseful, IDR_WAVE4},
	};
public:

	static void create();
	static APAudioPlayer* get();
	void PlayAudio(APJingle jingle, APJingleBehavior queue, bool epicVersion);
	void PlayAudio(APJingle jingle, APJingleBehavior queue) {
		PlayAudio(jingle, queue, false);
	}
};