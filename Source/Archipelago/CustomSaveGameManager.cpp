#include "../Memory.h"
#include "CustomSaveGameManager.h"

void CustomSaveGameManager::load()
{
	int num_items = Memory::get()->ReadPanelData<int>(SAVE_PANEL, SAVEGAME_NUM_ITEMS);

	if (num_items == 0) return;

	std::vector<byte> savegame_vector = Memory::get()->ReadPanelData<byte>(SAVE_PANEL, SAVEGAME_ARRAY_POINTER, SAVEGAME_ITEM_SIZE * num_items);
	std::vector<byte> cleaned_savegame_vector = {};
	for (int i = 0; i < savegame_vector.size(); i++) {
		int byte_index_within_block = i % SAVEGAME_ITEM_SIZE;
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
	std::vector<byte> savegame_vector(savegame_string.begin(), savegame_string.end());
	std::vector<byte> safe_savegame_vector = {};
	
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

	if (savegameAllocationPointer == -1) {
		savegameAllocationPointer = (uint64_t)VirtualAllocEx(Memory::get()->getHandle(), NULL, 0x100000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		Memory::get()->WritePanelData<uint64_t>(SAVE_PANEL, SAVEGAME_ARRAY_POINTER, { savegameAllocationPointer });
	}

	Memory::get()->WriteArray<byte>(SAVE_PANEL, SAVEGAME_ARRAY_POINTER, safe_savegame_vector);

	int num_items = num_items / SAVEGAME_ITEM_SIZE + (num_items % SAVEGAME_ITEM_SIZE != 0);

	int next_power_of_2 = 1;
	while (next_power_of_2 < num_items)
		next_power_of_2 *= 2;

	Memory::get()->WritePanelData<int>(SAVE_PANEL, SAVEGAME_ALLOCATION_SIZE, { next_power_of_2 });
	Memory::get()->WritePanelData<int>(SAVE_PANEL, SAVEGAME_NUM_ITEMS, { num_items });
}

bool CustomSaveGameManager::byte_index_is_safe(int i)
{
	if (i >= 0x10 && i < 0x18) {
		return false;
	}
	return i < 0x30;
}

void CustomSaveGameManager::updateValue(std::string key, std::set<std::string> value)
{
	std::set<std::string> current = {};
	if (store.contains(key)) current = store[key].get<std::set<std::string>>();

	for (std::string s : value) {
		current.insert(s);
	}

	store[key] = current;
	write();
}