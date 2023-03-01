#include "HUDManager.h"

#include "ASMBuilder.h"
#include "Memory.h"


#define SUBTITLE_MAX_LINE_LENGTH 70

HudManager::HudManager() {
	findSetSubtitleOffsets();
	//setSubtitleSize(SubtitleSize::Small);
	overwriteSubtitleFunction();
}

void HudManager::update(float deltaSeconds) {
	updateBannerMessages(deltaSeconds);
	updateSubtitleMessages(deltaSeconds);
}

void HudManager::queueBannerMessage(std::string text, RgbColor color, float duration) {
	queuedBannerMessages.push(BannerMessage(text, color, duration));
}

void HudManager::showSubtitleMessage(std::string text, float duration) {
	writeSubtitle({ text });
	subtitleOverrideTimeRemaining = duration;
}

void HudManager::clearSubtitleMessage() {
	subtitleOverrideTimeRemaining = 0;
	subtitlesDirty = true;
}

void HudManager::setWorldMessage(std::string text) {
	if (worldMessage != text) {
		worldMessage = text;
		subtitlesDirty = true;
	}
}

void HudManager::setStatusMessage(std::string text) {
	if (statusMessage != text) {
		statusMessage = text;
		subtitlesDirty = true;
	}
}

void HudManager::setActionHint(std::string text) {
	if (actionHint != text) {
		actionHint = text;
		subtitlesDirty = true;
	}
}

void HudManager::setSubtitleSize(SubtitleSize size) {
	if (setSubtitleOffset == 0 || largeSubtitlePointerOffset == 0) {
		return;
	}

	uint32_t sizeOffset;
	switch (size) {
	case SubtitleSize::Large:
		sizeOffset = 0;
		break;
	case SubtitleSize::Medium:
		sizeOffset = 0x08;
		break;
	case SubtitleSize::Small:
		sizeOffset = 0x10;
	default:
		sizeOffset = 0;
	}

	uint32_t fontPointerOffset = largeSubtitlePointerOffset + sizeOffset;
	Memory::get()->WriteRelative(reinterpret_cast<void*>(setSubtitleOffset), &fontPointerOffset, 0x4);
}

void HudManager::updateBannerMessages(float deltaSeconds) {
	// If we're currently displaying a banner, decrement remaining message time and, if time still remains, return.
	if (bannerTimeRemaining > 0) {
		bannerTimeRemaining -= deltaSeconds;
		if (bannerTimeRemaining > 0) {
			return;
		}
	}

	// If we have a queued banner message, display it now.
	if (queuedBannerMessages.size() > 0) {
		const BannerMessage& message = queuedBannerMessages.front();

		Memory::get()->DisplayHudMessage(message.text, { message.color.R, message.color.G, message.color.B });
		bannerTimeRemaining = message.duration;

		queuedBannerMessages.pop();
	}
}

void HudManager::updateSubtitleMessages(float deltaSeconds) {
	// If we're currently displaying a subtitle message, decrement its remaining time. If this exhausts the remaining time, mark
	//   subtitles as needing refresh.
	if (subtitleOverrideTimeRemaining > 0) {
		subtitleOverrideTimeRemaining -= deltaSeconds;
		if (subtitleOverrideTimeRemaining <= 0) {
			// Mark subtitles as needing refreshing.
			subtitlesDirty = true;
		}
		else {
			// Do not process any further subtitle logic, since we're still overriding the contents.
			return;
		}
	}

	if (subtitlesDirty) {
		// When displaying system-level messages like input hints or status displays, we use the built-in subtitle functionality
		//   to render the text. This shows up to three lines of text, represented as separate strings.
		// We have up to three messages to show to the user, which we will show in a fixed order:
		//  - world messages		Quarry Laser requires Shapers and Erasers.
		//  - status messages		Speed boost! 30 seconds remaining.
		//  - action hints			[Hold TAB] Skip puzzle (have 2 skips).
		// Additionally, we only want to show complete messages, so if any of these messages would not be fully representable in
		//   the three available lines, don't show them.
		std::vector<std::string> lines;
		if (!actionHint.empty()) {
			std::vector<std::string> expandedHint = separateLines(actionHint);
			lines.insert(lines.begin(), expandedHint.begin(), expandedHint.end());
		}

		if (!statusMessage.empty()) {
			std::vector<std::string> expandedStatus = separateLines(statusMessage);
			if (lines.size() + expandedStatus.size() <= 3) {
				lines.insert(lines.begin(), expandedStatus.begin(), expandedStatus.end());
			}
		}

		if (!worldMessage.empty()) {
			std::vector<std::string> expandedWorld = separateLines(worldMessage);
			if (lines.size() + expandedWorld.size() <= 3) {
				lines.insert(lines.begin(), expandedWorld.begin(), expandedWorld.end());
			}
		}

		writeSubtitle(lines);
		subtitlesDirty = false;
	}
}

