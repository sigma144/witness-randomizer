#include "HUDManager.h"

#include "ASMBuilder.h"
#include "Input.h"
#include "Memory.h"
#include <cassert>


#define STRING_DATA_SIZE 0x80

#define NOTIFICATION_FADEIN_TIME .2f
#define NOTIFICATION_FLASH_TIME 1.f
#define NOTIFICATION_HOLD_BRIGHT_TIME 8.f
#define NOTIFICATION_DIM_TIME 5.f
#define NOTIFICATION_HOLD_DIM_TIME 10.f
#define NOTIFICATION_FADEOUT_TIME 10.f
#define NOTIFICATION_TOTAL_TIME (NOTIFICATION_FADEIN_TIME + NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME + NOTIFICATION_DIM_TIME + NOTIFICATION_HOLD_DIM_TIME + NOTIFICATION_FADEOUT_TIME)

#define TIME_BETWEEN_NOTIFICATIONS 1.5f
#define NOTIFICATION_SCROLL_TIME 0.5f

// Verifies that sigscans are working by comparing them against hardcoded values in the Steam build.
#define DEBUG_SIGSCAN 0


HudManager::HudManager() {
	overwriteSubtitleFunction();
}

void HudManager::update(float deltaSeconds) {
	InputWatchdog* input = InputWatchdog::get();
	InteractionState interactionState = input->getInteractionState();

	updateInformationalMessages(deltaSeconds);
	if (interactionState != InteractionState::Menu) {
		updateBannerMessages(deltaSeconds);
		updateNotifications(deltaSeconds);
	}

	bool isSolving = (interactionState == Focusing || interactionState == Solving);
	if (isSolving && solveTweenFactor < 1.f) {
		solveTweenFactor = std::min(solveTweenFactor + deltaSeconds * 0.8f, 1.f);
		hudTextDirty = true;
	}
	else if (!isSolving && solveTweenFactor > 0.f) {
		solveTweenFactor = std::max(solveTweenFactor - deltaSeconds * 3.f, 0.f);
		hudTextDirty = true;
	}

	if (hudTextDirty) {
		updatePayloads();
	}
}

void HudManager::queueBannerMessage(const std::string& text, RgbColor color, float duration) {
	queuedBannerMessages.push(BannerMessage(text, color, duration));
}

void HudManager::queueNotification(const std::string& text, RgbColor color) {
	queuedNotifications.push(Notification(text, color));
}

void HudManager::showInformationalMessage(InfoMessageCategory category, const std::string& text) {
	if (informationalMessages[static_cast<int>(category)] == text) {
		return;
	}

	informationalMessages[static_cast<int>(category)] = text;

	// Determine which message to actually show, based on priority.
	shouldShowInformationalMessage = false;
	for (const std::string& message : informationalMessages) {
		if (!message.empty()) {
			shouldShowInformationalMessage = true;
			if (currentInformationalMessage != message) {
				currentInformationalMessage = message;
				hudTextDirty = true;
			}

			return;
		}
	}
}

void HudManager::clearInformationalMessage(InfoMessageCategory category) {
	showInformationalMessage(category, "");
}

void HudManager::setWalkStatusMessage(const std::string& text) {
	if (walkStatusMessage != text) {
		walkStatusMessage = text;
		hudTextDirty = true;
	}
}

void HudManager::setSolveStatusMessage(const std::string& text) {
	if (solveStatusMessage != text) {
		solveStatusMessage = text;
		hudTextDirty = true;
	}
}

void HudManager::setDebugText(const std::string& text) {
	if (debugText != text) {
		debugText = text;
		hudTextDirty = true;
	}
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
		float multiplier = 0.7f + 0.3f * std::max(seenNotifications - 4, 1);

		iterator->age += deltaSeconds * multiplier;
		if (iterator->age >= NOTIFICATION_TOTAL_TIME) {
			iterator = activeNotifications.erase(iterator);
			continue;
		}

		// Determine whether or not the notification is in a time block where it is fading in or out and, if so, mark it as dirty.
		float relativeAge = iterator->age;
		if (relativeAge < NOTIFICATION_FADEIN_TIME + NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME) {
			hudTextDirty = true;
		}

		relativeAge -= (NOTIFICATION_FADEIN_TIME + NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME);
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

			timeToNextNotification = TIME_BETWEEN_NOTIFICATIONS;
			hudTextDirty = true;
		}
	}
}

void HudManager::updateInformationalMessages(float deltaSeconds) {
	if (shouldShowInformationalMessage && informationalMessageFade < 1.f) {
		informationalMessageFade = std::min(1.f, informationalMessageFade + deltaSeconds * 3.f);
		hudTextDirty = true;
	}
	else if (!shouldShowInformationalMessage && informationalMessageFade > 0.f) {
		informationalMessageFade = std::max(0.f, informationalMessageFade - deltaSeconds * 3.f);
		hudTextDirty = true;
	}
}

