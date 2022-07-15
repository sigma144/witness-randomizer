#pragma once

#include <functional>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>

#include "Archipelago\Client\apclientpp\apclient.hpp"
#include <windows.h>
// https://github.com/erayarslan/WriteProcessMemory-Example
// http://stackoverflow.com/q/32798185
// http://stackoverflow.com/q/36018838
// http://stackoverflow.com/q/1387064
class Memory
{
private:
	std::recursive_mutex mtx;

public:


	Memory(const std::string& processName);
	int findGlobals();
	void findMovementSpeed();
	void findActivePanel();
	void findPlayerPosition();
	int GetActivePanel();
	static __int64 ReadStaticInt(__int64 offset, int index, const std::vector<byte>& data, size_t bytesToEOL = 4);
	~Memory();

	Memory(const Memory& memory) = delete;
	Memory& operator=(const Memory& other) = delete;

	template <class T>
	uintptr_t AllocArray(int id, int numItems) {
		uintptr_t ptr = reinterpret_cast<uintptr_t>(VirtualAllocEx(_handle, 0, numItems * sizeof(T), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		return ptr;
	}

	template <class T>
	uintptr_t AllocArray(int id, size_t numItems) {
		return AllocArray<T>(id, static_cast<int>(numItems));
	}

	bool Read(LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize) {
		std::lock_guard<std::recursive_mutex> lock(mtx);

		if (!retryOnFail) return ReadProcessMemory(_handle, lpBaseAddress, lpBuffer, nSize, nullptr);
		for (int i = 0; i < 10000; i++) {
			if (ReadProcessMemory(_handle, lpBaseAddress, lpBuffer, nSize, nullptr)) {
				return true;
			}
		}
		return false;
	}

	bool Write(LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (!retryOnFail) return WriteProcessMemory(_handle, lpBaseAddress, lpBuffer, nSize, nullptr);
		for (int i = 0; i < 10000; i++) {
			if (WriteProcessMemory(_handle, lpBaseAddress, lpBuffer, nSize, nullptr)) {
				return true;
			}
		}
		return false;
	}

	template <class T>
	std::vector<T> ReadArray(int panel, int offset, int size) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (size == 0) return std::vector<T>();
		if (offset == 0x230 || offset == 0x238) { //Traced edge data - this moves sometimes so it should not be cached
			//Invalidate cache entry for old array address
			_computedAddresses.erase(reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, panel * 8, offset })));
		}
		_arraySizes[std::make_pair(panel, offset)] = size;
		return ReadData<T>({ GLOBALS, 0x18, panel * 8, offset, 0 }, size);
	}

	template <class T>
	void WriteArray(int panel, int offset, const std::vector<T>& data) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (data.size() == 0) return;
		if (data.size() > _arraySizes[std::make_pair(panel, offset)]) {
			//Invalidate cache entry for old array address
			_computedAddresses.erase(reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, panel * 8, offset })));
			//Allocate new array in process memory
			uintptr_t ptr = AllocArray<T>(panel, data.size());
			WritePanelData<uintptr_t>(panel, offset, { ptr });
		}
		WriteData<T>({ GLOBALS, 0x18, panel * 8, offset, 0 }, data);
	}

	template <class T>
	void WriteArray(int panel, int offset, const std::vector<T>& data, bool force) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (force) _arraySizes[std::make_pair(panel, offset)] = 0;
		WriteArray(panel, offset, data);
	}

	template <class T>
	std::vector<T> ReadPanelData(int panel, int offset, size_t size) {
		if (size == 0) return std::vector<T>();
		return ReadData<T>({ GLOBALS, 0x18, panel * 8, offset }, size);
	}

	template <class T>
	T ReadPanelData(int panel, int offset) {
		return ReadData<T>({ GLOBALS, 0x18, panel * 8, offset }, 1)[0];
	}

	template <class T>
	void WritePanelData(int panel, int offset, const std::vector<T>& data) {
		WriteData<T>({ GLOBALS, 0x18, panel * 8, offset }, data);
	}

	void WriteMovementSpeed(float speed) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (speed == 0) return;
		float sprintSpeed = this->ReadData<float>({ RUNSPEED }, 1)[0];
		if (sprintSpeed == 0.0f) return; // sanity check, to avoid an accidental div0
		float multiplier = speed / sprintSpeed;
		if (multiplier == 1.0f) return;
		this->WriteData<float>({ RUNSPEED }, { speed });
		this->WriteData<float>({ ACCELERATION }, { this->ReadData<float>({ACCELERATION}, 1)[0] * multiplier });
		this->WriteData<float>({ DECELERATION }, { this->ReadData<float>({DECELERATION}, 1)[0] * multiplier });
	}

	std::vector<float> ReadPlayerPosition() {
		return this->ReadData<float>({ CAMERAPOSITION }, 3);
	}

	void OpenDoor(int id) {
		CallVoidFunction(id, 0x14008EB60);
	}

	void UpdateEntityPosition(int id) {
		CallVoidFunction(id, 0x140184F00); // Entity::has_moved_in_a_non_position_way - Still works even if it HAS moved in a non position way, for some reason :P
	}

	void ActivateLaser(int id) {
		CallVoidFunction(id, 0x1400AF520);
	}

	void RemoveMesh(int id);

	void DisplayHudMessage(std::string s);

	void ClearOffsets() { _computedAddresses = std::map<uintptr_t, uintptr_t>(); }

	static int GLOBALS;
	static int RUNSPEED;
	static int CAMERAPOSITION;
	static std::vector<int> ACTIVEPANELOFFSETS;
	static int ACCELERATION;
	static int DECELERATION;
	static bool showMsg;
	static int globalsTests[3];
	static HWND errorWindow;
	bool retryOnFail = true;


private:
	template<class T>
	std::vector<T> ReadData(const std::vector<int>& offsets, size_t numItems) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		std::vector<T> data;
		data.resize(numItems);
		if (Read(ComputeOffset(offsets), &data[0], sizeof(T) * numItems)) {
			return data;
		}
		ThrowError(offsets, false);
		return {};
	}

	template <class T>
	void WriteData(const std::vector<int>& offsets, const std::vector<T>& data) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (Write(ComputeOffset(offsets), &data[0], sizeof(T) * data.size())) {
			return;
		}
		ThrowError(offsets, true);
	}

	using ScanFunc = std::function<void(__int64 offset, int index, const std::vector<byte>& data)>;
	using ScanFunc2 = std::function<bool(__int64 offset, int index, const std::vector<byte>& data)>;
	void executeSigScan(const std::vector<byte>& scanBytes, const ScanFunc2& scanFunc);

	void ThrowError(std::string message);
	void ThrowError(const std::vector<int>& offsets, bool rw_flag);
	void ThrowError();

	void* ComputeOffset(std::vector<int> offsets);

	void CallVoidFunction(int id, uint64_t functionAdress);

	std::map<uintptr_t, uintptr_t> _computedAddresses;
	std::map<std::pair<int, int>, int> _arraySizes;
	uintptr_t _baseAddress = 0;
	LPVOID _messageAddress = 0;
	HANDLE _handle = nullptr;

	friend class Randomizer;
	friend class Special;
};