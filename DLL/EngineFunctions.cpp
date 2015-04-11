#include "EngineFunctions.h"
#include "EngineState.h"
#include "EnginePointers.h"
#include <string>
#include <sstream>
#include "CustomChat.h"

std::uintptr_t HUDPrint, SoundPlay, ChatSend, GenMD5, Connect, CommandExecute, 
			   DisplayMenu, LoadMap, CommandInject, ChatLocalSend,
			   OverlayResolve;
float* pDrawDistance;
char* VehicleControl;
std::int16_t* stringIndex;
bool* displayAttentionBox;

CRITICAL_SECTION critSection;

using namespace EngineTypes;

/* Critical sections are used to protect all engine functions as a precaution
   rather than assuming they're all thread safe. Won't help if HAC and Halo
   call the same non-thread safe function at the same time. */
void initCritSection() {
	InitializeCriticalSectionAndSpinCount(&critSection, 1024);
}

void destroyCritSection() {
	DeleteCriticalSection(&critSection);
}

void HUDMessage(const std::wstring& message) {
	
}

void HUDMessage(const std::string& message) {
	Chat::medalText(message);
}

void HUDMessage(const WCHAR* message) {
	
}

void PlayMPSound(multiplayer_sound index) {

}

/* TODO: Not thread safe - should send a copy of the buffer to be sent,
 * not the original as it may have been modified by the time the networking
 * thread requires it. Warning, memory leak!
 */
void ChatMessage(const WCHAR* message, CHAT_TYPE ctype) {
	
}

void chatLocal(const std::string& message) {

}

void GenerateMD5(const char* data, DWORD data_length, char* output) {

}

