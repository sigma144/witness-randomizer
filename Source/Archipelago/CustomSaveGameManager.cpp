#include "../Memory.h"
#include "CustomSaveGameManager.h"
#include <vector>
#include <string>

void CustomSaveGameManager::initialize()
{
	int num_items = Memory::get()->ReadPanelData<int>(SAVE_PANEL, SAVEGAME_NUM_ITEMS);

	if (num_items == 0) return;

	std::vector<uint8_t> savegame_vector = Memory::get()->ReadArray<uint8_t>(SAVE_PANEL, SAVEGAME_ARRAY_POINTER, SAVEGAME_ITEM_SIZE * num_items);
	std::vector<uint8_t> cleaned_savegame_vector = {};
	for (int i = 0; i < savegame_vector.size(); i++) {
		if (!byte_index_is_safe(i)) continue;
	
		if (savegame_vector[i] == 0x00) break;

		cleaned_savegame_vector.push_back(savegame_vector[i]);
	}

	if (cleaned_savegame_vector.empty()) return;

	std::string cleaned_savegame_string;
	cleaned_savegame_string.assign(cleaned_savegame_vector.begin(), cleaned_savegame_vector.end());
	store = nlohmann::json::parse(cleaned_savegame_string);

	write();
}

void CustomSaveGameManager::write()
{
	std::string savegame_string = store.dump();
	std::vector<uint8_t> savegame_vector(savegame_string.begin(), savegame_string.end());

	std::vector<uint8_t> safe_savegame_vector = {};
	
	int vector_index = 0;
	int savegame_index = 0;
	while (vector_index < savegame_vector.size()) {
		if (byte_index_is_safe(savegame_index)) {
			if (savegame_vector[vector_index] == 0x00) break;
			safe_savegame_vector.push_back(savegame_vector[vector_index]);
			vector_index++;
		}
		else {
			safe_savegame_vector.push_back(0x00);
		}
		savegame_index++;
	}
	safe_savegame_vector.push_back(0x00);

	if (savegameAllocationPointer == -1) {
		savegameAllocationPointer = (uint64_t)VirtualAllocEx(Memory::get()->getHandle(), NULL, 0x100000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		Memory::get()->WritePanelData<uint64_t>(SAVE_PANEL, SAVEGAME_ARRAY_POINTER, { savegameAllocationPointer });
	}

	WriteProcessMemory(Memory::get()->getHandle(), reinterpret_cast<LPVOID>(savegameAllocationPointer), safe_savegame_vector.data(), safe_savegame_vector.size(), nullptr);

	int size = safe_savegame_vector.size();
	int num_items = size / SAVEGAME_ITEM_SIZE + (size % SAVEGAME_ITEM_SIZE != 0);

	int next_power_of_2 = 1;
	while (next_power_of_2 < 0x100000)
		next_power_of_2 *= 2;

	Memory::get()->WritePanelData<int>(SAVE_PANEL, SAVEGAME_ALLOCATION_SIZE, { next_power_of_2 });
	Memory::get()->WritePanelData<int>(SAVE_PANEL, SAVEGAME_NUM_ITEMS, { num_items });
}

bool CustomSaveGameManager::byte_index_is_safe(int i)
{
	int within_block = i % SAVEGAME_ITEM_SIZE;
	/*if (within_block >= 0x10 && within_block < 0x18) {
		return false;
	}*/
	return within_block >= 0x18 && within_block < 0x30;
}
