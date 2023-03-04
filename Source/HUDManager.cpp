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

	// TODO: find a sane size for this
	uint64_t dataPayload = (uint64_t)VirtualAllocEx(memory->_handle, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	uint64_t address_tempLineCount = dataPayload;
	uint64_t address_tempHorizAlignment = dataPayload + 0x08;
	uint64_t address_tempVerticalAlignment = dataPayload + 0x10;
	uint64_t address_tempString = dataPayload + 0x18;

	const float margin_width = 0.f;//0.02f;
	const float line_spacing = 1.0f;

	uint32_t tempLineCount = 4;
	WriteProcessMemory(memory->_handle, (LPVOID)address_tempLineCount, &tempLineCount, sizeof(uint32_t), NULL);

	float tempHorizAlignment = 0.5f;
	WriteProcessMemory(memory->_handle, (LPVOID)address_tempHorizAlignment, &tempHorizAlignment, sizeof(float), NULL);

	float tempVerticalAlignment = 0.5f;
	WriteProcessMemory(memory->_handle, (LPVOID)address_tempVerticalAlignment, &tempVerticalAlignment, sizeof(float), NULL);

	char tempString1[] = "i am, at this moment...";
	char tempString2[] = "oh, what do the kids say these days?";
	char tempString3[] = "\"pogged out of my gourd\"";
	char tempString4[] = "over centering some text";

	uint32_t line_data_length = SUBTITLE_MAX_LINE_LENGTH;

	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + line_data_length * 0), tempString4, sizeof(tempString4), NULL);
	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + line_data_length * 1), tempString3, sizeof(tempString3), NULL);
	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + line_data_length * 2), tempString2, sizeof(tempString2), NULL);
	WriteProcessMemory(memory->_handle, (LPVOID)(address_tempString + line_data_length * 3), tempString1, sizeof(tempString1), NULL);

