#include "InitHooks.h"
#include "Patcher.h"
#include "codefinder.h"
#include "PatchGroup.h"
#include "Shared.h"
#include "Direct3D.h"
#include "DebugHelper.h"
#include <sstream>
#include <string>
#include <d3d9.h>
#include <iostream>

namespace HookManager {

void find_close();
void find_reset();
void find_map_type();
void find_map_name();

LAUNCHCB launchCB, quitCB;
void* launchArg = NULL, * quitArg = NULL;
std::vector <PatchGroup*> hooks;
void uninstall();
void installHooks();
PatchGroup* launch_event_hook();
void installPatches();
void locateAddresses();
//PatchGroup* reactor_mp_patch();
PatchGroup* d3dHooks();
PatchGroup* secondaryD3DHook();
PatchGroup* tertiaryD3DHook();

void launchCallback(LAUNCHCB callback, void* data) {
	launchCB = callback;
	launchArg = data;
}

void quitCallback(LAUNCHCB callback, void* data) {
	quitCB = callback;
	quitArg = data;
}

void install() {	
	installHooks();
	installPatches();
	locateAddresses();
}

void uninstall() {
	hooks.clear();
}

void launchHook() {
	if(launchCB != nullptr) {
		launchCB(launchArg);
	}
}

void post_launch() {
	MessageBox(NULL, "1", "1", 0);
	hooks.emplace_back(d3dHooks());
}

void quitHook() {
	if(quitCB != nullptr) {
		quitCB(quitArg);
	}
}

void installPatches() {
	//hooks.emplace_back(reactor_mp_patch());
}

PatchGroup* testing();

void net_log(char* message) {
	std::cout << message << std::endl;
}
__declspec(naked) void net_log_stub() {
	__asm {
		push eax
		lea eax, [esp + 4]
		push eax
		call net_log
		add esp, 4
		pop eax
	}
}


PatchGroup* network_log() {
	MessageBox(NULL, "1", "1", 0);
	short signature[] = { 0x55, 0x8B, 0xEC, 0x80, 0x3D, 0x38, 0xFB, 0x43, 0x02, 0x00, 0x75, 0x18 };
	MessageBox(NULL, "1", "2", 0);					  
	PatchGroup *group = new PatchGroup();
	MessageBox(NULL, "1", "3", 0);
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, net_log_stub, CaveHook::JMP_TP));
	MessageBox(NULL, "1", "4", 0);
	if(!group->install()) {
		MessageBox(NULL, "1", "5", 0);
		delete group;
		DebugHelper::Translate("Net log hook failed :(");
		throw HookException("Net log failed!");
	}
	MessageBox(NULL, "1", "6", 0);
	DebugHelper::Translate("Net log hook installed :)");
	return group;
}


void installHooks() {
	//hooks.emplace_back(network_log());
	hooks.emplace_back(testing());
}

void locateAddresses() {
	find_close();
	find_reset();
	find_map_type();
	find_map_name();
}

PatchGroup* testing() {
	short signature[] = { 0x66, 0x29, 0x94, 0x31, 0x8e, 0x02, 0x00, 0x00, 0xE8 };

	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, Test, CaveHook::CALL_TP));

	if(!group->install()) {
		delete group;
		throw HookException("Firing hook failed!");
	}

	DebugHelper::Translate("Hook installed :)");
	return group;
}

PatchGroup* d3dHooks() {
	PatchGroup* group = secondaryD3DHook();

	if(group) {
		return group;
	}

	group = tertiaryD3DHook();

	if(group) {
		return group;
	}

	throw HookException("Direct3D hook failed!");
}

struct EnumData {
    DWORD processID;
    HWND window;
};

BOOL CALLBACK EnumProc(HWND window, LPARAM lParam) {
    EnumData& ed = *(EnumData*)lParam;
    DWORD processID;

    GetWindowThreadProcessId(window, &processID);

    if(ed.processID == processID) {
        ed.window = window;
        return FALSE;
    }

    return TRUE;
}

