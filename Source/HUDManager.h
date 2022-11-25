#pragma once

#include <memory>
#include <string>
#include <queue>

#include "DataTypes.h"


class Memory;


class HudManager {
public:

	HudManager(const std::shared_ptr<Memory>& memory) : memory(memory) {}
	
	void update(float deltaSeconds);

	// Queue a message to be shown at the top of the screen, such as "Received Shapers".
	void queueBannerMessage(std::string text, RgbColor color = RgbColor(), float duration = 8.f);

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

private:

	void updateBannerMessages(float deltaSeconds);
	void updateSubtitleMessages(float deltaSeconds);

	void writeSubtitle(std::vector<std::string> lines);

	static std::vector<std::string> splitByNewline(std::string input);

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

	std::shared_ptr<Memory> memory;

};