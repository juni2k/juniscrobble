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

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_WA_IPC && lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_STATUS) {
		/* Playback status has changed... are we playing something? */
		if (SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 1) {
			/* yeah, we are. what's the artist/title? */

			/* Here's the thing: IPC_GET_PLAYING_TITLE works, but it returns an "$artist - $title" string.
			We could split that, but it would provide incorrect values for artists containing a dash.
			The best way seems to be to fetch the	filename, then ask Winamp for metadata about that file. This sucks. */
			wchar_t *filename = GetPlayingFileName();

			extendedFileInfoStructW info;

			/* The result buffer gets nuked after SendMessage() returns.
			Make sure to copy relevant data out of it as soon as possible. */
			wchar_t result[256] = {0};

			/* query the artist */
			wchar_t artist[256];
			info.filename = filename;
			info.metadata = L"artist";
			info.ret = result;
			info.retlen = sizeof(result) / sizeof(wchar_t);
			/* Return value doesn't seem to matter. I get 0 which, according to the API, means that the decoder
			doesn't support this method. Whatever. It works. */
			SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);
			memcpy(artist, result, sizeof(artist));

			/* Calling IPC_GET_EXTENDED_FILE_INFOW wrecks the data inside filename but we need to use it again
			to fetch the title... Let's request it again. SIIIGHHH. */
			filename = GetPlayingFileName();


			/* query the title */
			wchar_t title[256];
			info.filename = filename;
			info.metadata = L"title";
			info.ret = result;
			info.retlen = sizeof(result) / sizeof(wchar_t);
			SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);
			memcpy(title, result, sizeof(title));

			wchar_t message[1024];
			wsprintf(message, L"artist: %s\ntitle: %s", artist, title);
			MessageBox(plugin.hwndParent, message, L"Juniscrobble", MB_OK);
		}
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