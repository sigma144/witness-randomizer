#include "../Watchdog.h"
#include <chrono>
#include <queue>
#include <set>
#include <utility>
#include "../../App/resource.h"

enum APJingle
{
	None,

	EPProgression,
	EPUseful,
	EPFiller,
	EPTrap,

	PanelProgression,
	PanelUseful,
	PanelFiller,
	PanelTrap,

	IncomingProgression,
	IncomingUseful,
	IncomingFiller,
	IncomingTrap,

	DogProgression,
	DogUseful,
	DogFiller,
	DogTrap,

	UnderstatedFiller,
	UnderstatedProgression,
	UnderstatedTrap,
	UnderstatedUseful,

	Victory,
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
		{"filler", IDR_WAVE40}
	}},
	{"Ebm", {
		{"progression", IDR_WAVE42},
		{"useful", IDR_WAVE48},
		{"trap", IDR_WAVE45},
		{"filler", IDR_WAVE39}
	}},
	{"B", {
		{"progression", IDR_WAVE41},
		{"useful", IDR_WAVE47},
		{"trap", IDR_WAVE44},
		{"filler", IDR_WAVE38}
	}},
};

class APAudioPlayer : public Watchdog
{
private:
	APAudioPlayer();

	virtual void action();

	static APAudioPlayer* _singleton;

	std::queue<std::pair<APJingle,bool>> QueuedAudio = {};

	std::chrono::system_clock::time_point lastPanelJinglePlayedTime;
	std::chrono::system_clock::time_point lastEPJinglePlayedTime;
	APJingle lastPanelJinglePlayed;
	APJingle lastEPJinglePlayed;
	int panelChain;
	int epChain;

	void PlayJingle(int resource, bool async);
	void PlayAppropriateJingle(APJingle jingle, bool epicVersion, bool async);

	std::map<APJingle, std::vector<int>> jingleVersions = {
		{PanelFiller, {IDR_WAVE5, IDR_WAVE6, IDR_WAVE22, IDR_WAVE23}},
		{PanelUseful, {IDR_WAVE15, IDR_WAVE16, IDR_WAVE32, IDR_WAVE33}},
		{PanelProgression, {IDR_WAVE8, IDR_WAVE9, IDR_WAVE10, IDR_WAVE28, IDR_WAVE29}},
		{PanelTrap, {IDR_WAVE12, IDR_WAVE13, IDR_WAVE31}},

		{EPFiller, {IDR_WAVE1, IDR_WAVE18}},
		{EPProgression, {IDR_WAVE2, IDR_WAVE19}},
		{EPTrap, {IDR_WAVE3, IDR_WAVE20}},
		{EPUseful, {IDR_WAVE4, IDR_WAVE21}},

		{UnderstatedFiller, {IDR_WAVE1}},
		{UnderstatedProgression, {IDR_WAVE2}},
		{UnderstatedTrap, {IDR_WAVE3}},
		{UnderstatedUseful, {IDR_WAVE4}},

		{IncomingFiller, {IDR_WAVE24}},
		{IncomingProgression, {IDR_WAVE25}},
		{IncomingTrap, {IDR_WAVE26}},
		{IncomingUseful, {IDR_WAVE27}},

		{DogFiller, {IDR_WAVE35}},
		{DogProgression, {IDR_WAVE36}},
		{DogTrap, {IDR_WAVE34}},
		{DogUseful, {IDR_WAVE37}},

		{Victory, {IDR_WAVE30}},
		{FirstJingle, {IDR_WAVE50}},
		{DeathLink, {IDR_WAVE51}}
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

		{IncomingFiller, IDR_WAVE24},
		{IncomingProgression, IDR_WAVE25},
		{IncomingTrap, IDR_WAVE26},
		{IncomingUseful, IDR_WAVE27},

		{UnderstatedFiller, {IDR_WAVE1}},
		{UnderstatedProgression, {IDR_WAVE2}},
		{UnderstatedTrap, {IDR_WAVE3}},
		{UnderstatedUseful, {IDR_WAVE4}},

		{DogFiller, {IDR_WAVE35}},
		{DogProgression, {IDR_WAVE36}},
		{DogTrap, {IDR_WAVE34}},
		{DogUseful, {IDR_WAVE37}},

		{Victory, {IDR_WAVE30}},
		{FirstJingle, {IDR_WAVE50}},
	};

	std::set<APJingle> epJingles = {
		EPFiller,
		EPProgression,
		EPTrap,
		EPUseful
	};

	std::set<APJingle> panelJingles = {
		PanelFiller,
		PanelUseful,
		PanelProgression,
		PanelTrap
	};

	std::set<APJingle> dogJingles = {
		DogFiller,
		DogUseful,
		DogProgression,
		DogTrap
	};

	std::set<APJingle> understatedJingles = {
		UnderstatedFiller,
		UnderstatedProgression,
		UnderstatedTrap,
		UnderstatedUseful
	};

public:

	static void create();
	static APAudioPlayer* get();
	void PlayAudio(APJingle jingle, APJingleBehavior queue, bool epicVersion);
	void PlayAudio(APJingle jingle, APJingleBehavior queue) {
		PlayAudio(jingle, queue, false);
	}
	void PlayFinalRoomJingle(std::string type, APJingleBehavior queue, double finalRoomMusicTimer);
};