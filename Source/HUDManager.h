#pragma once

#include <string>
#include <queue>

#include "DataTypes.h"


class Memory;


class HudManager {
public:

	HudManager();
	
	void update(float deltaSeconds);

	// Queue a message to be shown at the top of the screen, such as "Connected to Archipelago".
	void queueBannerMessage(std::string text, RgbColor color = RgbColor(), float duration = 5.f);

	// Queue a message to be shown in the notifications block, such as "Received Shapers".
	void queueNotification(std::string text, RgbColor color = RgbColor());

	// Show a specific message using the subtitle display, overriding any other messages.
	void showSubtitleMessage(std::string text, float duration);
	void clearSubtitleMessage();

	// Show a message about the world to the player, such as "This puzzle requires Triangles."
	void setWorldMessage(std::string text);
	void clearWorldMessage() { setWorldMessage(std::string()); };

	// Show a message to the player, such as "Slowed! 10 seconds remaining."
	void setWalkStatusMessage(std::string text);
	void clearWalkStatusMessage() { setWalkStatusMessage(std::string()); };

	// Show an input action hint to the player while in solve mode, such as "Hold [TAB]: Skip Puzzle".
	void setSolveStatusMessage(std::string text);
	void clearSolveStatusMessage() { setSolveStatusMessage(std::string()); };

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
	void updateNotifications(float deltaSeconds);

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

	float solveFadePercent = 0.f;

	std::queue<BannerMessage> queuedBannerMessages;
	float bannerTimeRemaining = 0.f;

	struct Notification {
		Notification(std::string text, RgbColor color) : text(text), color(color) {}

		std::string text;
		RgbColor color;
		float age = 0.f;
	};

	std::queue<Notification> queuedNotifications;
	std::vector<Notification> activeNotifications;
	float timeToNextNotification = 0.f;

	float subtitleOverrideTimeRemaining = 0.f;

	std::string worldMessage;
	std::string walkStatusMessage;
	std::string solveStatusMessage;
	bool hudTextDirty = false;

	void findSetSubtitleOffsets();

	struct HudTextLine {
		RgbColor textColor = RgbColor(1.f,1.f,1.f,1.f);
		RgbColor shadowColor = RgbColor(0.f,0.f,0.f,1.f);
		std::string text;

		const static uint32_t maxLineSize;

		const static uint32_t address_textColor;
		const static uint32_t address_shadowColor;
		const static uint32_t address_string;
		const static uint32_t totalDataSize;
	};

	struct HudTextBlock {
		// The vertical position of the block on screen. Computed based on the total height of the block.
		// 0.f = bottom, 1.f = top.
		float verticalPosition = 0.f;

		// The horizontal position of the origin of the block on screen. This does not take into account the width of the
		//   string and should be used in conjunction with the horizontal alignment parameter to determine the final position.
		// 0.f = left, 1.f = right.
		float horizontalPosition = 0.f;

		// The offset of the horizontal origin of the block, based on the width of the string. Used to justify text.
		// 0.f = left, 0.5f = center, 1.f = right.
		float horizontalAlignment = 0.f;

		std::vector<HudTextLine> lines;

		const static uint32_t address_lineCount;
		const static uint32_t address_verticalPosition;
		const static uint32_t address_horizontalPosition;
		const static uint32_t address_horizontalAlignment;
		const static uint32_t totalDataSize;
	};

	struct HudTextPayload {
		std::vector<HudTextBlock> blocks;

		const static uint32_t address_blockCount;
		const static uint32_t totalDataSize;
	};

	uint64_t address_hudTextPayload = 0;
	void overwriteSubtitleFunction();
	void writePayload() const;

	uint64_t setSubtitleOffset;
	uint32_t largeSubtitlePointerOffset;

};