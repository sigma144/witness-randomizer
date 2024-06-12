#pragma once

#include <map>
#include <string>
#include <vector>

class AsmBuilder
{
public:

	AsmBuilder(uint32_t rootAddress);

	AsmBuilder& add(std::vector<uint8_t> bytes);

	// Insert a reference to the given address, relative to the current instruction, for use in MOV-like operations.
	AsmBuilder& rel(uint32_t referenceAddress);

	// Flow control helpers for use in JMP-like operations. Use mark() to identify a point you want to jump to, and
	//   use jump() to write a relative address to that mark.
	AsmBuilder& mark(std::string label);
	AsmBuilder& jump(std::string label);

	// Writes a specific value as bytes.
	template<typename T>
	AsmBuilder& val(T value) {
		uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&value));
		builder.insert(builder.end(), bytes, bytes + sizeof(T));
		
		return *this;
	}

	const uint8_t* get() const;
	const int size() const;

	// Prints the string of bytes to the console in hexadecimal format.
	void printBytes() const;

private:
	uint32_t rootAddress;
	std::vector<uint8_t> builder;

	std::map<std::string, uint32_t> bookmarks;
	std::multimap<std::string, uint32_t> unmatchedJumps;

};