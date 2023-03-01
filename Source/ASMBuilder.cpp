#include "ASMBuilder.h"
#include <sstream>

#include <windows.h>
#include <debugapi.h>
#include <iomanip>

AsmBuilder::AsmBuilder(uint32_t rootAddress) : rootAddress(rootAddress)
{
}

AsmBuilder& AsmBuilder::add(std::vector<uint8_t> bytes)
{
	builder.insert(builder.end(), bytes.begin(), bytes.end());
	return *this;
}

AsmBuilder& AsmBuilder::rel(uint32_t referenceAddress)
{
	// Global references are relative to the address of the next operation. Here, we assume that the next operation
	//   is always 4 bytes after the given address.
	uint32_t currentAddress = rootAddress + ((uint32_t)builder.size() + 4);
	uint32_t relativeAddress = referenceAddress - currentAddress;
	uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&relativeAddress));
	builder.insert(builder.end(), bytes, bytes + 4);

	return *this;
}

AsmBuilder& AsmBuilder::abs(uint32_t referenceAddress)
{
	// Global references are relative to the address of the next operation. Here, we assume that the next operation
	//   is always 4 bytes after the given address.
	uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&referenceAddress));
	builder.insert(builder.end(), bytes, bytes + 4);

	return *this;
}

AsmBuilder& AsmBuilder::abs(uint64_t referenceAddress)
{
	// Global references are relative to the address of the next operation. Here, we assume that the next operation
	//   is always 4 bytes after the given address.
	uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&referenceAddress));
	builder.insert(builder.end(), bytes, bytes + 8);

	return *this;
}

const uint8_t* AsmBuilder::get() const
{
	return &builder[0];
}

const int AsmBuilder::size() const
{
	return (int)builder.size();
}

void AsmBuilder::printBytes() const
{
	std::wostringstream byteString;
	for (uint8_t byte : builder)
	{
		byteString << std::hex << std::setw(2) << std::setfill(L'0') << byte;
	}

	byteString << std::endl;
	OutputDebugStringW(byteString.str().c_str());
}