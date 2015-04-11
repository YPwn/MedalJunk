#include "InitHooks.h"
#include "DebugHelper.h"
#include "PatchGroup.h"
#include "Shared.h"
#include <Windows.h>
#include <process.h>
#include <iostream>

unsigned int __stdcall hacLaunch(void*);
void unload();
void onLaunch(void* args);
void onQuit(void* args);

char* aa = "aa";
char* crap = "crap";
char* wat = "wat";

typedef void (foo)(char*, char*, char*);
foo* test;

void console_test() {
	HANDLE file = CreateFile("test.log", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	DebugHelper::DisplayAddress((DWORD)file, 16);
	MessageBox(NULL, "wo", "ow", 0);
	test = (foo*)0x0051CBC0;

	
	AllocConsole();
	AttachConsole( GetCurrentProcessId() ) ;
	freopen( "CON", "w", stdout ) ;
	freopen( "CON", "r", stdin ) ;

	DebugHelper::DisplayAddress((DWORD)GetStdHandle(STD_INPUT_HANDLE), 16);
	test(aa, crap, wat);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
	if(reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(module);

		HANDLE initialiser = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, hacLaunch, 0, CREATE_SUSPENDED, 0));
		
		if(initialiser == NULL) {
			OutputDebugString("Thread failure.");
			return FALSE;
		}

		try {
			HookManager::install();
			console_test();
			HookManager::launchCallback(onLaunch, initialiser);
			HookManager::quitCallback(onQuit);
		} catch(HookException& e) {
			HookManager::uninstall();
			MessageBox(NULL, e.what(), "Error", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		ResumeThread(initialiser);
	}

	if(reason == DLL_PROCESS_DETACH) {
		unload();
	}
    
	return TRUE;
}


void onLaunch(void* args) {
	HANDLE initialiser = static_cast<HANDLE>(args);

	DWORD result = WaitForSingleObject(initialiser, 10000);
	
	if(FAILED(result)) {
		if(result == WAIT_TIMEOUT) {
			OutputDebugString("Init wait timed out.");
		} else if(result == WAIT_FAILED) {
			DebugHelper::Translate("Thread waiting");
		}
	}

	CloseHandle(initialiser);
}

void onQuit(void* args) {
}

void unload() {

}

void Test() {
	
	auto event = std::make_shared<PlayerKilled>(4, 1, 2, 2, GetTickCount());	
	dispatcher->enqueue(event);
}

#include "CustomChat.h"
#include "EnginePointers.h"
#include "EngineTypes.h"
#include "Optic.h"


unsigned int __stdcall hacLaunch(void*) {
	pResolution->width = 2560;
	pResolution->height = 1440;
	dispatcher = new EventDispatcher();
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, NULL, dispatcher->start, (void*)dispatcher, NULL, NULL);
	*map_type = 1;
	strcpy_s(map_name, 32, "guardian");
	*game_close = false;
	*map_reset = true;
	HookManager::post_launch();
	return 0;
}
