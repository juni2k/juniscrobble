#include "pch.h"
#include "Scrobbler.h"
#include "Track.h"

#include <chrono>
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

void Scrobbler::set_track_length(int seconds) {
  std::wstringstream message;
  message << L"Scrobbler [set track length]: ";

  if (this->staged_track != nullptr) {
    this->staged_track->length = seconds;

    message << seconds << L" seconds " << std::endl;
  }
  else {
    message << L"Could not set track length (no staged track)" << std::endl;
  }

  OutputDebugString(message.str().c_str());
}

void Scrobbler::stage_track(std::wstring artist, std::wstring title) {
  this->clear_staged_track();

  Track *track = new Track(artist, title);

  std::wstringstream message;
  message << "Scrobbler [staging]: " << *track << std::endl;
  OutputDebugString(message.str().c_str());

  this->staged_time = std::chrono::steady_clock::now();
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