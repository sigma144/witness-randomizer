#include "HUDManager.h"

#include "Memory.h"


void HudManager::update(float deltaSeconds) {
	updateBannerMessages(deltaSeconds);
	updateSubtitleMessages(deltaSeconds);
}

void HudManager::queueBannerMessage(std::string text, RgbColor color, float duration) {
	queuedBannerMessages.push(BannerMessage(text, color, duration));
}

void HudManager::showSubtitleMessage(std::string text, float duration) {
	writeSubtitle({ text });
	subtitleOverrideTimeRemaining = duration;
}

void HudManager::clearSubtitleMessage() {
	subtitleOverrideTimeRemaining = 0;
	subtitlesDirty = true;
}

void HudManager::setWorldMessage(std::string text) {
	if (worldMessage != text) {
		worldMessage = text;
		subtitlesDirty = true;
	}
}

void HudManager::setStatusMessage(std::string text) {
	if (statusMessage != text) {
		statusMessage = text;
		subtitlesDirty = true;
	}
}

void HudManager::setActionHint(std::string text) {
	if (actionHint != text) {
		actionHint = text;
		subtitlesDirty = true;
	}
}

void HudManager::updateBannerMessages(float deltaSeconds) {
	// If we're currently displaying a banner, decrement remaining message time and, if time still remains, return.
	if (bannerTimeRemaining > 0) {
		bannerTimeRemaining -= deltaSeconds;
		if (bannerTimeRemaining > 0) {
			return;
		}
	}

	// If we have a queued banner message, display it now.
	if (queuedBannerMessages.size() > 0) {
		const BannerMessage& message = queuedBannerMessages.front();

		memory->DisplayHudMessage(message.text, { message.color.R, message.color.G, message.color.B });
		bannerTimeRemaining = message.duration;

		queuedBannerMessages.pop();
	}
}

void HudManager::updateSubtitleMessages(float deltaSeconds) {
	// If we're currently displaying a subtitle message, decrement its remaining time. If this exhausts the remaining time, mark
	//   subtitles as needing refresh.
	if (subtitleOverrideTimeRemaining > 0) {
		subtitleOverrideTimeRemaining -= deltaSeconds;
		if (subtitleOverrideTimeRemaining <= 0) {
			// Mark subtitles as needing refreshing.
			subtitlesDirty = true;
		}
		else {
			// Do not process any further subtitle logic, since we're still overriding the contents.
			return;
		}
	}

	if (subtitlesDirty) {
		// When displaying system-level messages like input hints or status displays, we use the built-in subtitle functionality
		//   to render the text. This shows up to three lines of text, represented as separate strings.
		// We have up to three messages to show to the user, which we will show in a fixed order:
		//  - world messages		Quarry Laser requires Shapers and Erasers.
		//  - status messages		Speed boost! 30 seconds remaining.
		//  - action hints			[Hold TAB] Skip puzzle (have 2 skips).
		std::vector<std::string> lines;
		if (!worldMessage.empty()) lines.push_back(worldMessage);
		if (!statusMessage.empty()) lines.push_back(statusMessage);
		if (!actionHint.empty()) lines.push_back(actionHint);

		writeSubtitle(lines);
		subtitlesDirty = false;
	}
}

void HudManager::writeSubtitle(std::vector<std::string> lines) {
	// Subtitle rendering ignores newlines in favor of having separate draw calls for each line, so we need to split each input
	//   line by newline characters and append those to the list.
	std::vector<std::string> expandedLines;
	for (const std::string& line : lines) {
		std::vector<std::string> splitLines = splitByNewline(line);
		expandedLines.insert(expandedLines.end(), splitLines.begin(), splitLines.end());
	}

	// Since we want to keep this text as low on the screen as possible, we need to write our text using the lowest possible rows.
	//   In order to do this, we can compute an offset into the list of lines based on its size such that the last line in the
	//   list corresponds to the last line in the subtitle.
	int lineOffset = 3 - expandedLines.size(); // NOTE: If the message is longer than 3 lines, only the last 3 will be displayed.

	memory->DisplaySubtitles(
		0 - lineOffset >= 0 ? expandedLines[0 - lineOffset] : "",
		1 - lineOffset >= 0 ? expandedLines[1 - lineOffset] : "",
		2 - lineOffset >= 0 ? expandedLines[2 - lineOffset] : ""
	);
}

std::vector<std::string> HudManager::splitByNewline(std::string input) {
	std::vector<std::string> splitLines;

	std::string::size_type searchIndex = 0, previousIndex = 0;
	while (true) {
		searchIndex = input.find('\n', previousIndex);
		if (searchIndex != std::string::npos) {
			splitLines.push_back(input.substr(previousIndex, searchIndex - previousIndex));
			previousIndex = searchIndex + 1;
		}
		else {
			break;
		}
	}

	if (previousIndex < input.size()) {
		splitLines.push_back(input.substr(previousIndex));
	}

	return splitLines;
}