void HudManager::writeSubtitle(std::vector<std::string> lines) {
	// Subtitle rendering ignores newlines in favor of having separate draw calls for each line, so we need to split each input
	//   line by newline characters and append those to the list.
	std::vector<std::string> expandedLines;
	for (const std::string& line : lines) {
		std::vector<std::string> splitLines = separateLines(line);
		expandedLines.insert(expandedLines.end(), splitLines.begin(), splitLines.end());
	}

	// Since we want to keep this text as low on the screen as possible, we need to write our text using the lowest possible rows.
	//   In order to do this, we can compute an offset into the list of lines based on its size such that the last line in the
	//   list corresponds to the last line in the subtitle.
	int lineOffset = std::max(3 - (int)expandedLines.size(), 0); // NOTE: This will only display the first three lines.

	Memory::get()->DisplaySubtitles(
		0 - lineOffset >= 0 ? expandedLines[0 - lineOffset] : "",
		1 - lineOffset >= 0 ? expandedLines[1 - lineOffset] : "",
		2 - lineOffset >= 0 ? expandedLines[2 - lineOffset] : ""
	);
}

std::vector<std::string> HudManager::separateLines(std::string input) {
	// First, split the string into lines based on newlines.
	std::vector<std::string> splitLines;
	std::string::size_type searchIndex = 0, previousIndex = 0;
	while (true) {
		searchIndex = input.find('\n', previousIndex);
		if (searchIndex != std::string::npos) {
			splitLines.push_back(input.substr(previousIndex, searchIndex - previousIndex));
			previousIndex = searchIndex + 1;
		}
		else {
			break;
		}
	}

	if (previousIndex < input.size()) {
		splitLines.push_back(input.substr(previousIndex));
	}

	// Next, check every line for length and, if they are exceedingly long, split them into wrapped strings by length.
	std::vector<std::string> wrappedLines;
	for (const std::string& line : splitLines) {
		if (line.length() > SUBTITLE_MAX_LINE_LENGTH) {
			// The line is longer than our max width and needs to be wrapped.
			std::string choppedLine = line;
			while (choppedLine.length() > SUBTITLE_MAX_LINE_LENGTH) {
				// Find the closest space character to the end of the line.
				searchIndex = choppedLine.rfind(' ', SUBTITLE_MAX_LINE_LENGTH);
				if (searchIndex != std::string::npos) {
					// We found a space. Separate the string at that character and keep working.
					wrappedLines.push_back(choppedLine.substr(0, searchIndex));
					choppedLine = choppedLine.substr(searchIndex + 1);
				}
				else {
					// There was no space in the previous line. Simply split the string mid-word.
					wrappedLines.push_back(choppedLine.substr(0, SUBTITLE_MAX_LINE_LENGTH));
					choppedLine = choppedLine.substr(SUBTITLE_MAX_LINE_LENGTH);
				}
			}

			// Add the remainder of the string to the output.
			wrappedLines.push_back(choppedLine);
		}
		else {
			// The line is shorter than our max width and can be added to the output directly.
			wrappedLines.push_back(line);
		}
	}

	return wrappedLines;
}

