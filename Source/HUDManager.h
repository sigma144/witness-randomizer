#pragma once

#include <array>
#include <string>
#include <queue>

#include "DataTypes.h"

class Memory;


// Categories for the informational messages shown in the lower center of the screen. Ordered from highest to lowest priority.
enum class InfoMessageCategory : int {
	EnvironmentalPuzzle = 0,
	MissingSymbol = 1,
	ApHint = 2,
	Dog = 3,
	COUNT = 4
};

class HudManager {
public:

	HudManager();
	
	void update(float deltaSeconds);

	// Queue a message to be shown at the top of the screen, such as "Connected to Archipelago".
	void queueBannerMessage(const std::string& text, RgbColor color = RgbColor(), float duration = 5.f);

	// Queue a message to be shown in the notifications block, such as "Received Shapers".
	void queueNotification(const std::string& text, RgbColor flashColor = RgbColor());

	// Show a message in the center of the screen.
	void showInformationalMessage(InfoMessageCategory category, const std::string& text);
	void clearInformationalMessage(InfoMessageCategory category);

	// Show a message to the player, such as "Slowed! 10 seconds remaining."
	void setWalkStatusMessage(const std::string& text);
	void clearWalkStatusMessage() { setWalkStatusMessage(std::string()); };

	// Show an input action hint to the player while in solve mode, such as "Hold [TAB]: Skip Puzzle".
	void setSolveStatusMessage(const std::string& text);
	void clearSolveStatusMessage() { setSolveStatusMessage(std::string()); };

	// Show debug text in the upper-left corner of the screen.
	void setDebugText(const std::string& text);
	void clearDebugText() { setDebugText(std::string()); }

private:

	void updateBannerMessages(float deltaSeconds);
	void updateNotifications(float deltaSeconds);
	void updateInformationalMessages(float deltaSeconds);

	// Splits the given string into individual lines, first by chopping it by newlines, and second by wrapping any long strings.
	static std::vector<std::string> separateLines(std::string input, int maxLength = 80);

	struct BannerMessage {
		BannerMessage(std::string text, RgbColor color, float duration) : text(text), color(color), duration(duration) {}

		std::string text;
		RgbColor color;
		float duration;
	};

	float solveTweenFactor = 0.f;

	std::queue<BannerMessage> queuedBannerMessages;
	float bannerTimeRemaining = 0.f;

	struct Notification {
		Notification(std::string text, RgbColor flashColor) : text(text), flashColor(flashColor) {}

		std::string text;
		RgbColor flashColor;
		float age = 0.f;
	};

	std::queue<Notification> queuedNotifications;
	std::vector<Notification> activeNotifications;
	float timeToNextNotification = 0.f;

	std::array<std::string, static_cast<int>(InfoMessageCategory::COUNT)> informationalMessages;
	std::string currentInformationalMessage;
	bool shouldShowInformationalMessage;
	float informationalMessageFade = 0.f;

	std::string worldMessage;
	std::string walkStatusMessage;
	std::string solveStatusMessage;
	std::string debugText;
	bool hudTextDirty = false;


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

		// Additional offsets for controlling vertical position. These add fractional lines to the top and bottom of the block
		//   in order to adjust its total height and starting position.
		float linePaddingTop = 0.f;
		float linePaddingBottom = 0.f;

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
		const static uint32_t address_linePaddingTop;
		const static uint32_t address_linePaddingBottom;
		const static uint32_t address_horizontalPosition;
		const static uint32_t address_horizontalAlignment;
		const static uint32_t totalDataSize;
	};

	struct HudTextPayload {
		std::vector<HudTextBlock> blocks;

		const static uint32_t address_blockCount;
		const static uint32_t totalDataSize;
	};

	uint32_t findSubtitleFunction();

	void overwriteSubtitleFunction();

	uint64_t address_readPayloadIndex = 0;
	uint64_t address_writePayloadIndex = 0;
	uint64_t address_hudTextPayload_1 = 0;
	uint64_t address_hudTextPayload_2 = 0;

	void updatePayloads();
	HudTextPayload buildPayload() const;
	void writePayload(const HudTextPayload& payload, uint64_t writeAddress) const;

	float getNotificationAlpha(const Notification& notification, float bright, float dim) const;
	RgbColor getNotificationShadowColor(const Notification& notification, float alpha) const;
	
	static float easeIn(float val);
	static float easeOut(float val);
	static float easeInOut(float val);

	static float shadowAlpha(float alpha);

	uint64_t setSubtitleOffset;
	uint32_t largeSubtitlePointerOffset;

};