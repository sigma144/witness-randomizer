#pragma once
#include <functional>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <windows.h>
// https://github.com/erayarslan/WriteProcessMemory-Example
// http://stackoverflow.com/q/32798185
// http://stackoverflow.com/q/36018838
// http://stackoverflow.com/q/1387064
class Memory
{
public:
	Memory(const std::string& processName);
	int findGlobals();
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
		if (!retryOnFail) return ReadProcessMemory(_handle, lpBaseAddress, lpBuffer, nSize, nullptr);
		for (int i = 0; i < 10000; i++) {
			if (ReadProcessMemory(_handle, lpBaseAddress, lpBuffer, nSize, nullptr)) {
				return true;
			}
		}
		return false;
	}

	bool Write(LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize) {
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

	template<class T>
	std::vector<T> ReadExeData(int offset, size_t numItems) {
		std::vector<T> data;
		data.resize(numItems);
		LPCVOID testing = reinterpret_cast<LPCVOID>((long long)offset + 0x140000000);
		if (Read(reinterpret_cast<LPCVOID>((long long)offset + 0x140000000), &data[0], sizeof(T) * numItems)) {
			return data;
		}
		if (!showMsg) throw std::exception();
		std::vector<int> arr = { offset };
		ThrowError(arr, false);
		return {};
	}

	template <class T>
	void WriteExeData(int offset, const std::vector<T>& data) {
		if (Write(reinterpret_cast<LPVOID>((long long)offset + 0x140000000), &data[0], sizeof(T) * data.size())) {
			return;
		}
		if (!showMsg) throw std::exception();
		std::vector<int> arr = { offset };
		ThrowError(arr, false);
	}

	template<class T>
	std::vector<T> ReadData(const std::vector<int>& offsets, size_t numItems) {
		std::vector<T> data;
		data.resize(numItems);
		if (Read(ComputeOffset(offsets), &data[0], sizeof(T) * numItems)) {
			return data;
		}
		if (!showMsg) throw std::exception();
		ThrowError(offsets, false);
		return {};
	}

	template <class T>
	void WriteData(const std::vector<int>& offsets, const std::vector<T>& data) {
		if (Write(ComputeOffset(offsets), &data[0], sizeof(T) * data.size())) {
			return;
		}
		if (!showMsg) throw std::exception();
		ThrowError(offsets, true);
	}

	void ClearOffsets() { _computedAddresses = std::map<uintptr_t, uintptr_t>(); }

	static int GLOBALS;
	static bool showMsg;
	static int globalsTests[3];
	bool retryOnFail = true;

private:
	void ThrowError(std::string message);
	void ThrowError(const std::vector<int>& offsets, bool rw_flag);
	void ThrowError();

	void* ComputeOffset(std::vector<int> offsets);

	std::map<uintptr_t, uintptr_t> _computedAddresses;
	std::map<std::pair<int, int>, int> _arraySizes;
	uintptr_t _baseAddress = 0;
	HANDLE _handle = nullptr;

	friend class Randomizer;
	friend class Special;
};