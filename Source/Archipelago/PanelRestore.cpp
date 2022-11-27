#include "PanelRestore.h"
#include "OriginalPanelData.h"
#include "../Randomizer.h"

void PanelRestore::RestoreOriginalPanelData(std::shared_ptr<Memory> memory) {
	for (auto& [key, value] : originalBackgroundModes) {
		memory->WritePanelData<int>(key, OUTER_BACKGROUND_MODE, { value });
	}

	for (auto& [key, value] : originalPathColors) {
		memory->WritePanelData<float>(key, PATH_COLOR, value);
	}

	for (auto& [key, value] : originalOuterBackgroundColors) {
		memory->WritePanelData<float>(key, OUTER_BACKGROUND, value);
	}

	for (auto& [key, value] : originalBackgroundRegionColors) {
		memory->WritePanelData<float>(key, BACKGROUND_REGION_COLOR, value);
	}
}