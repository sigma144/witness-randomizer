#include "../Watchdog.h"
#include <chrono>
#include <queue>
#include <set>
#include <utility>
#include <random>
#include <any>
#include "../../App/resource.h"

enum APJingle
{
	None,

	EPProgUseful,
	EPProgression,
	EPUseful,
	EPFiller,
	EPTrap,

	PanelProgUseful,
	PanelProgression,
	PanelUseful,
	PanelFiller,
	PanelTrap,

	IncomingProgUseful,
	IncomingProgression,
	IncomingUseful,
	IncomingFiller,
	IncomingTrap,

	DogProgUseful,
	DogProgression,
	DogUseful,
	DogFiller,
	DogTrap,

	UnderstatedProgUseful,
	UnderstatedFiller,
	UnderstatedProgression,
	UnderstatedTrap,
	UnderstatedUseful,

	EntityHunt,
	UnderstatedEntityHunt,

	Victory,
	UnderstatedVictory,
	FirstJingle,
	DeathLink,
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

inline const std::map<std::string, std::map<std::string, int>> pillarJingles = {
	{"E", {
		{"progression", IDR_WAVE43},
		{"useful", IDR_WAVE49},
		{"trap", IDR_WAVE46},
		{"filler", IDR_WAVE40},
		{"proguseful", IDR_WAVE68},
	}},
	{"Ebm", {
		{"progression", IDR_WAVE42},
		{"useful", IDR_WAVE48},
		{"trap", IDR_WAVE45},
		{"filler", IDR_WAVE39},
		{"proguseful", IDR_WAVE67},
	}},
	{"B", {
		{"progression", IDR_WAVE41},
		{"useful", IDR_WAVE47},
		{"trap", IDR_WAVE44},
		{"filler", IDR_WAVE38},
		{"proguseful", IDR_WAVE66},
	}},
};

inline const std::vector<int> entityHuntJingles = {
	IDR_WAVE52, // 1
	IDR_WAVE61, // 8
	IDR_WAVE62, // 9
	IDR_WAVE75, // 12
	IDR_WAVE56, // 5
	IDR_WAVE53, // 2
	IDR_WAVE55, // 4
	IDR_WAVE54, // 3
	IDR_WAVE74, // 14
	IDR_WAVE76, // 13
	IDR_WAVE57, // 6
	IDR_WAVE64, // 11
	IDR_WAVE58, // 7
	IDR_WAVE63, // 10
};

namespace SoLoud {
	class Soloud;
	class Wav;
}

class APAudioPlayer : public Watchdog
{
private:
	APAudioPlayer();

	virtual void action();

	static APAudioPlayer* _singleton;

	std::map<int, SoLoud::Wav*> preloadedAudioFiles = {};

	std::queue<std::pair<APJingle, std::any>> QueuedAudio = {};
	std::queue<std::pair<APJingle, std::any>> QueuedAsyncAudio = {};
	std::queue<int> QueueDirectAsyncAudio = {};

	std::chrono::system_clock::time_point lastPanelJinglePlayedTime;
	std::chrono::system_clock::time_point lastEPJinglePlayedTime;
	APJingle lastPanelJinglePlayed;
	APJingle lastEPJinglePlayed;
	int panelChain;
	int epChain;
	int lastEntityHuntIndex = -1;

	SoLoud::Wav* currentlyPlayingSyncSound;
	int currentlyPlayingSyncHandle = -1;

	int challengeHandle = -1;

	void PlayJingleMain(int resource);
	void PlayJingleBlocking(int resource);

	void PlayJingle(int resource, bool async) {
		if (async) PlayJingleBlocking(resource);
		else PlayJingleMain(resource);
	}

	void PlayAppropriateJingle(APJingle jingle, std::any extraInfo, bool async);

