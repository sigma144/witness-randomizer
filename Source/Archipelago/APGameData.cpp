#include "APGameData.h"
#include "Client/apclientpp/apclient.hpp"
#include "DrawIngameManager.h"

RgbColor getColorByItemFlag(const __int64 flags) {
	// Pick the appropriate color for this item based on its progression flags. Colors are loosely based on AP codes but
	//   much more vivid for contrast. See https://github.com/ArchipelagoMW/Archipelago/blob/main/NetUtils.py for details.
	if (flags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
		if (flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
			return { 240.f / 255.f, 210.f / 255.f, 0.0f };
		}
		else {
			return { 188 / 255.f, 81 / 255.f, 224 / 255.f };
		}
	}
	else if (flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
		// NOTE: "never exclude" here maps onto "useful" in the AP source.
		return { 43/255.f, 103/255.f, 255/255.f };
	}
	else if (flags & APClient::ItemFlags::FLAG_TRAP) {
		return { 214/255.f, 58/255.f, 34/255.5f };
	}
	else {
		return { 20/255.f, 222/255.f, 158/255.f };
	}
}

RgbColor getColorByItemIdOrFlag(const __int64 itemId, const __int64 flags) {
	if (itemId >= 158000 && itemId < 160000) {
		//TODO
	}
	return getColorByItemFlag(flags);
}

std::string getStringFromFlag(unsigned int flags) {
	if (flags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
		if (flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
			return "ProgUseful";
		}
		else {
			return "Progression";
		}
	}
	else if (flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
		// NOTE: "never exclude" here maps onto "useful" in the AP source.
		return "Useful";
	}
	else if (flags & APClient::ItemFlags::FLAG_TRAP) {
		return "Trap";
	}
	else {
		return "Filler";
	}
}

void populateWarpLookup() {
	for (Warp warp : allPossibleWarps) {
		warpLookup[warp.name] = warp;
	}
}

std::vector<WitnessDrawnSphere> makeEgg(Vector3 originalPosition, float scale, RgbColor color) {
	scale *= 1.3f;

	std::vector<WitnessDrawnSphere> spheresToDraw = {};

	spheresToDraw.push_back(WitnessDrawnSphere({ originalPosition, 0.04f * scale, color, true }));
	spheresToDraw.push_back(WitnessDrawnSphere({ originalPosition + Vector3(0, 0, 0.010f * scale), 0.036f * scale, color, true }));
	spheresToDraw.push_back(WitnessDrawnSphere({ originalPosition + Vector3(0, 0, 0.020f * scale), 0.03f * scale, color, true }));
	spheresToDraw.push_back(WitnessDrawnSphere({ originalPosition + Vector3(0, 0, 0.032f * scale), 0.02f * scale, color, true }));

	return spheresToDraw;
}