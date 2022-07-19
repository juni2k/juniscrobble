#include "pch.h"
#include "Scrobbler.h"
#include "Track.h"

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
  /* Make this mess readable. */
  using namespace std::chrono;

  /* Before clearing a staged track, check if we are past its runtime (which means it can be committed).
  * Usually, this is not necessary since there is (will be) an active Timer checking if the song is
  * past 60% of its runtime in which case it will be committed. The strategy implemented here is for
  * tracks that are shorter than the checking interval.
  *
  * Also, this would be a lot simpler if Winamp sent a message when a track has ended. It doesn't, you know. */
  if (this->staged_track != nullptr) {
    steady_clock::time_point trackEndTime = this->staged_time + seconds(this->staged_track->length);
    long long secondsPastTrackEnd = duration_cast<seconds>(steady_clock::now() - trackEndTime).count();

    std::wstringstream message;
    message << "Scrobbler: " << secondsPastTrackEnd << " seconds past track end ";
    if (secondsPastTrackEnd >= 0) {
      message << "(full listen)";
    }
    else {
      message << "(incomplete listen)";
    }
    message << std::endl;
    OutputDebugString(message.str().c_str());

    if (secondsPastTrackEnd >= 0) {
      this->commit_track();
    }
    else {
      this->clear_staged_track();
    }
  }

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

  this->submissions.push_back(std::move(*this->staged_track));

  /* clear_staged_track() tries to delete the object which is not what we want */
  this->staged_track = nullptr;
}

std::wstring Scrobbler::submissions_text() {
  std::wstringstream s;
  s << "Scrobbler queue:" << std::endl;

  std::list<Track>::iterator it;
  for (it = this->submissions.begin(); it != this->submissions.end(); it++) {
    s << *it << std::endl;
  }

  return s.str();
}