PatchGroup* secondaryD3DHook() {
	OutputDebugString("Sec hook");
	LPDIRECT3D9 d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	
	if(!d3d9) {
		OutputDebugString("Unable to create D3D9 instance");
		return NULL;
	}

	EnumData ed = { GetProcessId(GetCurrentProcess()), NULL };
	EnumWindows(EnumProc, reinterpret_cast<LPARAM>(&ed));

	if(!ed.window) {
		OutputDebugString("Unable to locate window to attach");
		d3d9->Release();
		return NULL;
	}

	LPDIRECT3DDEVICE9 device = NULL;
	D3DPRESENT_PARAMETERS params = {};
	params.Windowed = TRUE;
	params.hDeviceWindow = ed.window;

	HRESULT res = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, ed.window,
									 D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									 &params, &device);

	if(res != D3D_OK) {
		OutputDebugString("Unable to create temporary device");
		device->Release();
		d3d9->Release();
		return NULL;
	}

	PatchGroup *group = new PatchGroup();
	std::uintptr_t* vtable = *reinterpret_cast<std::uintptr_t**>(device);

	group->add(new CaveHook(vtable[42], 0, D3DHook::endScene, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalEndScene))); 
	group->add(new CaveHook(vtable[16], 0, D3DHook::reset, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalReset)));

	device->Release();
	d3d9->Release();

	if(!group->install()) {
		OutputDebugString("Unable to hook D3D functions");
		delete group;
		return NULL;
	}

	return group;
}

PatchGroup* tertiaryD3DHook() {	
	short signature[] = {0xC7, 0x06, -1, -1, -1, -1, 0x89, 0x86, -1, -1, -1, -1, 0x89, 0x86};
	DWORD address = FindCode(GetModuleHandle("d3d9.dll"), signature, sizeof(signature) / 2);
	
	if(address == NULL) {
		OutputDebugString("Unable to locate D3D9 signature");
		return NULL;
	}

	address += 2;

	PatchGroup *group = new PatchGroup();
	std::uintptr_t* vtable = reinterpret_cast<std::uintptr_t*>(*(std::uintptr_t*)address);

	group->add(new CaveHook(vtable[42], 0, D3DHook::endScene, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalEndScene))); 
	group->add(new CaveHook(vtable[16], 0, D3DHook::reset, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalReset)));
		DebugHelper::DisplayAddress((DWORD)vtable[42], 16);

	if(!group->install()) {
		OutputDebugString("Unable to hook D3D functions");
		delete group;
		return NULL;
	}

	return group;
}

//PatchGroup* reactor_mp_patch() {
//	short signature[] = {0x00, 0x00, 0x00, 0x00, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC6, 0x05,
//	                     -1, -1, -1, -1, 0x01, 0xC3, 0xCC};
//	BYTE replacement[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
//
//	PatchGroup *group = new PatchGroup();
//	group->add(new PatchHook(signature, sizeof(signature) / 2, 10, replacement, 7));
//
//	if(!group->install()) {
//		delete group;
//		throw HookException("Failed to apply FoV zoom rendering fix!");
//	}
//
//	return group;
//}

void find_map_name() {
	map_name = ((char*)map_type) + 0x24; // erp.
}

void find_close() {
	short signature[] = {-1, -1, -1, -1, 0x00, 0x8B, 0xF8, 0x0F, 0x85};
	std::uintptr_t pClose = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(pClose != NULL) {
		game_close = (bool*)*(std::uintptr_t*)pClose;
	} else {
		throw HookException("Unable to locate close bool");
	}
}

void find_reset() {
	short signature[] = {-1, -1, -1, -1, 0x00, 0x74, 0x16, 0x80, 0x3D, -1, -1, -1, -1, 0x00, 0x75, 0x0D};
	std::uintptr_t pReset = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(pReset != NULL) {
		map_reset = (bool*)*(std::uintptr_t*)pReset;
	} else {
		throw HookException("Unable to locate reset bool");
	}
}

void find_map_type() {
	short signature[] = {-1, -1, -1, -1, 0x03, 0x0F, 0x94, 0xC0, 0xC3, 0x32, 0xC0};
	std::uintptr_t pType = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(pType != NULL) {
		map_type = (std::uint32_t*)*(std::uintptr_t*)pType;
	} else {
		throw HookException("Unable to locate map type");
	}
}

};