#pragma once
#include <vector>

class AsmBuilder
{
public:

	AsmBuilder(uint32_t rootAddress);

	AsmBuilder& add(std::vector<uint8_t> bytes);
	AsmBuilder& rel(uint32_t referenceAddress);
	AsmBuilder& abs(uint32_t referenceAddress);
	AsmBuilder& abs(uint64_t referenceAddress);

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
};