void HudManager::findSetSubtitleOffsets() {
	// Find hud_draw_subtitles. We can do so by looking for where it loads its color information from constants, which seems to be
	//   a unique set of instructions:
	uint64_t drawSubtitleOffset = Memory::get()->executeSigScan({
		0x0F, 0x57, 0xFF,		// XORPS XMM7, XMM7
		0x0F, 0x28, 0xD7,
		0x0F, 0x28, 0xCF
	});

	if (drawSubtitleOffset == UINT64_MAX) {
		setSubtitleOffset = 0;
		largeSubtitlePointerOffset = 0;
		return;
	}

	// Next, find the address of the call that sets the subtitle size. Skip ahead:
	//  0x0F, 0x11, 0x44, 0x24, 0x40
	//  0x0F, 0x28, 0xC7
	//  0xE8, 0x60, 0xF8, 0x0F, 0x00	// CALL argb_color (makes black)
	//  0x48, 0x8D, 0x4C, 0x24, 0x40
	//  0x44, 0x8B, 0xE0
	//  0xE8, 0xF3, 0xF7, 0x0F, 0x00	// CALL argb_color (makes white)
	//  0x4C, 0x8B, 0x3D				// |
	//  ____, ____, ____, ____			// MOV R15, qword ptr [globals.subtitles_font]
	setSubtitleOffset = drawSubtitleOffset + 0x9 + 0x1D;

	// Finally, get the offset that is ordinarily used by this MOV operation, which corresponds to globals.subtitles_font. (We don't
	//   want to dereference this here, only get the address of the pointer itself, because we'll be changing which font we're
	//   pointing to rather than modifying the font itself.
	largeSubtitlePointerOffset = 0;
	if (!Memory::get()->ReadRelative(reinterpret_cast<void*>(setSubtitleOffset), &largeSubtitlePointerOffset, 0x4)) {
		setSubtitleOffset = 0;
		largeSubtitlePointerOffset = 0;
	}
}
void HudManager::overwriteSubtitleFunction() {
	Memory* memory = Memory::get();

	// Offsets of key functions. TODO: Determine these through sigscans.
	uint32_t func_hud_draw_subtitles = 0x1E99A0;
	uint32_t func_rendering_2D_right_handed = 0x25a330;
	uint32_t func_apply_2D_shader = 0x2581C0;
	uint32_t func_im_begin = 0x33f5b0;
	uint32_t func_im_flush = 0x33f5e0;
	uint32_t func_draw_text = 0x259380;

	// Offsets of key globals
	uint32_t globals_shader_text = 0x62D400;
	uint32_t global_subtitle_font = 0x62d458;

	// TODO: find a sane size for this
	uint64_t dataPayload = (uint64_t)VirtualAllocEx(memory->_handle, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	uint64_t address_tempLineCount = dataPayload;
	uint64_t address_tempString = dataPayload + 0x4;


	uint32_t tempLineCount = 4;
	WriteProcessMemory(memory->_handle, (LPVOID)address_tempLineCount, &tempLineCount, sizeof(uint32_t), NULL);


	char tempString1[] = "i have no idea what i am doing";
	char tempString2[] = "no, seriously, i have no idea what i am doing";
	char tempString3[] = "can someone send me, like, idk";
	char tempString4[] = "a functioning brain";

	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + 0x000), tempString4, sizeof(tempString4), NULL);
	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + 0x100), tempString3, sizeof(tempString3), NULL);
	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + 0x200), tempString2, sizeof(tempString2), NULL);
	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + 0x300), tempString1, sizeof(tempString1), NULL);

