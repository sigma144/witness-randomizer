#include "APAudioPlayer.h"

#include "windows.h"
#include "mmsystem.h"

#include "../../App/resource.h"

APAudioPlayer* APAudioPlayer::_singleton = nullptr;

APAudioPlayer::APAudioPlayer() : Watchdog(0.2f) {

}

void APAudioPlayer::action() {
	if (!QueuedAudio.size()) return;

	APJingle nextAudio = QueuedAudio.front();

	PlayAppropriateJingle(nextAudio);

	QueuedAudio.pop();
}

void APAudioPlayer::create() {
	if (_singleton == nullptr) {
		_singleton = new APAudioPlayer();
	}
}

APAudioPlayer* APAudioPlayer::get() {
	create();
	return _singleton;
}

void APAudioPlayer::PlayAudio(APJingle jingle, APJingleBehavior queue) {
	if (queue == APJingleBehavior::PlayImmediate || queue == APJingleBehavior::DontQueue && !QueuedAudio.size()) {
		PlayAppropriateJingle(jingle);
	}

	if (queue == APJingleBehavior::Queue){
		QueuedAudio.push(jingle);
	}
}

void APAudioPlayer::PlayAppropriateJingle(APJingle jingle) {
	auto now = std::chrono::system_clock::now();
	
	int versionToPlay = 0;

	if (lastJinglePlayed == jingle && (now - lastJinglePlayedTime) < std::chrono::seconds(30)) {
		lastJinglePlayedVersion++;
		versionToPlay = lastJinglePlayedVersion;
	}
	else {
		lastJinglePlayedVersion = 0;
	}
	
	lastJinglePlayedTime = now;
	lastJinglePlayed = jingle;

	if (versionToPlay > jingleVersions[jingle].size() - 1) versionToPlay = jingleVersions[jingle].size() - 1;

	int resource = jingleVersions[jingle][versionToPlay];

	PlaySound(MAKEINTRESOURCE(resource), NULL, SND_RESOURCE | SND_ASYNC);
}