#include "../Watchdog.h"
#include <chrono>
#include <queue>
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

class APAudioPlayer : public Watchdog
{
private:
	APAudioPlayer();

	virtual void action();

	static APAudioPlayer* _singleton;

	std::queue<APJingle> QueuedAudio = {};

	APJingle lastJinglePlayed;
	std::chrono::system_clock::time_point lastJinglePlayedTime;
	int lastJinglePlayedVersion;

	void PlayAppropriateJingle(APJingle jingle);

	std::map<APJingle, std::vector<int>> jingleVersions = {
		{PanelFiller, {IDR_WAVE5, IDR_WAVE6}},
		{PanelUseful, {IDR_WAVE12, IDR_WAVE13}},
		{PanelProgression, {IDR_WAVE7, IDR_WAVE8, IDR_WAVE9}},
		{PanelTrap, {IDR_WAVE10, IDR_WAVE11}},

		{EPFiller, {IDR_WAVE1}},
		{EPProgression, {IDR_WAVE2}},
		{EPTrap, {IDR_WAVE3}},
		{EPUseful, {IDR_WAVE4}},
	};
public:

	static void create();
	static APAudioPlayer* get();
	void PlayAudio(APJingle jingle, APJingleBehavior queue);
};