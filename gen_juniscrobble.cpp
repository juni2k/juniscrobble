/*

This code has seen things...

Juniscrobble for Winamp
(c) 2022 Martin Frederic <mew.tv>

*/

#include "pch.h"
#include <windows.h>

#include "gen_juniscrobble.h"

#include "wa_ipc.h"
#include <string>


// these are callback functions/events which will be called by Winamp
int  init(void);
void config(void);
void quit(void);


// this structure contains plugin information, version, name...
// GPPHDR_VER is the version of the winampGeneralPurposePlugin (GPP) structure
winampGeneralPurposePlugin plugin = {
  GPPHDR_VER,  // version of the plugin, defined in "gen_myplugin.h"
  PLUGIN_NAME, // name/title of the plugin, defined in "gen_myplugin.h"
  init,        // function name which will be executed on init event
  config,      // function name which will be executed on config event
  quit,        // function name which will be executed on quit event
  0,           // handle to Winamp main window, loaded by winamp when this dll is loaded
  0            // hinstance to this dll, loaded by winamp when this dll is loaded
};

WNDPROC WinampWndProc;

wchar_t* GetPlayingFileName() {
	return (wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);
}

std::wstring GetMetadata(const wchar_t *filename, const wchar_t *field) {
	/* The result buffer gets nuked after SendMessage() returns.
	Make sure to copy relevant data out of it as soon as possible. */
	wchar_t buf[256] = {0};

	extendedFileInfoStructW info;
	info.filename = filename;
	info.metadata = field;
	info.ret = buf;
	info.retlen = sizeof(buf) / sizeof(wchar_t);
	/* Return value doesn't seem to matter. I get 0 which, according to the API, means that the decoder
		doesn't support this method. Whatever. It works. */
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

	std::wstring result(buf);
	return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_WA_IPC && lParam == IPC_PLAYING_FILEW) {
		/* We are playing a file! Path is in wParam. */

		/* Here's the thing: IPC_GET_PLAYING_TITLE works, but it returns an "$artist - $title" string.
		We could split that, but it would provide incorrect values for artists containing a dash.
		The best way seems to be to fetch the	filename, then ask Winamp for metadata about that file. This sucks. */
		wchar_t* filename = (wchar_t *)wParam;

		/* query artist/title */
		std::wstring artist = GetMetadata(filename, L"artist");
		std::wstring title = GetMetadata(filename, L"title");

		wchar_t message[1024];
		wsprintf(message, L"artist: %s\r\ntitle: %s", artist.c_str(), title.c_str());
		MessageBox(plugin.hwndParent, message, L"Juniscrobble", MB_OK);
	}

	// Forward event to original WndProc
	return CallWindowProc(WinampWndProc, hwnd, message, wParam, lParam);
}

// event functions follow

int init() {
	/* Inject our custom WndProc for capturing playback events. Also, back up the original one. */
	WinampWndProc = (WNDPROC)SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG)WndProc);

	return 0;
}

void config() {
	//A basic messagebox that tells you the 'config' event has been triggered.
	//You can change this later to do whatever you want (including nothing)
	MessageBox(plugin.hwndParent, L"Config event triggered for gen_myplugin.", L"", MB_OK);
}

void quit() {
	//A basic messagebox that tells you the 'quit' event has been triggered.
	//If everything works you should see this message when you quit Winamp once your plugin has been installed.
	//You can change this later to do whatever you want (including nothing)
	//MessageBox(0, L"Quit event triggered for gen_myplugin.", L"", MB_OK);
}


// This is an export function called by winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
	return &plugin;
}