	std::map<APJingle, std::vector<int>> jingleVersions = {
		{PanelProgUseful, {IDR_WAVE77, IDR_WAVE78, IDR_WAVE79, IDR_WAVE80, IDR_WAVE81}},
		{PanelFiller, {IDR_WAVE5, IDR_WAVE6, IDR_WAVE22, IDR_WAVE23}},
		{PanelUseful, {IDR_WAVE15, IDR_WAVE16, IDR_WAVE32, IDR_WAVE33, IDR_WAVE59}},
		{PanelProgression, {IDR_WAVE8, IDR_WAVE9, IDR_WAVE10, IDR_WAVE28, IDR_WAVE29}},
		{PanelTrap, {IDR_WAVE12, IDR_WAVE13, IDR_WAVE31}},

		{EPProgUseful, {IDR_WAVE72, IDR_WAVE71}},
		{EPFiller, {IDR_WAVE1, IDR_WAVE18}},
		{EPProgression, {IDR_WAVE2, IDR_WAVE19}},
		{EPTrap, {IDR_WAVE3, IDR_WAVE20}},
		{EPUseful, {IDR_WAVE4, IDR_WAVE21}},

		{UnderstatedProgUseful, {IDR_WAVE71}},
		{UnderstatedFiller, {IDR_WAVE1}},
		{UnderstatedProgression, {IDR_WAVE2}},
		{UnderstatedTrap, {IDR_WAVE3}},
		{UnderstatedUseful, {IDR_WAVE4}},
		{UnderstatedEntityHunt, {IDR_WAVE60}},

		{IncomingProgUseful, {IDR_WAVE73}},
		{IncomingFiller, {IDR_WAVE24}},
		{IncomingProgression, {IDR_WAVE25}},
		{IncomingTrap, {IDR_WAVE26}},
		{IncomingUseful, {IDR_WAVE27}},

		{DogProgUseful, {IDR_WAVE70}},
		{DogFiller, {IDR_WAVE35}},
		{DogProgression, {IDR_WAVE36}},
		{DogTrap, {IDR_WAVE34}},
		{DogUseful, {IDR_WAVE37}},

		{UnderstatedVictory, {IDR_WAVE69}},
		{Victory, {IDR_WAVE30}},
		{FirstJingle, {IDR_WAVE50}},
		{DeathLink, {IDR_WAVE51}}
	};

	std::map<APJingle, int> jingleEpicVersions = {
		{PanelProgUseful, IDR_WAVE65},
		{PanelFiller, IDR_WAVE7},
		{PanelUseful, IDR_WAVE17},
		{PanelProgression, IDR_WAVE11},
		{PanelTrap, IDR_WAVE14},

		{EPProgUseful, IDR_WAVE71},
		{EPFiller, IDR_WAVE1},
		{EPProgression, IDR_WAVE2},
		{EPTrap, IDR_WAVE3},
		{EPUseful, IDR_WAVE4},

		{IncomingProgUseful, IDR_WAVE73},
		{IncomingFiller, IDR_WAVE24},
		{IncomingProgression, IDR_WAVE25},
		{IncomingTrap, IDR_WAVE26},
		{IncomingUseful, IDR_WAVE27},

		{UnderstatedProgUseful, {IDR_WAVE71}},
		{UnderstatedFiller, {IDR_WAVE1}},
		{UnderstatedProgression, {IDR_WAVE2}},
		{UnderstatedTrap, {IDR_WAVE3}},
		{UnderstatedUseful, {IDR_WAVE4}},

		{DogProgUseful, {IDR_WAVE70}},
		{DogFiller, {IDR_WAVE35}},
		{DogProgression, {IDR_WAVE36}},
		{DogTrap, {IDR_WAVE34}},
		{DogUseful, {IDR_WAVE37}},

		{UnderstatedVictory, {IDR_WAVE69}},
		{Victory, {IDR_WAVE30}},
		{FirstJingle, {IDR_WAVE50}},
		{DeathLink, {IDR_WAVE51}},
	};

	std::set<APJingle> epJingles = {
		EPFiller,
		EPProgUseful,
		EPProgression,
		EPTrap,
		EPUseful
	};

	std::set<APJingle> panelJingles = {
		PanelFiller,
		PanelUseful,
		PanelProgUseful,
		PanelProgression,
		PanelTrap
	};

	std::set<APJingle> dogJingles = {
		DogFiller,
		DogUseful,
		DogProgUseful,
		DogProgression,
		DogTrap
	};

	std::set<APJingle> understatedJingles = {
		UnderstatedFiller,
		UnderstatedProgUseful,
		UnderstatedProgression,
		UnderstatedTrap,
		UnderstatedUseful,
		UnderstatedEntityHunt,
		UnderstatedVictory,
	};

	std::mt19937 rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());

public:

	static void create();
	static APAudioPlayer* get();
	void PlayAudio(APJingle jingle, APJingleBehavior queue, std::any extraInfo);
	void PlayAudio(APJingle jingle, APJingleBehavior queue) {
		PlayAudio(jingle, queue, false);
	}
	void PlayFinalRoomJingle(std::string type, APJingleBehavior queue, double finalRoomMusicTimer);
	void ResetRampingCooldownPanel();
	void ResetRampingCooldownEP();

	bool HasCustomChallengeSong();
	void StartCustomChallengeSong();
	void StopCustomChallengeSong();
	bool ChallengeSongHasEnded();

	SoLoud::Soloud* gSoloud; // SoLoud engine

	bool challengeSongIsPlaying = false;
};