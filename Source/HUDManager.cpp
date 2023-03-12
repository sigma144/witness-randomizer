#include "HUDManager.h"

#include "ASMBuilder.h"
#include "Input.h"
#include "Memory.h"


#define MAX_STRING_LENGTH 0x80

#define NOTIFICATION_FLASH_TIME 1.f
#define NOTIFICATION_HOLD_BRIGHT_TIME 3.f
#define NOTIFICATION_DIM_TIME 5.f
#define NOTIFICATION_HOLD_DIM_TIME 10.f
#define NOTIFICATION_FADE_TIME 10.f
#define NOTIFICATION_TOTAL_TIME (NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME + NOTIFICATION_DIM_TIME + NOTIFICATION_HOLD_DIM_TIME + NOTIFICATION_FADE_TIME)


HudManager::HudManager() {
	findSetSubtitleOffsets();
	//setSubtitleSize(SubtitleSize::Small);
	overwriteSubtitleFunction();
	writePayload();
}

void HudManager::update(float deltaSeconds) {
	updateBannerMessages(deltaSeconds);
	updateNotifications(deltaSeconds);
	//updateSubtitleMessages(deltaSeconds);

	InputWatchdog* input = InputWatchdog::get();
	InteractionState interactionState = input->getInteractionState();
	bool isSolving = (interactionState == Focusing || interactionState == Solving);

	if (isSolving && solveFadePercent < 1.f) {
		solveFadePercent = std::min(solveFadePercent + deltaSeconds, 1.f);
		hudTextDirty = true;
	}
	else if (!isSolving && solveFadePercent > 0.f) {
		solveFadePercent = std::max(solveFadePercent - deltaSeconds * 3.f, 0.f);
		hudTextDirty = true;
	}

	if (hudTextDirty) {
		writePayload();
		hudTextDirty = false;
	}
}

void HudManager::queueBannerMessage(std::string text, RgbColor color, float duration) {
	queuedBannerMessages.push(BannerMessage(text, color, duration));
}

void HudManager::queueNotification(std::string text, RgbColor color) {
	queuedNotifications.push(Notification(text, color));
}

void HudManager::showSubtitleMessage(std::string text, float duration) {
	writeSubtitle({ text });
	subtitleOverrideTimeRemaining = duration;
}
void HudManager::clearSubtitleMessage() {
	subtitleOverrideTimeRemaining = 0;
	hudTextDirty = true;
}

void HudManager::setWorldMessage(std::string text) {
	if (worldMessage != text) {
		worldMessage = text;
		hudTextDirty = true;
	}
}

void HudManager::setWalkStatusMessage(std::string text) {
	if (walkStatusMessage != text) {
		walkStatusMessage = text;
		hudTextDirty = true;
	}
}

void HudManager::setSolveStatusMessage(std::string text) {
	if (solveStatusMessage != text) {
		solveStatusMessage = text;
		hudTextDirty = true;
	}
}

