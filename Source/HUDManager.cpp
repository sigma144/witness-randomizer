#include "HUDManager.h"

#include "Memory.h"


#define SUBTITLE_MAX_LINE_LENGTH 70

HudManager::HudManager() {
	findSetSubtitleOffsets();
	setSubtitleSize(SubtitleSize::Small);
}

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

void HudManager::setSubtitleSize(SubtitleSize size) {
	if (setSubtitleOffset == 0 || largeSubtitlePointerOffset == 0) {
		return;
	}

	uint32_t sizeOffset;
	switch (size) {
	case SubtitleSize::Large:
		sizeOffset = 0;
		break;
	case SubtitleSize::Medium:
		sizeOffset = 0x08;
		break;
	case SubtitleSize::Small:
		sizeOffset = 0x10;
	}

	uint32_t fontPointerOffset = largeSubtitlePointerOffset + sizeOffset;
	Memory::get()->WriteRelative(reinterpret_cast<void*>(setSubtitleOffset), &fontPointerOffset, 0x4);
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

		Memory::get()->DisplayHudMessage(message.text, { message.color.R, message.color.G, message.color.B });
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
		// Additionally, we only want to show complete messages, so if any of these messages would not be fully representable in
		//   the three available lines, don't show them.
		std::vector<std::string> lines;
		if (!actionHint.empty()) {
			std::vector<std::string> expandedHint = separateLines(actionHint);
			lines.insert(lines.begin(), expandedHint.begin(), expandedHint.end());
		}

		if (!statusMessage.empty()) {
			std::vector<std::string> expandedStatus = separateLines(statusMessage);
			if (lines.size() + expandedStatus.size() <= 3) {
				lines.insert(lines.begin(), expandedStatus.begin(), expandedStatus.end());
			}
		}

		if (!worldMessage.empty()) {
			std::vector<std::string> expandedWorld = separateLines(worldMessage);
			if (lines.size() + expandedWorld.size() <= 3) {
				lines.insert(lines.begin(), expandedWorld.begin(), expandedWorld.end());
			}
		}

		writeSubtitle(lines);
		subtitlesDirty = false;
	}
}

void HudManager::writeSubtitle(std::vector<std::string> lines) {
	// Subtitle rendering ignores newlines in favor of having separate draw calls for each line, so we need to split each input
	//   line by newline characters and append those to the list.
	std::vector<std::string> expandedLines;
	for (const std::string& line : lines) {
		std::vector<std::string> splitLines = separateLines(line);
		expandedLines.insert(expandedLines.end(), splitLines.begin(), splitLines.end());
	}

	// Since we want to keep this text as low on the screen as possible, we need to write our text using the lowest possible rows.
	//   In order to do this, we can compute an offset into the list of lines based on its size such that the last line in the
	//   list corresponds to the last line in the subtitle.
	int lineOffset = std::max(3 - (int)expandedLines.size(), 0); // NOTE: This will only display the first three lines.

	Memory::get()->DisplaySubtitles(
		0 - lineOffset >= 0 ? expandedLines[0 - lineOffset] : "",
		1 - lineOffset >= 0 ? expandedLines[1 - lineOffset] : "",
		2 - lineOffset >= 0 ? expandedLines[2 - lineOffset] : ""
	);
}

std::vector<std::string> HudManager::separateLines(std::string input) {
	// First, split the string into lines based on newlines.
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

	// Next, check every line for length and, if they are exceedingly long, split them into wrapped strings by length.
	std::vector<std::string> wrappedLines;
	for (const std::string& line : splitLines) {
		if (line.length() > SUBTITLE_MAX_LINE_LENGTH) {
			// The line is longer than our max width and needs to be wrapped.
			std::string choppedLine = line;
			while (choppedLine.length() > SUBTITLE_MAX_LINE_LENGTH) {
				// Find the closest space character to the end of the line.
				searchIndex = choppedLine.rfind(' ', SUBTITLE_MAX_LINE_LENGTH);
				if (searchIndex != std::string::npos) {
					// We found a space. Separate the string at that character and keep working.
					wrappedLines.push_back(choppedLine.substr(0, searchIndex));
					choppedLine = choppedLine.substr(searchIndex + 1);
				}
				else {
					// There was no space in the previous line. Simply split the string mid-word.
					wrappedLines.push_back(choppedLine.substr(0, SUBTITLE_MAX_LINE_LENGTH));
					choppedLine = choppedLine.substr(SUBTITLE_MAX_LINE_LENGTH);
				}
			}

			// Add the remainder of the string to the output.
			wrappedLines.push_back(choppedLine);
		}
		else {
			// The line is shorter than our max width and can be added to the output directly.
			wrappedLines.push_back(line);
		}
	}

	return wrappedLines;
}

void HudManager::findSetSubtitleOffsets() {
	// Find hud_draw_subtitles. We can do so by looking for where it loads its color information from constants, which seems to be
	//   a unique set of instructions:
	uint64_t drawSubtitleOffset = Memory::get()->executeSigScan({
		0x0F, 0x57, 0xFF,		// XORPS XMM7, XMM7
		0x0F, 0x28, 0xD7,
		0x0F, 0x28, 0xCF
	});

	if (drawSubtitleOffset == UINT64_MAX) {
		setSubtitleOffset = 0;
		largeSubtitlePointerOffset = 0;
		return;
	}

	// Next, find the address of the call that sets the subtitle size. Skip ahead:
	//  0x0F, 0x11, 0x44, 0x24, 0x40
	//  0x0F, 0x28, 0xC7
	//  0xE8, 0x60, 0xF8, 0x0F, 0x00	// CALL argb_color (makes black)
	//  0x48, 0x8D, 0x4C, 0x24, 0x40
	//  0x44, 0x8B, 0xE0
	//  0xE8, 0xF3, 0xF7, 0x0F, 0x00	// CALL argb_color (makes white)
	//  0x4C, 0x8B, 0x3D				// |
	//  ____, ____, ____, ____			// MOV R15, qword ptr [globals.subtitles_font]
	setSubtitleOffset = drawSubtitleOffset + 0x9 + 0x1D;

	// Finally, get the offset that is ordinarily used by this MOV operation, which corresponds to globals.subtitles_font. (We don't
	//   want to dereference this here, only get the address of the pointer itself, because we'll be changing which font we're
	//   pointing to rather than modifying the font itself.
	largeSubtitlePointerOffset = 0;
	if (!Memory::get()->ReadRelative(reinterpret_cast<void*>(setSubtitleOffset), &largeSubtitlePointerOffset, 0x4)) {
		setSubtitleOffset = 0;
		largeSubtitlePointerOffset = 0;
	}
}