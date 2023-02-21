#pragma once

#include <string>
#include <queue>

#include "DataTypes.h"


class Memory;


class HudManager {
public:

	HudManager();
	
	void update(float deltaSeconds);

	// Queue a message to be shown at the top of the screen, such as "Received Shapers".
	void queueBannerMessage(std::string text, RgbColor color = RgbColor(), float duration = 5.f);

	// Show a specific message using the subtitle display, overriding any other messages.
	void showSubtitleMessage(std::string text, float duration);
	void clearSubtitleMessage();

	// Show a message about the world to the player, such as "This puzzle requires Triangles."
	void setWorldMessage(std::string text);
	void clearWorldMessage() { setWorldMessage(std::string()); };

	// Show a status message to the player, such as "Slowed! 10 seconds remaining."
	void setStatusMessage(std::string text);
	void clearStatusMessage() { setStatusMessage(std::string()); };

	// Show an input action hint to the player, such as "Hold [TAB]: Skip Puzzle".
	void setActionHint(std::string text);
	void clearActionHint() { setActionHint(std::string()); };

	enum SubtitleSize {
		Large,
		Medium,
		Small
	};

	// Sets the default size of the subtitle. NOTE: When subtitles are drawn, the game will attempt to pick progressively smaller
	//   fonts until it finds one where the entire subtitle line can fit on screen horizontally. As such, if we set a large font
	//   here, then pass a long string, the game may not use the large font for that message.
	void setSubtitleSize(SubtitleSize size);

private:

	void updateBannerMessages(float deltaSeconds);
	void updateSubtitleMessages(float deltaSeconds);

	void writeSubtitle(std::vector<std::string> lines);

	// Splits the given string into individual lines, first by chopping it by newlines, and second by wrapping any long strings.
	static std::vector<std::string> separateLines(std::string input);

	struct BannerMessage {
		BannerMessage(std::string text, RgbColor color, float duration) : text(text), color(color), duration(duration) {}

		std::string text;
		RgbColor color;
		float duration;
	};

	std::queue<BannerMessage> queuedBannerMessages;
	float bannerTimeRemaining = 0.f;

	float subtitleOverrideTimeRemaining = 0.f;

	std::string worldMessage;
	std::string actionHint;
	std::string statusMessage;
	bool subtitlesDirty = false;

	void findSetSubtitleOffsets();

	uint64_t setSubtitleOffset;
	uint32_t largeSubtitlePointerOffset;

};