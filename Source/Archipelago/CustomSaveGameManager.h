#include <cstdint>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <any>

#define SAVEGAME_NUM_ITEMS 0x488
#define SAVEGAME_ALLOCATION_SIZE 0x48C
#define SAVEGAME_ARRAY_POINTER 0x490

#define SAVE_PANEL 0x03505  // Tutorial Gate Close - This panel can't be seen by the player.

#define SAVEGAME_ITEM_SIZE 0x34

class Memory;

class CustomSaveGameManager
{
private:
	CustomSaveGameManager() {
		initialize();
	};

	nlohmann::json store;

	void write();
	bool byte_index_is_safe(int i);

	uint64_t savegameAllocationPointer = -1;
public:
	static CustomSaveGameManager& get() {
		static CustomSaveGameManager INSTANCE;
		return INSTANCE;
	}

	void updateValue(std::string key, std::set<std::string> value)
	{
		std::set<std::string> current = {};
		if (store.contains(key)) current = store[key].get<std::set<std::string>>();

		for (std::string s : value) {
			current.insert(s);
		}

		store[key] = current;
		write();
	}

	void updateValue(std::string key, std::set<int> value)
	{
		std::set<int> current = {};
		if (store.contains(key)) current = store[key].get<std::set<int>>();

		for (int i : value) {
			current.insert(i);
		}

		store[key] = current;
		write();
	}

	template <class T>
	T readValue(std::string key, T defaultValue) {
		if (!store.contains(key)) {
			store[key] = defaultValue;
			write();
		}
		return store[key].get<T>();
	}

	void initialize();
};