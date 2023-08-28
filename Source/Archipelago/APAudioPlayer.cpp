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

void APAudioPlayer::PlayAppropriateJingle(APJingle jingle, bool epicVersion, bool async) {
	auto now = std::chrono::system_clock::now();
	
	int versionToPlay = 0;

	if (lastJinglePlayed == jingle && (now - lastJinglePlayedTime) < std::chrono::seconds(60) && !epicVersion) {
		lastJinglePlayedVersion++;
		versionToPlay = lastJinglePlayedVersion;
	}
	else if (epicVersion) {
		versionToPlay = 10000;
		lastJinglePlayedVersion = versionToPlay;
	}
	else {
		lastJinglePlayedVersion = 0;
	}
	
	lastJinglePlayedTime = now;
	lastJinglePlayed = jingle;

	if (versionToPlay > jingleVersions[jingle].size() - 1) versionToPlay = jingleVersions[jingle].size() - 1;

	int resource = jingleVersions[jingle][versionToPlay];

	if (epicVersion) {
		resource = jingleEpicVersions[jingle];
	}

	if (async) PlaySound(MAKEINTRESOURCE(resource), NULL, SND_RESOURCE | SND_ASYNC);
	else PlaySound(MAKEINTRESOURCE(resource), NULL, SND_RESOURCE);
}