std::vector<std::string> HudManager::separateLines(std::string input, int maxLength) {
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
		if (line.length() > maxLength) {
			// The line is longer than our max width and needs to be wrapped.
			std::string choppedLine = line;
			while (choppedLine.length() > maxLength) {
				// Find the closest space character to the end of the line.
				searchIndex = choppedLine.rfind(' ', maxLength);
				if (searchIndex != std::string::npos) {
					// We found a space. Separate the string at that character and keep working.
					wrappedLines.push_back(choppedLine.substr(0, searchIndex));
					choppedLine = choppedLine.substr(searchIndex + 1);
				}
				else {
					// There was no space in the previous line. Simply split the string mid-word.
					wrappedLines.push_back(choppedLine.substr(0, maxLength));
					choppedLine = choppedLine.substr(maxLength);
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

uint32_t HudManager::findSubtitleFunction() {
	Memory* memory = Memory::get();
	uint64_t processBaseAddress = memory->getBaseAddress();

	// Find a unique spot in hud_get_subtitles.
	uint64_t nearSubtitleOffset = memory->executeSigScan({
		0x48, 0x89, 0xB4, 0x24,			// MOV qword ptr [RSP + 0xC8], RSI
		0xC8, 0x00, 0x00, 0x00,
		0x48, 0x8D, 0x4B, 0x08,			// LEA RCX, [RBX + 0x8]
		0x4C, 0x8D, 0x84, 0x24,			// LEA R8, [RSP + 0xE8]
		0xE8, 0x00, 0x00, 0x00
	}, processBaseAddress);

	if (nearSubtitleOffset == UINT64_MAX) {
		throw std::exception("Unable to find marker within hud_draw_subtitles.");
		return UINT32_MAX;
	}

	// Work backwards to find the function.
	uint64_t nearSubtitleAddress = processBaseAddress + nearSubtitleOffset;
	uint64_t functionStartOffset = memory->executeSigScan({
		0xCC, 0xCC, 0xCC, 0xCC,			// padding
		0x48, 0x81, 0xEC,				// SUB RSP, 0xD8
		0xD8, 0x00, 0x00, 0x00
	}, nearSubtitleAddress - 0x80);

	if (functionStartOffset == UINT64_MAX || functionStartOffset > 0x80) {
		throw std::exception("Unable to find start of hud_draw_subtitles.");
		return UINT32_MAX;
	}

	uint64_t functionStartAddress = nearSubtitleAddress - 0x80 + functionStartOffset + 0x4;
	uint32_t relativeAddress = (uint32_t)(functionStartAddress - processBaseAddress);

#if DEBUG_SIGSCAN
	if (relativeAddress != 0x1E99A0) {
		throw std::exception("Found address for hud_draw_subtitles does not match expected value.");
		return UINT32_MAX;
	}
#endif

	return relativeAddress;
}

void HudManager::overwriteSubtitleFunction() {
	Memory* memory = Memory::get();
	uint64_t processBaseAddress = memory->getBaseAddress();

	// Find the base subtitle function.
	uint32_t func_hud_draw_subtitles = findSubtitleFunction();
	if (func_hud_draw_subtitles == UINT32_MAX) return;

	// Scan forward from there to find remaining offsets.
	uint32_t func_rendering_2D_right_handed = memory->scanForRelativeAddress({
		0x44, 0x0F, 0x29, 0x44,		// MOVAPS xmmword ptr [rsp+0x70], xmm8
		0x24, 0x70,
		0x33, 0xC9,					// XOR ECX, ECX
		0xE8 /*...*/				// CALL ...
	}, func_hud_draw_subtitles) + 0x4;

	uint32_t globals_shader_text = memory->scanForRelativeAddress({
		0x44, 0x0F, 0x29, 0x44,		// MOVAPS xmmword ptr [rsp+0x70], xmm8
		0x24, 0x70,
		0x33, 0xC9,					// XOR ECX, ECX
		0xE8 /*...*/				// CALL ...
	}, func_hud_draw_subtitles, 7) + 0x4;

	uint32_t func_apply_2D_shader = memory->scanForRelativeAddress({
		0x44, 0x0F, 0x29, 0x44,		// MOVAPS xmmword ptr [rsp+0x70], xmm8
		0x24, 0x70,
		0x33, 0xC9,					// XOR ECX, ECX
		0xE8 /*...*/				// CALL ...
	}, func_hud_draw_subtitles, 12) + 0x4;

	uint32_t func_get_string_width = memory->scanForRelativeAddress({
		0x49, 0x8B, 0xCF,			// MOV RCX, R15
		0x49, 0x8B, 0x14, 0x06,		// MOV RDX, qword ptr [R14 + RAX * 0x1]
		0xE8						// CALL ...
	}, func_hud_draw_subtitles) + 0x4;

	uint32_t screen_target_width = memory->scanForRelativeAddress({
		0x8B, 0x07,					// MOV EAX, dword ptr [RDI]
		0x66, 0x0F, 0x6E, 0x05		// MOVD XMM0, dword ptr [...]
	}, func_hud_draw_subtitles);
	uint32_t screen_target_height = screen_target_width + 0x4;

	uint32_t global_subtitle_font = memory->scanForRelativeAddress({
		0x48, 0x8D, 0x4C, 0x24, 0x40, // LEA RCX, [RSP+0x40]
		0x44, 0x8B, 0xE0,			// MOV R12D, EAX
		0xE8						// CALL ...
	}, func_hud_draw_subtitles, 0x7) + 0x4 + 0x10;

	uint32_t func_im_begin = memory->scanForRelativeAddress({
		0xF3, 0x41, 0x0F, 0x59, 0xF8, // MULSS XMM7, XMM8
		0xF3, 0x0F, 0x58, 0xF8,		// ADDSS XMM7, XMM0
		0xE8						// CALL ...
	}, func_hud_draw_subtitles) + 0x4;

	uint32_t func_im_flush = memory->scanForRelativeAddress({
		0x44, 0x0F, 0x28, 0x54,		// MOVAPS XMM10, xmmword ptr [rsp+0x50]
		0x24, 0x50,
		0x44, 0x0F, 0x28, 0x4C,		// MOVAPS XMM9, xmmword ptr [rsp+0x60]
		0x24, 0x60,
		0xE8 /*...*/				// CALL ...
	}, func_hud_draw_subtitles) + 0x4;

	uint32_t func_draw_text = memory->scanForRelativeAddress({
		0xF3, 0x41, 0x0F, 0x59, 0xF2, // MULSS XMM6, XMM10
		0x0F, 0x28, 0xCE,			// MOVAPS XMM1, XMM6
		0xE8 /*...*/				// CALL ...
	}, func_hud_draw_subtitles) + 0x4;

#if DEBUG_SIGSCAN
	assert(func_get_string_width == 0x3430A0);
	assert(func_rendering_2D_right_handed == 0x25a330);
	assert(func_apply_2D_shader == 0x2581C0);
	assert(func_im_begin == 0x33f5b0);
	assert(func_im_flush == 0x33f5e0);
	assert(func_draw_text == 0x259380);

	assert(globals_shader_text == 0x62D400);
	assert(global_subtitle_font == 0x62D458);
	assert(screen_target_width == 0x469A5F0);
	assert(screen_target_height == 0x469A5F4);
#endif

	// Reserve memory for the payloads. (See updatePayloads() for more information.)
	address_readPayloadIndex = (uint64_t)VirtualAllocEx(memory->_handle, NULL, 0x1008, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	address_writePayloadIndex = address_readPayloadIndex + 0x4;
	address_hudTextPayload_1 = address_writePayloadIndex + 0x4;
	address_hudTextPayload_2 = address_hudTextPayload_1 + 0x800;

	// Initialize the payloads.
	writePayload(HudTextPayload(), address_hudTextPayload_1);
	writePayload(HudTextPayload(), address_hudTextPayload_2);

	const float margin_width = 0.02f;
	const float line_spacing = 1.0f;
	const float line_top_adjust = 0.25f;
	const float shadow_depth_pixels = 2.f;

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
	// Determine which payload to read from and mark that we are doing so. (See updatePayloads() for more information.)
	// Output:
	//     R13				void* read_head
	////////////
	// - Retrieve the counter for the last written payload, then mark that we're now reading to it.
	//     R12D				int32 writePayloadIndex
	//     R13				int32* address_readPayloadIndex
	func.add({ 0x49,0xBC })					// MOVABS R12, <address_writePayloadIndex>
		.val(address_writePayloadIndex)
		.add({ 0x45,0x8B,0x24,0x24 })		// MOV R12D, dword ptr [R12]
		.add({ 0x49,0xBD })					// MOVABS R13, <address_readPayloadIndex>
		.val(address_readPayloadIndex)
		.add({ 0x45,0x89,0x65,0x00 });		// MOV dword ptr [R13], R12D
	// - Pick which payload to use.
	//     R13				void* read_head
	func.add({ 0x41,0x83,0xFC,0x00 })		// CMP R12D, 0x0
		.add({ 0x0F,0x85 })					// JNE <usePayload2>
		.jump("usePayload2");
		// Load payload 1.
	func.add({ 0x49,0xBD })					// MOVABS R13, <address_hudTextPayload_1>
		.val(address_hudTextPayload_1)
		.add({ 0xE9 })						// JMP <payloadSelected>
		.jump("payloadSelected");
		// Load payload 2.
	func.mark("usePayload2");
	func.add({ 0x49,0xBD })					// MOVABS R13, <address_hudTextPayload_1>
		.val(address_hudTextPayload_2);
		// Done.
	func.mark("payloadSelected");
	////////////

	////////////
	// Load data about the overall set of blocks into stable registers.
	// Input:
	//     R13				void* read_head
	// Output:
	//     R14D:4			int32 block_count
	////////////
	// - Load the number of blocks into R14.
	func.add({ 0x45,0x8B,0xB5 })			// MOV R14D, dword ptr [R13 + payload::address_blockCount]
		.val(HudManager::HudTextPayload::address_blockCount);
	// - Move the read head forward by the total payload size.
	func.add({ 0x49,0x81,0xC5 })			// ADD R13, payload::totalDataSize
		.val(HudManager::HudTextPayload::totalDataSize);
	////////////

	////////////
	// Early-out if we have no blocks.
	// Input:
	//     R14D:4			int32 block_count
	////////////
	func.add({ 0x45,0x85,0xF6 })			// TEST R14D,R14D
		.add({ 0x0F,0x84 })					// JE <noBlocks>
		.jump("noBlocks");
	////////////

	////////////
	// Start the block loop.
	////////////
	func.mark("blockLoop");
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
	// Verify that this block has lines and, if not, move the read head forward and skip the 
	// Input:
	//     R13				void* read_head
	//     R12D:4			in32_line_count
	////////////
	// - Test for lines and, if found, skip forward.
	func.add({ 0x45,0x85,0xE4 })			// TEST R12D,R12D
		.add({ 0x0F,0x85 })					// JNE <hasLines>
		.jump("hasLines");
	// - We found no lines. Advance the read head, then skip past the block.
	func.add({ 0x49,0x81,0xC5 })			// ADD R13, block::totalDataSize
		.val(HudManager::HudTextBlock::totalDataSize)
		.add({ 0xE9 })						// JMP <noLines>
		.jump("noLines");
	// - End of the early-out logic.
	func.mark("hasLines");
	
	////////////
	// Save off the horizontal position and alignment parameters for the block.
	// Input:
	//     R13				void* read_head
	////////////
	// - Save the horizontal alignment.
	//     R8D:4			<buffer for loading block data>
	func.add({ 0x45,0x8B,0x85 })			// MOV R8D, dword ptr [r13 + block::address_horizontalAlignment]
		.val(HudManager::HudTextBlock::address_horizontalAlignment)
		.add({ 0x44,0x89,0x84,0x24 })		// MOV dword ptr [rsp + stack_currentBlock_horizAlignment], R8D
		.val(stack_currentBlock_horizAlignment);
	// - Save the horizontal position.
	//     R8D:4			<buffer for loading block data>
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
	//     XMM0_Da:4		float draw_area_y
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
	//     R8D:4			<buffer for loading constant>
	//     XMM2_Da:4		float line_height
	func.add({ 0x41,0xB8 })					// MOV R8D, <line_spacing>
		.val(line_spacing)
		.add({ 0x66,0x41,0x0F,0x6E,0xD0 })	// MOVD XMM2, R8D
		.add({ 0xF3,0x0F,0x59,0xD1 });		// MULSS XMM2, XMM1
	// - Save off the line height for future reference.
	func.add({ 0xF3,0x0F,0x11,0x94,0x24 })	// MOVSS dword ptr [RSP + stack_line_height], XMM2
		.val(stack_line_height);
	// - Compute the total number of lines in the block minus one and convert it to a float. We're specifically subtracting
	//   one here because we will be multiplying this value by the spaced line height in order to compute the size of the
	//   block, and we don't want the last line to be padded.
	// 
	//   Our formula will eventually be [(font_height + spacing) * (rows - 1) + font_height], which evaluates to
	//   [rows * font_height + (rows - 1) * spacing].
	//     R8D:4			int32 line_count - 1
	//     XMM3_Da:4		float spaced_lines
	func.add({ 0x45,0x89,0xE0 })			// MOV R8D, R12D
		.add({ 0x41,0x83,0xE8,0x01 })		// SUB R8D, 0x1
		.add({ 0xF3,0x41,0x0F,0x2A,0xD8 });	// CVTSI2SS XMM3, R8D
	// - Add additional lines to the count based on the block's padding parameters.
	//     XMM3_Da:4		float spaced_lines
	//     XMM4_Da:4		float address_linePaddingBottom
	func.add({ 0x66,0x41,0x0F,0x6E,0xA5 })	// MOVD XMM4, [R13 + block::address_linePaddingTop]
		.val(HudManager::HudTextBlock::address_linePaddingTop)
		.add({ 0xF3,0x0F,0x58,0xDC })		// ADDSS XMM3, XMM4
		.add({ 0x66,0x41,0x0F,0x6E,0xA5 })	// MOVD XMM4, [R13 + block::address_linePaddingBottom]
		.val(HudManager::HudTextBlock::address_linePaddingBottom)
		.add({ 0xF3,0x0F,0x58,0xDC });		// ADDSS XMM3, XMM4
	// - Multiply the bottom line padding by the line spacing parameter to get the offset for the first actual line.
	//     XMM4_Da:4		float padding_offset
	func.add({ 0xF3,0x0F,0x59,0xE2 });		// MULSS XMM4, XMM2
	// - Multiply the number of spaced lines by the line spacing.
	//     XMM3_Da:4		float block_height
	func.add({ 0xF3,0x0F,0x59,0xDA });		// MULSS XMM3, XMM2
	// - Add the height of an un-spaced line to get the total height of the block.
	//     XMM3_Da:4		float block_height
	func.add({ 0xF3,0x0F,0x58,0xD9 });		// ADDSS XMM3, XMM1
	// - Next, remove a fraction of a line's worth of text from the block in order to compensate for the fact that the
	//   font's height already has spacing built-in.
	//     R8D:4			<buffer for loading constant>
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
	// - Add the vertical padding to the relative position.
	//     XMM0_Da:4		float relative_position
	func.add({ 0xF3,0x0F,0x5C,0xC4 });		// SUBSS XMM0, XMM4
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
	func.mark("lineLoop");
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
	func.add({ 0x45,0x85,0xE4 })			// TEST R12D,R12D
		.add({ 0x0F,0x85 })					// JNE <loopStart>
		.jump("lineLoop");
	////////////

	////////////
	// Landing point for the early-out when a block has no lines.
	////////////
	func.mark("noLines");
	////////////

	////////////
	// Jump back in the block loop if we haven't reached the end.
	////////////
	func.add({ 0x45,0x85,0xF6 })			// TEST R14D,R14D
		.add({ 0x0F,0x85 })					// JNE <loopStart>
		.jump("blockLoop");
	////////////

	////////////
	// Landing point for the early-out when we have no blocks.
	////////////
	func.mark("noBlocks");
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
	//func.printBytes();
	memory->WriteRelative(reinterpret_cast<const LPVOID>((uint64_t)func_hud_draw_subtitles), func.get(), func.size());
}

void HudManager::updatePayloads() {
	Memory* memory = Memory::get();

	// Since the game and the client run on different threads, there is no guarantee that the game will not be
	//   attempting to read payload data at the same time that the client is attempting to write it, which may cause a
	//   crash. In order to guard against that, we use two different payloads so that there will always be one that is
	//   safe to write to and one safe to read from.
	// To coordinate between the two threads, we have two flags: one that indicates the payload that has been most
	//   recently read from, and one that indicates the one that has most recently been written to. At the start of
	//   every read/write operation, these flags are checked to see what is safe to do, if anything.
	// When reading, the game checks to see which payload has been written to most recently, then selects that one to
	//   read from. Before actually reading the payload, it immediately indicates that it has switched to that payload,
	//   meaning both that it is no longer safe to write to that payload and also that it is now safe(-ish) to write to
	//   the other one.
	// When writing, the client checks to see which payload the game is reading from in order to determine which
	//   payload is explicitly unsafe to write to. It then checks to see if it has already written data to the other
	//   one and, if it has not, does so, then marks that it has finished writing. This write-once strategy is required
	//   because as soon as the client indicates that the payload has been written to, the game may start reading from
	//   it at any point, meaning that it is no longer safe to assume that the payload can be safely written to.

	int32_t readPayloadIndex, writePayloadIndex;
	memory->ReadAbsolute((LPVOID)(address_readPayloadIndex), &readPayloadIndex, sizeof(int32_t));
	memory->ReadAbsolute((LPVOID)(address_writePayloadIndex), &writePayloadIndex, sizeof(int32_t));

	if (readPayloadIndex == writePayloadIndex) {
		// The game has most recently read from the payload we have most recently written to and will continue to do so
		//   until informed otherwise, so it is now safe to write to the other one.
		writePayloadIndex = writePayloadIndex == 0 ? 1 : 0;

		// Build and write the payload to the given address.
		HudTextPayload payload = buildPayload();
		writePayload(payload, writePayloadIndex == 0 ? address_hudTextPayload_1 : address_hudTextPayload_2);

		// Finally, now that we are finished writing the payload, mark that it is ready for reading.
		memory->WriteAbsolute((LPVOID)(address_writePayloadIndex), &writePayloadIndex, sizeof(int32_t));
		hudTextDirty = false;
	}
}

HudManager::HudTextPayload HudManager::buildPayload() const {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	bool isSolving = (interactionState == Focusing || interactionState == Solving);
	float solveFadePercent = solveTweenFactor;

	HudTextPayload payload;

	// Show the solve mode status message.
	if (!solveStatusMessage.empty() && solveFadePercent > 0.f) {
		// Status messages go in the lower-left corner of the screen.
		HudTextBlock solveTextBlock;
		solveTextBlock.horizontalAlignment = 0.f;
		solveTextBlock.horizontalPosition = 0.f;
		solveTextBlock.verticalPosition = 0.f;

		// When the game fades in the border, it immediately shows it at a low transparency factor rather than fading
		//   it in from nothing.
		float solveTextAlpha = (solveFadePercent * 0.95f) + 0.05f;

		std::vector<std::string> expandedLines = separateLines(solveStatusMessage);
		for (const std::string& lineText : expandedLines) {
			HudTextLine lineData;
			lineData.textColor = RgbColor(1.f, 1.f, 1.f, 0.7f * solveTextAlpha);
			lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, shadowAlpha(solveTextAlpha));
			lineData.text = lineText;

			solveTextBlock.lines.push_back(lineData);
		}

		payload.blocks.push_back(solveTextBlock);
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
			lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, shadowAlpha(walkTextAlpha));
			lineData.text = lineText;

			walkTextBlock.lines.push_back(lineData);
		}

		payload.blocks.push_back(walkTextBlock);
	}

	// Show notifications. Notifications go in the top-right corner of the screen.
	if (!activeNotifications.empty()) {
		float brightAlpha = 0.8f - 0.15f * solveFadePercent;
		float dimAlpha = 0.5f * (1 - solveFadePercent);

		// If we have more than one notification, separate the first notification from the rest so that we can smoothly
		//   scroll the blocks down as new ones come in.
		const Notification& firstNotification = activeNotifications[0];
		std::vector<std::string> firstNotificationLines = separateLines(firstNotification.text, 60);

		// Draw the rest of the notifications first so that they will be drawn under the first notification.
		if (activeNotifications.size() > 1) {
			HudTextBlock otherNotificationBlock;
			otherNotificationBlock.horizontalAlignment = 1.f;
			otherNotificationBlock.horizontalPosition = 1.f;
			otherNotificationBlock.verticalPosition = 1.f;

			// Offset the block by the height of the first notification, scaled based on how recently that notification
			//   faded in, so as to provide a nice scroll effect as notifications arrive.
			float scrollPercent = std::min(1.f, firstNotification.age / NOTIFICATION_SCROLL_TIME);
			otherNotificationBlock.linePaddingTop = firstNotificationLines.size() * easeInOut(scrollPercent);

			for (int i = 1; i < activeNotifications.size(); i++) {
				const Notification& currentNotification = activeNotifications[i];
				float notificationAlpha = getNotificationAlpha(currentNotification, brightAlpha, dimAlpha);

				std::vector<std::string> expandedLines = separateLines(currentNotification.text, 60);
				for (const std::string& lineText : expandedLines) {
					HudTextLine lineData;
					lineData.textColor = RgbColor(1.f, 1.f, 1.f, notificationAlpha);
					lineData.shadowColor = getNotificationShadowColor(currentNotification, notificationAlpha);
					lineData.text = lineText;

					otherNotificationBlock.lines.push_back(lineData);
				}
			}

			payload.blocks.push_back(otherNotificationBlock);
		}

		// Draw the first notification in its own block.
		HudTextBlock firstNotificationBlock;
		firstNotificationBlock.horizontalAlignment = 1.f;
		firstNotificationBlock.horizontalPosition = 1.f;
		firstNotificationBlock.verticalPosition = 1.f;

		float notificationAlpha = getNotificationAlpha(firstNotification, brightAlpha, dimAlpha);
		RgbColor notificationColor = RgbColor(1.f, 1.f, 1.f, notificationAlpha);
		RgbColor shadowColor = getNotificationShadowColor(firstNotification, notificationAlpha);
		firstNotificationLines = separateLines(firstNotification.text, 60);
		for (const std::string& lineText : firstNotificationLines) {
			HudTextLine lineData;
			lineData.textColor = notificationColor;
			lineData.shadowColor = shadowColor;
			lineData.text = lineText;

			firstNotificationBlock.lines.push_back(lineData);
		}

		payload.blocks.push_back(firstNotificationBlock);
	}

	// Show informational message in the center of the screen.
	if (!currentInformationalMessage.empty() && informationalMessageFade > 0.f) {
		HudTextBlock informationalMessageBlock;
		informationalMessageBlock.horizontalAlignment = 0.5f;
		informationalMessageBlock.horizontalPosition = 0.5f;
		informationalMessageBlock.verticalPosition = 0.15f;

		float curvedFade = easeOut(informationalMessageFade);

		std::vector<std::string> expandedLines = separateLines(currentInformationalMessage);
		for (const std::string& lineText : expandedLines) {
			HudTextLine lineData;
			lineData.textColor = RgbColor(1.f, 1.f, 1.f, 1.f * curvedFade);
			lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, shadowAlpha(curvedFade));
			lineData.text = lineText;

			informationalMessageBlock.lines.push_back(lineData);
		}

		payload.blocks.push_back(informationalMessageBlock);
	}

	// Show debug text in the upper-left corner of the screen.
	if (!debugText.empty()) {
		HudTextBlock debugMessageBlock;
		debugMessageBlock.verticalPosition = 1.f;

		std::vector<std::string> expandedLines = separateLines(debugText);
		for (const std::string& lineText : expandedLines) {
			HudTextLine lineData;
			lineData.textColor = RgbColor(1.f, 1.f, 1.f, 1.f);
			lineData.shadowColor = RgbColor(0.f, 0.f, 0.f, shadowAlpha(1.f));
			lineData.text = lineText;

			debugMessageBlock.lines.push_back(lineData);
		}

		payload.blocks.push_back(debugMessageBlock);
	}

	return payload;
}

void HudManager::writePayload(const HudTextPayload& payload, uint64_t writeAddress) const {
	Memory* memory = Memory::get();

	// Write payload information.
	uint32_t blockCount = (uint32_t)payload.blocks.size();
	memory->WriteAbsolute((LPVOID)(writeAddress + HudTextPayload::address_blockCount), &blockCount, sizeof(uint32_t));

	// Advance the write head past the payload information.
	writeAddress += HudTextPayload::totalDataSize;

	for (const HudTextBlock& block : payload.blocks) {
		// Write block information.
		uint32_t lineCount = (uint32_t)block.lines.size();
		memory->WriteAbsolute((LPVOID)(writeAddress + HudTextBlock::address_lineCount), &lineCount, sizeof(uint32_t));
		memory->WriteAbsolute((LPVOID)(writeAddress + HudTextBlock::address_verticalPosition), &block.verticalPosition, sizeof(float));
		memory->WriteAbsolute((LPVOID)(writeAddress + HudTextBlock::address_linePaddingTop), &block.linePaddingTop, sizeof(float));
		memory->WriteAbsolute((LPVOID)(writeAddress + HudTextBlock::address_linePaddingBottom), &block.linePaddingBottom, sizeof(float));
		memory->WriteAbsolute((LPVOID)(writeAddress + HudTextBlock::address_horizontalPosition), &block.horizontalPosition, sizeof(float));
		memory->WriteAbsolute((LPVOID)(writeAddress + HudTextBlock::address_horizontalAlignment), &block.horizontalAlignment, sizeof(float));

		// Advance write pointer past the block information.
		writeAddress += HudTextBlock::totalDataSize;

		for (const HudTextLine& line : block.lines) {
			// Write line information.
			uint32_t argbTextColor = line.textColor.argb();
			memory->WriteAbsolute((LPVOID)(writeAddress + HudTextLine::address_textColor), &argbTextColor, sizeof(uint32_t));

			uint32_t arbgShadowColor = line.shadowColor.argb();
			memory->WriteAbsolute((LPVOID)(writeAddress + HudTextLine::address_shadowColor), &arbgShadowColor, sizeof(uint32_t));

			char stringBuff[STRING_DATA_SIZE];
			strncpy_s(stringBuff, line.text.c_str(), _TRUNCATE);
			memory->WriteAbsolute((LPVOID)(writeAddress + HudTextLine::address_string), stringBuff, sizeof(stringBuff));

			// Advance write pointer past the line information.
			writeAddress += HudTextLine::totalDataSize;
		}
	}
}

float HudManager::getNotificationAlpha(const Notification& notification, float bright, float dim) const {
	float targetAge = NOTIFICATION_FADEIN_TIME;
	if (notification.age < targetAge) {
		// The notification is brand-new and fading in to 100% alpha.
		return easeIn(notification.age / NOTIFICATION_FADEIN_TIME);
	}

	float previousTargetAge = targetAge;
	targetAge += NOTIFICATION_FLASH_TIME;
	if (notification.age < targetAge) {
		// The notification has flashed in and is dimming to its normal color.
		float flashPercentage = 1.f - (notification.age - previousTargetAge) / NOTIFICATION_FLASH_TIME;
		return easeInOut(flashPercentage) * (1.f - bright) + bright;
	}

	previousTargetAge = targetAge;
	targetAge += NOTIFICATION_HOLD_BRIGHT_TIME;
	if (notification.age < targetAge) {
		// The notification is holding at its bright value.
		return bright;
	}

	previousTargetAge = targetAge;
	targetAge += NOTIFICATION_DIM_TIME;
	if (notification.age < targetAge) {
		// The notification is dimming from the bright to dim values.
		float dimPercentage = 1 - (notification.age - previousTargetAge) / NOTIFICATION_DIM_TIME;
		return easeInOut(dimPercentage) * (bright - dim) + dim;
	}

	previousTargetAge = targetAge;
	targetAge += NOTIFICATION_HOLD_DIM_TIME;
	if (notification.age < targetAge) {
		// The notification is holding at the dim value.
		return dim;
	}

	previousTargetAge = targetAge;
	targetAge += NOTIFICATION_FADEOUT_TIME;
	if (notification.age < targetAge) {
		// The notification is fading out from dim to invisible.
		float fadePercentage = 1 - (notification.age - previousTargetAge) / NOTIFICATION_FADEOUT_TIME;
		return easeOut(fadePercentage) * dim;
	}

	return 0.f;
}

RgbColor HudManager::getNotificationShadowColor(const Notification& notification, float alpha) const {
	if (notification.age < NOTIFICATION_FADEIN_TIME) {
		// The notification is brand-new and is flashing in.
		RgbColor color = RgbColor::lerpHSV(RgbColor(0.f, 0.f, 0.f), notification.flashColor, notification.age / NOTIFICATION_FADEIN_TIME);
		color.A = alpha;

		return color;
	}
	else if (notification.age < (NOTIFICATION_FADEIN_TIME + NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME)) {
		// The notification is relatively recent and its shadow is fading out to normal color and dimness.
		float flashPercent = 1.f - (notification.age - NOTIFICATION_FADEIN_TIME) / (NOTIFICATION_FLASH_TIME + NOTIFICATION_HOLD_BRIGHT_TIME);
		float fadedAlpha = shadowAlpha(alpha);

		RgbColor color = RgbColor::lerpHSV(RgbColor(0.f, 0.f, 0.f), notification.flashColor, flashPercent);
		//RgbColor color = notification.flashColor;
		color.A = (alpha - fadedAlpha) * flashPercent + fadedAlpha;

		return color;
	}
	else {
		//RgbColor color = notification.flashColor;
		//color.A = shadowAlpha(alpha);
		//return color;

		return RgbColor(0.f, 0.f, 0.f, shadowAlpha(alpha));
	}
}

float HudManager::easeIn(float val)
{
	return std::pow(val, 2.f);
}

float HudManager::easeOut(float val)
{
	return 1.f - std::pow(1.f - val, 2.f);
}

float HudManager::easeInOut(float val)
{
	return val * val * (3.f - 2.f * val);
}

float HudManager::shadowAlpha(float alpha)
{
	return std::pow(alpha * 0.7f, 3.f);
}

const uint32_t HudManager::HudTextPayload::address_blockCount =			0x00;
const uint32_t HudManager::HudTextPayload::totalDataSize =				0x04;

const uint32_t HudManager::HudTextBlock::address_lineCount =			0x00;
const uint32_t HudManager::HudTextBlock::address_verticalPosition =		0x04;
const uint32_t HudManager::HudTextBlock::address_linePaddingTop =		0x08;
const uint32_t HudManager::HudTextBlock::address_linePaddingBottom =	0x0C;
const uint32_t HudManager::HudTextBlock::address_horizontalPosition =	0x10;
const uint32_t HudManager::HudTextBlock::address_horizontalAlignment =	0x14;
const uint32_t HudManager::HudTextBlock::totalDataSize =				0x18;

const uint32_t HudManager::HudTextLine::maxLineSize =					0x80;

const uint32_t HudManager::HudTextLine::address_textColor =				0x00;
const uint32_t HudManager::HudTextLine::address_shadowColor =			0x04;
const uint32_t HudManager::HudTextLine::address_string =				0x08;
const uint32_t HudManager::HudTextLine::totalDataSize =					0x08 + HudManager::HudTextLine::maxLineSize;