void HudManager::setSubtitleSize(SubtitleSize size) {
	//if (setSubtitleOffset == 0 || largeSubtitlePointerOffset == 0) {
	//	return;
	//}
	//uint32_t sizeOffset;
	//switch (size) {
	//case SubtitleSize::Large:
	//	sizeOffset = 0;
	//	break;
	//case SubtitleSize::Medium:
	//	sizeOffset = 0x08;
	//	break;
	//case SubtitleSize::Small:
	//	sizeOffset = 0x10;
	//default:
	//	sizeOffset = 0;
	//}
	//uint32_t fontPointerOffset = largeSubtitlePointerOffset + sizeOffset;
	//Memory::get()->WriteRelative(reinterpret_cast<void*>(setSubtitleOffset), &fontPointerOffset, 0x4);
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

void HudManager::updateNotifications(float deltaSeconds) {
	int seenNotifications = 0;

	std::vector<Notification>::iterator iterator = activeNotifications.begin();
	while (iterator != activeNotifications.end()) {
		// Fade notifications out more quickly if there are a lot of them.
		seenNotifications++;
		float multiplier = 0.75f + 0.25f * seenNotifications;

		iterator->age += deltaSeconds * multiplier;
		if (iterator->age >= NOTIFICATION_TOTAL_TIME) {
			iterator = activeNotifications.erase(iterator);
			continue;
		}

		// Determine whether or not the notification is in a time block where it is fading in or out and, if so, mark it as dirty.
		float relativeAge = iterator->age;
		if (relativeAge < NOTIFICATION_FLASH_TIME) {
			hudTextDirty = true;
		}

		relativeAge -= (NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME);
		if (relativeAge > 0 && relativeAge < NOTIFICATION_DIM_TIME) {
			hudTextDirty = true;
		}

		relativeAge -= (NOTIFICATION_DIM_TIME + NOTIFICATION_HOLD_DIM_TIME);
		if (relativeAge > 0) {
			hudTextDirty = true;
		}

		iterator++;
	}

	timeToNextNotification -= deltaSeconds;
	if (timeToNextNotification <= 0) {
		if (queuedNotifications.size() > 0) {
			const Notification& notification = queuedNotifications.front();
			activeNotifications.insert(activeNotifications.begin(), notification);
			queuedNotifications.pop();

			timeToNextNotification = 0.75f;
		}
	}
}

void HudManager::updateSubtitleMessages(float deltaSeconds) {
	// If we're currently displaying a subtitle message, decrement its remaining time. If this exhausts the remaining time, mark
	//   subtitles as needing refresh.
	if (subtitleOverrideTimeRemaining > 0) {
		subtitleOverrideTimeRemaining -= deltaSeconds;
		if (subtitleOverrideTimeRemaining <= 0) {
			// Mark subtitles as needing refreshing.
			hudTextDirty = true;
		}
		else {
			// Do not process any further subtitle logic, since we're still overriding the contents.
			return;
		}
	}

	if (hudTextDirty) {
		// When displaying system-level messages like input hints or status displays, we use the built-in subtitle functionality
		//   to render the text. This shows up to three lines of text, represented as separate strings.
		// We have up to three messages to show to the user, which we will show in a fixed order:
		//  - world messages		Quarry Laser requires Shapers and Erasers.
		//  - status messages		Speed boost! 30 seconds remaining.
		//  - action hints			[Hold TAB] Skip puzzle (have 2 skips).
		// Additionally, we only want to show complete messages, so if any of these messages would not be fully representable in
		//   the three available lines, don't show them.
		std::vector<std::string> lines;
		if (!solveStatusMessage.empty()) {
			std::vector<std::string> expandedHint = separateLines(solveStatusMessage);
			lines.insert(lines.begin(), expandedHint.begin(), expandedHint.end());
		}

		if (!walkStatusMessage.empty()) {
			std::vector<std::string> expandedStatus = separateLines(walkStatusMessage);
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
		hudTextDirty = false;
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
		if (line.length() > MAX_STRING_LENGTH) {
			// The line is longer than our max width and needs to be wrapped.
			std::string choppedLine = line;
			while (choppedLine.length() > MAX_STRING_LENGTH) {
				// Find the closest space character to the end of the line.
				searchIndex = choppedLine.rfind(' ', MAX_STRING_LENGTH);
				if (searchIndex != std::string::npos) {
					// We found a space. Separate the string at that character and keep working.
					wrappedLines.push_back(choppedLine.substr(0, searchIndex));
					choppedLine = choppedLine.substr(searchIndex + 1);
				}
				else {
					// There was no space in the previous line. Simply split the string mid-word.
					wrappedLines.push_back(choppedLine.substr(0, MAX_STRING_LENGTH));
					choppedLine = choppedLine.substr(MAX_STRING_LENGTH);
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
	uint32_t func_get_string_width = 0x3430A0;
	uint32_t func_rendering_2D_right_handed = 0x25a330;
	uint32_t func_apply_2D_shader = 0x2581C0;
	uint32_t func_im_begin = 0x33f5b0;
	uint32_t func_im_flush = 0x33f5e0;
	uint32_t func_draw_text = 0x259380;

	// Offsets of key globals
	uint32_t globals_shader_text = 0x62D400;
	uint32_t global_subtitle_font = 0x62D458;
	uint32_t screen_target_width = 0x469A5F0;
	uint32_t screen_target_height = 0x469A5F4;

	// Reserve memory for the payload.
	address_hudTextPayload = (uint64_t)VirtualAllocEx(memory->_handle, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//uint64_t address_tempLineCount = dataPayload;
	//uint64_t address_tempHorizAlignment = dataPayload + 0x08;
	//uint64_t address_tempVerticalAlignment = dataPayload + 0x10;
	//uint64_t address_tempString = dataPayload + 0x18;

	const float margin_width = 0.02f;
	const float line_spacing = 1.0f;
	const float line_top_adjust = 0.25f;
	const float shadow_depth_pixels = 2.f;

	//uint64_t temp_writer = address_payload;

	//// payload info
	//uint32_t tempBlockCount = 3;
	//WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + payloadInfo_address_blockCount), &tempBlockCount, sizeof(uint32_t), NULL);
	//temp_writer += payloadInfo_totalSize;

	//// block info
	//for (int blockId = 0; blockId < tempBlockCount; blockId++) {
	//	uint32_t tempLineCount = 4;
	//	WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + blockInfo_address_lineCount), &tempLineCount, sizeof(uint32_t), NULL);

	//	float tempHorizAlignment = blockId * 0.5f;
	//	WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + blockInfo_address_horizontalAlignment), &tempHorizAlignment, sizeof(float), NULL);

	//	float tempHorizPosition = blockId * 0.5f;
	//	WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + blockInfo_address_horizontalPosition), &tempHorizPosition, sizeof(float), NULL);

	//	float tempVerticalAlignment = blockId * 0.5f;
	//	WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + blockInfo_address_vertPosition), &tempVerticalAlignment, sizeof(float), NULL);

	//	temp_writer += blockInfo_totalSize;

	//	for (int lineId = 0; lineId < tempLineCount; lineId++) {
	//		RgbColor tempColor(1.f,tempHorizAlignment,lineId*0.3f,0.5f+lineId*0.15f);
	//		uint32_t tempArgb = tempColor.argb();
	//		WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + lineInfo_address_color), &tempArgb, sizeof(uint32_t), NULL);

	//		std::stringstream lineText;
	//		lineText << "BLOCK: " << blockId + 1 << ", string: " << (lineId + 1);

	//		char stringBuff[SUBTITLE_MAX_LINE_LENGTH];
	//		strcpy_s(stringBuff, lineText.str().c_str());
	//		WriteProcessMemory(memory->_handle, (LPVOID)(temp_writer + lineInfo_address_string), stringBuff, sizeof(stringBuff), NULL);

	//		temp_writer += lineInfo_totalSize;
	//	}
	//}

////// BEGIN REPLACEMENT FUNCTION

	// The data structure:
	//  - int32 num_blocks
	//  - block[] blocks:
	//    - int32 num_lines
	//    - float horiz_alignment
	//    - float vert_alignment
	//    - line[] lines: (packed from last to first)
	//      - int32 packed_color
	//      - char[] string

	AsmBuilder func(func_hud_draw_subtitles);

	const uint32_t stack_cache_RBX = 0xB8;
	const uint32_t stack_cache_R12 = 0xB0;
	const uint32_t stack_cache_R13 = 0xA8;
	const uint32_t stack_cache_R14 = 0xA0;

	const uint32_t stack_y_origin = 0x9A;
	const uint32_t stack_line_height = 0x90;
	const uint32_t stack_margin_pixels = 0x88;
	const uint32_t stack_draw_area_x = 0x80;
	const uint32_t stack_draw_area_y = 0x78;
	const uint32_t stack_currentBlock_horizAlignment = 0x70;
	const uint32_t stack_currentBlock_horizPosition = 0x68;
	const uint32_t stack_currentLine_xPos = 0x60;
	const uint32_t stack_currentLine_yPos = 0x58;

	////////////
	// Save off registers to the stack.
	////////////
	func.add({ 0x48,0x81,0xEC })			// SUB RSP, 0xD8	<- it's what the main function does and i don't want to mess with it
		.add({ 0xD8,0x00,0x00,0x00 })
		.add({ 0x48,0x89,0x9C,0x24 })		// MOV qword ptr [RSP + stack_cache_RBX], RBX
		.val(stack_cache_RBX)
		.add({ 0x4C,0x89,0xA4,0x24 })		// MOV qword ptr [RSP + stack_cache_R12], R12
		.val(stack_cache_R12)
		.add({ 0x4C,0x89,0xAC,0x24 })		// MOV qword ptr [RSP + stack_cache_R13], R13
		.val(stack_cache_R13)
		.add({ 0x4C,0x89,0xB4,0x24 })		// MOV qword ptr [RSP + stack_cache_R14], R14
		.val(stack_cache_R14);
	////////////

	////////////
	// Perform rendering setup.
	////////////
	// - Set rendering mode to right-handed 2D, scaled on pixels.
	//     ECX				renderingSpace (0 uses pixels as units, 1 is unit UV)
	func.add({ 0x33, 0xC9 })				// XOR ECX, ECX
		.add({ 0xE8 })						// CALL <rendering_2D_right_handed>
		.rel(func_rendering_2D_right_handed);
	// - Set the current 2D shader to shader_text.
	//     RCX				Shader* shader
	func.add({ 0x48,0x8B,0x0D })			// MOV RCX, qword ptr [<shader>]
		.rel(globals_shader_text)
		.add({ 0xE8 })						// CALL <apply_2D_shader>
		.rel(func_apply_2D_shader);
	// - Set parameters and call im_begin(3).
	//     ECX				vertsPerPrimitive
	func.add({ 0xB9,0x03,0x00,0x00,0x00 })	// MOV ECX, 0x3
		.add({ 0xE8 }).rel(func_im_begin);	// CALL <im_begin>
	////////////

	////////////
	// Compute a margin based on the screen's width, then compute the actual bounds of the screen.
	////////////
	// - Load the screen's width and convert it to a float.
	//     R8D:4				int32 screen_width
	//     XMM0_Da:4			float screen_width
	func.add({ 0x44,0x8B,0x05 })			// MOV R8D, dword ptr [RIP + <screen_target_width>]
		.rel(screen_target_width)
		.add({ 0xF3,0x41,0x0F,0x2A,0xC0 });	// CVTSI2SS XMM0, R8D
	// - Compute the margin, then save it off to the stack for future reference.
	//     R8D:4				<buffer for loading constant>
	//     XMM1_Da:4			float margin_pixels
	func.add({ 0x41,0xB8 })					// MOV R8D, <margin_width>
		.val(margin_width)
		.add({ 0x66,0x41,0x0F,0x6E,0xC8 })	// MOVD XMM1, R8D
		.add({ 0xF3,0x0F,0x59,0xC8 })		// MULSS XMM1, XMM0
		.add({ 0xF3,0x0F,0x11,0x8C,0x24 })	// MOVSS dword ptr [RSP + stack_margin_pixels], XMM1
		.val(stack_margin_pixels);
	// - Determine the actual drawable width of the screen by subtracting twice the margin from the actual width.
	//     XMM1_Da:4			float double_margin_pixels
	//     XMM0_Da:4			float draw_area_x
	func.add({ 0xF3,0x0F,0x58,0xC9 })		// ADDSS XMM1, XMM1
		.add({ 0xF3,0x0F,0x5C,0xC1 })		// SUBSS XMM0, XMM1
		.add({ 0xF3,0x0F,0x11,0x84,0x24 })	// MOVSS dword ptr [RSP + stack_draw_area_x], XMM0
		.val(stack_draw_area_x);
	// - Load the screen's height and convert it to a float.
	//     R8D:4				int32 screen_height
	//     XMM0_Da:4			float screen_height
	func.add({ 0x44,0x8B,0x05 })			// MOV R8D, dword ptr [RIP + <screen_target_height>]
		.rel(screen_target_height)
		.add({ 0xF3,0x41,0x0F,0x2A,0xC0 });	// CVTSI2SS XMM0, R8D
	// - Determine the drawable height of the screen by subtracting twice the margin from the actual height.
	//     XMM0_Da:4			float draw_area_y
	func.add({ 0xF3,0x0F,0x5C,0xC1 })		// SUBSS XMM0, XMM1
		.add({ 0xF3,0x0F,0x11,0x84,0x24 })	// MOVSS dword ptr [RSP + stack_draw_area_y], XMM0
		.val(stack_draw_area_y);
	////////////

	////////////
	// Load data about the overall set of blocks into stable registers.
	// Output:
	//     R13				void* read_head
	//     R14D:4			int32 block_count
	////////////
	// - Load the address of the payload into R13, which we will use as a read head as we move through our data.
	func.add({ 0x49,0xBD })					// MOVABS R13, <address_payload>
		.abs(address_hudTextPayload)
	// - Load the number of blocks into R14.
		.add({ 0x45,0x8B,0xB5 })			// MOV R14D, dword ptr [R13 + payload::address_blockCount]
		.val(HudManager::HudTextPayload::address_blockCount)
	// - Move the read head forward by the total payload size.
		.add({ 0x49,0x81,0xC5 })			// ADD R13, payload::totalDataSize
		.val(HudManager::HudTextPayload::totalDataSize);
	////////////

	////////////
	// Start the block loop.
	////////////
	int blockLoopStart = func.size();
	////////////

	////////////
	// Decrement the block counter.
	////////////
	func.add({ 0x41,0xFF,0xCE });			// DEC R14D
	////////////
	
	////////////
	// Load the line count into a stable register.
	// Input:
	//     R13				void* read_head
	// Output:
	//     R12D:4			int32 line_count
	////////////
	func.add({ 0x45,0x8B,0xA5 })			// MOV R12D, dword ptr [R13 + block::address_lineCount]
		.val(HudManager::HudTextBlock::address_lineCount);
	////////////
	
	////////////
	// Save off the horizontal position and alignment parameters for the block.
	// Input:
	//     R13				void* read_head
	////////////
	// - Save the horizontal alignment.
	//     R8D:4			(scratch for transferring)
	func.add({ 0x45,0x8B,0x85 })			// MOV R8D, dword ptr [r13 + block::address_horizontalAlignment]
		.val(HudManager::HudTextBlock::address_horizontalAlignment)
		.add({ 0x44,0x89,0x84,0x24 })		// MOV dword ptr [rsp + stack_currentBlock_horizAlignment], R8D
		.val(stack_currentBlock_horizAlignment);
	// - Save the horizontal position.
	//     R8D:4			(scratch for transferring)
	func.add({ 0x45,0x8B,0x85 })			// MOV R8D, dword ptr [r13 + block::address_horizontalPosition]
		.val(HudManager::HudTextBlock::address_horizontalPosition)
		.add({ 0x44,0x89,0x84,0x24 })		// MOV dword ptr [rsp + stack_currentBlock_horizPosition], R8D
		.val(stack_currentBlock_horizPosition);
	////////////

	////////////
	// Compute the Y origin of the entire block of text based on its line count.
	// Input:
	//     R12D:4			int32 line_count
	//     R13				void* read_head
	////////////
	// - Load the drawable area of the screen.
	func.add({ 0xF3,0x0F,0x10,0x84,0x24 })	// MOVSS XMM0, dword ptr [RSP + stack_draw_area_y]
		.val(stack_draw_area_y);
	// - Retrieve our font and find its line height.
	//     R8:8				Dynamic_Font* font
	//     XMM1_Da:4		float font::height
	func.add({ 0x4C,0x8B,0x05 })			// MOV R8, qword ptr [RIP + <font>]
		.rel(global_subtitle_font)
		.add({ 0xF3,0x41,0x0F,0x10,0x48 })	// MOVSS XMM1, dword ptr [R8 + 0x64]
		.add({ 0x64 });
	// - Load the line spacing parameter, then multiply it by the font height to find the actual line height.
	//     R8D:4				<buffer for loading constant>
	//     XMM2_Da:4			float line_height
	func.add({ 0x41,0xB8 })					// MOV R8D, <line_spacing>
		.val(line_spacing)
		.add({ 0x66,0x41,0x0F,0x6E,0xD0 })	// MOVD XMM2, R8D
		.add({ 0xF3,0x0F,0x59,0xD1 });		// MULSS XMM2, XMM1
	// - Save off the line height for future reference.
	func.add({ 0xF3,0x0F,0x11,0x94,0x24 })	// MOVSS dword ptr [RSP + stack_line_height], XMM2
		.val(stack_line_height);
	// - Compute the total height of all but one of the lines based on the spaced line height. (We need to do this because
	//   we only want to add spacing between every row. Our formula will be: [(font_height + spacing) * (N-1) + font_height],
	//   which evaluates to [N * font_height + (N-1) * spacing].
	//     R8D:4			int32 line_count - 1
	//     XMM3_Da:4		float (font_height + spacing) * 3
	func.add({ 0x45,0x89,0xE0 })			// MOV R8D, R12D
		.add({ 0x41,0x83,0xE8,0x01 })		// SUB R8D, 0x1
		.add({ 0xF3,0x41,0x0F,0x2A,0xD8 })	// CVTSI2SS XMM3, R8D
		.add({ 0xF3,0x0F,0x59,0xDA });		// MULSS XMM3, XMM2
	// - Add the height of an un-spaced line to get the total height of the block.
	//     XMM3_Da:4		float block_height
	func.add({ 0xF3,0x0F,0x58,0xD9 });		// ADDSS XMM3, XMM1
	// - Next, remove a fraction of a line's worth of text from the block in order to compensate for the fact that the
	//   font's height already has spacing built-in.
	//     XMM2_Da:4		float adjustment
	//     XMM3_Da:4		float block_height
	func.add({ 0x41,0xB8 })					// MOV R8D, <line_top_adjust>
		.val(line_top_adjust)
		.add({ 0x66,0x41,0x0F,0x6E,0xD0 })	// MOVD XMM2, R8D
		.add({ 0xF3,0x0F,0x59,0xD1 })		// MULSS XMM2, XMM1
		.add({ 0xF3,0x0F,0x5C,0xDA });		// SUBSS XMM3, XMM2
	// - Subtract the block height from the drawable area of the screen to find the relative position of the block if it
	//   were to be placed at the top of the screen, then multiply it by the vertical positioning parameter in order to
	//   place it at the correct height. Our formula is: [(draw_area_y - block_height) * vertical_alignment]
	//     XMM1_Da:4		float vertical_alignment
	//     XMM0_Da:4		float relative_position
	func.add({ 0xF3,0x0F,0x5C,0xC3 })		// SUBSS XMM0, XMM3
		.add({ 0x66,0x41,0x0F,0x6E,0x8D })	// MOVD XMM1, [R13 + block::address_vertPosition]
		.val(HudManager::HudTextBlock::address_verticalPosition)
		.add({ 0xF3,0x0F,0x59,0xC1 });		// MULSS XMM0, XMM1
	// - Load the margin, add it to the relative position in order to find the final absolute position, then save it off.
	//     XMM0_Da:4		float absolute_position
	//     XMM1_Da:4		float margin
	func.add({ 0xF3,0x0F,0x10,0x8C,0x24 })	// MOVSS XMM1, dword ptr [RSP + stack_margin_pixels]
		.val(stack_margin_pixels)
		.add({ 0xF3,0x0F,0x58,0xC1 })		// ADDSS XMM0, XMM1
		.add({ 0xF3,0x0F,0x11,0x84,0x24 })	// MOVSS dword ptr [RSP + stack_y_origin], XMM0
		.val(stack_y_origin);
	////////////

	////////////
	// Advance the read head to past the block info.
	////////////
	func.add({ 0x49,0x81,0xC5 })			// ADD R13, block::totalDataSize
		.val(HudManager::HudTextBlock::totalDataSize);
	////////////

	////////////
	// Start the line loop.
	// NOTE: R12D goes from being line_count to current_index here.
	////////////
	int lineLoopStart = func.size();
	////////////

	////////////
	// Decrement the line counter.
	////////////
	func.add({ 0x41,0xFF,0xCC });			// DEC R12D
	////////////

	////////////
	// Determine string width by calling Dynamic_Font::get_string_width_in_pixels.
	// get_string_width_in_pixels parameters:
	//     RCX:8			Dynamic_Font* font
	//     RDX:8			char* string
	//     R8D:4			int32 flags		<-- always 0x20 as far as i can tell
	// Input:
	//     R13:8			void* read_head
	// Output:
	//     XMM0_Da:4		float string_width
	////////////
	// - Set font
	func.add({ 0x48,0x8B,0x0D })			// MOV RCX, qword ptr [RIP + <font>]
		.rel(global_subtitle_font);
	// - Set string pointer
	func.add({ 0x4C,0x89,0xEA })			// MOV RDX, R13
		.add({ 0x48,0x81,0xC2 })			// ADD RDX, line::address_string
		.val(HudManager::HudTextLine::address_string);
	// - Set flags
	func.add({ 0x41,0xB8 })					// MOV R8D, 0x20
		.val<uint32_t>(0x20);
	// - Call get_string_width_in_pixels
	func.add({ 0xE8 })						// CALL <Dynamic_Font::get_string_width_in_pixels>
		.rel(func_get_string_width);
	////////////

	////////////
	// Determine the X origin of the string based on its width and the horizontal alignment parameter. The formula here is:
	//   [(draw_area_x * horizontal_position) - (string_width * horizontal_alignment)]
	// Input:
	//     XMM0_Da:4		float string_width
	// Output:
	//     XMM1_Da:4		float xPos
	////////////
	// - Multiply the string width by the horizontal alignment parameter in order to get its offset from the origin.
	//     XMM0_Da:4		float string_offset
	func.add({ 0x66,0x0F,0x6E,0x8C,0x24 })	// MOVD XMM1, dword ptr [RSP + stack_currentBlock_horizPosition]
		.val(stack_currentBlock_horizPosition)
		.add({ 0xF3,0x0F,0x59,0xC1 });		// MULSS XMM0, XMM1
	// - Retrieve the drawable area, then multiply it by the horizontal positioning parameter in order to get the origin
	//   of the string.
	//     XMM2_Da:4		float horizontal_alignment
	//     XMM1_Da:4		float string_origin
	func.add({ 0x66,0x0F,0x6E,0x94,0x24 })	// MOVD XMM2, dword ptr [RSP + stack_currentBlock_horizAlignment]
		.val(stack_currentBlock_horizAlignment)
		.add({ 0xF3,0x0F,0x10,0x8C,0x24 })	// MOVSS XMM1, dword ptr [RSP + stack_draw_area_x]
		.val(stack_draw_area_x)
		.add({ 0xF3,0x0F,0x59,0xCA });		// MULSS XMM1, XMM2
	// - Subtract the offset from the origin to get the position of the string relative to the drawable area.
	//     XMM1_Da:4		float relative_position
	func.add({ 0xF3,0x0F,0x5C,0xC8 });		// SUBSS XMM1, XMM0
	// - Load the margin, then add it to the relative position in order to find the final absolute position.
	//     XMM0_Da:4		float margin
	//     XMM1_Da:4		float xPos
	func.add({ 0xF3,0x0F,0x10,0x84,0x24 })	// MOVSS XMM0, dword ptr [RSP + stack_margin_pixels]
		.val(stack_margin_pixels)
		.add({ 0xF3,0x0F,0x58,0xC8 });		// ADDSS XMM1, XMM0

	////////////
	// Compute the Y origin for this specific line based on the block's origin and the current line count.
	// Input:
	//     R12D:4			int32 current_index
	// Output:
	//     XMM2_Da:4		float yPos
	////////////
	// - Convert the loop counter to a float.
	//     XMM2_Da:4		float line_count
	func.add({ 0xF3,0x41,0x0F,0x2A,0xD4 });	// CVTSI2SS XMM2, R12D
	// - Retrieve the line height, then multiply it by the line count in order to get the position of this line relative
	//   to the block's origin.
	//     XMM3_Da:4		float relative_position
	func.add({ 0xF3,0x0F,0x10,0x9C,0x24 })	// MOVSS XMM3, dword ptr [RSP + stack_line_height]
		.val(stack_line_height)
		.add({ 0xF3,0x0F,0x59,0xDA });		// MULSS XMM3, XMM2
	// Retrieve the Y origin of the text block, then add the relative offset in order to get our final position.
	//     XMM2_Da:4		float yPos
	func.add({ 0xF3,0x0F,0x10,0x94,0x24 })	// MOVSS XMM2, dword ptr [RSP + stack_y_origin]
		.val(stack_y_origin)
		.add({ 0xF3,0x0F,0x58,0xD3 });		// ADDSS XMM2, XMM3
	////////////

	////////////
	// Save xPos and yPos to the stack so that we can reload it between draw_text calls.
	// Input:
	//     XMM1_Da:4		float xPos
	//     XMM2_Da:4		float yPos
	////////////
	func.add({ 0xF3,0x0F,0x11,0x8C,0x24 })	// MOVSS dword ptr [RSP + stack_currentLine_xPos], XMM1
		.val(stack_currentLine_xPos)
		.add({ 0xF3,0x0F,0x11,0x94,0x24 })	// MOVSS dword ptr [RSP + stack_currentLine_yPos], XMM2
		.val(stack_currentLine_yPos);
	////////////

	////////////
	// Set parameters and call draw_text for the drop shadow.
	// Input:
	//     R13:8			void* read_head
	//     XMM1_Da:4		float xPos
	//     XMM2_Da:4		float yPos
	// draw_text parameters:
	//     RCX:8			Dynamic_Font* font
	//     XMM1_Da:4		float xPos			<-- already set
	//     XMM2_Da:4		float yPos			<-- already set
	//     R9D:4			ulong color (packed via argb_color())
	//     Stack[0x28]		char* strings
	//     Stack[0x30]		int32 flags			<-- always 0x20, no idea what these are
	////////////
	// - Set font.
	func.add({ 0x48,0x8B,0x0D })			// MOV RCX, qword ptr [RIP + <font>]
		.rel(global_subtitle_font);
	// - Set color.
	func.add({ 0x45,0x8B,0x8D })			// MOV R9D, dword ptr [R13 + line::address_color]
		.val(HudManager::HudTextLine::address_shadowColor);
	// - Set string pointer
	func.add({ 0x4D,0x89,0xE8 })			// MOV R8, R13
		.add({ 0x49,0x81,0xC0 })			// ADD R8, line::address_string
		.val(HudManager::HudTextLine::address_string)
		.add({ 0x4C,0x89,0x44,0x24,0x20 });	// MOV qword ptr [RSP + 0x20], R8
	// - Set flags
	func.add({ 0xC7, 0x44, 0x24, 0x28 })	// MOV dword ptr [RSP + 0x28], 0x20
		.val<uint32_t>(0x20);
	// - Call draw_text
	func.add({ 0xE8 }).rel(func_draw_text);	// CALL <draw_text>
	////////////

	////////////
	// Restore xPos and yPos from the stack and offset it by the shadow depth.
	// Output:
	//     XMM1_Da:4		float xPos
	//     XMM2_Da:4		float yPos
	////////////
	// - Read the values from the stack.
	//     XMM1_Da:4		float xPos
	//     XMM2_Da:4		float yPos
	func.add({ 0xF3,0x0F,0x10,0x8C,0x24 })	// MOVSS XMM1, dword ptr [RSP + stack_currentLine_xPos]
		.val(stack_currentLine_xPos)
		.add({ 0xF3,0x0F,0x10,0x94,0x24 })	// MOVSS XMM2, dword ptr [RSP + stack_currentLine_yPos]
		.val(stack_currentLine_yPos);
	// - Load the offset.
	//     R8D				<scratch for loading const>
	//     XMM3				float line_top_adjust
	func.add({ 0x41,0xB8 })					// MOV R8D, <line_top_adjust>
		.val(shadow_depth_pixels)
		.add({ 0x66,0x41,0x0F,0x6E,0xD8 });	// MOVD XMM3, R8D
	// - Adjust the X and Y positions.
	func.add({ 0xF3,0x0F,0x5C,0xCB })		// SUBSS XMM1, XMM3
		.add({ 0xF3,0x0F,0x58,0xD3 });		// ADDSS XMM2, XMM3
	////////////

	////////////
	// Set parameters and call draw_text for the main text.
	// Input:
	//     R13:8			void* read_head
	//     XMM1_Da:4		float xPos
	//     XMM2_Da:4		float yPos
	// draw_text parameters:
	//     RCX:8			Dynamic_Font* font
	//     XMM1_Da:4		float xPos			<-- already set
	//     XMM2_Da:4		float yPos			<-- already set
	//     R9D:4			ulong color (packed via argb_color())
	//     Stack[0x28]		char* strings
	//     Stack[0x30]		int32 flags			<-- always 0x20, no idea what these are
	////////////
	// - Set font.
	func.add({ 0x48,0x8B,0x0D })			// MOV RCX, qword ptr [RIP + <font>]
		.rel(global_subtitle_font);
	// - Set color.
	func.add({ 0x45,0x8B,0x8D })			// MOV R9D, dword ptr [R13 + line::address_color]
		.val(HudManager::HudTextLine::address_textColor);
	// - Set string pointer
	func.add({ 0x4D,0x89,0xE8 })			// MOV R8, R13
		.add({ 0x49,0x81,0xC0 })			// ADD R8, line::address_string
		.val(HudManager::HudTextLine::address_string)
		.add({ 0x4C,0x89,0x44,0x24,0x20 });	// MOV qword ptr [RSP + 0x20], R8
	// - Set flags
	func.add({ 0xC7, 0x44, 0x24, 0x28 })	// MOV dword ptr [RSP + 0x28], 0x20
		.val<uint32_t>(0x20);
	// - Call draw_text
	func.add({ 0xE8 }).rel(func_draw_text);	// CALL <draw_text>
	////////////
	
	////////////
	// Advance the read head to past the line info.
	////////////
	func.add({ 0x49,0x81,0xC5 })			// ADD R13, line::totalSize
		.val(HudManager::HudTextLine::totalDataSize);
	////////////

	////////////
	// Jump back in the line loop if we haven't reached the end.
	////////////
	func.add({ 0x45,0x85,0xE4 });			// TEST R12D,R12D
	func.add({ 0x0F,0x85 })					// JNE <loopStart>
		.val((lineLoopStart - (func.size() + 4)));
	////////////

	////////////
	// Jump back in the line loop if we haven't reached the end.
	////////////
	func.add({ 0x45,0x85,0xF6 });			// TEST R14D,R14D
	func.add({ 0x0F,0x85 })					// JNE <loopStart>
		.val((blockLoopStart - (func.size() + 4)));
	////////////

	////////////
	// Call im_flush().
	////////////
	func.add({ 0xE8 }).rel(func_im_flush);
	////////////

	////////////
	// Restore registers from the stack.
	////////////
	func.add({ 0x48,0x8B,0x9C,0x24 })		// MOV RBX, qword ptr [RSP + stack_cache_RBX]
		.val(stack_cache_RBX)
		.add({ 0x4C,0x8B,0xA4,0x24 })		// MOV R12, qword ptr [RSP + stack_cache_R12]
		.val(stack_cache_R12)
		.add({ 0x4C,0x8B,0xAC,0x24 })		// MOV R13, qword ptr [RSP + stack_cache_R13]
		.val(stack_cache_R13)
		.add({ 0x4C,0x8B,0xB4,0x24 })		// MOV R14, qword ptr [RSP + stack_cache_R14]
		.val(stack_cache_R14)
		.add({ 0x48,0x81,0xC4 })			// ADD RSP, 0xD8
		.add({ 0xD8,0x00,0x00,0x00 });
	////////////

	////////////
	// Return.
	////////////
	func.add({ 0xC3 });						// RET
	////////////

////// END REPLACEMENT FUNCTION

	// Write function to memory.
	func.printBytes();
	bool bSuccess = WriteProcessMemory(memory->_handle, reinterpret_cast<const LPVOID>(func_hud_draw_subtitles + memory->_baseAddress), func.get(), func.size(), NULL);
}

void HudManager::writePayload() const {
	Memory* memory = Memory::get();

	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	bool isSolving = (interactionState == Focusing || interactionState == Solving);

	// TEMP: static payload
	HudTextPayload temp_payload;

	// Show the solve mode status message.
	if (!solveStatusMessage.empty() && solveFadePercent > 0.f) {
		// Status messages go in the lower-left corner of the screen.
		HudTextBlock solveTextBlock;
		solveTextBlock.horizontalAlignment = 0.f;
		solveTextBlock.horizontalPosition = 0.f;
		solveTextBlock.verticalPosition = 0.f;

		// When the game fades in the border, it immediately shows it at a low transparency factor rather than fading it in.
		float solveTextAlpha = (solveFadePercent * 0.9f) + 0.1f;

		std::vector<std::string> expandedLines = separateLines(solveStatusMessage);
		for (const std::string& lineText : expandedLines) {
			HudTextLine lineData;
			lineData.textColor = RgbColor(1.f, 1.f, 1.f, 0.7f * solveTextAlpha);
			lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, 0.4f * solveTextAlpha);
			lineData.text = lineText;

			solveTextBlock.lines.push_back(lineData);
		}

		temp_payload.blocks.push_back(solveTextBlock);
	}

	// Show the walk mode status message.
	if (!walkStatusMessage.empty() && solveFadePercent < 1.f) {
		// Status messages go in the lower-left corner of the screen.
		HudTextBlock walkTextBlock;
		walkTextBlock.horizontalAlignment = 0.f;
		walkTextBlock.horizontalPosition = 0.f;
		walkTextBlock.verticalPosition = 0.f;

		float walkTextAlpha = 1.f - (isSolving ? std::clamp(2 * solveFadePercent, 0.f, 1.f) : solveFadePercent);

		std::vector<std::string> expandedLines = separateLines(walkStatusMessage);
		for (const std::string& lineText : expandedLines) {
			HudTextLine lineData;
			lineData.textColor = RgbColor(1.f, 1.f, 1.f, 0.7f * walkTextAlpha);
			lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, 0.4f * walkTextAlpha);
			lineData.text = lineText;

			walkTextBlock.lines.push_back(lineData);
		}

		temp_payload.blocks.push_back(walkTextBlock);
	}

	// Show notifications.
	float notificationBrightValue = 1.f - 0.4f * solveFadePercent;
	float notificationDimValue = 0.6f - 0.3f * solveFadePercent;
	if (!activeNotifications.empty()) {
		// Notifications go in the top-right corner of the screen.
		HudTextBlock notificationBlock;
		notificationBlock.horizontalAlignment = 1.f;
		notificationBlock.horizontalPosition = 1.f;
		notificationBlock.verticalPosition = 1.f;

		for (const Notification& notification : activeNotifications) {
			// Determine the alpha value of the notification based on its age.
			float alpha;
			if (notification.age < NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME) {
				alpha = notificationBrightValue;
			}
			else if (notification.age < NOTIFICATION_DIM_TIME + (NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME)) {
				float relativeDimTime = notification.age - (NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME);
				float dimPercent = relativeDimTime / NOTIFICATION_DIM_TIME;
				alpha = (1.f - dimPercent) * (notificationBrightValue - notificationDimValue) + notificationDimValue;
			}
			else if (notification.age < NOTIFICATION_HOLD_DIM_TIME + (NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME + NOTIFICATION_DIM_TIME)) {
				alpha = notificationDimValue;
			}
			else {
				float relativeFadeTime = notification.age - ((NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME + NOTIFICATION_DIM_TIME + NOTIFICATION_HOLD_DIM_TIME));
				float fadePercent = relativeFadeTime / NOTIFICATION_FADE_TIME;
				alpha = (1.f - fadePercent) * notificationDimValue;
			}

			// Determine the foreground color of the text.
			RgbColor textColor = notification.color;
			textColor.A = alpha;
			if (notification.age < NOTIFICATION_FLASH_TIME) {
				float flashPercent = 1.f - notification.age / NOTIFICATION_FLASH_TIME;
				textColor.R = (1.f - textColor.R) * flashPercent + textColor.R;
				textColor.G = (1.f - textColor.G) * flashPercent + textColor.G;
				textColor.B = (1.f - textColor.B) * flashPercent + textColor.B;
				textColor.A = (1.f - notificationBrightValue) * flashPercent + notificationBrightValue;
			}

			std::vector<std::string> expandedLines = separateLines(notification.text);
			for (const std::string& lineText : expandedLines) {
				HudTextLine lineData;
				lineData.textColor = textColor;
				lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, 0.6f * alpha);
				lineData.text = lineText;

				notificationBlock.lines.push_back(lineData);
			}
		}

		temp_payload.blocks.push_back(notificationBlock);
	}

	//// TEMP: hint
	//temp_block.horizontalAlignment = 0.5f;
	//temp_block.horizontalPosition = 0.5f;
	//temp_block.verticalPosition = 0.25f;
	//temp_block.lines.clear();

	//temp_line.textColor = RgbColor(1.f, 1.f, 1.f, 1.f);
	//temp_line.shadowColor = RgbColor(0.f, 0.f, 0.f, 0.6f);
	//temp_line.text = "Your Shapers may be found in blastron's world at";
	//temp_block.lines.push_back(temp_line);
	//temp_line.text = "Inside Deku Tree back room Skulltulla 2.";
	//temp_block.lines.push_back(temp_line);
	//temp_payload.blocks.push_back(temp_block);

	//temp_payload.blocks.push_back(temp_block);

	// HACK: The rendering code currently crashes if there are zero blocks present. Add a placeholder block.
	if (temp_payload.blocks.size() == 0) {
		HudTextBlock testBlock;
		testBlock.verticalPosition = 1;
		HudTextLine testLine;
		testLine.textColor = RgbColor(1.f, 1.f, 1.f, 0.1f);
		testLine.shadowColor = RgbColor(0.f, 0.f, 0.f, 0.1f);
		testLine.text = "we must always have a text block or we crash, oops";
		testBlock.lines.push_back(testLine);
		temp_payload.blocks.push_back(testBlock);
	}
	
	uint64_t writeAddress = address_hudTextPayload;

	// Write payload information.
	uint32_t blockCount = (uint32_t)temp_payload.blocks.size();
	WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextPayload::address_blockCount), &blockCount, sizeof(uint32_t), NULL);

	// Advance the write head past the payload information.
	writeAddress += HudTextPayload::totalDataSize;

	for (const HudTextBlock& block : temp_payload.blocks) {
		// Write block information.
		uint32_t lineCount = (uint32_t)block.lines.size();
		WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextBlock::address_lineCount), &lineCount, sizeof(uint32_t), NULL);
		WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextBlock::address_horizontalPosition), &block.horizontalPosition, sizeof(float), NULL);
		WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextBlock::address_horizontalAlignment), &block.horizontalAlignment, sizeof(float), NULL);
		WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextBlock::address_verticalPosition), &block.verticalPosition, sizeof(float), NULL);

		// Advance write pointer past the block information.
		writeAddress += HudTextBlock::totalDataSize;

		for (const HudTextLine& line : block.lines) {
			// Write line information.
			uint32_t argbTextColor = line.textColor.argb();
			WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextLine::address_textColor), &argbTextColor, sizeof(uint32_t), NULL);

			uint32_t arbgShadowColor = line.shadowColor.argb();
			WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextLine::address_shadowColor), &arbgShadowColor, sizeof(uint32_t), NULL);

			char stringBuff[MAX_STRING_LENGTH];
			strncpy_s(stringBuff, line.text.c_str(), MAX_STRING_LENGTH);
			stringBuff[MAX_STRING_LENGTH - 1] = 0; // forcibly null-terminate in case the string is too long
			WriteProcessMemory(memory->_handle, (LPVOID)(writeAddress + HudTextLine::address_string), stringBuff, sizeof(stringBuff), NULL);

			// Advance write pointer past the line information.
			writeAddress += HudTextLine::totalDataSize;
		}
	}
}


const uint32_t HudManager::HudTextPayload::address_blockCount =			0x00;
const uint32_t HudManager::HudTextPayload::totalDataSize =				0x08;

const uint32_t HudManager::HudTextBlock::address_lineCount =			0x00;
const uint32_t HudManager::HudTextBlock::address_verticalPosition =		0x08;
const uint32_t HudManager::HudTextBlock::address_horizontalPosition =	0x10;
const uint32_t HudManager::HudTextBlock::address_horizontalAlignment =	0x18;
const uint32_t HudManager::HudTextBlock::totalDataSize =				0x20;

const uint32_t HudManager::HudTextLine::maxLineSize =					0x80;

const uint32_t HudManager::HudTextLine::address_textColor =				0x00;
const uint32_t HudManager::HudTextLine::address_shadowColor =			0x08;
const uint32_t HudManager::HudTextLine::address_string =				0x10;
const uint32_t HudManager::HudTextLine::totalDataSize =					0x10 + HudManager::HudTextLine::maxLineSize;