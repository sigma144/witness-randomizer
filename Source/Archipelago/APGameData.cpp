#include "APGameData.h"
#include "Client/apclientpp/apclient.hpp"

RgbColor getColorByItemFlag(const __int64 flags) {
	// Pick the appropriate color for this item based on its progression flags. Colors are loosely based on AP codes but somewhat
	// paler to match the Witness aesthetic. See https://github.com/ArchipelagoMW/Archipelago/blob/main/NetUtils.py for details.

	if (flags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
		return { 0.82f, 0.76f, 0.96f };
	}
	else if (flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
		// NOTE: "never exclude" here maps onto "useful" in the AP source.
		return { 0.68f, 0.75f, 0.94f };
	}
	else if (flags & APClient::ItemFlags::FLAG_TRAP) {
		return { 1.f, 0.7f, 0.67f };
	}
	else {
		return { 0.81f, 1.f, 1.f };
	}
}

RgbColor getColorByItemIdOrFlag(const __int64 itemId, const __int64 flags) {
	if (itemId >= 158000 && itemId < 160000) {
		//TODO
	}
	return getColorByItemFlag(flags);
}