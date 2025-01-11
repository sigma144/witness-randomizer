#pragma once

#include <set>
#include <vector>
#include <filesystem>

#include <windows.h>
#include <shlobj.h>
#include <strsafe.h>
#include <iostream>
#include <fstream>

inline int isAprilFoolsValue = -1;
inline std::wstring finalUuid = L"";

class Utilities {

public:

	// Find the first instance of a search sequence within a specified range of the source data.
	template<typename T>
	static int findSequence(const std::vector<T>& sourceData, const std::vector<T>& searchSequence, int startIndex, int endIndex) {
		for (int sourceIndex = startIndex; sourceIndex < endIndex; sourceIndex++) {
			bool foundMatch = true;

			for (int comparisonIndex = 0; comparisonIndex < searchSequence.size(); comparisonIndex++) {
				if (sourceData[sourceIndex + comparisonIndex] != searchSequence[comparisonIndex])
				{
					foundMatch = false;
					break;
				}
			}

			if (foundMatch) {
				return sourceIndex;
			}
		}

		return -1;
	}

	// Find the first instance of a search sequence within the entirety of the source data.
	template<typename T>
	static int findSequence(const std::vector<T>& sourceData, const std::vector<T>& searchSequence) {
		return findSequence(sourceData, searchSequence, 0, sourceData.size() - searchSequence.size());
	}

	// Find all instances of a search sequence within a specified range in the source data.
	template<typename T>
	static std::vector<int> findAllSequences(const std::vector<T>& sourceData, const std::vector<T>& searchSequence, int startIndex, int endIndex) {
		std::vector<int> foundIndices;

		int searchIndex = startIndex;
		for (searchIndex = startIndex; searchIndex < endIndex;) {
			int foundIndex = findSequence(sourceData, searchSequence, searchIndex, endIndex);
			if (foundIndex != -1) {
				foundIndices.push_back(foundIndex);
				searchIndex = foundIndex + searchSequence.size();
			}
			else {
				break;
			}
		}

		return foundIndices;
	}

	// Find all instances of a search sequence within the entirety of the source data.
	template<typename T>
	static std::vector<int> findAllSequences(const std::vector<T>& sourceData, const std::vector<T>& searchSequence) {
		return findAllSequences(sourceData, searchSequence, 0, sourceData.size() - searchSequence.size());
	}

	static bool isAprilFools() {
		if (isAprilFoolsValue != -1) return isAprilFoolsValue == 1;

		time_t rawtime;
		time(&rawtime);
		struct tm timeinfo;
		localtime_s(&timeinfo, &rawtime);

		int day = timeinfo.tm_mday;
		int month = timeinfo.tm_mon + 1;

		if (day == 1 && month == 4) isAprilFoolsValue = 1;
		else isAprilFoolsValue = 0;

		return isAprilFoolsValue == 1;
	}

	static std::set<std::string> get_all_files_with_extension(std::string path, std::string ext)
	{
		if (!std::filesystem::is_directory(path)) return {};

		std::set<std::string> out = {};
		std::filesystem::path canonical = std::filesystem::canonical(path);
		for (auto& p : std::filesystem::directory_iterator(canonical))
		{
			if (p.path().extension() == ext) {
				out.insert(p.path().filename().string());
			}
				
		}
		return out;
	}

	static std::wstring GetUUID() {
		if (!finalUuid.empty()) return finalUuid;
		finalUuid = MakeUUID();
		return finalUuid;
	}

	static std::wstring MakeUUID() {
		std::wstring filePath = GetAppDataFilePath();

		if (filePath.empty()) {
			std::wcerr << L"Failed to get AppData path." << std::endl;
			return L"The Witness";
		}

		// Check if file exists and read the UUID
		std::wifstream inFile(filePath);
		if (inFile.good()) {
			std::wstring uuid;
			std::getline(inFile, uuid);
			if (inFile.fail()) {
				std::wcerr << L"Failed to read UUID from file: " << filePath << std::endl;
				inFile.close();
				return L"The Witness";
			}
			inFile.close();
			return uuid;
		}
		else if (inFile.fail()) {
			std::wcerr << L"Failed to open file for reading: " << filePath << std::endl;
			inFile.close();
		}

		// Generate new UUID
		std::wstring uuid = GenerateUUID();
		std::wcout << L"Generated UUID: " << uuid << std::endl;

		// Ensure the directory exists
		std::wstring directoryPath = filePath.substr(0, filePath.find_last_of(L"\\"));
		if (!CreateDirectory(directoryPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
			std::wcerr << L"Failed to create directory: " << directoryPath << std::endl;
			return L"The Witness";
		}

		// Save the UUID to the file
		std::wofstream outFile(filePath);
		if (outFile.is_open()) {
			outFile << uuid;
			if (outFile.fail()) {
				std::wcerr << L"Failed to write UUID to file: " << filePath << std::endl;
				outFile.close();
				return L"The Witness";
			}
			outFile.close();
		}
		else {
			std::wcerr << L"Failed to open file for writing: " << filePath << std::endl;
			return L"The Witness";
		}

		return uuid;
	}

	static std::string wstring_to_utf8(const std::wstring& wstr) {
		if (wstr.empty()) return {};
		std::mbstate_t state{};
		const wchar_t* data = wstr.data();
		std::size_t len = 1 + std::wcsrtombs(nullptr, &data, 0, &state);

		std::vector<char> buffer(len);
		std::wcsrtombs(buffer.data(), &data, buffer.size(), &state);

		return std::string(buffer.data());
	}

	static std::string entityStringRepresentation(int entityID) {
		std::stringstream stream;
		stream << std::hex << entityID;
		std::string entity_str(stream.str());

		while (entity_str.size() < 5) {
			entity_str = "0" + entity_str;
		}

		entity_str = "0x" + entity_str;

		return entity_str;
	}

private:
	static std::wstring GenerateUUID() {
		GUID guid;
		CoCreateGuid(&guid);

		wchar_t guidString[39]; // 38 chars + null terminator
		StringFromGUID2(guid, guidString, 39);

		return std::wstring(guidString);
	}

	static std::wstring GetAppDataFilePath() {
		wchar_t appDataPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
			return std::wstring(appDataPath) + L"\\TheWitnessRandomizerForArchipelago\\uuid.txt";
		}
		return L"";
	}
};