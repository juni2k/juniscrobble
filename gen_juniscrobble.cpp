/*

This code has seen things...

Juniscrobble for Winamp
(c) 2022 Martin Frederic <mew.tv>

*/

#include "pch.h"

#include "gen_juniscrobble.h"
#include "Scrobbler.h"

#define FETCH_TRACK_LENGTH_AGAIN_TIMER_ID 1337


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
Scrobbler scrobbler;
time_t lastPlayMessage = -1; /* Not that cool, but it is ok on Windows. */

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

void FetchTrackLengthAgain(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int trackLengthS = SendMessage(plugin.hwndParent, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);

	/* TODO: maybe try multiple times? Does the return value depend on the disk speed?
	Maybe Winamp returns crap when the track hasn't really started playing yet? */

	if (trackLengthS == -1) {
		OutputDebugString(L"Even after waiting, Winamp couldn't return a reasonable track length.\r\n"
		L"This track cannot be scrobbled. Sorry about that.\r\n");

		scrobbler.clear_staged_track();
	}
	else {
		OutputDebugString(L"Tatata! The results are in!\r\n");
		scrobbler.set_track_length(trackLengthS);
	}

	/* We don't need this any longer. Into the trashcan it goes! */
	KillTimer(hwnd, FETCH_TRACK_LENGTH_AGAIN_TIMER_ID);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_WA_IPC && lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_STATUS) {
		/* Playback status has changed! */
		/* This fires after IPC_PLAYING_FILEW, so the scrobbler already has relevant data. */
		OutputDebugString(L"Status change: ");
		int status = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
		int trackLengthS;
		switch (status) {
		case 1:
			/* Playing track. Feed its runtime to the scrobbler so it knows what to do
			when staging a *new* track. Since IPC_CB_MISC comes *after* IPC_PLAYING_FILEW,
			the scrobbler already knows about the *current* track. */

			/* Fun fact: when there's an automatic track change (as it happens in a playlist),
			Winamp sends the same playing message *twice*. No idea why. So, if we get it again
			in less than 2 seconds, treat it as invalid. Yeah, these are vague heuristics.
			I don't like it either. */
		{
			time_t thisPlayMessage = time(NULL);
			if (lastPlayMessage != -1) {
				int diffS = round(difftime(thisPlayMessage, lastPlayMessage));
				if (diffS < 2) {
					wchar_t ps[128];
					wsprintf(ps, L"Playing track (filtered; doing nothing instead; diff: %d sec)\r\n", diffS);
					OutputDebugString(ps);
					break;
				}
			}
			lastPlayMessage = thisPlayMessage;
		}

			OutputDebugString(L"Playing track\r\n");
			/* query track runtime */
			/* For the demo song, this returns -1. No idea why. I also tried fetching the length using
			IPC_GET_BASIC_FILE_INFOW instead, but that throws an access violation. So, "sometimes returns -1" is
			the best behaviour I can get out of that ancient media player. Man... */
			/* Update (10 minutes later): It ACTUALLY WORKS if you try again later. WHAT THE FUCK. */
			trackLengthS = SendMessage(plugin.hwndParent, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);
			if (trackLengthS == -1) {
				OutputDebugString(L"Winamp says this track has a length of -1.\r\n"
					L"But sometimes, we can get a valid length by trying again in a second.\r\n");

				SetTimer(plugin.hwndParent, FETCH_TRACK_LENGTH_AGAIN_TIMER_ID, 1000, (TIMERPROC)FetchTrackLengthAgain);
			}
			else {
				wchar_t ps[64];
				wsprintf(ps, L"-> %d seconds\r\n", trackLengthS);
				OutputDebugString(ps);
				scrobbler.set_track_length(trackLengthS);
			}
			break;
		case 3:
			/* TODO: make appropriate decisions */
			OutputDebugString(L"Paused track\r\n");
			break;
		case 0:
			/* TODO: make appropriate decisions */
			OutputDebugString(L"Not playing anything\r\n");
			break;
		default:
			/* You never know... */
			OutputDebugString(L"Impossible status! What the hell?\r\n");
		}
	}

	if (message == WM_WA_IPC && lParam == IPC_PLAYING_FILEW) {
		/* We are playing a file! Path is in wParam. */

		/* Here's the thing: IPC_GET_PLAYING_TITLE works, but it returns an "$artist - $title" string.
		We could split that, but it would provide incorrect values for artists containing a dash.
		The best way seems to be to fetch the	filename, then ask Winamp for metadata about that file. This sucks. */
		wchar_t* filename = (wchar_t *)wParam;

		/* query artist/title */
		std::wstring artist = GetMetadata(filename, L"artist");
		std::wstring title = GetMetadata(filename, L"title");

		scrobbler.stage_track(artist, title);
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