////// BEGIN REPLACEMENT FUNCTION

	AsmBuilder func(func_hud_draw_subtitles);

	// Save off registers to the stack.
	func.add({ 0x48,0x81,0xEC })			// SUB RSP, 0xD8	<- it's what the main function does and i don't want to mess with it
		.add({ 0xD8,0x00,0x00,0x00 })
		.add({ 0x48,0x89,0x9C,0x24 })		// MOV qword ptr [RSP + 0xF0], RBX
		.add({ 0xF0,0x00,0x00,0x00 })
		.add({ 0x4C,0x89,0xA4,0x24 })		// MOV qword ptr [RSP + 0xF8], R12
		.add({ 0xF8,0x00,0x00,0x00 });

	// Set rendering mode to right-handed 2D.
	//     ECX				renderingSpace (0 is indexed on pixels, 1 is unit UV)
	func.add({ 0x33, 0xC9 })				// XOR ECX, ECX
		.add({ 0xE8 }).rel(func_rendering_2D_right_handed);	// CALL <rendering_2D_right_handed>

	// Set the current 2D shader to shader_text.
	//     ECX				Shader* shader
	func.add({ 0x48,0x8B,0x0D })			// MOV RCX, qword ptr [<shader>]
		.rel(globals_shader_text)
		.add({ 0xE8 }).rel(func_apply_2D_shader);	// CALL <apply_2D_shader>

	// Set parameters and call im_begin(3).
	//     ECX				vertsPerPrimitive
	func.add({ 0xB9,0x03,0x00,0x00,0x00 })	// MOV ECX, 0x3
		.add({ 0xE8 }).rel(func_im_begin);	// CALL <im_begin>

	// Get information necessary for the loop. Note that we are drawing text from bottom to top, so our loop runs backwards.
	//     R12D				int currentIndex
	func.add({ 0x49,0xBC })					// MOV R12, <line count>
		.abs(address_tempLineCount)
		.add({ 0x45,0x8B,0x24,0x24 });		// MOV R12D, dword ptr [R12]

	int loopStart = func.size();

	//
	// Write temporary string to screen at 0,0.
	//

	// Decrement the counter so that we're pointing at the next string.
	func.add({ 0x41,0xFF,0xCC });			// DEC R12D

	// Set parameters and call draw_text.
	// draw_text parameters:
	//     RCX:8			Dynamic_Font* font
	//     XMM1_Da:4		float xPos
	//     XMM2_Da:4		float yPos
	//     R9D:4			ulong color (packed via argb_color())
	//     Stack[0x28]		char* strings
	//     Stack[0x30]		int zOrder
	// scratch registers:
	//     R10				string address
	//     XMM3				loop counter as float
	//     XMM4				vertical stride
	// - Set font
	func.add({ 0x48, 0x8B, 0x0D })			// MOV RCX, qword ptr [RIP + <font>]
		.rel(global_subtitle_font);
	// - Set xPos and yPos. (TEMP: 0x0)
	func.add({ 0x0F, 0x57, 0xC9 })			// XORPS XMM1, XMM1
		.add({ 0x0F, 0x57, 0xD2 });			// XORPS XMM2, XMM2
	// - Compute the Y offset based on the current line.
	func.add({ 0xF3,0x41,0x0F,0x2A,0xDC })	// CVTSI2SS XMM3, R12D					<-- convert the loop counter to a float
		.add({ 0xF3,0x0F,0x10,0x61,0x64 })	// MOVSS XMM4, DWORD PTR [RCX + 0x64]	<-- get font's line height, which is our vertical stride
		.add({ 0xF3,0x0F,0x59,0xE3 })		// MULSS XMM4, XMM3						<-- multiply the vertical stride by the loop counter to get the line offset
		.add({ 0xF3,0x0F,0x58,0xD4 });		// ADDSS XMM2, XMM4						<-- add line offset to yPos
	// - Set color
	func.add({ 0x41, 0xB9 })				// MOV R9D, 0xFFFFFFFF
		.add({ 0xFF, 0xFF, 0xFF, 0xFF });
	// - Set string pointer
	func.add({ 0x49, 0xBA })				// MOV R10, <string address>
		.abs(address_tempString);
	// - Adjust the string pointer by an offset based on the current index.
	func.add({ 0x41,0xBB })					// MOV R11D, <line allocation length>	<-- TEMP: currently a hardcoded value of 0x100
		.val((uint32_t)0x100)
		.add({ 0x45,0x0F,0xAF,0xDC })		// IMUL R11D, R12D						<-- multiply line allocation by loop count to get target address offset
		.add({ 0x4D,0x01,0xDA });			// ADD R10,R11							<-- add the address offset. note that it's safe to go from a 32->64 bit int here, since the upper half is zeroed

	// TEMP: offset by index to make sure the loop is working
	//func.add({ 0x49,0x83,0xC2,0x02 });		// ADD R10, 0x2
	
	func.add({ 0x4C, 0x89, 0x54, 0x24, 0x20 });	// MOV qword ptr [RSP + 0x20], R10
	// - Set zOrder
	func.add({ 0xC7, 0x44, 0x24, 0x28 })	// MOV dword ptr [RSP + 0x28], 0x20
		.add({ 0x20, 0x00, 0x00, 0x00 });
	// - Call draw_text
	func.add({ 0xE8 }).rel(func_draw_text);	// CALL <draw_text>

	// Jump back in the loop if we haven't reached the end.
	func.add({ 0x45,0x85,0xE4 });			// TEST R12D,R12D
	int loopEnd = func.size();
	func.add({ 0x75 })						// JMP <loopStart>
		.val((int8_t)(loopStart - (loopEnd + 2)));

	// Call im_flush().
	func.add({ 0xE8 }).rel(func_im_flush);

	// Restore registers from the stack.
	func.add({ 0x48,0x8B,0x9C,0x24 })		// MOV RBX, qword ptr [RSP + 0xF0]
		.add({ 0xF0,0x00,0x00,0x00 })
		.add({ 0x4C,0x8B,0xA4,0x24 })		// MOV R12, qword ptr [RSP + 0xF8]
		.add({ 0xF8,0x00,0x00,0x00 })
		.add({ 0x48,0x81,0xC4 })			// ADD RSP,0xD8
		.add({ 0xD8,0x00,0x00,0x00 });
		
	// Return.
	func.add({ 0xC3 });						// RET

////// END REPLACEMENT FUNCTION

	// Write function to memory.
	func.printBytes();
	bool bSuccess = WriteProcessMemory(memory->_handle, reinterpret_cast<const LPVOID>(func_hud_draw_subtitles + memory->_baseAddress), func.get(), func.size(), NULL);
}