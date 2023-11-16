#include "APAudioPlayer.h"

#include "windows.h"
#include "mmsystem.h"

#include "../../App/resource.h"

APAudioPlayer* APAudioPlayer::_singleton = nullptr;

APAudioPlayer::APAudioPlayer() : Watchdog(0.1f) {

}

void APAudioPlayer::action() {
	if (!QueuedAudio.size()) return;

	std::pair<APJingle, bool> nextAudio = QueuedAudio.front();

	PlayAppropriateJingle(nextAudio.first, nextAudio.second, false);

	QueuedAudio.pop();
}

void APAudioPlayer::create() {
	if (_singleton == nullptr) {
		_singleton = new APAudioPlayer();
		_singleton->start();
	}
}

APAudioPlayer* APAudioPlayer::get() {
	create();
	return _singleton;
}

void APAudioPlayer::PlayAudio(APJingle jingle, APJingleBehavior queue, bool epicVersion) {
	if (queue == APJingleBehavior::PlayImmediate) {
		PlayAppropriateJingle(jingle, epicVersion, true);
	}

	if (queue == APJingleBehavior::Queue){
		QueuedAudio.push({ jingle, epicVersion });
	}

	if (queue == APJingleBehavior::DontQueue) {
		if (!QueuedAudio.size()) QueuedAudio.push({ jingle, epicVersion });
	}
}

void APAudioPlayer::PlayJingle(int resource, bool async) {
	if (async) PlaySound(MAKEINTRESOURCE(resource), NULL, SND_RESOURCE | SND_ASYNC);
	else PlaySound(MAKEINTRESOURCE(resource), NULL, SND_RESOURCE);
}

void APAudioPlayer::PlayAppropriateJingle(APJingle jingle, bool epicVersion, bool async) {
	auto now = std::chrono::system_clock::now();

	if (understatedJingles.count(jingle)) {
		int resource = jingleVersions[jingle][0];

		PlayJingle(resource, async);
		return;
	}
	
	if ((now - lastPanelJinglePlayedTime) > std::chrono::seconds(60)) {
		panelChain = -1;
		lastPanelJinglePlayed = APJingle::None;
	}
	if ((now - lastEPJinglePlayedTime) > std::chrono::seconds(60)) {
		epChain = -1;
		lastEPJinglePlayed = APJingle::None;
	}

	int versionToPlay = 0;

	if (panelJingles.count(jingle) || dogJingles.count(jingle)) {
		panelChain++;
		if (lastPanelJinglePlayed == jingle && !epicVersion) panelChain++; // If the same jingle just played, advance the counter another time so the same jingle doesn't play twice.
		
		lastPanelJinglePlayed = epicVersion ? APJingle::None : jingle;

		versionToPlay = panelChain / 2;
		lastPanelJinglePlayedTime = now;
	}
	else if (epJingles.count(jingle)) {
		epChain++;
		if (lastEPJinglePlayed == jingle && !epicVersion) epChain++;

		lastEPJinglePlayed = epicVersion ? APJingle::None : jingle;

		versionToPlay = epChain / 2;
		lastEPJinglePlayedTime = now;
	}

	if (epicVersion) {
		lastPanelJinglePlayedTime = now; // Epic doesn't interrupt a panel chain. Also, Epic should still advance the counter by one.

		int resource = jingleEpicVersions[jingle];
		PlayJingle(resource, async);
		return;
	}

	if (versionToPlay < 0) versionToPlay = 0;
	if (versionToPlay > jingleVersions[jingle].size() - 1) versionToPlay = jingleVersions[jingle].size() - 1;

	int resource = jingleVersions[jingle][versionToPlay];

	PlayJingle(resource, async);
}