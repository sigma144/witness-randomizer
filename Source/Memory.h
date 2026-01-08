#pragma once
#include <functional>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <array>
#include <wtypes.h>

// Note: Little endian
#define LONG_TO_BYTES(val) \
	static_cast<byte>((val & 0x00000000000000FF) >> 0x00), \
	static_cast<byte>((val & 0x000000000000FF00) >> 0x08), \
	static_cast<byte>((val & 0x0000000000FF0000) >> 0x10), \
	static_cast<byte>((val & 0x00000000FF000000) >> 0x18), \
	static_cast<byte>((val & 0x000000FF00000000) >> 0x20), \
	static_cast<byte>((val & 0x0000FF0000000000) >> 0x28), \
	static_cast<byte>((val & 0x00FF000000000000) >> 0x30), \
	static_cast<byte>((val & 0xFF00000000000000) >> 0x38)

// Note: Little endian
#define INT_TO_BYTES(val) \
	static_cast<byte>((val & 0x000000FF) >> 0x00), \
	static_cast<byte>((val & 0x0000FF00) >> 0x08), \
	static_cast<byte>((val & 0x00FF0000) >> 0x10), \
	static_cast<byte>((val & 0xFF000000) >> 0x18)

#define IF_GE(...) __VA_ARGS__, 0x72 // jb
#define IF_LT(...) __VA_ARGS__, 0x73 // jae
#define IF_NE(...) __VA_ARGS__, 0x74 // je
#define IF_EQ(...) __VA_ARGS__, 0x75 // jne
#define IF_GT(...) __VA_ARGS__, 0x76 // jbe
#define IF_LE(...) __VA_ARGS__, 0x77 // ja
#define THEN(...) ARGCOUNT(__VA_ARGS__), __VA_ARGS__

#define ARGCOUNT(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value
#define DO_WHILE_GT_ZERO(...) __VA_ARGS__, 0x77, static_cast<byte>(-2 - ARGCOUNT(__VA_ARGS__)) // Must end on a 'dec' instruction to set zero flags correclty.

// https://github.com/erayarslan/WriteProcessMemory-Example
// http://stackoverflow.com/q/32798185
// http://stackoverflow.com/q/36018838
// http://stackoverflow.com/q/1387064
class Memory
{
private:
	Memory();
	~Memory();

	static Memory* _singleton;
public:
	static void create();
	static Memory* get();
	void invalidateCache();

	Memory(const Memory& memory) = delete;
	Memory& operator=(const Memory& other) = delete;

	template <class T>
	uintptr_t AllocArray(int numItems) {
		uintptr_t ptr = reinterpret_cast<uintptr_t>(VirtualAllocEx(_handle, 0, numItems * sizeof(T), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		return ptr;
	}

	template <class T>
	uintptr_t AllocArray(size_t numItems) {
		return AllocArray<T>(static_cast<int>(numItems));
	}

	LPVOID getHandle() {
		return _handle;
	}

	uint64_t getBaseAddress() {
		return _baseAddress;
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
		auto search = _arraySizes.find({panel, offset});
		if (search == _arraySizes.end() || data.size() > search->second) {
			//Invalidate cache entry for old array address
			_computedAddresses.erase(reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, panel * 8, offset })));
			//Allocate new array in process memory
			uintptr_t ptr = AllocArray<T>(data.size());
			_arraySizes[{panel, offset}] = (int)data.size();
			WritePanelData<uintptr_t>(panel, offset, { ptr });
		}
		WriteData<T>({ GLOBALS, 0x18, panel * 8, offset, 0 }, data);
	}

	template <class T>
	void WriteArray(int panel, int offset, const std::vector<T>& data, bool force) {
		if (force) _arraySizes[{panel, offset}] = 0;
		WriteArray(panel, offset, data);
	}

	template <class T>
	void WriteToArray(int panel, int offset, int index, T data) {
		auto search = _arraySizes.find({panel, offset});
		if (search != _arraySizes.end() && index >= search->second) {
			ThrowError("Out of bound array write");
		}
		WriteData<T>({ GLOBALS, 0x18, panel * 8, offset, index * static_cast<int>(sizeof(T)) }, { data });
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
	void WritePanelData(int panel, int offset, T data) {
		std::vector<T> dataVector = { data };
		WritePanelData<T>(panel, offset, dataVector);
	}

	template <class T>
	void WritePanelData(int panel, int offset, const std::vector<T>& data) {
		WriteData<T>({ GLOBALS, 0x18, panel * 8, offset }, data);
	}

	// Clear cached offsets computed by ComputeOffset.
	void ClearOffsets() { _computedAddresses = std::map<uintptr_t, uintptr_t>(); }

	using ScanFunc = std::function<void(__int64 offset, int index, const std::vector<byte>& data)>;
	bool ScanForBytes(const std::vector<byte>& scanBytes, const ScanFunc& scanFunc);
	static __int64 ReadStaticInt(__int64 offset, int index, const std::vector<byte>& data, size_t bytesToEOL = 4);

	int GLOBALS = 0;
	bool showMsg = false;
	int globalsTests[3] = {
		0x62D0A0, //Steam and Epic Games
		0x62B0A0, //Good Old Games
		0x5B28C0 //Older Versions
	};
	bool retryOnFail = true;

	private:
		void findGlobals();
		void fixTriangleNegation();
		void setupCustomSymbols();

		template<class T>
		std::vector<T> ReadData(const std::vector<int>& offsets, size_t numItems) {
			std::vector<T> data;
			data.resize(numItems);
			if (Read(ComputeOffset(offsets), &data[0], sizeof(T) * numItems)) {
				return data;
			}
			ThrowError(offsets, false);
			return {};
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

		template <class T>
		void WriteData(const std::vector<int>& offsets, const std::vector<T>& data) {
			if (Write(ComputeOffset(offsets), &data[0], sizeof(T) * data.size())) {
				return;
			}
			ThrowError(offsets, true);
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