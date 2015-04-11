#include "Optic.h"
#include "OpticPack.h"
#include "OpticLua.h"
#include "OpticAPI.h"
#include "misc.h"
#include "console.h"
#include "OpticEventHandler.h"
#include "OpticHooks.h"
#include "OpticMisc.h"
#include "EventDispatcher.h"
#include "CustomChat.h"
#include "Shared.h"
#include <sstream>
#include <shlobj.h>
#include <Windows.h>
#include <memory>
#include <process.h>
#include <iostream>
namespace Optic {

std::unique_ptr<OpticEventHandler> handler(nullptr);

void render() {
	if(handler != nullptr) {
		handler->render();
	}
}

void load(const std::string& name) {
	if(handler != nullptr) {
		unload();
	}

	try {
		std::string zipName(name + ".zip");
		std::unique_ptr<OpticPack> pack(new OpticPack(zipName));
		std::unique_ptr<OpticEventHandler> listener(new OpticEventHandler(std::move(pack), dispatcher));
		handler = std::move(listener);
		HANDLE handle = (HANDLE)_beginthreadex(NULL, NULL, handler->start, static_cast<void*>(handler.get()), NULL, NULL);
		CloseHandle(handle);
		std::cout << "Optic pack loaded!";
	} catch(OpticException& e) {
		std::cout << e.what();
	} catch(std::exception& e) {
		std::cout << e.what();
	} catch(...) {
		Chat::medalText("Optic fail");
	}
}

void unload() {
	if(handler != nullptr) {
		handler->enqueue(std::make_shared<Event>(THREAD_EXIT));
		handler->wait();
		handler.reset();
		Optic::Hooks::uninstall();
		Chat::medalText("Unloaded");
	}
}

void reset() {
	if(handler != nullptr) {
		handler->reset();
	}
}

void lost() {
	if(handler != nullptr) {
		handler->lost();
	}
}

void list() {
	std::string expression = getWorkingDir() + "packs\\*.zip";
	WIN32_FIND_DATA FindFileData;
	HANDLE search = FindFirstFile(expression.c_str(), &FindFileData);

	if(search != INVALID_HANDLE_VALUE) {
		do {
			std::stringstream ss;
			try {
				OpticPackMeta meta = OpticPack::meta(FindFileData.cFileName);
				ss << meta.name << " by " << meta.author << " - " << meta.description;
				C_TEXT_COLOUR colour = C_TEXT_GREEN;

				Version version = {1, 0, 0};
				int status = versionCheck(meta.apiVersion, version);

				if(status == VERSION_RELATION::TENTATIVE_OKAY) {
					ss << " (out of date)";
					colour = C_TEXT_YELLOW;
				} else if(status == VERSION_RELATION::VERSION_IN_FUTURE) {
					ss << " (version in future?)";
					colour = C_TEXT_RED;
				} else if(status == VERSION_RELATION::UNKNOWN_VERSION) {
					ss << " (invalid version)";
					colour = C_TEXT_RED;
				} else if(status == VERSION_RELATION::OUT_OF_DATE) {
					ss << " (out of date)";
					colour = C_TEXT_RED;
				}
				
				Chat::medalText(ss.str());
			} catch(OpticPackException& e) {
				ss << FindFileData.cFileName << " - " << e.what();
				Chat::medalText(ss.str());
			}
		} while(FindNextFile(search, &FindFileData) != 0);

		FindClose(search);
	} else {
		Chat::medalText("error");
	}
}

};