////// BEGIN REPLACEMENT FUNCTION

	AsmBuilder func(func_hud_draw_subtitles);

	const uint32_t stack_cache_RBX = 0xB8;
	const uint32_t stack_cache_R12 = 0xB0;
	const uint32_t stack_cache_R13 = 0xA8;

	const uint32_t stack_y_origin = 0xA0;
	const uint32_t stack_line_height = 0x9A;
	const uint32_t stack_margin_pixels = 0x90;
	const uint32_t stack_draw_area_x = 0x88;
	const uint32_t stack_draw_area_y = 0x80;

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
		.val(stack_cache_R13);
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
	// Load the line count into a stable register.
	// Output:
	//     R12D:4			int32 line_count
	////////////
	func.add({ 0x49,0xBC })					// MOVABS R12, <line_count>
		.abs(address_tempLineCount)
		.add({ 0x45,0x8B,0x24,0x24 });		// MOV R12D, dword ptr [R12]
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
	// Compute the Y origin of the entire block of text based on its line count.
	// Input:
	//     R12D:4			int32 line_count
	//     XMM0_Da:4		float draw_area_y
	////////////
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
	// - Subtract the block height from the drawable area of the screen to find the relative position of the block if it
	//   were to be placed at the top of the screen, then multiply it by the vertical alignment parameter in order to
	//   place it at the correct height. Our formula is: [(draw_area_y - block_height) * vertical_alignment]
	//     R8:8				float* vertical_alignment
	//     XMM1_Da:4		float vertical_alignment
	//     XMM0_Da:4		float relative_position
	func.add({ 0xF3,0x0F,0x5C,0xC3 })		// SUBSS XMM0, XMM3
		.add({ 0x49,0xB8 })					// MOVABS R8, <vertical alignment address>
		.abs(address_tempVerticalAlignment)
		.add({ 0x45,0x8B,0x00 })			// MOV R8D, dword ptr [R8]
		.add({ 0x66,0x41,0x0F,0x6E,0xC8 })	// MOVD XMM1, R8D
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
	// Start the line loop.
	// NOTE: R12D goes from being line_count to current_index here.
	////////////
	int loopStart = func.size();
	////////////

	////////////
	// Decrement the counter so that we're pointing at the next string.
	////////////
	func.add({ 0x41,0xFF,0xCC });			// DEC R12D
	////////////

	////////////
	// Compute the address of our current string.
	// Input:
	//     R12D:4			int32 current_index
	// Output:
	//     R13:8			char* line_address
	////////////
	// - Get the address of the start of our buffer.
	//     R13:8			char* root_address
	func.add({ 0x49, 0xBD })				// MOV R13, <root_address>
		.abs(address_tempString);
	// - Adjust the string pointer by an offset based on the current index.
	//     R11D:4			int32 line_data_length
	func.add({ 0x41,0xBB })					// MOV R11D, <line_data_length>
		.val(line_data_length)
		.add({ 0x45,0x0F,0xAF,0xDC })		// IMUL R11D, R12D			<-- multiply line length by loop count to get target address offset
		.add({ 0x4D,0x01,0xDD });			// ADD R13, R11				<-- add the address offset. note that it's safe to go from a 32->64 bit int here, since the upper half of a numbered register is zeroed
	////////////

	////////////
	// Determine string width by calling Dynamic_Font::get_string_width_in_pixels.
	// get_string_width_in_pixels parameters:
	//     RCX:8			Dynamic_Font* font
	//     RDX:8			char* string
	//     R8D:4			int32 flags		<-- always 0x20 as far as i can tell
	// Input:
	//     R13:8			char* line_address
	// Output:
	//     XMM0_Da:4		float string_width
	////////////
	// - Set font
	func.add({ 0x48,0x8B,0x0D })			// MOV RCX, qword ptr [RIP + <font>]
		.rel(global_subtitle_font);
	// - Set string pointer
	func.add({ 0x4C,0x89,0xEA });			// MOV RDX, R13 
	// - Set flags
	func.add({ 0x41,0xB8 })					// MOV R8D, 0x20
		.val<uint32_t>(0x20);
	// - Call get_string_width_in_pixels
	func.add({ 0xE8 })						// CALL <Dynamic_Font::get_string_width_in_pixels>
		.rel(func_get_string_width);
	////////////

	////////////
	// Determine the X origin of the string based on its width and the horizontal alignment parameter. The formula here is:
	//   [(draw_area_x - string_width) * horizontal_alignment]
	// Input:
	//     XMM0_Da:4		float string_width
	// Output:
	//     XMM1_Da:4		float xPos
	////////////
	// - Retrieve the drawable area, then subtract the string width from it in order to get the relative position of the string
	//   in the drawable area, were it to be aligned all the way to the right.
	//     XMM1_Da:4		float relative_position
	func.add({ 0xF3,0x0F,0x10,0x8C,0x24 })	// MOVSS XMM1, dword ptr [RSP + stack_draw_area_x]
		.val(stack_draw_area_x)
		.add({ 0xF3,0x0F,0x5C,0xC8 });		// SUBSS XMM1, XMM0
	// - Multiply the relative position by the horizontal alignment parameter.
	//     R8:8				float* horizontal_alignment
	//     XMM0_Da:4		float horizontal_alignment
	//     XMM1_Da:4		float relative_position
	func.add({ 0x49,0xB8 })					// MOVABS R8, <horizontal alignment address>
		.abs(address_tempHorizAlignment)
		.add({ 0x45,0x8B,0x00 })			// MOV R8D, dword ptr [R8]
		.add({ 0x66,0x41,0x0F,0x6E,0xC0 })	// MOVD XMM0, R8D
		.add({ 0xF3,0x0F,0x59,0xC8 });		// MULSS XMM1, XMM0
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
	//     XMM2_Da:4
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
	// Set parameters and call draw_text.
	// Input:
	//     R13:8			char* string
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
	func.add({ 0x41, 0xB9 })				// MOV R9D, 0xFFFFFFFF
		.add({ 0xFF, 0xFF, 0xFF, 0xFF });
	// - Set string pointer
	func.add({ 0x4C, 0x89, 0x6C, 0x24, 0x20 });	// MOV qword ptr [RSP + 0x20], R13
	// - Set flags
	func.add({ 0xC7, 0x44, 0x24, 0x28 })	// MOV dword ptr [RSP + 0x28], 0x20
		.val<uint32_t>(0x20);
	// - Call draw_text
	func.add({ 0xE8 }).rel(func_draw_text);	// CALL <draw_text>
	////////////

	////////////
	// Jump back in the loop if we haven't reached the end.
	////////////
	func.add({ 0x45,0x85,0xE4 });			// TEST R12D,R12D
	int loopEnd = func.size();
	func.add({ 0x0F,0x85 })					// JNE <loopStart>
		.val((loopStart - (loopEnd + 6)));
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
		.add({ 0x48,0x81,0xC4 })			// ADD RSP,0xD8
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