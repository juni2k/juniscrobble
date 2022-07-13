#include "pch.h"
#include "Scrobbler.h"
#include "Track.h"

#include <queue>
#include <sstream>

void Scrobbler::clear_staged_track() {
  OutputDebugString(L"Scrobbler [clearing]\r\n");

  if (this->staged_track != nullptr) {
    delete this->staged_track;
    this->staged_track = nullptr;
  }
  else {
    OutputDebugString(L"Nothing to clear!\r\n");
  }
}

void Scrobbler::stage_track(std::wstring artist, std::wstring title) {
  this->clear_staged_track();

  Track *track = new Track(artist, title);

  std::wstringstream message;
  message << "Scrobbler [staging]: " << *track << std::endl;
  OutputDebugString(message.str().c_str());

  this->staged_track = track;

  // TODO: send Last.fm a "Now playing" notification here
}

void Scrobbler::commit_track() {
  std::wstringstream message;
  message << "Scrobbler [committing]: " << *this->staged_track << std::endl;
  OutputDebugString(message.str().c_str());

  this->submissions.push_back(this->staged_track);

  this->clear_staged_track();
}